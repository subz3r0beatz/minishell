#!/bin/bash

# --- Colors for Output ---
GREEN="\033[32m"
RED="\033[31m"
YELLOW="\033[33m"
RESET="\033[0m"

MINISHELL="./minishell"

# --- Temp Files ---
BASH_OUT="bash_out.tmp"
BASH_ERR="bash_err.tmp"
MINI_OUT="mini_out.tmp"
MINI_ERR="mini_err.tmp"

test_count=0
test_passed=0

# Ensure minishell is compiled
if [ ! -f "$MINISHELL" ]; then
    echo -e "${RED}Error: $MINISHELL not found. Compile your project first.${RESET}"
    exit 1
fi

# --- Helper to clean and normalize environment outputs ---
# Prevents false negatives from Bash-specific variables (like BASH_VERSION, _)
normalize_env() {
    local file=$1
    local is_null_terminated=$2

    # If the output is null-terminated (env -0), convert to newlines for sorting
    if [ "$is_null_terminated" = true ]; then
        tr '\0' '\n' < "$file" > "$file.tmp"
        mv "$file.tmp" "$file"
    fi

    # Strip variables that natively differ between bash and custom shells, then sort
    grep -vE '^(declare -x )?(BASH|SHLVL|PWD|OLDPWD|_|LS_COLORS|XDG)' "$file" | sort > "$file.norm"
    mv "$file.norm" "$file"
}

# --- Core Testing Function ---
run_test() {
    local builtin=$1
    local cmd_string=$2
    local is_env_test=false
    local is_null_test=false

    # Flag for specialized diffing
    if [[ "$builtin" == "env" || "$builtin" == "export" ]]; then is_env_test=true; fi
    if [[ "$cmd_string" == *"-0"* || "$cmd_string" == *"--null"* ]]; then is_null_test=true; fi

    ((test_count++))
    printf "Test %-40s " "[$cmd_string]"

    # 1. Run standard Bash (using bash -c to avoid killing the script on exit)
    bash -c "$cmd_string" > "$BASH_OUT" 2> "$BASH_ERR"
    local bash_status=$?

    # 2. Run Minishell
    # We evaluate the string into an array so quotes (like "hello\nworld") stay grouped
    eval "mini_args=($cmd_string)"
    
    # Syntax required by your main.c: ./minishell <builtin> <fd_out_file> <builtin> <args...>
    $MINISHELL "$builtin" "$MINI_OUT" "${mini_args[@]}" 2> "$MINI_ERR"
    local mini_status=$?

    # 3. Normalize outputs if it's an environment dump to avoid false diffs
    if [ "$is_env_test" = true ]; then
        normalize_env "$BASH_OUT" "$is_null_test"
        normalize_env "$MINI_OUT" "$is_null_test"
    fi

    # 4. Compare outputs
    diff -u "$BASH_OUT" "$MINI_OUT" > diff.log
    local out_diff=$?

    if [ $out_diff -eq 0 ] && [ $bash_status -eq $mini_status ]; then
        echo -e "${GREEN}PASS${RESET}"
        ((test_passed++))
    else
        echo -e "${RED}FAIL${RESET}"
        if [ $bash_status -ne $mini_status ]; then
            echo -e "  ${YELLOW}Exit Status Differs:${RESET} Bash returned $bash_status, Minishell returned $mini_status"
        fi
        if [ $out_diff -ne 0 ]; then
            echo -e "  ${YELLOW}Stdout Differs:${RESET}"
            cat diff.log | sed 's/^/    /'
        fi
        
        # Optionally show stderr differences if you want strict matching
        # echo "  Bash Stderr: $(cat $BASH_ERR)"
        # echo "  Mini Stderr: $(cat $MINI_ERR)"
    fi
}

echo "======================================"
echo "    Extensive Minishell Tester"
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
run_test "export" "export BAD-VAR=123" # Should test failure

# --- 5. UNSET Tests ---
echo -e "\n${YELLOW}Testing unset (-v)...${RESET}"
run_test "unset" "unset PATH"
run_test "unset" "unset -v PATH"
run_test "unset" "unset DOES_NOT_EXIST"
run_test "unset" "unset -v BAD-NAME" # Should test failure

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
if [ $test_passed -eq $test_count ]; then
    echo -e "${GREEN}SUCCESS! All $test_count tests passed.${RESET}"
else
    echo -e "${RED}FAILED: $test_passed / $test_count tests passed.${RESET}"
fi

# Cleanup
rm -f "$BASH_OUT" "$BASH_ERR" "$MINI_OUT" "$MINI_ERR" diff.log
