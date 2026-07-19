#!/bin/bash

# ==============================================================================
#  MINISHELL ULTIMATE TESTER (Logic + Valgrind + Automatic GDB Tracing)
# ==============================================================================

# --- Settings ---
ENABLE_MALLOC_STRESS=true   # Set to false to skip the hardcore malloc testing

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

# --- Temp Files ---
BASH_OUT="$ROOT_DIR/bash_out.tmp"
BASH_ERR="$ROOT_DIR/bash_err.tmp"
MINI_OUT="$ROOT_DIR/mini_out.tmp"
MINI_ERR="$ROOT_DIR/mini_err.tmp"
VALGRIND_LOG="$ROOT_DIR/valgrind_out.log"
MALLOC_LOG="$ROOT_DIR/val_malloc.log"
GDB_SCRIPT="$ROOT_DIR/gdb_script.gdb"

test_count=0
test_passed=0
valgrind_passed=0
malloc_passed=0

# Ensure minishell and debuggers are installed
if [ ! -f "$MINISHELL" ]; then
    echo -e "${RED}Error: $MINISHELL not found. Compile your project first.${RESET}"
    exit 1
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

void *malloc(size_t size) {
    static void *(*real_malloc)(size_t) = NULL;
    char *target_env;
    int target_alloc;
    static int alloc_count = 0;

    if (!real_malloc)
        real_malloc = dlsym(RTLD_NEXT, "malloc");

    alloc_count++;
    target_env = getenv("FAIL_MALLOC_AT");
    if (target_env) {
        target_alloc = atoi(target_env);
        if (target_alloc > 0 && alloc_count == target_alloc) {
            // If GDB trace mode is active, trigger a breakpoint right here!
            if (getenv("TRACE_MALLOC")) {
                raise(SIGTRAP); 
            }
            return NULL; // SABOTAGE!
        }
    }
    return real_malloc(size);
}
EOF
    gcc -shared -fPIC -ldl "$ROOT_DIR/faulty_malloc.c" -o "$ROOT_DIR/faulty_malloc.so"
    if [ ! -f "$ROOT_DIR/faulty_malloc.so" ]; then
        echo -e "${RED}Failed to compile faulty_malloc.so!${RESET}"
        exit 1
    fi
fi

# --- Helper: Extract GDB Trace ---
get_gdb_trace() {
    local test_i=$1
    local mode=$2 # "malloc" or "crash"
    
    # Create GDB batch commands
    cat <<EOF > "$GDB_SCRIPT"
set env FAIL_MALLOC_AT $test_i
set env LD_PRELOAD $ROOT_DIR/faulty_malloc.so
$( [ "$mode" == "malloc" ] && echo "set env TRACE_MALLOC 1" )
run
bt 10
EOF
    # Run GDB and parse out the first relevant line of C code
    local gdb_out=$(gdb -batch -x "$GDB_SCRIPT" --args $MINISHELL "$builtin" "$ROOT_DIR/mini_trash.tmp" "${mini_args[@]}" 2>/dev/null)
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

# --- Core Testing Function ---
run_test() {
    local builtin=$1
    local cmd_string=$2
    local is_env_test=false
    local is_null_test=false
    local bash_cmd="$cmd_string"

    if [[ "$builtin" == "env" || "$builtin" == "export" ]]; then is_env_test=true; fi
    if [[ "$cmd_string" == *"-0"* || "$cmd_string" == *"--null"* ]]; then is_null_test=true; fi
    if [[ "$builtin" == "unset" ]]; then bash_cmd="${cmd_string/unset /unset -v }"; fi

    ((test_count++))
    printf "Test %-40s " "[$cmd_string]"

    # 1. Run standard Bash
    bash -c "$bash_cmd" > "$BASH_OUT" 2> "$BASH_ERR"
    local bash_status=$?

    # 2. Run Minishell wrapped in Valgrind (Baseline)
    eval "mini_args=($cmd_string)"
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
             --errors-for-leak-kinds=all --log-file="$VALGRIND_LOG" \
             $MINISHELL "$builtin" "$MINI_OUT" "${mini_args[@]}" > /dev/null 2> "$MINI_ERR"
    local mini_status=$?

    # 3. Analyze Baseline Valgrind Log
    local val_total_errors=$(awk '/ERROR SUMMARY:/ {sum += $4} END {print sum}' "$VALGRIND_LOG")
    local valgrind_ok=true
    if [[ -z "$val_total_errors" || "$val_total_errors" -ne 0 ]]; then
        valgrind_ok=false
    else
        ((valgrind_passed++))
    fi

    # 4. MALLOC FAULT INJECTION STRESS TEST
    local malloc_status="${CYAN}CLEAN${RESET}"
    local malloc_ok=true
    local TOTAL_ALLOCS=$(awk '/total heap usage:/ {sum += $5} END {print sum}' "$VALGRIND_LOG" | tr -d ',')

    if [ "$ENABLE_MALLOC_STRESS" = true ] && [ "$valgrind_ok" = true ] && [ -n "$TOTAL_ALLOCS" ] && [ "$TOTAL_ALLOCS" -gt 0 ]; then
        for (( i=1; i<=TOTAL_ALLOCS; i++ )); do
            FAIL_MALLOC_AT=$i LD_PRELOAD="$ROOT_DIR/faulty_malloc.so" $MINISHELL "$builtin" "$ROOT_DIR/mini_trash.tmp" "${mini_args[@]}" > /dev/null 2> "$ROOT_DIR/malloc_err.tmp"
            local crash_status=$?
            
            # 4a. Check if it Segfaulted
            if [ $crash_status -eq 139 ] || [ $crash_status -eq 134 ] || [ $crash_status -gt 128 ]; then
                local m_orig=$(get_gdb_trace "$i" "malloc")
                local c_orig=$(get_gdb_trace "$i" "crash")
                
                malloc_status="${MAGENTA}SEGFAULT @ alloc #$i${RESET}"
                if [ -n "$m_orig" ]; then malloc_status="${malloc_status}\n      ${YELLOW}Malloc:${RESET} $m_orig"; fi
                if [ -n "$c_orig" ]; then malloc_status="${malloc_status}\n      ${RED}Crash: ${RESET} $c_orig"; fi
                
                malloc_ok=false
                break
            fi

            # 4b. Check if it silently failed without printing an error
            if ! grep -qiE "malloc|allocate|memory" "$ROOT_DIR/malloc_err.tmp"; then
                local m_orig=$(get_gdb_trace "$i" "malloc")
                
                malloc_status="${YELLOW}SILENT FAIL @ alloc #$i${RESET}"
                if [ -n "$m_orig" ]; then malloc_status="${malloc_status}\n      ${YELLOW}Malloc:${RESET} $m_orig"; fi
                
                malloc_ok=false
                break
            fi
        done
        rm -f "$ROOT_DIR/mini_trash.tmp" "$ROOT_DIR/malloc_err.tmp"
    fi

    if [ "$malloc_ok" = true ]; then ((malloc_passed++)); fi

    # 5. Normalize and Compare outputs
    if [ "$is_env_test" = true ]; then
        normalize_env "$BASH_OUT" "$is_null_test"
        normalize_env "$MINI_OUT" "$is_null_test"
    fi
    diff -u "$BASH_OUT" "$MINI_OUT" > "$ROOT_DIR/diff.log"
    local out_diff=$?

    # --- Print Formatting ---
    local val_str="${RED}FAILED${RESET}"
    if [ "$valgrind_ok" = true ]; then val_str="${CYAN}CLEAN${RESET}"; fi

    if [ $out_diff -eq 0 ] && [ $bash_status -eq $mini_status ]; then
        echo -e "${GREEN}PASS${RESET} | Val: $val_str | Malloc: $malloc_status"
        ((test_passed++))
    else
        echo -e "${RED}FAIL${RESET} | Val: $val_str | Malloc: $malloc_status"
        if [ $bash_status -ne $mini_status ]; then echo -e "  ${YELLOW}Exit Differs:${RESET} Bash=$bash_status, Mini=$mini_status"; fi
        if [ $out_diff -ne 0 ]; then echo -e "  ${YELLOW}Stdout Differs:${RESET}"; cat "$ROOT_DIR/diff.log" | sed 's/^/    /'; fi
    fi

    # Print Valgrind trace if baseline memory leaked
    if [ "$valgrind_ok" = false ]; then
        echo -e "  ${MAGENTA}Valgrind Report:${RESET}"
        cat "$VALGRIND_LOG" | grep -v "Command:" | sed 's/^/    /'
    fi
}

echo "=========================================================="
echo "    Extensive Minishell Tester (Hardcore Malloc Mode) "
echo "=========================================================="

# --- 1. CD Tests ---
echo -e "\n${YELLOW}Testing cd (-L, -P, -e)...${RESET}"
run_test "cd" "cd ."
run_test "cd" "cd .."
run_test "cd" "cd /tmp"
run_test "cd" "cd -L /tmp"
run_test "cd" "cd -P /tmp"
run_test "cd" "cd -e /tmp"
run_test "cd" "cd /does_not_exist"

# --- 2. PWD Tests ---
echo -e "\n${YELLOW}Testing pwd (-L, -P)...${RESET}"
run_test "pwd" "pwd"
run_test "pwd" "pwd -L"
run_test "pwd" "pwd -P"

# --- 3. ECHO Tests ---
echo -e "\n${YELLOW}Testing echo (-n, -e, -E)...${RESET}"
run_test "echo" "echo"
run_test "echo" "echo hello world"
run_test "echo" "echo -n hello world"
run_test "echo" "echo -nnnn hello"
run_test "echo" "echo -e 'hello\nworld\t!'"
run_test "echo" "echo -E 'hello\nworld\t!'"
run_test "echo" "echo -ne 'test\n'"
run_test "echo" "echo -e '\x41\x42\x43'"

# --- 4. EXPORT Tests ---
echo -e "\n${YELLOW}Testing export (-p, -n)...${RESET}"
run_test "export" "export"
run_test "export" "export -p"
run_test "export" "export VAR_TEST=123"
run_test "export" "export -n PATH"
run_test "export" "export BAD-VAR=123" 

# --- 5. UNSET Tests ---
echo -e "\n${YELLOW}Testing unset (-v)...${RESET}"
run_test "unset" "unset PATH"
run_test "unset" "unset DOES_NOT_EXIST"
run_test "unset" "unset BAD-NAME" 

# --- 6. ENV Tests ---
echo -e "\n${YELLOW}Testing env (-a, -i, -0, -u, -C, -S)...${RESET}"
run_test "env" "env"
run_test "env" "env -i"
run_test "env" "env -0"
run_test "env" "env -u PATH"
run_test "env" "env -C /tmp pwd"
run_test "env" "env -a ARGV0 echo hello"
run_test "env" "env -S 'echo hello split string'"

# Long Format Env Tests
run_test "env" "env --ignore-environment"
run_test "env" "env --null"
run_test "env" "env --unset=PATH"
run_test "env" "env --chdir=/tmp pwd"
run_test "env" "env --argv0=ARGV0 echo hello"
run_test "env" "env --split-string='echo hello split string'"

# --- 7. HARDCORE: CD & PWD Stress ---
echo -e "\n${YELLOW}HARDCORE: Directory Edge Cases...${RESET}"
run_test "cd" "cd -L -P -L -P /tmp"
run_test "cd" "cd -- -L" 
run_test "cd" "cd ''"
run_test "cd" "cd /tmp/../tmp/../tmp/../tmp"
run_test "pwd" "pwd -LLLLPPPPLLLLPPPP"
run_test "pwd" "pwd -- -L"

# --- 8. HARDCORE: ECHO Stress ---
echo -e "\n${YELLOW}HARDCORE: Echo Parsing Anomalies...${RESET}"
run_test "echo" "echo -neE -en -n -e -E 'mix and match'"
run_test "echo" "echo -e 'Before\cAfter'"
run_test "echo" "echo '' '' '   ' ''"
run_test "echo" "echo -e '\x41\x42\x43 \0104\0105\0106'"
run_test "echo" "echo -e '\\\\\\\\'"
run_test "echo" "echo -e '\x \0 \c'"

# --- 9. HARDCORE: EXPORT & UNSET Stress ---
echo -e "\n${YELLOW}HARDCORE: Hashmap & Key Validation...${RESET}"
run_test "export" "export _VALID=1 2INVALID=2 ALSO_VALID=3"
run_test "export" "export NULL_VAR EMPTY_VAR="
run_test "export" "export WEIRD_VAR=\"hello=world=test=123\""
run_test "export" "export ="
run_test "export" "export '===='"
run_test "unset" "unset _VALID 2INVALID ALSO_VALID DOESNT_EXIST"

# --- 10. HARDCORE: ENV Stress ---
echo -e "\n${YELLOW}HARDCORE: Env Execution & Nested Environments...${RESET}"
run_test "env" "env -i A=1 A=2 A=3"
run_test "env" "env -u PATH -u HOME -u USER"
run_test "env" "env -i A=1 B=2 env -i C=3 pwd"
run_test "env" "env -i -S 'A=1 B=2 env -i C=3 pwd'"
run_test "env" "env -0 -i NEW_VAR=123 NEW_VAR2=456"

# --- 11. HARDCORE: System, Starvation, and POSIX Extremes ---
echo -e "\n${YELLOW}HARDCORE: System, Starvation, and POSIX Extremes...${RESET}"
mkdir -p /tmp/mini_cdpath_test
export CDPATH=/tmp
run_test "cd" "cd mini_cdpath_test"
unset CDPATH

export OLDPWD=/tmp
run_test "cd" "cd -"

unset OLDPWD
run_test "cd" "cd -"

OLD_HOME=$HOME
unset HOME
run_test "cd" "cd"
export HOME=$OLD_HOME

run_test "env" "env -i ls"
run_test "env" "env sh -c 'exit 42'"
run_test "env" "env sh -c 'kill -9 \$$'"

run_test "export" "export MULTILINE=$'Line1\nLine2\tTabbed'"
run_test "cd" "cd /tmp /var /usr"

rm -rf /tmp/mini_cdpath_test

# --- 12. HARDCORE: Exit Trapdoors ---
echo -e "\n${YELLOW}HARDCORE: Exit Trapdoors...${RESET}"
run_test "exit" "exit 0"
run_test "exit" "exit 42"
run_test "exit" "exit -42"
run_test "exit" "exit 9223372036854775807"
run_test "exit" "exit 9223372036854775808"
run_test "exit" "exit 42 42"
run_test "exit" "exit hello"
run_test "exit" "exit 42hello"
run_test "exit" "exit -- -42"

# --- 13. HARDCORE: Env Execution Exit Codes (126/127) ---
echo -e "\n${YELLOW}HARDCORE: Env Execution Exit Codes (126/127)...${RESET}"
run_test "env" "env /tmp"
touch /tmp/mini_no_exec
chmod 000 /tmp/mini_no_exec
run_test "env" "env /tmp/mini_no_exec"
rm -f /tmp/mini_no_exec
run_test "env" "env /does_not_exist_mini"

# --- 14. HARDCORE: The Rug Pull (Deleted Working Directory) ---
echo -e "\n${YELLOW}HARDCORE: The Rug Pull (Deleted Working Directory)...${RESET}"
mkdir -p /tmp/mini_rugpull
cd /tmp/mini_rugpull
rm -rf /tmp/mini_rugpull
# We are now executing tests from a directory that no longer exists!
run_test "pwd" "pwd"
run_test "cd" "cd ."
run_test "cd" "cd .."
cd "$ROOT_DIR" || cd /tmp

# --- 15. HARDCORE: State Corruption ---
echo -e "\n${YELLOW}HARDCORE: State Corruption...${RESET}"
OLD_PWD_VAR=$PWD
unset PWD
run_test "pwd" "pwd"
export PWD=$OLD_PWD_VAR

OLD_OLDPWD=$OLDPWD
unset OLDPWD
run_test "cd" "cd -"
export OLDPWD=$OLD_OLDPWD

echo -e "\n=========================================================="
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

# Cleanup
rm -f "$BASH_OUT" "$BASH_ERR" "$MINI_OUT" "$MINI_ERR" "$VALGRIND_LOG" "$MALLOC_LOG" "$ROOT_DIR/diff.log" "$ROOT_DIR/faulty_malloc.c" "$ROOT_DIR/faulty_malloc.so" "$ROOT_DIR/mini_trash.tmp" "$ROOT_DIR/malloc_err.tmp" "$GDB_SCRIPT"
