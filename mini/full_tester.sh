#!/bin/bash

# --- Colors for Output ---
GREEN="\033[32m"
RED="\033[31m"
YELLOW="\033[33m"
CYAN="\033[36m"
MAGENTA="\033[35m"
RESET="\033[0m"

MINISHELL="./minishell"

# --- Temp Files ---
BASH_OUT="bash_out.tmp"
BASH_ERR="bash_err.tmp"
MINI_OUT="mini_out.tmp"
MINI_ERR="mini_err.tmp"
VALGRIND_LOG="valgrind_out.log"

test_count=0
test_passed=0
valgrind_passed=0

# Ensure minishell is compiled
if [ ! -f "$MINISHELL" ]; then
    echo -e "${RED}Error: $MINISHELL not found. Compile your project first.${RESET}"
    exit 1
fi

# Ensure Valgrind is installed
if ! command -v valgrind &> /dev/null; then
    echo -e "${RED}Error: valgrind is not installed. Please install it to run these checks.${RESET}"
    exit 1
fi

# --- Helper to clean and normalize environment outputs ---
normalize_env() {
    local file=$1
    local is_null_terminated=$2

    if [ "$is_null_terminated" = true ]; then
        tr '\0' '\n' < "$file" > "$file.tmp"
        mv "$file.tmp" "$file"
    fi

    # Added GLIBC and LD_ to filter out Valgrind's injected variables
    grep -vE '^(declare -x )?(BASH|SHLVL|PWD|OLDPWD|_|LS_COLORS|XDG|GLIBC|LD_)' "$file" | sort > "$file.norm"
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

    # Force Bash to always evaluate unset strictly as variables
    if [[ "$builtin" == "unset" ]]; then
        bash_cmd="${cmd_string/unset /unset -v }"
    fi

    ((test_count++))
    printf "Test %-40s " "[$cmd_string]"

    # 1. Run standard Bash (using the modified bash_cmd)
    bash -c "$bash_cmd" > "$BASH_OUT" 2> "$BASH_ERR"
    local bash_status=$?

    # 2. Run Minishell wrapped in Valgrind (using the original cmd_string)
    eval "mini_args=($cmd_string)"
    
    valgrind --leak-check=full \
             --show-leak-kinds=all \
             --track-origins=yes \
             --errors-for-leak-kinds=all \
             --log-file="$VALGRIND_LOG" \
             $MINISHELL "$builtin" "$MINI_OUT" "${mini_args[@]}" 2> "$MINI_ERR"
             
    local mini_status=$?

    # 3. Analyze Valgrind Log
    local val_total_errors=$(awk '/ERROR SUMMARY:/ {sum += $4} END {print sum}' "$VALGRIND_LOG")
    local valgrind_ok=true

    # If sum isn't 0 (or is completely empty due to a crash)
    if [[ -z "$val_total_errors" || "$val_total_errors" -ne 0 ]]; then
        valgrind_ok=false
    else
        ((valgrind_passed++))
    fi

    # 4. Normalize outputs
    if [ "$is_env_test" = true ]; then
        normalize_env "$BASH_OUT" "$is_null_test"
        normalize_env "$MINI_OUT" "$is_null_test"
    fi

    # 5. Compare outputs
    diff -u "$BASH_OUT" "$MINI_OUT" > diff.log
    local out_diff=$?

    # --- Print Results ---
    if [ $out_diff -eq 0 ] && [ $bash_status -eq $mini_status ]; then
        if [ "$valgrind_ok" = true ]; then
            echo -e "${GREEN}PASS${RESET} | Valgrind: ${CYAN}CLEAN${RESET}"
        else
            echo -e "${GREEN}PASS${RESET} | Valgrind: ${MAGENTA}LEAKS/ERRORS DETECTED${RESET}"
        fi
        ((test_passed++))
    else
        if [ "$valgrind_ok" = true ]; then
            echo -e "${RED}FAIL${RESET} | Valgrind: ${CYAN}CLEAN${RESET}"
        else
            echo -e "${RED}FAIL${RESET} | Valgrind: ${MAGENTA}LEAKS/ERRORS DETECTED${RESET}"
        fi
        
        if [ $bash_status -ne $mini_status ]; then
            echo -e "  ${YELLOW}Exit Status Differs:${RESET} Bash=$bash_status, Mini=$mini_status"
        fi
        if [ $out_diff -ne 0 ]; then
            echo -e "  ${YELLOW}Stdout Differs:${RESET}"
            cat diff.log | sed 's/^/    /'
        fi
    fi

    # Print Valgrind trace if there were memory issues
    if [ "$valgrind_ok" = false ]; then
        echo -e "  ${MAGENTA}Valgrind Report:${RESET}"
        cat "$VALGRIND_LOG" | grep -v "Command:" | sed 's/^/    /'
    fi
}

echo "======================================"
echo "    Extensive Minishell Tester"
echo "        (with Valgrind Memory Checks) "
echo "======================================"

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
run_test "unset" "unset -v PATH"
run_test "unset" "unset DOES_NOT_EXIST"
run_test "unset" "unset -v BAD-NAME" 

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

echo -e "\n======================================"

# --- 7. HARDCORE: CD & PWD Stress ---
echo -e "\n${YELLOW}HARDCORE: Directory Edge Cases...${RESET}"
# Mixed/conflicting flags (last one should win)
run_test "cd" "cd -L -P -L -P /tmp"
# Double dash to signal end of flags
run_test "cd" "cd -- -L" 
# Empty string (Should fail gracefully, no segfault)
run_test "cd" "cd ''"
# Traversing back and forth multiple times
run_test "cd" "cd /tmp/../tmp/../tmp/../tmp"
# Overloading pwd flags
run_test "pwd" "pwd -LLLLPPPPLLLLPPPP"
run_test "pwd" "pwd -- -L"

# --- 8. HARDCORE: ECHO Stress ---
echo -e "\n${YELLOW}HARDCORE: Echo Parsing Anomalies...${RESET}"
# Conflicting flags stacked together
run_test "echo" "echo -neE -en -n -e -E 'mix and match'"
# The \c escape should instantly cut off the rest of the string
run_test "echo" "echo -e 'Before\cAfter'"
# Empty strings and weird spacing
run_test "echo" "echo '' '' '   ' ''"
# Octal and Hex bounds limits
run_test "echo" "echo -e '\x41\x42\x43 \0104\0105\0106'"
# Escaping the escapes
run_test "echo" "echo -e '\\\\\\\\'"
# Missing hex/octal values (should fail gracefully or print literals)
run_test "echo" "echo -e '\x \0 \c'"

# --- 9. HARDCORE: EXPORT & UNSET Stress ---
echo -e "\n${YELLOW}HARDCORE: Hashmap & Key Validation...${RESET}"
# Multiple valid and invalid variables in one line (should process valids, error on invalids)
run_test "export" "export _VALID=1 2INVALID=2 ALSO_VALID=3"
# Exporting empty values vs null values
run_test "export" "export NULL_VAR EMPTY_VAR="
run_test "export" "export -p"
# Value containing multiple equals signs
run_test "export" "export WEIRD_VAR=\"hello=world=test=123\""
# Just an equals sign
run_test "export" "export ="
run_test "export" "export '===='"
# Unsetting multiple keys, including invalid ones and non-existent ones
run_test "unset" "unset _VALID 2INVALID ALSO_VALID DOESNT_EXIST"

# --- 10. HARDCORE: ENV Stress ---
echo -e "\n${YELLOW}HARDCORE: Env Execution & Nested Environments...${RESET}"
# Redefining the same variable multiple times in one line
run_test "env" "env -i A=1 A=2 A=3"
# Unsetting core environment variables manually
run_test "env" "env -u PATH -u HOME -u USER"
# Nested Envs: env calling env calling pwd
run_test "env" "env -i A=1 B=2 env -i C=3 pwd"
# Using -S (Split string) to completely reconstruct an execution
run_test "env" "env -i -S 'A=1 B=2 env -i C=3 pwd'"
# Null terminated with modifications
run_test "env" "env -0 -i NEW_VAR=123 NEW_VAR2=456"
# Forcing failure: directory instead of executable
run_test "env" "env -i /tmp"

# --- 11. HARDCORE: System, Starvation, and POSIX Extremes ---
echo -e "\n${YELLOW}HARDCORE: System, Starvation, and POSIX Extremes...${RESET}"

# 1. The CDPATH & OLDPWD Features
echo -e "mkdir -p /tmp/mini_cdpath_test"
mkdir -p /tmp/mini_cdpath_test
echo -e "export CDPATH=/tmp"
export CDPATH=/tmp
run_test "cd" "cd mini_cdpath_test"
echo -e "unset CDPATH"
unset CDPATH

echo -e "export OLDPWD=/tmp"
export OLDPWD=/tmp
run_test "cd" "cd -"

echo -e "unset OLDPWD"
unset OLDPWD
run_test "cd" "cd -"

# 2. Environment Starvation (No HOME)
echo -e "OLD_HOME=$HOME"
OLD_HOME=$HOME
echo -e "unset HOME"
unset HOME
run_test "cd" "cd"
echo -e "export HOME=$OLD_HOME"
export HOME=$OLD_HOME

run_test "env" "env -i ls"

# 3. Exit Code Propagation & Signal Death
run_test "env" "env sh -c 'exit 42'"
run_test "env" "env sh -c 'kill -9 \$$'"

# 4. Multi-line & Extreme Exporting
run_test "export" "export MULTILINE=$'Line1\nLine2\tTabbed'"
run_test "export" "export -p"

# 5. The "Too Many Arguments" Trap
run_test "cd" "cd /tmp /var /usr"

# Cleanup
rm -rf /tmp/mini_cdpath_test

# Cleanup CDPATH test directory
rm -rf /tmp/mini_cdpath_test

if [ $test_passed -eq $test_count ]; then
   	echo -e "${GREEN}Logic: SUCCESS! ($test_passed / $test_count)${RESET}"
else
   	echo -e "${RED}Logic: FAILED ($test_passed / $test_count)${RESET}"
fi

if [ $valgrind_passed -eq $test_count ]; then
  echo -e "${CYAN}Memory: ALL CLEAN! ($valgrind_passed / $test_count)${RESET}"
else
  echo -e "${MAGENTA}Memory: LEAKS DETECTED ($valgrind_passed / $test_count tests passed Valgrind)${RESET}"
fi

# Cleanup
rm -f "$BASH_OUT" "$BASH_ERR" "$MINI_OUT" "$MINI_ERR" "$VALGRIND_LOG" diff.log
