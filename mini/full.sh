#!/bin/bash

# ==============================================================================
#  MINISHELL ULTIMATE TESTER (Logic + Valgrind + Stderr + Malloc Fault Injection)
# ==============================================================================

# --- Argument Parsing ---
# Usage: ./test_mini.sh [QUIET=1/0] [MALLOC=1/0] [START_SECTION] [END_SECTION]

QUIET_FLAG=${1:-0}
QUIET_MODE=false
if [ "$QUIET_FLAG" == "1" ]; then
    QUIET_MODE=true
fi

MALLOC_FLAG=${2:-1}
ENABLE_MALLOC_STRESS=true
if [ "$MALLOC_FLAG" == "0" ]; then
    ENABLE_MALLOC_STRESS=false
fi

START_SECTION=${3:-1}
if [ -n "$3" ] && [ -z "$4" ]; then
    END_SECTION=$3
elif [ -n "$4" ]; then
    END_SECTION=$4
else
    END_SECTION=19
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
SUPP_FILE="$ROOT_DIR/readline.supp"

# --- Multithreading Setup ---
if command -v nproc &> /dev/null; then
    MAX_JOBS=$(nproc)
else
    MAX_JOBS=4
fi

if [ "$MAX_JOBS" -gt 2 ]; then
    ((MAX_JOBS--))
fi

TMP_DIR="/dev/shm/mini_tester_$$"
mkdir -p "$TMP_DIR"

display_count=0
last_queued_print=0
test_count=0
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

    if (dladdr(__builtin_return_address(0), &info) && info.dli_fname) {
        if (strstr(info.dli_fname, "minishell")) {
            alloc_count++;
            target_env = getenv("FAIL_MALLOC_AT");
            if (target_env) {
                target_alloc = atoi(target_env);
                if (target_alloc > 0 && alloc_count == target_alloc) {
                    if (getenv("TRACE_MALLOC")) {
                        raise(SIGTRAP); 
                    }
                    return NULL;
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

check_sec() {
    if [ "$1" -ge "$START_SECTION" ] && [ "$1" -le "$END_SECTION" ]; then
        return 0
    fi
    return 1
}

# --- Helper: Extract GDB Trace ---
get_gdb_trace() {
    local test_i=$1
    local mode=$2
    local test_dir=$3
    local GDB_SCRIPT="$test_dir/gdb_script.gdb"
    cat <<EOF > "$GDB_SCRIPT"
set env FAIL_MALLOC_AT $test_i
set env LD_PRELOAD $ROOT_DIR/faulty_malloc.so
$( [ "$mode" == "malloc" ] && echo "set env TRACE_MALLOC 1" )
run < $test_dir/in.tmp
bt 10
EOF
    local gdb_out=$(gdb -batch -x "$GDB_SCRIPT" --args $MINISHELL 2>/dev/null)
    echo "$gdb_out" | grep -vE "faulty_malloc\.c|malloc\.c|libc\.so|/nptl/|/sysdeps/|/csu/|pthread_kill\.c|raise\.c|abort\.c" | grep -iE "\.c:" | sed -E 's/^[ \t]*#[0-9]+[ \t]+(0x[0-9a-f]+ in )?//' | awk '{printf (NR==1 ? "" : "\n        -> ") $0}'
}

# --- Helper: Clean and Normalize Environment Outputs ---
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

# --- Helper: Clean Readline Prompts and Output Noise ---
normalize_stdout() {
    local file=$1
    sed -i -E 's/^[a-zA-Z0-9_\.-]+@[a-zA-Z0-9_\.-]+:.*\$ //g' "$file"
    sed -i -E '/^\$ $/d' "$file"
    sed -i -E 's/^\$ //g' "$file"
    sed -i -E '/^exit$/d' "$file"
}

# --- Helper: Clean Stderr ---
normalize_stderr() {
    local file=$1
    local is_bash=$2

    sed -i -E '/sh: [0-9]+: getcwd\(\) failed/d' "$file"
    if [ "$is_bash" = true ]; then
        sed -E 's/^bash: line [0-9]+: //g; s/^bash: //g' "$file" > "${file}.norm"
    else
        sed -E 's/^minishell: //g' "$file" > "${file}.norm"
    fi
    mv "${file}.norm" "$file"
}

# --- Core Testing Function ---
run_test() {
    local test_id=$1
    local cmd_string=$2
    local mode=$3
    
    local test_dir="$TMP_DIR/$test_id"
    mkdir -p "$test_dir"
    touch "$TMP_DIR/running_$test_id"
    
    local IN_TMP="$test_dir/in.tmp"
    local B_OUT="$test_dir/bash_out.tmp"
    local B_ERR="$test_dir/bash_err.tmp"
    local M_OUT="$test_dir/mini_out.tmp"
    local M_ERR="$test_dir/mini_err.tmp"
    local V_LOG="$test_dir/valgrind.log"
    local M_ERR_LOG="$test_dir/malloc_err.tmp"
    
    # Format input script so shell exits cleanly with the status of the final command
    printf "%s\nexit \$?\n" "$cmd_string" > "$IN_TMP"

    local is_env_test=false
    local is_null_test=false

    if [[ "$cmd_string" == *"env"* || "$cmd_string" == *"export"* ]]; then is_env_test=true; fi
    if [[ "$cmd_string" == *"-0"* || "$cmd_string" == *"--null"* ]]; then is_null_test=true; fi

    local display_title="${cmd_string//$'\n'/ ; }"
    if [ ${#display_title} -gt 55 ]; then
        display_title="${display_title:0:52}..."
    fi

    local display_out=""
    display_out+=$(printf "Test %-55s " "[$display_title]")

    # 1. Run standard Bash
    bash < "$IN_TMP" > "$B_OUT" 2> "$B_ERR"
    local bash_status=$?

    # 2. Run Minishell wrapped in Valgrind
    local val_supp=""
    if [ -f "$SUPP_FILE" ]; then
        val_supp="--suppressions=$SUPP_FILE"
    fi

    valgrind $val_supp --leak-check=full --show-leak-kinds=all --track-origins=yes \
             --errors-for-leak-kinds=all --log-file="$V_LOG" \
             $MINISHELL < "$IN_TMP" > "$M_OUT" 2> "$M_ERR"
    local mini_status=$?

    # 3. Analyze Baseline Valgrind Output
    local val_total=$(awk '/ERROR SUMMARY:/ {sum += $4} END {print sum}' "$V_LOG")
    local valgrind_ok=true
    local val_pass_num=0
    if [[ -z "$val_total" || "$val_total" -ne 0 ]]; then
        valgrind_ok=false
    else
        val_pass_num=1
    fi

    # 4. MALLOC FAULT INJECTION
    local malloc_status="${CYAN}CLEAN${RESET}"
    local malloc_ok=true
    local malloc_pass_num=0
    local TOTAL_ALLOCS=$(awk '/total heap usage:/ {sum += $5} END {print sum}' "$V_LOG" | tr -d ',')

    if [ "$ENABLE_MALLOC_STRESS" = true ] && [ "$valgrind_ok" = true ] && [ -n "$TOTAL_ALLOCS" ] && [ "$TOTAL_ALLOCS" -gt 0 ]; then
        for (( i=1; i<=TOTAL_ALLOCS; i++ )); do
            FAIL_MALLOC_AT=$i LD_PRELOAD="$ROOT_DIR/faulty_malloc.so" $MINISHELL < "$IN_TMP" > /dev/null 2> "$M_ERR_LOG"
            local crash_status=$?
            
            if [ $crash_status -eq 139 ] || [ $crash_status -eq 134 ] || [ $crash_status -eq 137 ] || [ $crash_status -eq 136 ]; then
                local m_orig=$(get_gdb_trace "$i" "malloc" "$test_dir")
                local c_orig=$(get_gdb_trace "$i" "crash" "$test_dir")
                malloc_status="${MAGENTA}SEGFAULT @ alloc #$i${RESET}"
                if [ -n "$m_orig" ]; then malloc_status="${malloc_status}\n      ${YELLOW}Malloc:${RESET} $m_orig"; fi
                if [ -n "$c_orig" ]; then malloc_status="${malloc_status}\n      ${RED}Crash: ${RESET} $c_orig"; fi
                malloc_ok=false
                break
            fi

            if ! grep -qiE "malloc|allocate|memory" "$M_ERR_LOG"; then
                if [ $crash_status -eq $mini_status ]; then
                    continue
                fi
                local m_orig=$(get_gdb_trace "$i" "malloc" "$test_dir")
                malloc_status="${YELLOW}SILENT FAIL @ alloc #$i${RESET}"
                if [ -n "$m_orig" ]; then malloc_status="${malloc_status}\n      ${YELLOW}Malloc:${RESET} $m_orig"; fi
                malloc_ok=false
                break
            fi
        done
    fi
    if [ "$malloc_ok" = true ]; then malloc_pass_num=1; fi

    # 5. Normalize and Compare Output
    normalize_stdout "$M_OUT"
    normalize_stdout "$B_OUT"

    local out_diff=0
    local err_diff=0

    if [ "$mode" == "flag_error" ]; then
        if [ ! -s "$M_ERR" ]; then
            echo -e "Expected an error message on stderr, but standard error was empty." > "$test_dir/err_diff.log"
            err_diff=1
        fi
    else
        if [ "$is_env_test" = true ]; then
            normalize_env "$B_OUT" "$is_null_test"
            normalize_env "$M_OUT" "$is_null_test"
        fi
        diff -u "$B_OUT" "$M_OUT" > "$test_dir/diff.log"
        out_diff=$?

        normalize_stderr "$B_ERR" true
        normalize_stderr "$M_ERR" false
        diff -u "$B_ERR" "$M_ERR" > "$test_dir/err_diff.log"
        err_diff=$?
    fi

    # 6. Format Output
    local val_str="${RED}FAILED${RESET}"
    local logic_pass_num=0
    if [ "$valgrind_ok" = true ]; then val_str="${CYAN}CLEAN${RESET}"; fi

    if [ $out_diff -eq 0 ] && [ $err_diff -eq 0 ] && [ $bash_status -eq $mini_status ]; then
        display_out+="$(echo -e "${GREEN}PASS${RESET} | Val: $val_str | Malloc: $malloc_status")\n"
        logic_pass_num=1
    else
        display_out+="$(echo -e "${RED}FAIL${RESET} | Val: $val_str | Malloc: $malloc_status")\n"
        if [ $bash_status -ne $mini_status ]; then display_out+="$(echo -e "  ${YELLOW}Exit Differs:${RESET} Bash=$bash_status, Mini=$mini_status")\n"; fi
        if [ $out_diff -ne 0 ]; then display_out+="$(echo -e "  ${YELLOW}Stdout Differs:${RESET}")\n$(tr -d '\0' < "$test_dir/diff.log" | sed 's/^/    /')\n"; fi
        if [ $err_diff -ne 0 ]; then display_out+="$(echo -e "  ${YELLOW}Stderr Differs:${RESET}")\n$(cat "$test_dir/err_diff.log" | sed 's/^/    /')\n"; fi
    fi

    if [ "$valgrind_ok" = false ]; then
        display_out+="$(echo -e "  ${MAGENTA}Valgrind Report:${RESET}")\n"
        display_out+="$(grep -E -A50000 "ERROR SUMMARY|Invalid|definitely lost|indirectly lost|possibly lost|uninitialised" "$V_LOG" | sed 's/^/    /')\n"
    fi

    if [ "$QUIET_MODE" = true ] && [ "$logic_pass_num" -eq 1 ] && [ "$valgrind_ok" = true ] && [ "$malloc_ok" = true ]; then
        display_out=""
    fi

    echo -e "$display_out" > "$test_dir/display.log"
    echo "$logic_pass_num $val_pass_num $malloc_pass_num" > "$test_dir/status.dat"
    rm -f "$TMP_DIR/running_$test_id"
    touch "$test_dir/done.flag"
}

queue_header() {
    ((display_count++))
    local item_dir="$TMP_DIR/$display_count"
    mkdir -p "$item_dir"
    echo -e "\n${YELLOW}$1${RESET}" > "$item_dir/display.log"
    touch "$item_dir/done.flag"
}

queue_test() {
    ((display_count++))
    ((test_count++))
    while [ $(ls "$TMP_DIR"/running_* 2>/dev/null | wc -l) -ge "$MAX_JOBS" ]; do
        sleep 0.05
    done
    run_test "$display_count" "$1" "strict" &
}

queue_flag_test() {
    ((display_count++))
    ((test_count++))
    while [ $(ls "$TMP_DIR"/running_* 2>/dev/null | wc -l) -ge "$MAX_JOBS" ]; do
        sleep 0.05
    done
    run_test "$display_count" "$1" "flag_error" &
}

flush_queue() {
    local start=$((last_queued_print + 1))
    local end=$display_count

    if [ "$start" -le "$end" ]; then
        for (( i=start; i<=end; i++ )); do
            while [ ! -f "$TMP_DIR/$i/done.flag" ]; do
                sleep 0.05
            done
        done

        for (( i=start; i<=end; i++ )); do
            local log_content=$(tr -d '\0' < "$TMP_DIR/$i/display.log" 2>/dev/null)
            if [ -n "$log_content" ]; then
                printf "%s\n" "$log_content"
            fi
        done
    fi
    last_queued_print=$display_count
}

echo "=========================================================="
echo "    Extensive Minishell Tester (Sections $START_SECTION to $END_SECTION) "
echo "=========================================================="

if check_sec 1; then
queue_header "--- 1. Testing cd Builtin & Canonicalization..."
queue_test "cd ."
queue_test "cd .."
queue_test "cd /tmp"
queue_test "cd -L /tmp"
queue_test "cd -P /tmp"
queue_test "cd -e /tmp"
queue_test "cd /does_not_exist"
queue_test "cd /tmp/../tmp/../tmp"
queue_test "cd -"
queue_test "cd ~"
    flush_queue
fi

if check_sec 2; then
queue_header "--- 2. Testing pwd Builtin..."
queue_test "pwd"
queue_test "pwd -L"
queue_test "pwd -P"
queue_test "pwd -LLLLPPPPLLLLPPPP"
    flush_queue
fi

if check_sec 3; then
queue_header "--- 3. Testing echo Builtin & Escapes..."
queue_test "echo"
queue_test "echo hello world"
queue_test "echo -n hello world"
queue_test "echo -nnnn hello"
queue_test "echo -e 'hello\nworld\t!'"
queue_test "echo -E 'hello\nworld\t!'"
queue_test "echo -ne 'test\n'"
queue_test "echo -e '\x41\x42\x43'"
queue_test "echo -e 'Before\cAfter'"
queue_test "echo -e '\\\\\\\\'"
    flush_queue
fi

if check_sec 4; then
queue_header "--- 4. Testing export Builtin & Identifier Parsing..."
queue_test "export"
queue_test "export -p"
queue_test "export VAR_TEST=123"
queue_test "export VAR_TEST+=456"
queue_test "export BAD-VAR=123"
queue_test "export _VALID=1 2INVALID=2 ALSO_VALID=3"
queue_test "export NULL_VAR EMPTY_VAR="
queue_test "export WEIRD_VAR=\"hello=world=test=123\""
    flush_queue
fi

if check_sec 5; then
queue_header "--- 5. Testing unset Builtin..."
queue_test "unset PATH"
queue_test "unset DOES_NOT_EXIST"
queue_test "unset BAD-NAME"
queue_test "unset -v PATH"
    flush_queue
fi

if check_sec 6; then
queue_header "--- 6. Testing env Builtin & Options..."
queue_test "env"
queue_test "env -i"
queue_test "env -0"
queue_test "env -u PATH"
queue_test "env -C /tmp pwd"
queue_test "env -a ARGV0 echo hello"
queue_test "env -S 'echo hello split string'"
queue_test "env --ignore-environment"
queue_test "env --null"
queue_test "env --chdir=/tmp pwd"
    flush_queue
fi

if check_sec 7; then
queue_header "--- 7. Testing exit Builtin..."
queue_test "exit 0"
queue_test "exit 42"
queue_test "exit -42"
queue_test "exit 9223372036854775807"
queue_test "exit 9223372036854775808"
queue_test "exit 42 42"
queue_test "exit hello"
queue_test "exit 42hello"
queue_test "exit -- -42"
    flush_queue
fi

if check_sec 8; then
queue_header "--- 8. SINGLE-SHELL SESSIONS: Persistent Shell State Sequences..."
queue_test $'export A=10\nexport B=20\necho "A=$A B=$B"\nunset A\necho "A=$A B=$B"'
queue_test $'cd /tmp\npwd\ncd ..\npwd'
queue_test $'export VAR=hello\nexport VAR+=_world\necho $VAR'
queue_test $'export X=1\n(export X=2; echo "subshell X=$X")\necho "parent X=$X"'
queue_test $'cd /tmp\n(cd /var; echo "subshell pwd="; pwd)\necho "parent pwd="; pwd'
queue_test $'export FOO=bar\nenv | grep FOO\nunset FOO\nenv | grep FOO'
queue_test $'ls /does_not_exist\necho "Status 1: $?"\nls -d /tmp\necho "Status 2: $?"'
    flush_queue
fi

if check_sec 9; then
queue_header "--- 9. Variable, Tilde, & Special Expansions..."
queue_test "echo \$USER"
queue_test "ls /does_not_exist; echo \$?"
queue_test "echo \$0"
queue_test "echo \$\$"
queue_test "echo \$!"
queue_test "echo ~"
queue_test "echo ~/"
queue_test "echo '\$USER' \"\$USER\" \$USER"
queue_test "export FOO=bar; echo \$FOO"
queue_test "echo \$NOSUCHVARIABLE_XYZ_123"
queue_test "echo \$1 \$2 \$99"
    flush_queue
fi

if check_sec 10; then
queue_header "--- 10. Quotes, Whitespace, & Parsing Anomalies..."
queue_test "echo '' '' '   ' ''"
queue_test "echo \"\" \"   \" \"\""
queue_test "echo   a    b      c  "
queue_test "echo 'a   b   c'"
queue_test "echo \"a   b   c\""
queue_test "echo \"'hello'\""
queue_test "echo '\"hello\"'"
queue_test "echo \"\$USER's laptop\""
    flush_queue
fi

if check_sec 11; then
queue_header "--- 11. Redirections (<, >, >>)..."
queue_test "echo hello > /tmp/mini_test_r1.txt; cat /tmp/mini_test_r1.txt; rm -f /tmp/mini_test_r1.txt"
queue_test "echo line1 > /tmp/mini_test_r2.txt; echo line2 >> /tmp/mini_test_r2.txt; cat /tmp/mini_test_r2.txt; rm -f /tmp/mini_test_r2.txt"
queue_test "cat < /etc/hostname"
queue_test "> /tmp/mini_empty.txt; ls -l /tmp/mini_empty.txt; rm -f /tmp/mini_empty.txt"
queue_test "cat < /tmp/file_does_not_exist_xyz"
queue_test "echo first > /tmp/m1.txt > /tmp/m2.txt; cat /tmp/m1.txt; echo \"---\"; cat /tmp/m2.txt; rm -f /tmp/m1.txt /tmp/m2.txt"
    flush_queue
fi

if check_sec 12; then
queue_header "--- 12. Pipelines (|)..."
queue_test "echo hello | cat"
queue_test "cat /etc/hostname | grep -o a | wc -l"
queue_test "ls -la | grep srcs | wc -l"
queue_test "export TEST_PIPE=42 | echo hello; echo \$TEST_PIPE"
queue_test "ls /does_not_exist | wc -l"
queue_test "echo hello | cat | cat | cat | grep h"
    flush_queue
fi

if check_sec 13; then
queue_header "--- 13. Logical Operators (&&, ||)..."
queue_test "true && echo yes"
queue_test "false || echo no"
queue_test "false && echo no"
queue_test "true || echo no"
queue_test "echo 1 && echo 2 || echo 3"
queue_test "ls /does_not_exist && echo success || echo failed"
queue_test "false || false || echo third_time_charm"
    flush_queue
fi

if check_sec 14; then
queue_header "--- 14. Subshells (...)..."
queue_test "(echo inside subshell)"
queue_test "(export SUB_VAR=sub); echo \$SUB_VAR"
queue_test "((echo nested))"
queue_test "(echo hello) > /tmp/mini_sub_out.txt; cat /tmp/mini_sub_out.txt; rm -f /tmp/mini_sub_out.txt"
queue_test "(cd /tmp && pwd); pwd"
    flush_queue
fi

if check_sec 15; then
queue_header "--- 15. Control Structures & Semicolons (;)..."
queue_test "echo 1; echo 2; echo 3"
queue_test "pwd; cd /tmp; pwd"
queue_test ";;"
queue_test "echo 1; ; echo 2"
    flush_queue
fi

if check_sec 16; then
queue_header "--- 16. HARDCORE: Rug Pull (Deleted Directory)..."
mkdir -p /tmp/mini_rugpull
cd /tmp/mini_rugpull
rm -rf /tmp/mini_rugpull
queue_test "pwd"
queue_test "cd ."
queue_test "cd .."
cd "$ROOT_DIR" || cd /tmp
    flush_queue
fi

if check_sec 17; then
queue_header "--- 17. HARDCORE: Executable & Path Resolution (126/127)..."
queue_test "env /tmp"
touch /tmp/mini_no_exec
chmod 000 /tmp/mini_no_exec
queue_test "/tmp/mini_no_exec"
queue_test "/does_not_exist_mini_bin"
queue_test "''"
queue_test "."
queue_test ".."
    flush_queue
rm -f /tmp/mini_no_exec
fi

if check_sec 18; then
queue_header "--- 18. HARDCORE: State Corruption & Gaslighting..."
OLD_PWD_VAR=$PWD
export PWD=/completely/fake/path
queue_test "pwd"
queue_test "pwd -L"
queue_test "pwd -P"
export PWD=$OLD_PWD_VAR

OLD_OLDPWD=$OLDPWD
unset OLDPWD
queue_test "cd -"
export OLDPWD=$OLD_OLDPWD
    flush_queue
fi

if check_sec 19; then
queue_header "--- 19. FLAG PARSING: Invalid Builtin Options..."
queue_flag_test "cd -Z /tmp"
queue_flag_test "pwd -Z"
queue_flag_test "export -Z"
queue_flag_test "unset -Z"
queue_flag_test "env -Z"
queue_flag_test "exit -Z"
    flush_queue
fi

echo -e "\n=========================================================="
echo -e "${YELLOW}Waiting for all background jobs and tests to finish...${RESET}"
wait

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
