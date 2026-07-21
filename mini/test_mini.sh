#!/bin/bash

# ==============================================================================
#  MINISHELL ULTIMATE TESTER (Logic + Valgrind + Stderr + Malloc Fault Injection)
# ==============================================================================

# --- Argument Parsing ---
# Usage: ./supra_tester.sh [MALLOC=1/0] [START_SECTION] [END_SECTION]

MALLOC_FLAG=${1:-1}
START_SECTION=${2:-1}
if [ -n "$2" ] && [ -z "$3" ]; then
    END_SECTION=$2
elif [ -n "$3" ]; then
    END_SECTION=$3
else
    END_SECTION=19
fi

ENABLE_MALLOC_STRESS=true
if [ "$MALLOC_FLAG" == "0" ]; then
    ENABLE_MALLOC_STRESS=false
fi

# --- Colors for Output ---
GREEN="\033[32m"
RED="\033[31m"
YELLOW="\033[33m"
CYAN="\033[36m"
MAGENTA="\033[35m"
RESET="\033[0m"

# --- Absolute Paths ---
ROOT_DIR="$(pwd)"
MINISHELL="$ROOT_DIR/minishell"

# --- Multithreading Setup ---
# Dynamically fetch available CPU threads for portability
if command -v nproc &> /dev/null; then
    MAX_JOBS=$(nproc)
else
    MAX_JOBS=4 # Safe fallback
fi

# Keep one thread completely free so the desktop remains responsive 
if [ "$MAX_JOBS" -gt 2 ]; then
    ((MAX_JOBS--))
fi

TMP_DIR="/dev/shm/mini_tester_$$"
mkdir -p "$TMP_DIR"

display_count=0       # Tracks the visual order of headers and tests
last_queued_print=0   # Tracks the last print section for the Watcher
test_count=0          # Tracks the actual number of tests executed
test_passed=0
valgrind_passed=0
malloc_passed=0

make re > /dev/null

# Ensure minishell and debuggers are installed
if [ ! -f "$MINISHELL" ]; then
    echo -e "${RED}Error: $MINISHELL not found. Compile your project first.${RESET}"; exit 1
fi
if ! command -v valgrind &> /dev/null; then
    echo -e "${RED}Error: valgrind is not installed.${RESET}"; exit 1
fi
if ! command -v gdb &> /dev/null; then
    echo -e "${RED}Error: gdb is not installed. Required for stack traces.${RESET}"; exit 1
fi

# --- 0. Build the Malloc Saboteur ---
if [ "$ENABLE_MALLOC_STRESS" = true ]; then
    echo -e "${YELLOW}Compiling Malloc Saboteur (with GDB Tripwires)...${RESET}"
cat << 'EOF' > "$ROOT_DIR/faulty_malloc.c"
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

void *malloc(size_t size) {
    static void *(*real_malloc)(size_t) = NULL;
    char *target_env;
    int target_alloc;
    static int alloc_count = 0;
    Dl_info info;

    if (!real_malloc)
        real_malloc = dlsym(RTLD_NEXT, "malloc");

    /* Dynamically trace the caller's origin */
    if (dladdr(__builtin_return_address(0), &info) && info.dli_fname) {
        /* Only increment the counter and sabotage if the caller is the minishell binary */
        if (strstr(info.dli_fname, "minishell")) {
            alloc_count++;
            target_env = getenv("FAIL_MALLOC_AT");
            if (target_env) {
                target_alloc = atoi(target_env);
                if (target_alloc > 0 && alloc_count == target_alloc) {
                    if (getenv("TRACE_MALLOC")) {
                        raise(SIGTRAP); 
                    }
                    return NULL; /* SABOTAGE! */
                }
            }
        }
    }
    return real_malloc(size);
}
EOF
    gcc -shared -fPIC -ldl "$ROOT_DIR/faulty_malloc.c" -o "$ROOT_DIR/faulty_malloc.so"
    if [ ! -f "$ROOT_DIR/faulty_malloc.so" ]; then
        echo -e "${RED}Failed to compile faulty_malloc.so!${RESET}"; exit 1
    fi
fi

# --- Helper: Check Section Range ---
check_sec() {
    if [ "$1" -ge "$START_SECTION" ] && [ "$1" -le "$END_SECTION" ]; then
        return 0
    fi
    return 1
}

# --- Helper: Extract GDB Trace (Thread-Safe) ---
get_gdb_trace() {
    local test_i=$1
    local mode=$2
    local GDB_SCRIPT="$test_dir/gdb_script.gdb"
    cat <<EOF > "$GDB_SCRIPT"
set env FAIL_MALLOC_AT $test_i
set env LD_PRELOAD $ROOT_DIR/faulty_malloc.so
$( [ "$mode" == "malloc" ] && echo "set env TRACE_MALLOC 1" )
run
bt 10
EOF
    local gdb_out=$(gdb -batch -x "$GDB_SCRIPT" --args $MINISHELL "$builtin" "$M_TRASH" "${mini_args[@]}" 2>/dev/null)
    echo "$gdb_out" | grep -v "faulty_malloc.c" | grep -v "malloc.c" | grep -v "libc.so" | grep -m 1 -iE "\.c:" | sed -E 's/^[ \t]*#[0-9]+[ \t]+(0x[0-9a-f]+ in )?//'
}

# --- Helper to clean and normalize environment outputs ---
normalize_env() {
    local file=$1
    local is_null_terminated=$2
    if [ "$is_null_terminated" = true ]; then
        tr '\0' '\n' < "$file" > "$file.tmp"
        mv "$file.tmp" "$file"
    fi
    grep -vE '^(declare -x )?(BASH|SHLVL|PWD|OLDPWD|_|LS_COLORS|XDG|GLIBC|LD_|FAIL_MALLOC_AT|TRACE_MALLOC)' "$file" | sort > "$file.norm"
    mv "$file.norm" "$file"
}

# --- Helper to clean and normalize stderr ---
normalize_stderr() {
    local file=$1
    local is_bash=$2
    local cmd=$3

    if [ "$is_bash" = true ]; then
        sed -E 's/^bash: line [0-9]+: //g; s/^bash: //g' "$file" > "${file}.norm"
    else
        sed -E 's/^minishell: //g' "$file" > "${file}.norm"
    fi
    
    if [ -n "$cmd" ]; then
        sed -i -E "s/^$cmd: //g" "${file}.norm"
    fi
    mv "${file}.norm" "$file"
}

# --- Core Testing Function (Isolated Subshell) ---
run_test() {
    local test_id=$1
    local builtin=$2
    local cmd_string=$3
    local mode=$4 # NEW: 'flag_error' toggle
    
    # 1. Isolate the environment
    local test_dir="$TMP_DIR/$test_id"
    mkdir -p "$test_dir"
    touch "$TMP_DIR/running_$test_id" # Create a lock file to track active CPU usage
    
    local B_OUT="$test_dir/bash_out.tmp"
    local B_ERR="$test_dir/bash_err.tmp"
    local M_OUT="$test_dir/mini_out.tmp"
    local M_ERR="$test_dir/mini_err.tmp"
    local V_LOG="$test_dir/valgrind.log"
    local M_ERR_LOG="$test_dir/malloc_err.tmp"
    local M_TRASH="$test_dir/mini_trash.tmp"
    
    local is_env_test=false
    local is_null_test=false
    local bash_cmd="$cmd_string"

    if [[ "$builtin" == "env" || "$builtin" == "export" ]]; then is_env_test=true; fi
    if [[ "$cmd_string" == *"-0"* || "$cmd_string" == *"--null"* ]]; then is_null_test=true; fi
    if [[ "$builtin" == "unset" ]]; then bash_cmd="${cmd_string/unset /unset -v }"; fi

    local display_out=""
    display_out+=$(printf "Test %-40s " "[$cmd_string]")

    # 2. Run standard Bash
    bash -c "$bash_cmd" > "$B_OUT" 2> "$B_ERR"
    local bash_status=$?

    # 3. Run Minishell wrapped in Valgrind
    eval "mini_args=($cmd_string)"
    valgrind --leak-check=full --show-leak-kinds=all \
             --errors-for-leak-kinds=all --log-file="$V_LOG" \
             $MINISHELL "$builtin" "$M_OUT" "${mini_args[@]}" > /dev/null 2> "$M_ERR"
    local mini_status=$?

    # 4. Analyze Baseline
    local val_total=$(awk '/ERROR SUMMARY:/ {sum += $4} END {print sum}' "$V_LOG")
    local valgrind_ok=true
    local val_pass_num=0
    if [[ -z "$val_total" || "$val_total" -ne 0 ]]; then
        valgrind_ok=false
    else
        val_pass_num=1
    fi

    # 5. MALLOC FAULT INJECTION
    local malloc_status="${CYAN}CLEAN${RESET}"
    local malloc_ok=true
    local malloc_pass_num=0
    local TOTAL_ALLOCS=$(awk '/total heap usage:/ {sum += $5} END {print sum}' "$V_LOG" | tr -d ',')

    if [ "$ENABLE_MALLOC_STRESS" = true ] && [ "$valgrind_ok" = true ] && [ -n "$TOTAL_ALLOCS" ] && [ "$TOTAL_ALLOCS" -gt 0 ]; then
        for (( i=1; i<=TOTAL_ALLOCS; i++ )); do
            FAIL_MALLOC_AT=$i LD_PRELOAD="$ROOT_DIR/faulty_malloc.so" $MINISHELL "$builtin" "$M_TRASH" "${mini_args[@]}" > /dev/null 2> "$M_ERR_LOG"
            local crash_status=$?
            
            if [ $crash_status -eq 139 ] || [ $crash_status -eq 134 ] || [ $crash_status -eq 137 ] || [ $crash_status -eq 136 ]; then
                local m_orig=$(get_gdb_trace "$i" "malloc")
                local c_orig=$(get_gdb_trace "$i" "crash")
                malloc_status="${MAGENTA}SEGFAULT @ alloc #$i${RESET}"
                if [ -n "$m_orig" ]; then malloc_status="${malloc_status}\n      ${YELLOW}Malloc:${RESET} $m_orig"; fi
                if [ -n "$c_orig" ]; then malloc_status="${malloc_status}\n      ${RED}Crash: ${RESET} $c_orig"; fi
                malloc_ok=false
                break
            fi

            if ! grep -qiE "malloc|allocate|memory" "$M_ERR_LOG"; then
                # If test exited cleanly matching bash/normal mini status, let it pass
                if [ $crash_status -eq $mini_status ]; then
                    continue
                fi
                local m_orig=$(get_gdb_trace "$i" "malloc")
                malloc_status="${YELLOW}SILENT FAIL @ alloc #$i${RESET}"
                if [ -n "$m_orig" ]; then malloc_status="${malloc_status}\n      ${YELLOW}Malloc:${RESET} $m_orig"; fi
                malloc_ok=false
                break
            fi
        done
    fi
    if [ "$malloc_ok" = true ]; then malloc_pass_num=1; fi

		# 6. Normalize and Compare
    local out_diff=0
    local err_diff=0

    if [ "$mode" == "flag_error" ]; then
        # FLAG ERROR MODE: Exit code MUST match Bash exactly, and stderr must have text. Diff is ignored.
        if [ ! -s "$M_ERR" ]; then
            echo -e "Expected an error message on stderr, but it was completely empty." > "$test_dir/err_diff.log"
            err_diff=1
        fi
        # The script will automatically check if $mini_status equals $bash_status in Step 7!
    else
        # STRICT MODE: Exact matching with standard Bash
        if [ "$is_env_test" = true ]; then
            normalize_env "$B_OUT" "$is_null_test"
            normalize_env "$M_OUT" "$is_null_test"
        fi
        diff -u "$B_OUT" "$M_OUT" > "$test_dir/diff.log"
        out_diff=$?

        normalize_stderr "$B_ERR" true "$builtin"
        normalize_stderr "$M_ERR" false "$builtin"
        diff -u "$B_ERR" "$M_ERR" > "$test_dir/err_diff.log"
        err_diff=$?
    fi

    # 7. Format Output String
    local val_str="${RED}FAILED${RESET}"
    local logic_pass_num=0
    if [ "$valgrind_ok" = true ]; then val_str="${CYAN}CLEAN${RESET}"; fi

    if [ $out_diff -eq 0 ] && [ $err_diff -eq 0 ] && [ $bash_status -eq $mini_status ]; then
        display_out+="$(echo -e "${GREEN}PASS${RESET} | Val: $val_str | Malloc: $malloc_status")\n"
        logic_pass_num=1
    else
        display_out+="$(echo -e "${RED}FAIL${RESET} | Val: $val_str | Malloc: $malloc_status")\n"
        if [ $bash_status -ne $mini_status ]; then display_out+="$(echo -e "  ${YELLOW}Exit Differs:${RESET} Bash=$bash_status, Mini=$mini_status")\n"; fi
        if [ $out_diff -ne 0 ]; then display_out+="$(echo -e "  ${YELLOW}Stdout Differs:${RESET}")\n$(cat "$test_dir/diff.log" | sed 's/^/    /')\n"; fi
        if [ $err_diff -ne 0 ]; then display_out+="$(echo -e "  ${YELLOW}Stderr Differs:${RESET}")\n$(cat "$test_dir/err_diff.log" | sed 's/^/    /')\n"; fi
    fi

    # 8. Save Data for Parent Process
    echo -e "$display_out" > "$test_dir/display.log"
    echo "$logic_pass_num $val_pass_num $malloc_pass_num" > "$test_dir/status.dat"
    rm -f "$TMP_DIR/running_$test_id" # Remove active CPU lock
    touch "$test_dir/done.flag"       # Signal to the Watcher that it finished
}

# --- Asynchronous Queue Header ---
queue_header() {
    ((display_count++))
    local item_dir="$TMP_DIR/$display_count"
    mkdir -p "$item_dir"
    echo -e "\n${YELLOW}$1${RESET}" > "$item_dir/display.log"
    touch "$item_dir/done.flag" # Signal header is ready instantly
}

# --- Asynchronous Queue Dispatcher ---
queue_test() {
    ((display_count++))
    ((test_count++))
    
    # Block if we have reached the CPU limit (ignoring background Watchers)
    while [ $(ls "$TMP_DIR"/running_* 2>/dev/null | wc -l) -ge "$MAX_JOBS" ]; do
        sleep 0.1
    done
    
    # Spawn the test in the background
    run_test "$display_count" "$1" "$2" &
}

# --- Asynchronous Queue Flag Error Dispatcher ---
queue_flag_test() {
    ((display_count++))
    ((test_count++))
    
    # Block if we have reached the CPU limit
    while [ $(ls "$TMP_DIR"/running_* 2>/dev/null | wc -l) -ge "$MAX_JOBS" ]; do
        sleep 0.1
    done
    
    # Spawn the test in the background in flag error mode
    run_test "$display_count" "$1" "$2" "flag_error" &
}

# --- Asynchronous Queue Flusher (Non-Blocking) ---
flush_queue() {
    local start=$((last_queued_print + 1))
    local end=$display_count
    
    if [ "$start" -le "$end" ]; then
        # Spawn an independent Watcher for this section
        (
            # Poll until every test in this section creates its done.flag
            for (( i=start; i<=end; i++ )); do
                while [ ! -f "$TMP_DIR/$i/done.flag" ]; do
                    sleep 0.1
                done
            done
            
            # Assemble output and print it atomically so terminal lines don't mix
            local buffer=""
            for (( i=start; i<=end; i++ )); do
                buffer+=$(cat "$TMP_DIR/$i/display.log")
                buffer+=$'\n'
            done
            printf "%s" "$buffer"
        ) &
    fi
    last_queued_print=$display_count
}

echo "=========================================================="
echo "    Extensive Minishell Tester (Sections $START_SECTION to $END_SECTION) "
echo "=========================================================="

if check_sec 1; then
queue_header "--- 1. Testing cd (-L, -P, -e)..."
queue_test "cd" "cd ."
queue_test "cd" "cd .."
queue_test "cd" "cd /tmp"
queue_test "cd" "cd -L /tmp"
queue_test "cd" "cd -P /tmp"
queue_test "cd" "cd -e /tmp"
queue_test "cd" "cd /does_not_exist"
    flush_queue
fi

if check_sec 2; then
queue_header "--- 2. Testing pwd (-L, -P)..."
queue_test "pwd" "pwd"
queue_test "pwd" "pwd -L"
queue_test "pwd" "pwd -P"
    flush_queue
fi

if check_sec 3; then
queue_header "--- 3. Testing echo (-n, -e, -E)..."
queue_test "echo" "echo"
queue_test "echo" "echo hello world"
queue_test "echo" "echo -n hello world"
queue_test "echo" "echo -nnnn hello"
queue_test "echo" "echo -e 'hello\nworld\t!'"
queue_test "echo" "echo -E 'hello\nworld\t!'"
queue_test "echo" "echo -ne 'test\n'"
queue_test "echo" "echo -e '\x41\x42\x43'"
    flush_queue
fi

if check_sec 4; then
queue_header "--- 4. Testing export (-p, -n)..."
queue_test "export" "export"
queue_test "export" "export -p"
queue_test "export" "export VAR_TEST=123"
queue_test "export" "export -n PATH"
queue_test "export" "export BAD-VAR=123" 
    flush_queue
fi

if check_sec 5; then
queue_header "--- 5. Testing unset (-v)..."
queue_test "unset" "unset PATH"
queue_test "unset" "unset DOES_NOT_EXIST"
queue_test "unset" "unset BAD-NAME" 
    flush_queue
fi

if check_sec 6; then
queue_header "--- 6. Testing env (-a, -i, -0, -u, -C, -S)..."
queue_test "env" "env"
queue_test "env" "env -i"
queue_test "env" "env -0"
queue_test "env" "env -u PATH"
queue_test "env" "env -C /tmp pwd"
queue_test "env" "env -a ARGV0 echo hello"
queue_test "env" "env -S 'echo hello split string'"
queue_test "env" "env --ignore-environment"
queue_test "env" "env --null"
queue_test "env" "env --unset=PATH"
queue_test "env" "env --chdir=/tmp pwd"
queue_test "env" "env --argv0=ARGV0 echo hello"
queue_test "env" "env --split-string='echo hello split string'"
    flush_queue
fi

if check_sec 7; then
queue_header "--- 7. HARDCORE: CD & PWD Stress..."
queue_test "cd" "cd -L -P -L -P /tmp"
queue_test "cd" "cd -- -L" 
queue_test "cd" "cd ''"
queue_test "cd" "cd /tmp/../tmp/../tmp/../tmp"
queue_test "pwd" "pwd -LLLLPPPPLLLLPPPP"
queue_test "pwd" "pwd -- -L"
    flush_queue
fi

if check_sec 8; then
queue_header "--- 8. HARDCORE: Echo Parsing Anomalies..."
queue_test "echo" "echo -neE -en -n -e -E 'mix and match'"
queue_test "echo" "echo -e 'Before\cAfter'"
queue_test "echo" "echo '' '' '   ' ''"
queue_test "echo" "echo -e '\x41\x42\x43 \0104\0105\0106'"
queue_test "echo" "echo -e '\\\\\\\\'"
queue_test "echo" "echo -e '\x \0 \c'"
    flush_queue
fi

if check_sec 9; then
queue_header "--- 9. HARDCORE: Hashmap & Key Validation..."
queue_test "export" "export _VALID=1 2INVALID=2 ALSO_VALID=3"
queue_test "export" "export NULL_VAR EMPTY_VAR="
queue_test "export" "export WEIRD_VAR=\"hello=world=test=123\""
queue_test "export" "export ="
queue_test "export" "export '===='"
queue_test "unset" "unset _VALID 2INVALID ALSO_VALID DOESNT_EXIST"
    flush_queue
fi

if check_sec 10; then
queue_header "--- 10. HARDCORE: Env Execution & Nested Environments..."
queue_test "env" "env -i A=1 A=2 A=3"
queue_test "env" "env -u PATH -u HOME -u USER"
queue_test "env" "env -i A=1 B=2 env -i C=3 pwd"
queue_test "env" "env -i -S 'A=1 B=2 env -i C=3 pwd'"
queue_test "env" "env -0 -i NEW_VAR=123 NEW_VAR2=456"
    flush_queue
fi

if check_sec 11; then
queue_header "--- 11. HARDCORE: System, Starvation, and POSIX Extremes..."
mkdir -p /tmp/mini_cdpath_test
export CDPATH=/tmp
queue_test "cd" "cd mini_cdpath_test"
unset CDPATH

export OLDPWD=/tmp
queue_test "cd" "cd -"
unset OLDPWD
queue_test "cd" "cd -"

OLD_HOME=$HOME
unset HOME
queue_test "cd" "cd"
export HOME=$OLD_HOME

queue_test "env" "env -i ls"
queue_test "env" "env sh -c 'exit 42'"
queue_test "env" "env sh -c 'kill -9 \$$'"

queue_test "export" "export MULTILINE=$'Line1\nLine2\tTabbed'"
queue_test "cd" "cd /tmp /var /usr"

rm -rf /tmp/mini_cdpath_test
    flush_queue
fi

if check_sec 12; then
queue_header "--- 12. HARDCORE: Exit Trapdoors..."
queue_test "exit" "exit 0"
queue_test "exit" "exit 42"
queue_test "exit" "exit -42"
queue_test "exit" "exit 9223372036854775807"
queue_test "exit" "exit 9223372036854775808"
queue_test "exit" "exit 42 42"
queue_test "exit" "exit hello"
queue_test "exit" "exit 42hello"
queue_test "exit" "exit -- -42"
    flush_queue
fi

if check_sec 13; then
queue_header "--- 13. HARDCORE: Env Execution Exit Codes (126/127)..."
queue_test "env" "env /tmp"
touch /tmp/mini_no_exec
chmod 000 /tmp/mini_no_exec
queue_test "env" "env /tmp/mini_no_exec"
rm -f /tmp/mini_no_exec
queue_test "env" "env /does_not_exist_mini"
    flush_queue
fi

if check_sec 14; then
queue_header "--- 14. HARDCORE: The Rug Pull (Deleted Working Directory)..."
mkdir -p /tmp/mini_rugpull
cd /tmp/mini_rugpull
rm -rf /tmp/mini_rugpull
queue_test "pwd" "pwd"
queue_test "cd" "cd ."
queue_test "cd" "cd .."
cd "$ROOT_DIR" || cd /tmp
    flush_queue
fi

if check_sec 15; then
queue_header "--- 15. HARDCORE: State Corruption..."
OLD_PWD_VAR=$PWD
unset PWD
queue_test "pwd" "pwd"
export PWD=$OLD_PWD_VAR

OLD_OLDPWD=$OLDPWD
unset OLDPWD
queue_test "cd" "cd -"
export OLDPWD=$OLD_OLDPWD
    flush_queue
fi

if check_sec 16; then
queue_header "--- 16. HARDCORE: Hostile File Systems & Limits..."
mkdir -p /tmp/mini_hostile
ln -s loop /tmp/mini_hostile/loop 2>/dev/null
mkdir -p /tmp/mini_hostile/no_exec
chmod 000 /tmp/mini_hostile/no_exec

queue_test "cd" "cd /tmp/mini_hostile/loop"
queue_test "cd" "cd /tmp/mini_hostile/no_exec"

LONG_PATH="/"$(head -c 4100 < /dev/zero | tr '\0' 'a')
queue_test "cd" "cd $LONG_PATH"

chmod 777 /tmp/mini_hostile/no_exec 2>/dev/null
rm -rf /tmp/mini_hostile
    flush_queue
fi

if check_sec 17; then
queue_header "--- 17. HARDCORE: Export & Identifier Anomalies..."
queue_test "export" "export VAR=42 VAR"
queue_test "export" "export VAR+=append"
queue_test "export" "export _=test _VAR=123 1_VAR=123"
queue_test "export" "export =="
    flush_queue
fi

if check_sec 18; then
queue_header "--- 18. HARDCORE: PWD Gaslighting..."
OLD_PWD_VAR=$PWD
export PWD=/completely/fake/path

queue_test "pwd" "pwd"
queue_test "pwd" "pwd -L"
queue_test "pwd" "pwd -P"

queue_test "cd" "cd ."
queue_test "cd" "cd .."

export PWD=$OLD_PWD_VAR
    flush_queue
fi

if check_sec 19; then
queue_header "--- 19. FLAG PARSING: Invalid Builtin Options..."
queue_flag_test "cd" "cd -Z /tmp"
queue_flag_test "cd" "cd --badflag"
queue_flag_test "pwd" "pwd -Z"
queue_flag_test "pwd" "pwd --badflag"
queue_flag_test "export" "export -Z"
queue_flag_test "export" "export -INVALID=123"
queue_flag_test "export" "export --badflag"
queue_flag_test "unset" "unset -Z"
queue_flag_test "unset" "unset --badflag"
queue_flag_test "env" "env -Z"
queue_flag_test "env" "env --badflag"
queue_flag_test "exit" "exit -Z"
    flush_queue
fi

echo -e "\n=========================================================="
echo -e "${YELLOW}Waiting for all background jobs and tests to finish...${RESET}"
wait # This acts as a master barrier for both run_test and the Watchers

# Tally the final scores across all sections
for (( i=1; i<=display_count; i++ )); do
    if [ -f "$TMP_DIR/$i/status.dat" ]; then
        read l_pass v_pass m_pass < "$TMP_DIR/$i/status.dat"
        ((test_passed += l_pass))
        ((valgrind_passed += v_pass))
        ((malloc_passed += m_pass))
    fi
done

echo -e "\n=========================================================="
if [ $test_count -eq 0 ]; then
    echo -e "${YELLOW}No tests executed. Check your section ranges.${RESET}"
else
    if [ $test_passed -eq $test_count ]; then
        echo -e "${GREEN}Logic: SUCCESS! ($test_passed / $test_count)${RESET}"
    else
        echo -e "${RED}Logic: FAILED ($test_passed / $test_count)${RESET}"
    fi

    if [ $valgrind_passed -eq $test_count ]; then
        echo -e "${CYAN}Memory (Normal): ALL CLEAN! ($valgrind_passed / $test_count)${RESET}"
    else
        echo -e "${MAGENTA}Memory (Normal): LEAKS DETECTED ($valgrind_passed / $test_count tests passed)${RESET}"
    fi

    if [ "$ENABLE_MALLOC_STRESS" = true ]; then
        if [ $malloc_passed -eq $test_count ]; then
            echo -e "${GREEN}Memory (Malloc Crash): ALL SURVIVED! ($malloc_passed / $test_count)${RESET}"
        else
            echo -e "${MAGENTA}Memory (Malloc Crash): CRASHES/LEAKS DETECTED ($malloc_passed / $test_count tests passed)${RESET}"
        fi
    fi
fi

# Cleanup
rm -rf "$TMP_DIR" "$ROOT_DIR/faulty_malloc.c" "$ROOT_DIR/faulty_malloc.so"
