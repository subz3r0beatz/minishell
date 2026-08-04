#!/usr/bin/env python3
"""
Minishell Universal Modern Dark GUI Test Harness
Single-file self-contained Tkinter GUI test harness with stack backtrace symbol
resolution for silent malloc failures, readline valgrind suppressions,
recompile/reset controls, file-isolated execution, and dark UI theme.
"""

import os
import sys
import shutil
import tempfile
import atexit
import argparse
import difflib
import queue
import threading
import subprocess
import re
from concurrent.futures import ThreadPoolExecutor, as_completed

import tkinter as tk
from tkinter import ttk, filedialog, messagebox

# --- Color Palette Constants (Catppuccin Mocha Dark Theme) ---
COLOR_BG_DARK       = "#1e1e2e"  # Window background
COLOR_BG_PANEL      = "#252538"  # Containers and cards
COLOR_BG_INPUT      = "#181825"  # Textboxes, logs, treeview
COLOR_BORDER        = "#313244"  # Subtle borders
COLOR_FG_TEXT       = "#cdd6f4"  # Main text
COLOR_FG_MUTED      = "#a6adc8"  # Secondary text

COLOR_ACCENT        = "#89b4fa"  # Primary accent blue
COLOR_ACCENT_HOVER  = "#b4befe"  # Hover state
COLOR_PASS          = "#a6e3a1"  # Soft pastel green
COLOR_FAIL          = "#f38ba8"  # Soft pastel red
COLOR_WARN          = "#f9e2af"  # Soft pastel yellow

COLOR_DIFF_ADD_BG   = "#283b2a"  # Green diff line background
COLOR_DIFF_ADD_FG   = "#a6e3a1"
COLOR_DIFF_SUB_BG   = "#3f2229"  # Red diff line background
COLOR_DIFF_SUB_FG   = "#f38ba8"

# --- Embedded Readline Suppression File Content ---
READLINE_SUPP_CONTENT = r"""
{
   ignore_readline_leaks
   Memcheck:Leak
   ...
   obj:*/libreadline.so*
}
{
   ignore_readline_history_leaks
   Memcheck:Leak
   ...
   fun:add_history
}
"""

# --- Embedded C Hook Source Code ---
MALLOC_HOOK_SRC = r"""
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdatomic.h>
#include <string.h>
#include <execinfo.h>
#include <unistd.h>

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

static int is_minishell_caller(void *caller)
{
    Dl_info info;
    if (dladdr(caller, &info) && info.dli_fname)
    {
        if (strstr(info.dli_fname, "minishell"))
        {
            if (info.dli_sname)
            {
                if (strstr(info.dli_sname, "tsearch") ||
                    strstr(info.dli_sname, "environ") ||
                    strstr(info.dli_sname, "readline") ||
                    strstr(info.dli_sname, "rl_"))
                    return 0;
            }
            return 1;
        }
    }
    return 0;
}

static void log_callstack(void)
{
    void *frames[32];
    int size = backtrace(frames, 32);
    const char *hdr = "\n__HOOK_MALLOC_FAIL_STACK__\n";
    write(STDERR_FILENO, hdr, strlen(hdr));
    backtrace_symbols_fd(frames, size, STDERR_FILENO);
    const char *ftr = "__HOOK_STACK_END__\n";
    write(STDERR_FILENO, ftr, strlen(ftr));
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
    if (is_minishell_caller(__builtin_return_address(0)))
    {
        long idx = atomic_fetch_add(&g_alloc_count, 1) + 1;
        if (g_fail_index > 0 && idx == g_fail_index)
        {
            log_callstack();
            errno = ENOMEM;
            return NULL;
        }
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
    if (is_minishell_caller(__builtin_return_address(0)))
    {
        long idx = atomic_fetch_add(&g_alloc_count, 1) + 1;
        if (g_fail_index > 0 && idx == g_fail_index)
        {
            log_callstack();
            errno = ENOMEM;
            return NULL;
        }
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
    if (is_minishell_caller(__builtin_return_address(0)))
    {
        long idx = atomic_fetch_add(&g_alloc_count, 1) + 1;
        if (g_fail_index > 0 && idx == g_fail_index)
        {
            log_callstack();
            errno = ENOMEM;
            return NULL;
        }
    }
    return real_realloc(ptr, size);
}
"""

# --- Full Predefined Test Suite (All 19 Sections) ---
DEFAULT_TESTS = [
    # --- 1. Builtin: cd ---
    {"cat": "Builtin: cd", "cmd": "cd .", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd ..", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd /tmp", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd -L /tmp", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd -P /tmp", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd -e /tmp", "bash_cmp": False},
    {"cat": "Builtin: cd", "cmd": "cd /does_not_exist", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd /tmp/../tmp/../tmp", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd -", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd ~", "bash_cmp": True},

    # --- 2. Builtin: pwd ---
    {"cat": "Builtin: pwd", "cmd": "pwd", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "pwd -L", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "pwd -P", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "pwd -LLLLPPPPLLLLPPPP", "bash_cmp": True},

    # --- 3. Builtin: echo ---
    {"cat": "Builtin: echo", "cmd": "echo", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo hello world", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -n hello world", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -nnnn hello", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e 'hello\\nworld\\t!'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -E 'hello\\nworld\\t!'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -ne 'test\\n'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e '\\x41\\x42\\x43'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e 'Before\\cAfter'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e '\\\\\\\\'", "bash_cmp": True},

    # --- 4. Builtin: export ---
    {"cat": "Builtin: export", "cmd": "export", "bash_cmp": False},
    {"cat": "Builtin: export", "cmd": "export -p", "bash_cmp": False},
    {"cat": "Builtin: export", "cmd": "export VAR_TEST=123", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR_TEST+=456", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export BAD-VAR=123", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export _VALID=1 2INVALID=2 ALSO_VALID=3", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export NULL_VAR EMPTY_VAR=", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export WEIRD_VAR=\"hello=world=test=123\"", "bash_cmp": True},

    # --- 5. Builtin: unset ---
    {"cat": "Builtin: unset", "cmd": "unset PATH", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset DOES_NOT_EXIST", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset BAD-NAME", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset -v PATH", "bash_cmp": False},

    # --- 6. Builtin: env ---
    {"cat": "Builtin: env", "cmd": "env", "bash_cmp": False},
    {"cat": "Builtin: env", "cmd": "env -i", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -0", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -u PATH", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -C /tmp pwd", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -a ARGV0 echo hello", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -S 'echo hello split string'", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env --ignore-environment", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env --null", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env --chdir=/tmp pwd", "bash_cmp": True},

    # --- 7. Builtin: exit ---
    {"cat": "Builtin: exit", "cmd": "exit 0", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 42", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit -42", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 9223372036854775807", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 9223372036854775808", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 42 42", "bash_cmp": False},
    {"cat": "Builtin: exit", "cmd": "exit hello", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 42hello", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit -- -42", "bash_cmp": True},

    # --- 8. Persistent State ---
    {"cat": "Persistent State", "cmd": "export A=10\nexport B=20\necho \"A=$A B=$B\"\nunset A\necho \"A=$A B=$B\"", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "cd /tmp\npwd\ncd ..\npwd", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "export VAR=hello\nexport VAR+=_world\necho $VAR", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "export X=1\n(export X=2; echo \"subshell X=$X\")\necho \"parent X=$X\"", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "cd /tmp\n(cd /var; echo \"subshell pwd=\"; pwd)\necho \"parent pwd=\"; pwd", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "export FOO=bar\nenv | grep FOO\nunset FOO\nenv | grep FOO", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "ls /does_not_exist\necho \"Status 1: $?\"\nls -d /tmp\necho \"Status 2: $?\"", "bash_cmp": True},

    # --- 9. Expansions ---
    {"cat": "Expansions", "cmd": "echo $USER", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "ls /does_not_exist; echo $?", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo $0", "bash_cmp": False},
    {"cat": "Expansions", "cmd": "echo $$", "bash_cmp": False},
    {"cat": "Expansions", "cmd": "echo $!", "bash_cmp": False},
    {"cat": "Expansions", "cmd": "echo ~", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo ~/", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo '$USER' \"$USER\" $USER", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "export FOO=bar; echo $FOO", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo $NOSUCHVARIABLE_XYZ_123", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo $1 $2 $99", "bash_cmp": True},

    # --- 10. Quotes & Parsing ---
    {"cat": "Quotes & Parsing", "cmd": "echo '' '' '   ' ''", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"\" \"   \" \"\"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo   a    b      c  ", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo 'a   b   c'", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"a   b   c\"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"'hello'\"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo '\"hello\"'", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"$USER's laptop\"", "bash_cmp": True},

    # --- 11. Redirections ---
    {"cat": "Redirections", "cmd": "echo hello > /tmp/ms_test_r1.txt; cat /tmp/ms_test_r1.txt", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "echo line1 > /tmp/ms_test_r2.txt; echo line2 >> /tmp/ms_test_r2.txt; cat /tmp/ms_test_r2.txt", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "cat < /etc/hostname", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "> /tmp/ms_empty.txt; ls -l /tmp/ms_empty.txt", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "cat < /tmp/file_does_not_exist_xyz", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "echo first > /tmp/m1.txt > /tmp/m2.txt; cat /tmp/m1.txt; echo \"---\"; cat /tmp/m2.txt", "bash_cmp": True},

    # --- 12. Pipes ---
    {"cat": "Pipes", "cmd": "echo hello | cat", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "cat /etc/hostname | grep -o a | wc -l", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "ls -la | grep srcs | wc -l", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "export TEST_PIPE=42 | echo hello; echo $TEST_PIPE", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "ls /does_not_exist | wc -l", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "echo hello | cat | cat | cat | grep h", "bash_cmp": True},

    # --- 13. Logic Operators ---
    {"cat": "Logic Operators", "cmd": "true && echo yes", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "false || echo no", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "false && echo no", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "true || echo no", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "echo 1 && echo 2 || echo 3", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "ls /does_not_exist && echo success || echo failed", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "false || false || echo third_time_charm", "bash_cmp": True},

    # --- 14. Subshells ---
    {"cat": "Subshells", "cmd": "(echo inside subshell)", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(export SUB_VAR=sub); echo $SUB_VAR", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "((echo nested))", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(echo hello) > /tmp/ms_sub_out.txt; cat /tmp/ms_sub_out.txt", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(cd /tmp && pwd); pwd", "bash_cmp": True},

    # --- 15. Control & Semicolons ---
    {"cat": "Control & Semicolons", "cmd": "echo 1; echo 2; echo 3", "bash_cmp": True},
    {"cat": "Control & Semicolons", "cmd": "pwd; cd /tmp; pwd", "bash_cmp": True},
    {"cat": "Control & Semicolons", "cmd": ";;", "bash_cmp": True},
    {"cat": "Control & Semicolons", "cmd": "echo 1; ; echo 2", "bash_cmp": True},

    # --- 16. Rug Pull (Deleted Dir) ---
    {"cat": "Rug Pull", "cmd": "mkdir -p /tmp/ms_rugpull && cd /tmp/ms_rugpull && rm -rf /tmp/ms_rugpull && pwd", "bash_cmp": True},
    {"cat": "Rug Pull", "cmd": "mkdir -p /tmp/ms_rugpull && cd /tmp/ms_rugpull && rm -rf /tmp/ms_rugpull && cd .", "bash_cmp": True},
    {"cat": "Rug Pull", "cmd": "mkdir -p /tmp/ms_rugpull && cd /tmp/ms_rugpull && rm -rf /tmp/ms_rugpull && cd ..", "bash_cmp": True},

    # --- 17. Path Resolution & Exec ---
    {"cat": "Path & Exec", "cmd": "env /tmp", "bash_cmp": True},
    {"cat": "Path & Exec", "cmd": "/does_not_exist_mini_bin", "bash_cmp": True},
    {"cat": "Path & Exec", "cmd": "''", "bash_cmp": True},
    {"cat": "Path & Exec", "cmd": ".", "bash_cmp": True},
    {"cat": "Path & Exec", "cmd": "..", "bash_cmp": True},

    # --- 18. State Corruption ---
    {"cat": "State Corruption", "cmd": "export PWD=/completely/fake/path; pwd", "bash_cmp": True},
    {"cat": "State Corruption", "cmd": "export PWD=/completely/fake/path; pwd -L", "bash_cmp": True},
    {"cat": "State Corruption", "cmd": "export PWD=/completely/fake/path; pwd -P", "bash_cmp": True},
    {"cat": "State Corruption", "cmd": "unset OLDPWD; cd -", "bash_cmp": True},

    # --- 19. Flag Parsing (Errors) ---
    {"cat": "Flag Errors", "cmd": "cd -Z /tmp", "flag_error": True},
    {"cat": "Flag Errors", "cmd": "pwd -Z", "flag_error": True},
    {"cat": "Flag Errors", "cmd": "export -Z", "flag_error": True},
    {"cat": "Flag Errors", "cmd": "unset -Z", "flag_error": True},
    {"cat": "Flag Errors", "cmd": "env -Z", "flag_error": True},
    {"cat": "Flag Errors", "cmd": "exit -Z", "flag_error": True},
]

class EnvironmentManager:
    def __init__(self, ms_path="./minishell"):
        self.ms_path = os.path.abspath(ms_path)
        self.temp_dir = tempfile.mkdtemp(prefix="ms_gui_test_")
        self.hook_so = os.path.join(self.temp_dir, "libmalloc_hook.so")
        self.supp_file = os.path.join(self.temp_dir, "readline.supp")
        atexit.register(self.cleanup)

    def cleanup(self):
        if os.path.exists(self.temp_dir):
            shutil.rmtree(self.temp_dir)

    def build_hook(self):
        with open(self.supp_file, "w") as f:
            f.write(READLINE_SUPP_CONTENT)

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

def run_shell(cmd_str, executable, env=None, cwd=None, timeout=5):
    executable = os.path.abspath(executable)
    if env is None:
        env = os.environ.copy()
    try:
        proc = subprocess.run(
            [executable],
            input=cmd_str + "\n",
            capture_output=True,
            text=True,
            env=env,
            cwd=cwd,
            timeout=timeout
        )
        return proc.stdout, proc.stderr, proc.returncode
    except subprocess.TimeoutExpired:
        return "", "TIMEOUT", -1

def run_bash(cmd_str, cwd=None):
    try:
        proc = subprocess.run(
            ["bash", "--posix"],
            input=cmd_str + "\n",
            capture_output=True,
            text=True,
            cwd=cwd,
            timeout=5
        )
        return proc.stdout, proc.stderr, proc.returncode
    except subprocess.TimeoutExpired:
        return "", "TIMEOUT", -1

def normalize_env_output(output_str):
    """
    Sorts lines and filters out unstable OS/shell dynamic variables (PWD, SHLVL, _, etc.)
    """
    lines = output_str.splitlines()
    filtered = []
    ignore_prefixes = (
        "declare -x BASH", "declare -x SHLVL", "declare -x PWD", "declare -x OLDPWD",
        "declare -x _", "declare -x LS_COLORS", "declare -x XDG", "declare -x GLIBC",
        "declare -x LD_", "declare -x FAIL_MALLOC_", "declare -x TRACE_MALLOC",
        "BASH=", "SHLVL=", "PWD=", "OLDPWD=", "_=", "LS_COLORS=", "XDG=", "GLIBC=",
        "LD_=", "LD_PRELOAD=", "FAIL_MALLOC_", "TRACE_MALLOC="
    )
    for line in lines:
        if not any(line.startswith(p) for p in ignore_prefixes):
            filtered.append(line)
    return "\n".join(sorted(filtered))

def strip_hook_output(raw_stderr):
    clean = re.sub(r'__HOOK_MALLOC_FAIL_STACK__[\s\S]*?__HOOK_STACK_END__', '', raw_stderr)
    clean = re.sub(r'__HOOK_TOTAL_ALLOCS:\d+__', '', clean)
    return clean.strip()

def resolve_stack_trace(ms_path, raw_stderr):
    if "__HOOK_MALLOC_FAIL_STACK__" not in raw_stderr:
        return ""

    try:
        stack_chunk = raw_stderr.split("__HOOK_MALLOC_FAIL_STACK__")[1].split("__HOOK_STACK_END__")[0]
    except IndexError:
        return ""

    addresses = []
    raw_frames = []

    for line in stack_chunk.splitlines():
        line_str = line.strip()
        if not line_str or "libmalloc_hook.so" in line_str or "libc.so" in line_str:
            continue

        raw_frames.append(f"    ├─ {line_str}")

        # Always prefer relative offsets (+0xHEX) over randomized PIE addresses [0xHEX]
        match_off = re.search(r'\(\+(0x[0-9a-fA-F]+|[0-9a-fA-F]+)\)', line_str)
        if match_off:
            addresses.append(match_off.group(1))

    resolved_lines = []
    if shutil.which("addr2line") and addresses:
        cmd = ["addr2line", "-e", os.path.abspath(ms_path), "-f", "-C"] + addresses
        try:
            proc = subprocess.run(cmd, capture_output=True, text=True, timeout=3)
            lines = proc.stdout.splitlines()

            for i in range(0, len(lines) - 1, 2):
                func = lines[i].strip()
                loc = lines[i+1].strip()

                if loc not in ("?:0", "??:0", "??:?") and "malloc_hook" not in loc:
                    resolved_lines.append(f"    ├─ {loc} in {func}()")
        except Exception:
            pass

    if resolved_lines:
        return "  Callstack Location:\n" + "\n".join(resolved_lines)
    elif raw_frames:
        return "  Raw Callstack Frames:\n" + "\n".join(raw_frames)

    return ""

def execute_single_test(test_item, ms_path, hook_so_path, supp_file_path, opts):
    ms_path = os.path.abspath(ms_path)
    test_tmp_dir = tempfile.mkdtemp(prefix=f"ms_test_run_{test_item['id']}_")

    try:
        # Rewrite any /tmp/ms_ paths in command to be completely unique per test ID
        cmd_str = re.sub(r'/tmp/ms_', f'/tmp/ms_t{test_item["id"]}_', test_item["cmd"])

        result = {
            "id": test_item["id"],
            "cmd": cmd_str,
            "cat": test_item["cat"],
            "passed": True,
            "ms_out": "",
            "ms_err": "",
            "ms_code": 0,
            "bash_out": "",
            "bash_err": "",
            "bash_code": 0,
            "diff_text": "",
            "valgrind_log": "Not Run",
            "malloc_log": "Not Run",
            "failures": []
        }

        # Execute Minishell and Bash inside isolated test_tmp_dir
        ms_out, ms_err, ms_code = run_shell(cmd_str, ms_path, cwd=test_tmp_dir)
        bash_out, bash_err, bash_code = run_bash(cmd_str, cwd=test_tmp_dir)

        result["ms_out"] = ms_out
        result["ms_err"] = ms_err
        result["ms_code"] = ms_code
        result["bash_out"] = bash_out
        result["bash_err"] = bash_err
        result["bash_code"] = bash_code

        # 1. Output & Code Comparison / Flag Error Verification
        if test_item.get("flag_error", False):
            clean_err = strip_hook_output(ms_err)
            if not clean_err or ms_code == 0:
                result["passed"] = False
                result["failures"].append("Flag Option Check: Expected non-zero exit code and error message on STDERR.")
        elif test_item.get("bash_cmp", True) and not opts.get("skip_bash", False):
            if "env" in cmd_str or "export" in cmd_str:
                norm_ms = normalize_env_output(ms_out)
                norm_bash = normalize_env_output(bash_out)
                out_match = (norm_ms == norm_bash)
            else:
                out_match = (ms_out.strip() == bash_out.strip())

            code_match = (ms_code == bash_code)

            diff = difflib.unified_diff(
                bash_out.splitlines(keepends=True),
                ms_out.splitlines(keepends=True),
                fromfile="bash stdout",
                tofile="minishell stdout"
            )
            result["diff_text"] = "".join(diff)

            if not out_match or not code_match:
                result["passed"] = False
                err_reasons = []
                if not out_match:
                    err_reasons.append("STDOUT mismatch")
                if not code_match:
                    err_reasons.append(f"Exit status mismatch (minishell={ms_code}, bash={bash_code})")
                result["failures"].append("Bash Comparison: " + ", ".join(err_reasons))

        # 2. Valgrind & FD Leak Checks with Readline Suppression
        if not opts.get("skip_valgrind", False) and shutil.which("valgrind"):
            valgrind_cmd = [
                "valgrind",
                f"--suppressions={supp_file_path}",
                "--leak-check=full",
                "--show-leak-kinds=all",
                "--errors-for-leak-kinds=all",
                "--track-fds=yes",
                "--error-exitcode=99",
                ms_path
            ]
            try:
                proc = subprocess.run(valgrind_cmd, input=cmd_str + "\n", capture_output=True, text=True, cwd=test_tmp_dir, timeout=10)
                result["valgrind_log"] = proc.stderr
                if proc.returncode == 99:
                    result["passed"] = False
                    result["failures"].append("Valgrind: Memory leak or invalid memory access detected")
                if "FILE DESCRIPTORS: 4 open" in proc.stderr or "FILE DESCRIPTORS: 5 open" in proc.stderr:
                    if "Open file descriptor" in proc.stderr and "inherited from parent" not in proc.stderr:
                        result["passed"] = False
                        result["failures"].append("Valgrind: File Descriptor leak detected")
            except Exception as e:
                result["valgrind_log"] = f"Execution error: {str(e)}"

        # 3. Dynamic Malloc Interposition
        if not opts.get("skip_malloc", False):
            env = os.environ.copy()
            env["LD_PRELOAD"] = hook_so_path
            env["LOG_ALLOC_COUNT"] = "1"

            _, ms_err_log, _ = run_shell(cmd_str, ms_path, env=env, cwd=test_tmp_dir)
            
            total_allocs = 0
            for line in ms_err_log.splitlines():
                if "__HOOK_TOTAL_ALLOCS:" in line:
                    try:
                        total_allocs = int(line.split(":")[1].rstrip("_"))
                    except ValueError:
                        pass

            if total_allocs > 0:
                malloc_fail_logs = []
                for fail_idx in range(1, total_allocs + 1):
                    fail_env = os.environ.copy()
                    fail_env["LD_PRELOAD"] = hook_so_path
                    fail_env["FAIL_MALLOC_INDEX"] = str(fail_idx)

                    _, err_m, code_m = run_shell(cmd_str, ms_path, env=fail_env, cwd=test_tmp_dir)
                    
                    program_err = strip_hook_output(err_m)
                    callstack_loc = resolve_stack_trace(ms_path, err_m)

                    if code_m < 0 or code_m in (134, 137, 139):
                        result["passed"] = False
                        msg = f"Crash/Segfault at malloc #{fail_idx}/{total_allocs} (Exit Code: {code_m})"
                        if callstack_loc:
                            msg += f"\n{callstack_loc}"
                        result["failures"].append("Malloc Fault: " + msg)
                        malloc_fail_logs.append(msg)
                        break
                    elif not program_err:
                        result["passed"] = False
                        msg = f"Silent Failure (No error message printed to STDERR by Minishell) at malloc #{fail_idx}/{total_allocs}"
                        if callstack_loc:
                            msg += f"\n{callstack_loc}"
                        result["failures"].append("Malloc Fault: " + msg)
                        malloc_fail_logs.append(msg)
                        break

                result["malloc_log"] = "\n".join(malloc_fail_logs) if malloc_fail_logs else f"All {total_allocs} malloc failures handled safely."
            else:
                result["malloc_log"] = "No heap allocations recorded for this command."

        return result
    finally:
        shutil.rmtree(test_tmp_dir, ignore_errors=True)


class MinishellTestGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Minishell Modern Graphical Test Harness")
        self.root.geometry("1280x830")
        self.root.minsize(960, 640)

        self.env_mgr = EnvironmentManager()
        try:
            self.env_mgr.build_hook()
        except Exception as e:
            messagebox.showerror("Hook Compilation Error", str(e))

        self.tests_data = []
        for idx, item in enumerate(DEFAULT_TESTS):
            self.tests_data.append({
                "id": idx + 1,
                "cat": item["cat"],
                "cmd": item["cmd"],
                "bash_cmp": item.get("bash_cmp", True),
                "flag_error": item.get("flag_error", False),
                "selected": True,
                "status": "PENDING",
                "result": None
            })

        self.msg_queue = queue.Queue()
        self.is_running = False

        self._setup_dark_theme()
        self._build_ui()
        self._bind_shortcuts()
        self._populate_tree()
        self._update_stats_bar()
        self.root.after(100, self._poll_queue)

    def _setup_dark_theme(self):
        self.root.configure(bg=COLOR_BG_DARK)
        self.style = ttk.Style()
        self.style.theme_use("clam")

        # Global Styles
        self.style.configure(".", background=COLOR_BG_DARK, foreground=COLOR_FG_TEXT, font=("Segoe UI", 9))
        self.style.configure("TFrame", background=COLOR_BG_DARK)
        self.style.configure("Panel.TFrame", background=COLOR_BG_PANEL)

        # LabelFrame
        self.style.configure("TLabelframe", background=COLOR_BG_PANEL, bordercolor=COLOR_BORDER, borderwidth=1, relief="solid")
        self.style.configure("TLabelframe.Label", background=COLOR_BG_PANEL, foreground=COLOR_ACCENT, font=("Segoe UI", 9, "bold"))

        # Buttons
        self.style.configure("TButton", background=COLOR_BG_PANEL, foreground=COLOR_FG_TEXT, borderwidth=1, bordercolor=COLOR_BORDER, focuscolor="none", padding=(10, 5), font=("Segoe UI", 9, "bold"))
        self.style.map("TButton", background=[("active", COLOR_BORDER), ("disabled", COLOR_BG_DARK)], foreground=[("disabled", COLOR_FG_MUTED)])

        self.style.configure("Accent.TButton", background=COLOR_ACCENT, foreground="#11111b", borderwidth=0, padding=(12, 6))
        self.style.map("Accent.TButton", background=[("active", COLOR_ACCENT_HOVER), ("disabled", COLOR_BORDER)], foreground=[("disabled", COLOR_FG_MUTED)])

        # Checkbuttons
        self.style.configure("TCheckbutton", background=COLOR_BG_PANEL, foreground=COLOR_FG_TEXT, focuscolor="none")
        self.style.map("TCheckbutton", background=[("active", COLOR_BG_PANEL)])

        # Entry & Spinbox
        self.style.configure("TEntry", fieldbackground=COLOR_BG_INPUT, foreground=COLOR_FG_TEXT, bordercolor=COLOR_BORDER, insertcolor=COLOR_FG_TEXT, padding=5)
        self.style.configure("TSpinbox", fieldbackground=COLOR_BG_INPUT, foreground=COLOR_FG_TEXT, bordercolor=COLOR_BORDER, arrowcolor=COLOR_FG_TEXT, padding=5)

        # Treeview
        self.style.configure("Treeview", background=COLOR_BG_INPUT, foreground=COLOR_FG_TEXT, fieldbackground=COLOR_BG_INPUT, borderwidth=0, rowheight=28, font=("Consolas", 9))
        self.style.configure("Treeview.Heading", background=COLOR_BG_PANEL, foreground=COLOR_ACCENT, font=("Segoe UI", 9, "bold"), relief="flat", padding=6)
        self.style.map("Treeview", background=[("selected", "#363a4f")], foreground=[("selected", "#ffffff")])

        # Notebook
        self.style.configure("TNotebook", background=COLOR_BG_DARK, borderwidth=0)
        self.style.configure("TNotebook.Tab", background=COLOR_BG_PANEL, foreground=COLOR_FG_MUTED, padding=(14, 7), font=("Segoe UI", 9, "bold"), borderwidth=0)
        self.style.map("TNotebook.Tab", background=[("selected", COLOR_BG_INPUT)], foreground=[("selected", COLOR_ACCENT)])

        # Progressbar
        self.style.configure("Horizontal.TProgressbar", background=COLOR_ACCENT, troughcolor=COLOR_BG_PANEL, bordercolor=COLOR_BORDER, thickness=6)

    def _build_ui(self):
        # Top Header Card
        header_card = ttk.Frame(self.root, style="Panel.TFrame", padding=12)
        header_card.pack(fill=tk.X, side=tk.TOP, padx=12, pady=(12, 4))

        ttk.Label(header_card, text="Minishell Executable:", style="Panel.TFrame").pack(side=tk.LEFT, padx=(0, 6))
        self.ms_path_var = tk.StringVar(value="./minishell")
        ttk.Entry(header_card, textvariable=self.ms_path_var, width=22).pack(side=tk.LEFT, padx=(0, 6))
        ttk.Button(header_card, text="Browse", command=self._browse_binary).pack(side=tk.LEFT, padx=(0, 10))

        ttk.Button(header_card, text="🔨 Recompile", command=self._recompile_minishell).pack(side=tk.LEFT, padx=(0, 16))

        ttk.Label(header_card, text="Threads:", style="Panel.TFrame").pack(side=tk.LEFT, padx=(0, 6))
        self.jobs_var = tk.IntVar(value=os.cpu_count() or 4)
        ttk.Spinbox(header_card, from_=1, to=32, textvariable=self.jobs_var, width=3).pack(side=tk.LEFT, padx=(0, 16))

        # Checkboxes
        self.chk_bash = tk.BooleanVar(value=True)
        self.chk_valgrind = tk.BooleanVar(value=True)
        self.chk_malloc = tk.BooleanVar(value=True)

        ttk.Checkbutton(header_card, text="Bash Compare", variable=self.chk_bash, style="TCheckbutton").pack(side=tk.LEFT, padx=6)
        ttk.Checkbutton(header_card, text="Valgrind / FDs", variable=self.chk_valgrind, style="TCheckbutton").pack(side=tk.LEFT, padx=6)
        ttk.Checkbutton(header_card, text="Malloc Faults", variable=self.chk_malloc, style="TCheckbutton").pack(side=tk.LEFT, padx=6)

        self.btn_run = ttk.Button(header_card, text="▶  Run Selected (F5)", style="Accent.TButton", command=self.run_tests)
        self.btn_run.pack(side=tk.RIGHT, padx=(6, 0))

        # Progress bar
        self.progress_var = tk.DoubleVar(value=0)
        self.progress_bar = ttk.Progressbar(self.root, variable=self.progress_var, maximum=100, style="Horizontal.TProgressbar")
        self.progress_bar.pack(fill=tk.X, side=tk.TOP, padx=12, pady=2)

        # Shortcuts Legend Bar
        shortcut_bar = ttk.Frame(self.root, style="Panel.TFrame", padding=(12, 4))
        shortcut_bar.pack(fill=tk.X, side=tk.TOP, padx=12, pady=(0, 4))
        legend_text = "Shortcuts:  [↑/↓] Navigate Tests  |  [←/→] Switch Inspector Tabs  |  [Space] Select/Deselect  |  [F5 / Ctrl+R] Run  |  [/ / Ctrl+F] Search"
        ttk.Label(shortcut_bar, text=legend_text, style="Panel.TFrame", foreground=COLOR_FG_MUTED, font=("Segoe UI", 8)).pack(side=tk.LEFT)

        # Main Paned Workspace
        paned = ttk.PanedWindow(self.root, orient=tk.HORIZONTAL)
        paned.pack(fill=tk.BOTH, expand=True, padx=12, pady=4)

        # Left Column Frame
        left_frame = ttk.Frame(paned, width=460)
        paned.add(left_frame, weight=1)

        # Selection Control Row
        sel_btn_frame = ttk.Frame(left_frame)
        sel_btn_frame.pack(fill=tk.X, side=tk.TOP, pady=(0, 6))

        ttk.Button(sel_btn_frame, text="Select All", command=lambda: self._set_all_selected(True)).pack(side=tk.LEFT, padx=(0, 2))
        ttk.Button(sel_btn_frame, text="Deselect All", command=lambda: self._set_all_selected(False)).pack(side=tk.LEFT, padx=2)
        ttk.Button(sel_btn_frame, text="Select Failed", command=self._select_failed_only).pack(side=tk.LEFT, padx=2)
        ttk.Button(sel_btn_frame, text="🔄 Reset Selected", command=self._reset_selected_tests).pack(side=tk.LEFT, padx=2)

        # Search Box
        search_frame = ttk.Frame(left_frame)
        search_frame.pack(fill=tk.X, side=tk.TOP, pady=(0, 6))
        ttk.Label(search_frame, text="🔍").pack(side=tk.LEFT, padx=(0, 4))
        self.filter_var = tk.StringVar()
        self.filter_var.trace_add("write", lambda *args: self._populate_tree())
        self.search_entry = ttk.Entry(search_frame, textvariable=self.filter_var)
        self.search_entry.pack(fill=tk.X, expand=True, side=tk.LEFT)

        # Treeview
        tree_container = ttk.Frame(left_frame)
        tree_container.pack(fill=tk.BOTH, expand=True)

        columns = ("sel", "id", "status", "cat", "cmd")
        self.tree = ttk.Treeview(tree_container, columns=columns, show="headings", selectmode="browse")
        self.tree.heading("sel", text="[X]")
        self.tree.heading("id", text="ID")
        self.tree.heading("status", text="Status")
        self.tree.heading("cat", text="Category")
        self.tree.heading("cmd", text="Command")

        self.tree.column("sel", width=42, anchor="center")
        self.tree.column("id", width=42, anchor="center")
        self.tree.column("status", width=90, anchor="center")
        self.tree.column("cat", width=125, anchor="w")
        self.tree.column("cmd", width=160, anchor="w")

        tree_scroll = ttk.Scrollbar(tree_container, orient=tk.VERTICAL, command=self.tree.yview)
        self.tree.configure(yscrollcommand=tree_scroll.set)

        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        tree_scroll.pack(side=tk.RIGHT, fill=tk.Y)

        self.tree.tag_configure("PASS", foreground=COLOR_PASS)
        self.tree.tag_configure("FAIL", foreground=COLOR_FAIL)
        self.tree.tag_configure("RUNNING", foreground=COLOR_WARN)
        self.tree.tag_configure("PENDING", foreground=COLOR_FG_MUTED)

        self.tree.bind("<Button-1>", self._on_tree_click)
        self.tree.bind("<<TreeviewSelect>>", self._on_tree_select)

        # Right Inspector Panel
        right_frame = ttk.Frame(paned)
        paned.add(right_frame, weight=2)

        # Custom Input
        custom_card = ttk.LabelFrame(right_frame, text="Add Custom Command Test", padding=8)
        custom_card.pack(fill=tk.X, side=tk.TOP, pady=(0, 8))

        self.custom_cmd_var = tk.StringVar()
        self.custom_cmd_entry = ttk.Entry(custom_card, textvariable=self.custom_cmd_var)
        self.custom_cmd_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 6))
        ttk.Button(custom_card, text="➕ Add Test", command=self._add_custom_test).pack(side=tk.RIGHT)

        # Inspector Tabs
        self.notebook = ttk.Notebook(right_frame)
        self.notebook.pack(fill=tk.BOTH, expand=True)

        self.txt_overview = self._create_dark_text_tab("Overview", self.notebook)
        self.txt_diff = self._create_dark_text_tab("STDOUT Output / Diff", self.notebook, mono=True)
        self.txt_stderr = self._create_dark_text_tab("STDERR Logs", self.notebook, mono=True)
        self.txt_valgrind = self._create_dark_text_tab("Valgrind / FDs", self.notebook, mono=True)
        self.txt_malloc = self._create_dark_text_tab("Malloc Faults", self.notebook, mono=True)

        self.txt_diff.tag_config("add", background=COLOR_DIFF_ADD_BG, foreground=COLOR_DIFF_ADD_FG)
        self.txt_diff.tag_config("sub", background=COLOR_DIFF_SUB_BG, foreground=COLOR_DIFF_SUB_FG)
        self.txt_diff.tag_config("info", foreground=COLOR_ACCENT)

        # Status Bar
        self.status_bar_frame = ttk.Frame(self.root, style="Panel.TFrame", padding=(12, 6))
        self.status_bar_frame.pack(fill=tk.X, side=tk.BOTTOM, padx=12, pady=(4, 12))

        self.lbl_status = ttk.Label(self.status_bar_frame, text="Ready", style="Panel.TFrame", font=("Segoe UI", 9, "bold"))
        self.lbl_status.pack(side=tk.LEFT)

        self.lbl_metrics = ttk.Label(self.status_bar_frame, text="", style="Panel.TFrame", foreground=COLOR_FG_MUTED)
        self.lbl_metrics.pack(side=tk.RIGHT)

    def _bind_shortcuts(self):
        self.root.bind("<F5>", lambda e: self.run_tests())
        self.root.bind("<Control-r>", lambda e: self.run_tests())
        self.root.bind("<Control-f>", self._focus_search)
        self.root.bind("<slash>", self._focus_search)

        self.search_entry.bind("<Down>", lambda e: self._focus_tree())

        self.root.bind("<Left>", self._handle_tab_navigation)
        self.root.bind("<Right>", self._handle_tab_navigation)

        self.search_entry.bind("<Escape>", self._focus_tree)
        self.tree.bind("<space>", self._on_space_key)

    def _is_input_focused(self):
        focus = self.root.focus_get()
        return isinstance(focus, (ttk.Entry, tk.Entry))

    def _focus_search(self, event=None):
        self.search_entry.focus_set()
        self.search_entry.selection_range(0, tk.END)
        return "break"

    def _focus_tree(self, event=None):
        self.tree.focus_set()
        children = self.tree.get_children()
        if children and not self.tree.selection():
            self.tree.selection_set(children[0])
            self.tree.see(children[0])
        return "break"

    def _handle_tab_navigation(self, event):
        if self._is_input_focused():
            return None

        direction = -1 if event.keysym == "Left" else 1
        current = self.notebook.index(self.notebook.select())
        total = self.notebook.index("end")
        new_index = (current + direction) % total
        self.notebook.select(new_index)
        return "break"

    def _on_space_key(self, event):
        sel = self.tree.selection()
        if sel:
            t_id = self.tree.item(sel[0])["values"][1]
            for t in self.tests_data:
                if t["id"] == t_id:
                    t["selected"] = not t["selected"]
                    self._update_tree_row(sel[0], t)
                    break
        return "break"

    def _create_dark_text_tab(self, title, notebook, mono=False):
        frame = ttk.Frame(notebook)
        notebook.add(frame, text=title)

        font_family = "Consolas" if mono else "Segoe UI"
        txt = tk.Text(
            frame,
            wrap=tk.WORD if not mono else tk.NONE,
            font=(font_family, 10),
            bg=COLOR_BG_INPUT,
            fg=COLOR_FG_TEXT,
            insertbackground=COLOR_FG_TEXT,
            selectbackground="#363a4f",
            selectforeground="#ffffff",
            relief="flat",
            bd=0,
            padx=10,
            pady=10,
            state=tk.DISABLED
        )
        scroll = ttk.Scrollbar(frame, orient=tk.VERTICAL, command=txt.yview)
        txt.configure(yscrollcommand=scroll.set)

        txt.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scroll.pack(side=tk.RIGHT, fill=tk.Y)
        return txt

    def _write_read_only_text(self, txt_widget, content_writer):
        txt_widget.config(state=tk.NORMAL)
        txt_widget.delete("1.0", tk.END)
        content_writer()
        txt_widget.config(state=tk.DISABLED)

    def _browse_binary(self):
        filename = filedialog.askopenfilename(title="Select Minishell Binary")
        if filename:
            self.ms_path_var.set(filename)

    def _recompile_minishell(self):
        ms_path = os.path.abspath(self.ms_path_var.get())
        ms_dir = os.path.dirname(ms_path) if os.path.exists(ms_path) else "."

        self.lbl_status.config(text="Status: Recompiling (make)...", foreground=COLOR_WARN)
        self.root.update_idletasks()

        try:
            res = subprocess.run(["make"], cwd=ms_dir, capture_output=True, text=True)
            if res.returncode == 0:
                messagebox.showinfo("Recompile Success", "Build succeeded! 'make' returned 0.\n\n" + (res.stdout[-600:] if res.stdout else "No output."))
                self.lbl_status.config(text="Status: Recompile Successful", foreground=COLOR_PASS)
            else:
                messagebox.showerror("Recompile Failed", "Build failed!\n\n" + (res.stderr[-1000:] if res.stderr else "Compilation error."))
                self.lbl_status.config(text="Status: Recompile Failed", foreground=COLOR_FAIL)
        except Exception as e:
            messagebox.showerror("Error", f"Failed to run make: {str(e)}")
            self.lbl_status.config(text="Status: Recompile Error", foreground=COLOR_FAIL)

        self._update_stats_bar()

    def _reset_selected_tests(self):
        for t in self.tests_data:
            if t["selected"]:
                t["status"] = "PENDING"
                t["result"] = None
        self._populate_tree()

        sel = self.tree.selection()
        if sel:
            t_id = self.tree.item(sel[0])["values"][1]
            t_item = next((t for t in self.tests_data if t["id"] == t_id), None)
            if t_item:
                self._update_inspector(t_item)

    def _populate_tree(self):
        filter_str = self.filter_var.get().lower()
        selected_id = None
        sel = self.tree.selection()
        if sel:
            selected_id = self.tree.item(sel[0])["values"][1]

        for item in self.tree.get_children():
            self.tree.delete(item)

        for item in self.tests_data:
            if filter_str and filter_str not in item["cmd"].lower() and filter_str not in item["cat"].lower():
                continue

            sel_mark = "[X]" if item["selected"] else "[  ]"
            status_text = item["status"]
            tag = "PENDING"

            if item["status"] == "PASS":
                status_text = "✔ PASS"
                tag = "PASS"
            elif item["status"] == "FAIL":
                status_text = "✖ FAIL"
                tag = "FAIL"
            elif item["status"] == "RUNNING":
                status_text = "⏳ RUNNING"
                tag = "RUNNING"

            node = self.tree.insert("", tk.END, values=(sel_mark, item["id"], status_text, item["cat"], item["cmd"].replace("\n", "\\n")), tags=(tag,))
            if selected_id == item["id"]:
                self.tree.selection_set(node)

        self._update_stats_bar()

    def _update_tree_row(self, node, item):
        sel_mark = "[X]" if item["selected"] else "[  ]"
        status_text = item["status"]
        tag = "PENDING"

        if item["status"] == "PASS":
            status_text = "✔ PASS"
            tag = "PASS"
        elif item["status"] == "FAIL":
            status_text = "✖ FAIL"
            tag = "FAIL"
        elif item["status"] == "RUNNING":
            status_text = "⏳ RUNNING"
            tag = "RUNNING"

        self.tree.item(node, values=(sel_mark, item["id"], status_text, item["cat"], item["cmd"].replace("\n", "\\n")), tags=(tag,))
        self._update_stats_bar()

    def _update_stats_bar(self):
        total = len(self.tests_data)
        selected = sum(1 for t in self.tests_data if t["selected"])
        passed = sum(1 for t in self.tests_data if t["status"] == "PASS")
        failed = sum(1 for t in self.tests_data if t["status"] == "FAIL")

        if self.is_running:
            self.lbl_status.config(text="Status: Executing Tests...", foreground=COLOR_WARN)
        else:
            self.lbl_status.config(text="Status: Ready", foreground=COLOR_ACCENT)

        self.lbl_metrics.config(
            text=f"Total: {total}  |  Selected: {selected}  |  Passed: {passed}  |  Failed: {failed}"
        )

    def _on_tree_click(self, event):
        region = self.tree.identify("region", event.x, event.y)
        if region == "cell":
            column = self.tree.identify_column(event.x)
            if column == "#1":  # Selection checkbox column
                item_id = self.tree.identify_row(event.y)
                if item_id:
                    vals = self.tree.item(item_id)["values"]
                    t_id = vals[1]
                    for t in self.tests_data:
                        if t["id"] == t_id:
                            t["selected"] = not t["selected"]
                            self._update_tree_row(item_id, t)
                            break

    def _on_tree_select(self, event):
        sel = self.tree.selection()
        if not sel:
            return
        t_id = self.tree.item(sel[0])["values"][1]
        test_item = next((t for t in self.tests_data if t["id"] == t_id), None)
        if test_item:
            self._update_inspector(test_item)

    def _set_all_selected(self, val):
        for t in self.tests_data:
            t["selected"] = val
        self._populate_tree()

    def _select_failed_only(self):
        for t in self.tests_data:
            t["selected"] = (t["status"] == "FAIL")
        self._populate_tree()

    def _add_custom_test(self):
        cmd = self.custom_cmd_var.get().strip()
        if not cmd:
            return
        new_id = len(self.tests_data) + 1
        new_test = {
            "id": new_id,
            "cat": "Custom",
            "cmd": cmd,
            "bash_cmp": True,
            "selected": True,
            "status": "PENDING",
            "result": None
        }
        self.tests_data.append(new_test)
        self.custom_cmd_var.set("")
        self._populate_tree()

    def _update_inspector(self, test_item):
        res = test_item["result"]

        if not res:
            self._write_read_only_text(
                self.txt_overview,
                lambda: self.txt_overview.insert(tk.END, f"Command: {test_item['cmd']}\nStatus: {test_item['status']}\n\nRun test to inspect output.")
            )
            for txt in (self.txt_diff, self.txt_stderr, self.txt_valgrind, self.txt_malloc):
                self._write_read_only_text(txt, lambda: None)
            return

        # 1. Overview Tab
        def write_overview():
            ov = [
                f"Command:  {res['cmd']}",
                f"Category: {res['cat']}",
                f"Result:   {'PASS' if res['passed'] else 'FAIL'}",
                f"\nExit Statuses:",
                f"  Minishell: {res['ms_code']}",
                f"  Bash:      {res['bash_code']}"
            ]
            if res["failures"]:
                ov.append("\nFailures / Mismatches Detected:")
                for f in res["failures"]:
                    ov.append(f"  • {f}")
            self.txt_overview.insert(tk.END, "\n".join(ov))

        self._write_read_only_text(self.txt_overview, write_overview)

        # 2. STDOUT Tab (DIFF FIRST, THEN OUTPUTS)
        def write_diff():
            self.txt_diff.insert(tk.END, "=== UNIFIED DIFF (-Bash, +Minishell) ===\n", "info")
            if res["diff_text"]:
                for line in res["diff_text"].splitlines(keepends=True):
                    if line.startswith("+"):
                        self.txt_diff.insert(tk.END, line, "add")
                    elif line.startswith("-"):
                        self.txt_diff.insert(tk.END, line, "sub")
                    elif line.startswith("@"):
                        self.txt_diff.insert(tk.END, line, "info")
                    else:
                        self.txt_diff.insert(tk.END, line)
            else:
                self.txt_diff.insert(tk.END, "✔ STDOUT matches Bash output perfectly.\n")

            self.txt_diff.insert(tk.END, "\n=== MINISHELL STDOUT ===\n", "info")
            self.txt_diff.insert(tk.END, res["ms_out"] if res["ms_out"] else "(empty)\n")

            self.txt_diff.insert(tk.END, "\n=== BASH STDOUT ===\n", "info")
            self.txt_diff.insert(tk.END, res["bash_out"] if res["bash_out"] else "(empty)\n")

        self._write_read_only_text(self.txt_diff, write_diff)

        # 3. STDERR Tab
        def write_stderr():
            ms_e = res['ms_err'] if res['ms_err'].strip() else "(empty)"
            bash_e = res['bash_err'] if res['bash_err'].strip() else "(empty)"
            err_txt = f"=== MINISHELL STDERR ===\n{ms_e}\n\n=== BASH STDERR ===\n{bash_e}"
            self.txt_stderr.insert(tk.END, err_txt)

        self._write_read_only_text(self.txt_stderr, write_stderr)

        # 4. Valgrind Tab
        self._write_read_only_text(self.txt_valgrind, lambda: self.txt_valgrind.insert(tk.END, res["valgrind_log"]))

        # 5. Malloc Tab
        self._write_read_only_text(self.txt_malloc, lambda: self.txt_malloc.insert(tk.END, res["malloc_log"]))

    def run_tests(self):
        if self.is_running:
            return

        ms_path = os.path.abspath(self.ms_path_var.get())
        if not os.path.exists(ms_path):
            messagebox.showerror("Error", f"Minishell binary '{ms_path}' not found.")
            return

        selected_tests = [t for t in self.tests_data if t["selected"] and t["status"] == "PENDING"]

        if not selected_tests:
            already_run = sum(1 for t in self.tests_data if t["selected"] and t["status"] != "PENDING")
            if already_run > 0:
                messagebox.showinfo(
                    "Info",
                    "All selected tests have already been executed.\n\nClick '🔄 Reset Selected' to clear past results and re-run them."
                )
            else:
                messagebox.showwarning("Warning", "No tests selected.")
            return

        self.is_running = True
        self.btn_run.config(state=tk.DISABLED)
        self.progress_var.set(0)

        opts = {
            "skip_bash": not self.chk_bash.get(),
            "skip_valgrind": not self.chk_valgrind.get(),
            "skip_malloc": not self.chk_malloc.get()
        }

        threading.Thread(
            target=self._worker_thread,
            args=(selected_tests, ms_path, self.env_mgr.hook_so, self.env_mgr.supp_file, opts, self.jobs_var.get()),
            daemon=True
        ).start()

    def _worker_thread(self, tests, ms_path, hook_so_path, supp_file_path, opts, num_threads):
        total = len(tests)
        completed = 0

        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            future_to_test = {
                executor.submit(execute_single_test, t, ms_path, hook_so_path, supp_file_path, opts): t
                for t in tests
            }
            for future in as_completed(future_to_test):
                test_item = future_to_test[future]
                res = future.result()
                completed += 1
                self.msg_queue.put(("RESULT", test_item["id"], res, (completed / total) * 100))

        self.msg_queue.put(("DONE", None, None, 100))

    def _poll_queue(self):
        try:
            while True:
                msg_type, t_id, res, prog = self.msg_queue.get_nowait()
                if msg_type == "RESULT":
                    for t in self.tests_data:
                        if t["id"] == t_id:
                            t["result"] = res
                            t["status"] = "PASS" if res["passed"] else "FAIL"
                            break
                    self.progress_var.set(prog)
                    self._populate_tree()

                    sel = self.tree.selection()
                    if sel and self.tree.item(sel[0])["values"][1] == t_id:
                        t_item = next((t for t in self.tests_data if t["id"] == t_id), None)
                        if t_item:
                            self._update_inspector(t_item)

                elif msg_type == "DONE":
                    self.is_running = False
                    self.btn_run.config(state=tk.NORMAL)
                    self.progress_var.set(100)
                    self._update_stats_bar()
        except queue.Empty:
            pass

        self.root.after(100, self._poll_queue)


def main():
    root = tk.Tk()
    app = MinishellTestGUI(root)
    root.mainloop()

if __name__ == "__main__":
    main()
