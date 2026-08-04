#!/usr/bin/env python3
"""
Minishell Universal Multithreaded Test Harness
Single-file self-contained testing suite.
"""

import os
import sys
import shutil
import tempfile
import atexit
import argparse
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed

# --- ANSI Formatting ---
C_RESET  = "\033[0m"
C_BOLD   = "\033[1m"
C_RED    = "\033[31m"
C_GREEN  = "\033[32m"
C_YELLOW = "\033[33m"
C_CYAN   = "\033[36m"
C_DIM    = "\033[2m"

# --- Embedded C Hook Source Code ---
MALLOC_HOOK_SRC = r"""
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdatomic.h>
#include <string.h>

static void *(*real_malloc)(size_t) = NULL;
static void *(*real_calloc)(size_t, size_t) = NULL;
static void *(*real_realloc)(void *, size_t) = NULL;

static atomic_long g_alloc_count = 0;
static long g_fail_index = -1;
static int g_in_init = 0;
static char g_dummy_buf[65536];
static size_t g_dummy_pos = 0;

static void init_hooks(void)
{
    if (real_malloc || g_in_init)
        return;
    g_in_init = 1;
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    real_calloc = dlsym(RTLD_NEXT, "calloc");
    real_realloc = dlsym(RTLD_NEXT, "realloc");

    char *env = getenv("FAIL_MALLOC_INDEX");
    if (env)
        g_fail_index = atol(env);
    g_in_init = 0;
}

__attribute__((destructor)) static void cleanup_hook(void)
{
    if (getenv("LOG_ALLOC_COUNT"))
    {
        fprintf(stderr, "\n__HOOK_TOTAL_ALLOCS:%ld__\n", (long)g_alloc_count);
    }
}

void *malloc(size_t size)
{
    if (!real_malloc)
    {
        init_hooks();
        if (!real_malloc)
        {
            size_t old = g_dummy_pos;
            g_dummy_pos += (size + 7) & ~7;
            return &g_dummy_buf[old];
        }
    }
    long idx = atomic_fetch_add(&g_alloc_count, 1) + 1;
    if (g_fail_index > 0 && idx == g_fail_index)
    {
        errno = ENOMEM;
        return NULL;
    }
    return real_malloc(size);
}

void *calloc(size_t nmemb, size_t size)
{
    if (!real_calloc)
    {
        init_hooks();
        if (!real_calloc)
        {
            size_t bytes = nmemb * size;
            void *ptr = malloc(bytes);
            if (ptr)
                memset(ptr, 0, bytes);
            return ptr;
        }
    }
    long idx = atomic_fetch_add(&g_alloc_count, 1) + 1;
    if (g_fail_index > 0 && idx == g_fail_index)
    {
        errno = ENOMEM;
        return NULL;
    }
    return real_calloc(nmemb, size);
}

void *realloc(void *ptr, size_t size)
{
    if (!real_realloc)
    {
        init_hooks();
        if (!real_realloc)
            return NULL;
    }
    long idx = atomic_fetch_add(&g_alloc_count, 1) + 1;
    if (g_fail_index > 0 && idx == g_fail_index)
    {
        errno = ENOMEM;
        return NULL;
    }
    return real_realloc(ptr, size);
}
"""

# --- Expanded Comprehensive Test Cases ---
TEST_SUITES = [
    # --- Grammar & Operators ---
    {"cat": "Operators", "cmd": "echo subshell | (cat)", "bash_cmp": True},
    {"cat": "Operators", "cmd": "(cd /tmp && pwd) && pwd", "bash_cmp": True},
    {"cat": "Operators", "cmd": "echo first; echo second; echo third", "bash_cmp": True},
    {"cat": "Operators", "cmd": "true && echo success || echo failure", "bash_cmp": True},
    {"cat": "Operators", "cmd": "false && echo no || echo yes", "bash_cmp": True},
    {"cat": "Operators", "cmd": "false || echo recovered", "bash_cmp": True},
    {"cat": "Operators", "cmd": "(echo 1 && (echo 2 || echo 3)) | cat", "bash_cmp": True},
    {"cat": "Operators", "cmd": "echo bg_task &", "bash_cmp": False},

    # --- Expansions & Special Parameters ---
    {"cat": "Expansions", "cmd": "echo $USER", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo \"$USER\"", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo '$USER'", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo exit_code: $?", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo pid: $$", "bash_cmp": False}, # PID differs between shells
    {"cat": "Expansions", "cmd": "echo last_pid: $!", "bash_cmp": False},
    {"cat": "Expansions", "cmd": "echo argv0: $0", "bash_cmp": False},
    {"cat": "Expansions", "cmd": "echo ~", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "export FOO=bar && echo $FOO_BAZ", "bash_cmp": True},

    # --- Redirections ---
    {"cat": "Redirections", "cmd": "echo hello > /tmp/ms_test_out.txt && cat /tmp/ms_test_out.txt", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "echo append >> /tmp/ms_test_out.txt && cat /tmp/ms_test_out.txt", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "cat < /tmp/ms_test_out.txt", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "cat << EOF\nheredoc_line_1\nheredoc_line_2\nEOF", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "ls > /tmp/ms_test_out.txt | wc -l", "bash_cmp": True},

    # --- Builtin: echo ---
    {"cat": "Builtin: echo", "cmd": "echo hello world", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -n no_newline", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e 'line1\\nline2'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -E 'line1\\nline2'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -ne 'a\\tb'", "bash_cmp": True},

    # --- Builtin: pwd ---
    {"cat": "Builtin: pwd", "cmd": "pwd", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "pwd -L", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "pwd -P", "bash_cmp": True},

    # --- Builtin: cd ---
    {"cat": "Builtin: cd", "cmd": "cd /tmp && pwd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd -L /tmp && pwd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd -P /tmp && pwd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd -P -e /tmp && pwd", "bash_cmp": False}, # Custom flags
    {"cat": "Builtin: cd", "cmd": "cd .. && pwd", "bash_cmp": True},

    # --- Builtin: exit ---
    {"cat": "Builtin: exit", "cmd": "exit 0", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 42", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 255", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit -5", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 1 2 3", "bash_cmp": False}, # Usage error format differs
    {"cat": "Builtin: exit", "cmd": "exit invalid", "bash_cmp": False},

    # --- Builtin: export ---
    {"cat": "Builtin: export", "cmd": "export", "bash_cmp": False},
    {"cat": "Builtin: export", "cmd": "export -p", "bash_cmp": False},
    {"cat": "Builtin: export", "cmd": "export VAR1=val1 && echo $VAR1", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR1+=_append && echo $VAR1", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export -n VAR1 && echo $VAR1", "bash_cmp": False}, # Custom flag

    # --- Builtin: unset ---
    {"cat": "Builtin: unset", "cmd": "export VAR2=val2 && unset VAR2 && echo $VAR2", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "export VAR2=val2 && unset -v VAR2 && echo $VAR2", "bash_cmp": False}, # Custom flag

    # --- Builtin: env ---
    {"cat": "Builtin: env", "cmd": "env", "bash_cmp": False},
    {"cat": "Builtin: env", "cmd": "env -i", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -0", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env --null", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -u PATH", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env --unset=PATH", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -C /tmp pwd", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env --chdir=/tmp pwd", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -a custom_name sh -c 'echo $0'", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env --argv0=custom_name sh -c 'echo $0'", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -S \"echo split_test\"", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env --split-string=\"echo split_test\"", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env --help", "bash_cmp": False},
]

class EnvironmentManager:
    def __init__(self, ms_path="./minishell"):
        self.ms_path = os.path.abspath(ms_path)
        self.temp_dir = tempfile.mkdtemp(prefix="ms_test_")
        self.hook_so = os.path.join(self.temp_dir, "libmalloc_hook.so")
        atexit.register(self.cleanup)

    def cleanup(self):
        if os.path.exists(self.temp_dir):
            shutil.rmtree(self.temp_dir)

    def build_hook(self):
        c_file = os.path.join(self.temp_dir, "malloc_hook.c")
        with open(c_file, "w") as f:
            f.write(MALLOC_HOOK_SRC)
        
        compiler = shutil.which("gcc") or shutil.which("clang")
        if not compiler:
            raise RuntimeError("Neither gcc nor clang compiler found.")
        
        cmd = [compiler, "-shared", "-fPIC", "-O2", c_file, "-o", self.hook_so, "-ldl"]
        res = subprocess.run(cmd, capture_output=True, text=True)
        if res.returncode != 0:
            raise RuntimeError(f"Failed to compile allocation hook:\n{res.stderr}")

def run_shell(cmd_str, executable, env=None, timeout=5):
    if env is None:
        env = os.environ.copy()
    try:
        proc = subprocess.run(
            [executable],
            input=cmd_str + "\n",
            capture_output=True,
            text=True,
            env=env,
            timeout=timeout
        )
        return proc.stdout, proc.stderr, proc.returncode
    except subprocess.TimeoutExpired:
        return "", "TIMEOUT", -1

def run_bash(cmd_str):
    try:
        proc = subprocess.run(
            ["bash", "--posix"],
            input=cmd_str + "\n",
            capture_output=True,
            text=True,
            timeout=5
        )
        return proc.stdout, proc.stderr, proc.returncode
    except subprocess.TimeoutExpired:
        return "", "TIMEOUT", -1

def test_bash_comparison(test_item, ms_path):
    cmd_str = test_item["cmd"]
    ms_out, _, ms_code = run_shell(cmd_str, ms_path)
    bash_out, _, bash_code = run_bash(cmd_str)

    stdout_match = (ms_out.strip() == bash_out.strip())
    code_match = (ms_code == bash_code)

    if stdout_match and code_match:
        return True, ""

    logs = []
    if not stdout_match:
        logs.append(f"STDOUT Mismatch:\n  Minishell: {repr(ms_out)}\n  Bash:      {repr(bash_out)}")
    if not code_match:
        logs.append(f"Exit Code Mismatch:\n  Minishell: {ms_code}\n  Bash:      {bash_code}")
    return False, "\n".join(logs)

def test_valgrind_and_fds(cmd_str, ms_path):
    valgrind_cmd = [
        "valgrind",
        "--leak-check=full",
        "--show-leak-kinds=all",
        "--errors-for-leak-kinds=all",
        "--track-fds=yes",
        "--error-exitcode=99",
        ms_path
    ]
    try:
        proc = subprocess.run(
            valgrind_cmd,
            input=cmd_str + "\n",
            capture_output=True,
            text=True,
            timeout=10
        )
        if proc.returncode == 99:
            return False, f"Memory leak or invalid access detected:\n{proc.stderr}"
        if "FILE DESCRIPTORS: 4 open" in proc.stderr or "FILE DESCRIPTORS: 5 open" in proc.stderr:
            if "Open file descriptor" in proc.stderr and not "inherited from parent" in proc.stderr:
                return False, f"File Descriptor leak detected:\n{proc.stderr}"
        return True, ""
    except Exception as e:
        return False, f"Valgrind error: {str(e)}"

def test_malloc_injections(cmd_str, ms_path, hook_so_path):
    env = os.environ.copy()
    env["LD_PRELOAD"] = hook_so_path
    env["LOG_ALLOC_COUNT"] = "1"

    _, ms_err, _ = run_shell(cmd_str, ms_path, env=env)
    
    # Parse total allocations count
    total_allocs = 0
    for line in ms_err.splitlines():
        if "__HOOK_TOTAL_ALLOCS:" in line:
            try:
                total_allocs = int(line.split(":")[1].rstrip("_"))
            except ValueError:
                pass
            break

    if total_allocs == 0:
        return True, ""

    # Test sequential failures up to total_allocs
    for fail_idx in range(1, total_allocs + 1):
        fail_env = os.environ.copy()
        fail_env["LD_PRELOAD"] = hook_so_path
        fail_env["FAIL_MALLOC_INDEX"] = str(fail_idx)

        out, err, code = run_shell(cmd_str, ms_path, env=fail_env)

        # Check for crash (signals like SIGSEGV=139, SIGABRT=134)
        if code < 0 or code in (134, 139):
            return False, f"Crash/Segfault on Malloc #{fail_idx}/{total_allocs} (Code: {code})\nStderr: {err}"

        # Verify an error message was written to stderr on failure
        if not err.strip():
            return False, f"No error message printed on STDERR for Malloc failure #{fail_idx}/{total_allocs}"

    return True, ""

def process_test(test_item, env_mgr, opts):
    cmd_str = test_item["cmd"]
    res = {"item": test_item, "passed": True, "logs": []}

    # 1. Output & Exit Status Comparison against Bash
    if test_item.get("bash_cmp", True) and not opts.skip_bash:
        ok, log = test_bash_comparison(test_item, env_mgr.ms_path)
        if not ok:
            res["passed"] = False
            res["logs"].append(f"[{C_RED}BASH COMPARISON FAILED{C_RESET}]\n{log}")

    # 2. Valgrind & File Descriptor Leak Check
    if not opts.skip_valgrind and shutil.which("valgrind"):
        ok, log = test_valgrind_and_fds(cmd_str, env_mgr.ms_path)
        if not ok:
            res["passed"] = False
            res["logs"].append(f"[{C_RED}VALGRIND / FD LEAK FAILED{C_RESET}]\n{log}")

    # 3. Dynamic Malloc Failure Interposition
    if not opts.skip_malloc:
        ok, log = test_malloc_injections(cmd_str, env_mgr.ms_path, env_mgr.hook_so)
        if not ok:
            res["passed"] = False
            res["logs"].append(f"[{C_RED}MALLOC FAULT TEST FAILED{C_RESET}]\n{log}")

    return res

def main():
    parser = argparse.ArgumentParser(description="Minishell Test Harness")
    parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 4, help="Parallel threads")
    parser.add_argument("--ms", type=str, default="./minishell", help="Minishell binary path")
    parser.add_argument("--skip-valgrind", action="store_true", help="Skip valgrind tests")
    parser.add_argument("--skip-malloc", action="store_true", help="Skip malloc fault tests")
    parser.add_argument("--skip-bash", action="store_true", help="Skip bash output comparison")
    parser.add_argument("--filter", type=str, default="", help="Filter commands by substring")
    opts = parser.parse_args()

    if not os.path.exists(opts.ms):
        print(f"{C_RED}Error: Executable '{opts.ms}' not found. Build your minishell first!{C_RESET}")
        sys.exit(1)

    print(f"{C_CYAN}{C_BOLD}Setting up test harness environment...{C_RESET}")
    env_mgr = EnvironmentManager(ms_path=opts.ms)
    env_mgr.build_hook()

    tests = [t for t in TEST_SUITES if opts.filter.lower() in t["cmd"].lower()]
    print(f"{C_CYAN}{C_BOLD}Running {len(tests)} tests using {opts.jobs} parallel workers...{C_RESET}\n")

    passed_count = 0
    total_count = len(tests)

    with ThreadPoolExecutor(max_workers=opts.jobs) as executor:
        futures = {executor.submit(process_test, t, env_mgr, opts): t for t in tests}
        for future in as_completed(futures):
            res = future.result()
            t = res["item"]
            category = t["cat"]
            cmd = t["cmd"].replace("\n", "\\n")

            if res["passed"]:
                passed_count += 1
                print(f"[{C_GREEN}PASS{C_RESET}] [{C_DIM}{category}{C_RESET}] {C_BOLD}{cmd}{C_RESET}")
            else:
                print(f"[{C_RED}FAIL{C_RESET}] [{C_DIM}{category}{C_RESET}] {C_BOLD}{cmd}{C_RESET}")
                for log in res["logs"]:
                    print(f"  └─ {log}")
                print("-" * 65)

    print(f"\n{C_CYAN}{C_BOLD}=== TEST SUMMARY ==={C_RESET}")
    if passed_count == total_count:
        print(f"{C_GREEN}{C_BOLD}ALL {total_count} TESTS PASSED PERFECTLY! 🎉{C_RESET}")
    else:
        print(f"{C_RED}{C_BOLD}Passed: {passed_count}/{total_count} tests.{C_RESET}")
        sys.exit(1)

if __name__ == "__main__":
    main()
