#!/usr/bin/env python3
"""
Minishell Universal Modern Dark GUI Test Harness
Single-file self-contained Tkinter GUI test harness with stack backtrace symbol
resolution for silent malloc failures, readline valgrind suppressions, isolated multi-pass
execution (Base Command, CWD Probe, Env Probe, Malloc Faults, Valgrind, and Comprehensive Signal Phase),
customizable Bash executable selector with dynamic STDERR normalization, external JSON test suite persistence,
recompile/reset controls, file-isolated execution, automatic artifact cleanup, and dark UI theme.
"""

import os
import sys
import json
import shutil
import tempfile
import atexit
import argparse
import difflib
import glob
import queue
import threading
import subprocess
import re
import time
import pty
import select
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

TESTS_FILE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "tests.json")

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

DEFAULT_TESTS = [
    {"cat": "Normal Cmds", "cmd": "ls", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "ls -la", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "whoami", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "uname -s", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "cat /etc/passwd | grep -E 'root|nobody'", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd .", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd ..", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd /tmp", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd ''", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "pwd", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo hello world", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -n hello world", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR_TEST=123", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset PATH", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 0", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 42", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "echo hello > /tmp/ms_out.txt && cat /tmp/ms_out.txt; rm -f /tmp/ms_out.txt", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "cat << EOF\nline 1\nline 2\nEOF", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "cat << EOF1 << EOF2\nfirst\nEOF1\nsecond\nEOF2", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "echo hello | cat", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "cat /etc/passwd | grep -v root | wc -l", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "true && echo yes", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "false || echo no", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(echo inside subshell)", "bash_cmp": True},
    {"cat": "Flag Errors", "cmd": "cd -Z /tmp", "flag_error": True}
]

def load_tests_from_file():
    if os.path.exists(TESTS_FILE_PATH):
        try:
            with open(TESTS_FILE_PATH, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception as e:
            print(f"Warning: Failed to load {TESTS_FILE_PATH}: {e}. Falling back to default tests.")
    save_tests_to_file(DEFAULT_TESTS)
    return DEFAULT_TESTS

def save_tests_to_file(tests_list):
    clean_list = []
    for item in tests_list:
        clean_list.append({
            "cat": item.get("cat", "Custom"),
            "cmd": item.get("cmd", ""),
            "bash_cmp": item.get("bash_cmp", True),
            "flag_error": item.get("flag_error", False)
        })
    try:
        with open(TESTS_FILE_PATH, "w", encoding="utf-8") as f:
            json.dump(clean_list, f, indent=2)
    except Exception as e:
        print(f"Error saving tests to {TESTS_FILE_PATH}: {e}")

def cleanup_test_artifacts():
    patterns = ["/tmp/ms_*", "/tmp/mini_*"]
    for pattern in patterns:
        for path in glob.glob(pattern):
            if "minishell_gui_env_" in path:
                continue
            try:
                if os.path.isdir(path):
                    shutil.rmtree(path, ignore_errors=True)
                else:
                    os.remove(path)
            except Exception:
                pass

class EnvironmentManager:
    def __init__(self, ms_path="./minishell"):
        self.ms_path = os.path.abspath(ms_path)
        self.temp_dir = tempfile.mkdtemp(prefix="minishell_gui_env_")
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
            input=cmd_str,
            capture_output=True,
            text=True,
            env=env,
            cwd=cwd,
            timeout=timeout
        )
        return proc.stdout, proc.stderr, proc.returncode
    except subprocess.TimeoutExpired:
        return "", "TIMEOUT", -1

def run_bash(cmd_str, bash_executable, cwd=None):
    try:
        proc = subprocess.run(
            [bash_executable, "--posix"],
            input=cmd_str,
            capture_output=True,
            text=True,
            cwd=cwd,
            timeout=5
        )
        return proc.stdout, proc.stderr, proc.returncode
    except subprocess.TimeoutExpired:
        return "", "TIMEOUT", -1
    except Exception as e:
        return "", f"BASH_EXEC_ERROR: {str(e)}", -1

def normalize_stdout(raw_stdout):
    if not raw_stdout:
        return ""
    clean = raw_stdout.replace('\0', '\n')
    clean = re.sub(r'\x01|\x02|\x1B\[[0-9;]*[a-zA-Z]', '', clean)
    clean = re.sub(r'^[a-zA-Z0-9_\.-]+@[a-zA-Z0-9_\.-]+:.*\$ ', '', clean, flags=re.MULTILINE)
    clean = re.sub(r'^\$ ', '', clean, flags=re.MULTILINE)
    clean = re.sub(r'^\$ $', '', clean, flags=re.MULTILINE)
    clean = re.sub(r'^exit$', '', clean, flags=re.MULTILINE)
    return clean.strip()

def normalize_stderr(raw_stderr, is_bash=False, bash_executable=None):
    if not raw_stderr:
        return ""
    clean = re.sub(r'sh: [0-9]+: getcwd\(\) failed.*\n?', '', raw_stderr)
    if is_bash:
        if bash_executable:
            bin_name = os.path.basename(bash_executable)
            clean = re.sub(rf'^{re.escape(bash_executable)}: line [0-9]+: ', '', clean, flags=re.MULTILINE)
            clean = re.sub(rf'^{re.escape(bash_executable)}: ', '', clean, flags=re.MULTILINE)
            clean = re.sub(rf'^{re.escape(bin_name)}: line [0-9]+: ', '', clean, flags=re.MULTILINE)
            clean = re.sub(rf'^{re.escape(bin_name)}: ', '', clean, flags=re.MULTILINE)
        clean = re.sub(r'^[a-zA-Z0-9_\.\/-]+/bash: line [0-9]+: ', '', clean, flags=re.MULTILINE)
        clean = re.sub(r'^[a-zA-Z0-9_\.\/-]+/bash: ', '', clean, flags=re.MULTILINE)
        clean = re.sub(r'^bash: line [0-9]+: ', '', clean, flags=re.MULTILINE)
        clean = re.sub(r'^bash: ', '', clean, flags=re.MULTILINE)
        clean = re.sub(r"^`.*`$\n?", '', clean, flags=re.MULTILINE)
    else:
        clean = re.sub(r'^minishell: ', '', clean, flags=re.MULTILINE)
    return clean.strip()

def normalize_env_output(output_str):
    lines = output_str.splitlines()
    filtered = []
    ignore_prefixes = (
        "declare -x BASH", "declare -x SHLVL", "declare -x PWD", "declare -x OLDPWD",
        "declare -x _", "declare -x LS_COLORS", "declare -x XDG", "declare -x GLIBC",
        "declare -x LD_", "declare -x FAIL_MALLOC_", "declare -x TRACE_MALLOC",
        "declare -x BASH_EXECUTION_STRING", "declare -x HOSTNAME", "declare -x RANDOM",
        "BASH=", "SHLVL=", "PWD=", "OLDPWD=", "_=", "LS_COLORS=", "XDG=", "GLIBC=",
        "LD_=", "LD_PRELOAD=", "FAIL_MALLOC_", "TRACE_MALLOC=", "BASH_EXECUTION_STRING=",
        "HOSTNAME=", "RANDOM=", "PIPESTATUS=", "TERM=", "COLUMNS=", "LINES="
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

def execute_pty_signal_tests(ms_path, root_dir):
    logs = []
    failures = []

    def spawn_ms():
        master, slave = pty.openpty()
        proc = subprocess.Popen(
            [os.path.abspath(ms_path)],
            stdin=slave, stdout=slave, stderr=slave,
            cwd=root_dir, start_new_session=True, text=False
        )
        os.close(slave)
        return master, proc

    def read_all(master, timeout=0.4):
        out = b""
        while True:
            r, _, _ = select.select([master], [], [], timeout)
            if not r:
                break
            try:
                chunk = os.read(master, 1024)
                if not chunk:
                    break
                out += chunk
            except OSError:
                break
        return out.decode("utf-8", errors="replace")

    # Pass 1: Main Loop
    try:
        m, p = spawn_ms()
        read_all(m, 0.3)
        os.write(m, b"\x03")
        read_all(m, 0.3)
        os.write(m, b"echo $?\n")
        out2 = read_all(m, 0.3)
        os.write(m, b"exit\n")
        p.wait(timeout=1)
        os.close(m)

        if "130" in out2 or p.returncode == 0:
            logs.append("✔ [Interactive Prompt] Ctrl+C (SIGINT): OK (Prompt reset, status = 130)")
        else:
            failures.append("Interactive Prompt: Ctrl+C did not set exit status to 130.")
            logs.append("✖ [Interactive Prompt] Ctrl+C: FAILED")
    except Exception as e:
        logs.append(f"✖ [Interactive Prompt] Ctrl+C test error: {e}")

    # Pass 2: Heredoc Loop
    try:
        m, p = spawn_ms()
        read_all(m, 0.3)
        os.write(m, b"cat << EOF\n")
        read_all(m, 0.3)
        os.write(m, b"line 1\n")
        read_all(m, 0.2)
        os.write(m, b"\x03")
        out = read_all(m, 0.4)
        os.write(m, b"echo $?\n")
        out2 = read_all(m, 0.3)
        os.write(m, b"exit\n")
        p.wait(timeout=1)
        os.close(m)

        if ("130" in out2 or "130" in out) and p.returncode == 0:
            logs.append("✔ [Heredoc Loop] Ctrl+C (SIGINT): OK (Aborted heredoc, status = 130)")
        else:
            failures.append("Heredoc Loop: Ctrl+C failed to abort heredoc or set status 130.")
            logs.append("✖ [Heredoc Loop] Ctrl+C: FAILED")
    except Exception as e:
        logs.append(f"✖ [Heredoc Loop] Ctrl+C test error: {e}")

    # Pass 3: Child Binary Execution
    try:
        m, p = spawn_ms()
        read_all(m, 0.3)
        os.write(m, b"sleep 5\n")
        time.sleep(0.2)
        os.write(m, b"\x03")
        out = read_all(m, 0.4)
        os.write(m, b"echo $?\n")
        out2 = read_all(m, 0.3)
        os.write(m, b"exit\n")
        p.wait(timeout=1)
        os.close(m)

        if "130" in out2 or p.returncode == 0:
            logs.append("✔ [Child Execution] Ctrl+C (SIGINT): OK (Interrupted child, status = 130)")
        else:
            failures.append("Child Execution: Ctrl+C during child execution did not set status to 130.")
            logs.append("✖ [Child Execution] Ctrl+C: FAILED")
    except Exception as e:
        logs.append(f"✖ [Child Execution] Ctrl+C test error: {e}")

    return logs, failures

def execute_single_test(test_item, ms_path, bash_path, hook_so_path, supp_file_path, opts):
    ms_path = os.path.abspath(ms_path)
    root_dir = os.path.dirname(ms_path)
    cmd_raw = test_item["cmd"]

    ms_cmd_str = re.sub(r'/tmp/ms_', f'/tmp/ms_t{test_item["id"]}_ms_', cmd_raw)
    bash_cmd_str = re.sub(r'/tmp/ms_', f'/tmp/ms_t{test_item["id"]}_bash_', cmd_raw)
    bash_cmd_str = re.sub(r'(^|;|&&|\|\||\||\(|\n)(\s*)unset\b(?!\s+-)', r'\1\2unset -v', bash_cmd_str)

    result = {
        "id": test_item["id"],
        "cmd": cmd_raw,
        "cat": test_item["cat"],
        "passed": True,
        "ms_out": "",
        "ms_err": "",
        "ms_code": 0,
        "ms_cwd": None,
        "ms_env": "",
        "bash_out": "",
        "bash_err": "",
        "bash_code": 0,
        "bash_cwd": None,
        "bash_env": "",
        "diff_text": "",
        "env_diff_text": "",
        "valgrind_log": "Not Run",
        "malloc_log": "Not Run",
        "signal_log": "Not Run",
        "failures": []
    }

    # Pass 1: Base Command
    stdin_base = f"{ms_cmd_str}\nexit $?\n"
    bash_stdin_base = f"{bash_cmd_str}\nexit $?\n"

    raw_ms_out, ms_err, ms_code = run_shell(stdin_base, ms_path, cwd=root_dir)
    raw_bash_out, bash_err, bash_code = run_bash(bash_stdin_base, bash_executable=bash_path, cwd=root_dir)

    ms_out = normalize_stdout(raw_ms_out)
    bash_out = normalize_stdout(raw_bash_out)

    clean_ms_err = normalize_stderr(strip_hook_output(ms_err), is_bash=False)
    clean_bash_err = normalize_stderr(bash_err, is_bash=True, bash_executable=bash_path)

    result["ms_out"] = ms_out
    result["ms_err"] = clean_ms_err
    result["ms_code"] = ms_code
    result["bash_out"] = bash_out
    result["bash_err"] = clean_bash_err
    result["bash_code"] = bash_code

    if test_item.get("flag_error", False):
        if not clean_ms_err or ms_code == 0:
            result["passed"] = False
            result["failures"].append("Flag Option Check: Expected non-zero exit code and error message on STDERR.")
    elif test_item.get("bash_cmp", True) and not opts.get("skip_bash", False):
        if "env" in cmd_raw or "export" in cmd_raw:
            out_match = (normalize_env_output(ms_out) == normalize_env_output(bash_out))
        else:
            out_match = (ms_out == bash_out)

        err_match = (clean_ms_err == clean_bash_err)
        code_match = (ms_code == bash_code)

        diff = difflib.unified_diff(
            bash_out.splitlines(keepends=True),
            ms_out.splitlines(keepends=True),
            fromfile="bash stdout",
            tofile="minishell stdout"
        )
        result["diff_text"] = "".join(diff)

        if not out_match or not code_match or not err_match:
            result["passed"] = False
            err_reasons = []
            if not out_match:
                err_reasons.append("STDOUT mismatch")
            if not err_match:
                err_reasons.append("STDERR mismatch")
            if not code_match:
                err_reasons.append(f"Exit status mismatch (minishell={ms_code}, bash={bash_code})")
            result["failures"].append("Bash Comparison: " + ", ".join(err_reasons))

    # Pass 2: CWD Probe
    stdin_cwd = f"{ms_cmd_str}\npwd -P\nexit $?\n"
    bash_stdin_cwd = f"{bash_cmd_str}\npwd -P\nexit $?\n"

    ms_cwd_raw, _, _ = run_shell(stdin_cwd, ms_path, cwd=root_dir)
    bash_cwd_raw, _, _ = run_bash(bash_stdin_cwd, bash_executable=bash_path, cwd=root_dir)

    ms_cwd = ms_cwd_raw.strip().splitlines()[-1] if ms_cwd_raw.strip() else None
    bash_cwd = bash_cwd_raw.strip().splitlines()[-1] if bash_cwd_raw.strip() else None

    result["ms_cwd"] = ms_cwd
    result["bash_cwd"] = bash_cwd

    if ms_cwd and bash_cwd and ms_cwd != bash_cwd:
        result["passed"] = False
        result["failures"].append(f"CWD Mismatch: Minishell in '{ms_cwd}', expected '{bash_cwd}'")

    # Pass 3: Environment Variables Probe
    stdin_env = f"{ms_cmd_str}\nenv\nexit $?\n"
    bash_stdin_env = f"{bash_cmd_str}\nenv\nexit $?\n"

    ms_env_raw, _, _ = run_shell(stdin_env, ms_path, cwd=root_dir)
    bash_env_raw, _, _ = run_bash(bash_stdin_env, bash_executable=bash_path, cwd=root_dir)

    result["ms_env"] = ms_env_raw
    result["bash_env"] = bash_env_raw

    if ms_env_raw and bash_env_raw and test_item.get("bash_cmp", True) and not opts.get("skip_bash", False):
        norm_ms_env = normalize_env_output(ms_env_raw)
        norm_bash_env = normalize_env_output(bash_env_raw)

        env_diff = difflib.unified_diff(
            norm_bash_env.splitlines(keepends=True),
            norm_ms_env.splitlines(keepends=True),
            fromfile="bash env",
            tofile="minishell env"
        )
        result["env_diff_text"] = "".join(env_diff)

        if norm_ms_env != norm_bash_env and "env" not in cmd_raw and "export" not in cmd_raw:
            result["passed"] = False
            result["failures"].append("ENV Mismatch: Final environment variables do not match Bash output")

    # Pass 4: Valgrind
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
            proc = subprocess.run(valgrind_cmd, input=stdin_base, capture_output=True, text=True, cwd=root_dir, timeout=10)
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

    # Pass 5: Malloc Fault Injection
    if not opts.get("skip_malloc", False):
        env = os.environ.copy()
        env["LD_PRELOAD"] = hook_so_path
        env["LOG_ALLOC_COUNT"] = "1"

        _, ms_err_log, _ = run_shell(stdin_base, ms_path, env=env, cwd=root_dir)

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

                _, err_m, code_m = run_shell(stdin_base, ms_path, env=fail_env, cwd=root_dir)

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

    # Pass 6: Interactive Signal Phase
    if not opts.get("skip_signals", False):
        sig_logs, sig_failures = execute_pty_signal_tests(ms_path, root_dir)
        result["signal_log"] = "\n".join(sig_logs)
        if sig_failures:
            result["passed"] = False
            for sf in sig_failures:
                result["failures"].append(sf)

    return result


class MinishellTestGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Minishell Modern Graphical Test Harness")
        self.root.geometry("1280x830")
        self.root.minsize(960, 640)

        cleanup_test_artifacts()

        self.env_mgr = EnvironmentManager()
        try:
            self.env_mgr.build_hook()
        except Exception as e:
            messagebox.showerror("Hook Compilation Error", str(e))

        raw_tests = load_tests_from_file()
        self.tests_data = []
        for idx, item in enumerate(raw_tests):
            self.tests_data.append({
                "id": idx + 1,
                "cat": item.get("cat", "Custom"),
                "cmd": item.get("cmd", ""),
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

        self.style.configure(".", background=COLOR_BG_DARK, foreground=COLOR_FG_TEXT, font=("Segoe UI", 9))
        self.style.configure("TFrame", background=COLOR_BG_DARK)
        self.style.configure("Panel.TFrame", background=COLOR_BG_PANEL)

        self.style.configure("TLabelframe", background=COLOR_BG_PANEL, bordercolor=COLOR_BORDER, borderwidth=1, relief="solid")
        self.style.configure("TLabelframe.Label", background=COLOR_BG_PANEL, foreground=COLOR_ACCENT, font=("Segoe UI", 9, "bold"))

        self.style.configure("TButton", background=COLOR_BG_PANEL, foreground=COLOR_FG_TEXT, borderwidth=1, bordercolor=COLOR_BORDER, focuscolor="none", padding=(10, 5), font=("Segoe UI", 9, "bold"))
        self.style.map("TButton", background=[("active", COLOR_BORDER), ("disabled", COLOR_BG_DARK)], foreground=[("disabled", COLOR_FG_MUTED)])

        self.style.configure("Accent.TButton", background=COLOR_ACCENT, foreground="#11111b", borderwidth=0, padding=(12, 6))
        self.style.map("Accent.TButton", background=[("active", COLOR_ACCENT_HOVER), ("disabled", COLOR_BORDER)], foreground=[("disabled", COLOR_FG_MUTED)])

        self.style.configure("TCheckbutton", background=COLOR_BG_PANEL, foreground=COLOR_FG_TEXT, focuscolor="none")
        self.style.map("TCheckbutton", background=[("active", COLOR_BG_PANEL)])

        self.style.configure("TEntry", fieldbackground=COLOR_BG_INPUT, foreground=COLOR_FG_TEXT, bordercolor=COLOR_BORDER, insertcolor=COLOR_FG_TEXT, padding=5)
        self.style.configure("TSpinbox", fieldbackground=COLOR_BG_INPUT, foreground=COLOR_FG_TEXT, bordercolor=COLOR_BORDER, arrowcolor=COLOR_FG_TEXT, padding=5)

        self.style.configure("Treeview", background=COLOR_BG_INPUT, foreground=COLOR_FG_TEXT, fieldbackground=COLOR_BG_INPUT, borderwidth=0, rowheight=28, font=("Consolas", 9))
        self.style.configure("Treeview.Heading", background=COLOR_BG_PANEL, foreground=COLOR_ACCENT, font=("Segoe UI", 9, "bold"), relief="flat", padding=6)
        self.style.map("Treeview", background=[("selected", "#363a4f")], foreground=[("selected", "#ffffff")])

        self.style.configure("TNotebook", background=COLOR_BG_DARK, borderwidth=0)
        self.style.configure("TNotebook.Tab", background=COLOR_BG_PANEL, foreground=COLOR_FG_MUTED, padding=(14, 7), font=("Segoe UI", 9, "bold"), borderwidth=0)
        self.style.map("TNotebook.Tab", background=[("selected", COLOR_BG_INPUT)], foreground=[("selected", COLOR_ACCENT)])

        self.style.configure("Horizontal.TProgressbar", background=COLOR_ACCENT, troughcolor=COLOR_BG_PANEL, bordercolor=COLOR_BORDER, thickness=6)

    def _build_ui(self):
        header_card = ttk.Frame(self.root, style="Panel.TFrame", padding=12)
        header_card.pack(fill=tk.X, side=tk.TOP, padx=12, pady=(12, 4))

        # Row 1: Executable Paths
        r1 = ttk.Frame(header_card, style="Panel.TFrame")
        r1.pack(fill=tk.X, side=tk.TOP, pady=(0, 6))

        ttk.Label(r1, text="Minishell Executable:", style="Panel.TFrame").pack(side=tk.LEFT, padx=(0, 6))
        self.ms_path_var = tk.StringVar(value="./minishell")
        ttk.Entry(r1, textvariable=self.ms_path_var, width=18).pack(side=tk.LEFT, padx=(0, 4))
        ttk.Button(r1, text="Browse", command=self._browse_binary).pack(side=tk.LEFT, padx=(0, 10))

        default_bash = "/home/subz3r0/Downloads/bash-5.1.16/bash" if os.path.exists("/home/subz3r0/Downloads/bash-5.1.16/bash") else (shutil.which("bash") or "/bin/bash")
        ttk.Label(r1, text="Bash Executable:", style="Panel.TFrame").pack(side=tk.LEFT, padx=(0, 6))
        self.bash_path_var = tk.StringVar(value=default_bash)
        ttk.Entry(r1, textvariable=self.bash_path_var, width=32).pack(side=tk.LEFT, padx=(0, 4))
        ttk.Button(r1, text="Browse", command=self._browse_bash_binary).pack(side=tk.LEFT, padx=(0, 10))

        ttk.Button(r1, text="🔨 Recompile", command=self._recompile_minishell).pack(side=tk.LEFT)

        # Row 2: Controls, Checkboxes, Run
        r2 = ttk.Frame(header_card, style="Panel.TFrame")
        r2.pack(fill=tk.X, side=tk.TOP)

        ttk.Label(r2, text="Threads:", style="Panel.TFrame").pack(side=tk.LEFT, padx=(0, 6))
        self.jobs_var = tk.IntVar(value=os.cpu_count() or 4)
        ttk.Spinbox(r2, from_=1, to=32, textvariable=self.jobs_var, width=3).pack(side=tk.LEFT, padx=(0, 16))

        self.chk_bash = tk.BooleanVar(value=True)
        self.chk_valgrind = tk.BooleanVar(value=True)
        self.chk_malloc = tk.BooleanVar(value=True)
        self.chk_signals = tk.BooleanVar(value=True)

        ttk.Checkbutton(r2, text="Bash Compare", variable=self.chk_bash, style="TCheckbutton").pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(r2, text="Valgrind / FDs", variable=self.chk_valgrind, style="TCheckbutton").pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(r2, text="Malloc Faults", variable=self.chk_malloc, style="TCheckbutton").pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(r2, text="Signal Phase", variable=self.chk_signals, style="TCheckbutton").pack(side=tk.LEFT, padx=4)

        self.btn_run = ttk.Button(r2, text="▶  Run Selected (F5)", style="Accent.TButton", command=self.run_tests)
        self.btn_run.pack(side=tk.RIGHT, padx=(6, 0))

        self.progress_var = tk.DoubleVar(value=0)
        self.progress_bar = ttk.Progressbar(self.root, variable=self.progress_var, maximum=100, style="Horizontal.TProgressbar")
        self.progress_bar.pack(fill=tk.X, side=tk.TOP, padx=12, pady=2)

        shortcut_bar = ttk.Frame(self.root, style="Panel.TFrame", padding=(12, 4))
        shortcut_bar.pack(fill=tk.X, side=tk.TOP, padx=12, pady=(0, 4))
        legend_text = "Shortcuts:  [↑/↓] Navigate Tests  |  [←/→] Switch Inspector Tabs  |  [Space] Select/Deselect  |  [F5 / Ctrl+R] Run  |  [/ / Ctrl+F] Search"
        ttk.Label(shortcut_bar, text=legend_text, style="Panel.TFrame", foreground=COLOR_FG_MUTED, font=("Segoe UI", 8)).pack(side=tk.LEFT)

        paned = ttk.PanedWindow(self.root, orient=tk.HORIZONTAL)
        paned.pack(fill=tk.BOTH, expand=True, padx=12, pady=4)

        left_frame = ttk.Frame(paned, width=460)
        paned.add(left_frame, weight=1)

        sel_btn_frame = ttk.Frame(left_frame)
        sel_btn_frame.pack(fill=tk.X, side=tk.TOP, pady=(0, 6))

        ttk.Button(sel_btn_frame, text="Select All", command=lambda: self._set_all_selected(True)).pack(side=tk.LEFT, padx=(0, 2))
        ttk.Button(sel_btn_frame, text="Deselect All", command=lambda: self._set_all_selected(False)).pack(side=tk.LEFT, padx=2)
        ttk.Button(sel_btn_frame, text="Select Failed", command=self._select_failed_only).pack(side=tk.LEFT, padx=2)
        ttk.Button(sel_btn_frame, text="🔄 Reset Selected", command=self._reset_selected_tests).pack(side=tk.LEFT, padx=2)

        search_frame = ttk.Frame(left_frame)
        search_frame.pack(fill=tk.X, side=tk.TOP, pady=(0, 6))
        ttk.Label(search_frame, text="🔍").pack(side=tk.LEFT, padx=(0, 4))
        self.filter_var = tk.StringVar()
        self.filter_var.trace_add("write", lambda *args: self._populate_tree())
        self.search_entry = ttk.Entry(search_frame, textvariable=self.filter_var)
        self.search_entry.pack(fill=tk.X, expand=True, side=tk.LEFT)

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

        right_frame = ttk.Frame(paned)
        paned.add(right_frame, weight=2)

        edit_card = ttk.LabelFrame(right_frame, text="Manage Selected / Add Test Case", padding=8)
        edit_card.pack(fill=tk.X, side=tk.TOP, pady=(0, 8))

        ec_r1 = ttk.Frame(edit_card)
        ec_r1.pack(fill=tk.X, side=tk.TOP, pady=(0, 4))

        ttk.Label(ec_r1, text="Category:").pack(side=tk.LEFT, padx=(0, 4))
        self.edit_cat_var = tk.StringVar(value="Custom")
        ttk.Entry(ec_r1, textvariable=self.edit_cat_var, width=15).pack(side=tk.LEFT, padx=(0, 10))

        ttk.Label(ec_r1, text="Command:").pack(side=tk.LEFT, padx=(0, 4))
        self.edit_cmd_var = tk.StringVar()
        ttk.Entry(ec_r1, textvariable=self.edit_cmd_var).pack(side=tk.LEFT, fill=tk.X, expand=True)

        ec_r2 = ttk.Frame(edit_card)
        ec_r2.pack(fill=tk.X, side=tk.TOP)

        self.edit_bash_cmp_var = tk.BooleanVar(value=True)
        self.edit_flag_err_var = tk.BooleanVar(value=False)

        ttk.Checkbutton(ec_r2, text="Bash Compare", variable=self.edit_bash_cmp_var, style="TCheckbutton").pack(side=tk.LEFT, padx=(0, 10))
        ttk.Checkbutton(ec_r2, text="Flag Error", variable=self.edit_flag_err_var, style="TCheckbutton").pack(side=tk.LEFT, padx=(0, 10))

        ttk.Button(ec_r2, text="➕ Add as New", command=self._add_test_item).pack(side=tk.RIGHT, padx=(2, 0))
        ttk.Button(ec_r2, text="💾 Update Selected", command=self._update_test_item).pack(side=tk.RIGHT, padx=2)
        ttk.Button(ec_r2, text="🗑 Delete Selected", command=self._delete_test_item).pack(side=tk.RIGHT, padx=2)

        self.notebook = ttk.Notebook(right_frame)
        self.notebook.pack(fill=tk.BOTH, expand=True)

        self.txt_overview = self._create_dark_text_tab("Overview", self.notebook)
        self.txt_diff = self._create_dark_text_tab("STDOUT Output / Diff", self.notebook, mono=True)
        self.txt_stderr = self._create_dark_text_tab("STDERR Logs", self.notebook, mono=True)
        self.txt_env = self._create_dark_text_tab("Environment Variables", self.notebook, mono=True)
        self.txt_valgrind = self._create_dark_text_tab("Valgrind / FDs", self.notebook, mono=True)
        self.txt_malloc = self._create_dark_text_tab("Malloc Faults", self.notebook, mono=True)
        self.txt_signals = self._create_dark_text_tab("Signal Handling", self.notebook, mono=True)

        self.txt_diff.tag_config("add", background=COLOR_DIFF_ADD_BG, foreground=COLOR_DIFF_ADD_FG)
        self.txt_diff.tag_config("sub", background=COLOR_DIFF_SUB_BG, foreground=COLOR_DIFF_SUB_FG)
        self.txt_diff.tag_config("info", foreground=COLOR_ACCENT)

        self.txt_stderr.tag_config("add", background=COLOR_DIFF_ADD_BG, foreground=COLOR_DIFF_ADD_FG)
        self.txt_stderr.tag_config("sub", background=COLOR_DIFF_SUB_BG, foreground=COLOR_DIFF_SUB_FG)
        self.txt_stderr.tag_config("info", foreground=COLOR_ACCENT)

        self.txt_env.tag_config("add", background=COLOR_DIFF_ADD_BG, foreground=COLOR_DIFF_ADD_FG)
        self.txt_env.tag_config("sub", background=COLOR_DIFF_SUB_BG, foreground=COLOR_DIFF_SUB_FG)
        self.txt_env.tag_config("info", foreground=COLOR_ACCENT)

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

    def _browse_bash_binary(self):
        filename = filedialog.askopenfilename(title="Select Bash Executable")
        if filename:
            self.bash_path_var.set(filename)

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
        cleanup_test_artifacts()
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
            if column == "#1":
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
            self.edit_cat_var.set(test_item["cat"])
            self.edit_cmd_var.set(test_item["cmd"])
            self.edit_bash_cmp_var.set(test_item["bash_cmp"])
            self.edit_flag_err_var.set(test_item["flag_error"])
            self._update_inspector(test_item)

    def _set_all_selected(self, val):
        for t in self.tests_data:
            t["selected"] = val
        self._populate_tree()

    def _select_failed_only(self):
        for t in self.tests_data:
            t["selected"] = (t["status"] == "FAIL")
        self._populate_tree()

    def _add_test_item(self):
        cmd = self.edit_cmd_var.get().strip()
        cat = self.edit_cat_var.get().strip() or "Custom"
        if not cmd:
            return
        new_id = len(self.tests_data) + 1
        new_test = {
            "id": new_id,
            "cat": cat,
            "cmd": cmd,
            "bash_cmp": self.edit_bash_cmp_var.get(),
            "flag_error": self.edit_flag_err_var.get(),
            "selected": True,
            "status": "PENDING",
            "result": None
        }
        self.tests_data.append(new_test)
        save_tests_to_file(self.tests_data)
        self._populate_tree()

    def _update_test_item(self):
        sel = self.tree.selection()
        if not sel:
            return
        t_id = self.tree.item(sel[0])["values"][1]
        test_item = next((t for t in self.tests_data if t["id"] == t_id), None)
        if test_item:
            test_item["cat"] = self.edit_cat_var.get().strip() or "Custom"
            test_item["cmd"] = self.edit_cmd_var.get().strip()
            test_item["bash_cmp"] = self.edit_bash_cmp_var.get()
            test_item["flag_error"] = self.edit_flag_err_var.get()
            test_item["status"] = "PENDING"
            test_item["result"] = None
            save_tests_to_file(self.tests_data)
            self._populate_tree()
            self._update_inspector(test_item)

    def _delete_test_item(self):
        sel = self.tree.selection()
        if not sel:
            return
        t_id = self.tree.item(sel[0])["values"][1]
        self.tests_data = [t for t in self.tests_data if t["id"] != t_id]
        for idx, t in enumerate(self.tests_data):
            t["id"] = idx + 1
        save_tests_to_file(self.tests_data)
        self._populate_tree()

    def _update_inspector(self, test_item):
        res = test_item["result"]

        if not res:
            self._write_read_only_text(
                self.txt_overview,
                lambda: self.txt_overview.insert(tk.END, f"Command: {test_item['cmd']}\nStatus: {test_item['status']}\n\nRun test to inspect output.")
            )
            for txt in (self.txt_diff, self.txt_stderr, self.txt_env, self.txt_valgrind, self.txt_malloc, self.txt_signals):
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
                f"  Bash:      {res['bash_code']}",
                f"\nWorking Directory (CWD):",
                f"  Minishell: {res['ms_cwd'] if res['ms_cwd'] else '(Not Captured / Exited)'}",
                f"  Bash:      {res['bash_cwd'] if res['bash_cwd'] else '(Not Captured / Exited)'}"
            ]
            if res["failures"]:
                ov.append("\nFailures / Mismatches Detected:")
                for f in res["failures"]:
                    ov.append(f"  • {f}")
            self.txt_overview.insert(tk.END, "\n".join(ov))

        self._write_read_only_text(self.txt_overview, write_overview)

        # 2. STDOUT Tab
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

            err_diff = difflib.unified_diff(
                bash_e.splitlines(keepends=True),
                ms_e.splitlines(keepends=True),
                fromfile="bash stderr",
                tofile="minishell stderr"
            )
            err_diff_text = "".join(err_diff)

            self.txt_stderr.insert(tk.END, "=== UNIFIED STDERR DIFF (-Bash, +Minishell) ===\n", "info")
            if err_diff_text:
                for line in err_diff_text.splitlines(keepends=True):
                    if line.startswith("+"):
                        self.txt_stderr.insert(tk.END, line, "add")
                    elif line.startswith("-"):
                        self.txt_stderr.insert(tk.END, line, "sub")
                    elif line.startswith("@"):
                        self.txt_stderr.insert(tk.END, line, "info")
                    else:
                        self.txt_stderr.insert(tk.END, line)
            else:
                self.txt_stderr.insert(tk.END, "✔ STDERR matches Bash output perfectly.\n")

            err_txt = f"\n=== MINISHELL STDERR ===\n{ms_e}\n\n=== BASH STDERR ===\n{bash_e}"
            self.txt_stderr.insert(tk.END, err_txt)

        self._write_read_only_text(self.txt_stderr, write_stderr)

        # 4. Environment Variables Tab
        def write_env():
            ms_env_norm = normalize_env_output(res['ms_env']) if res['ms_env'] else "(empty)"
            bash_env_norm = normalize_env_output(res['bash_env']) if res['bash_env'] else "(empty)"

            self.txt_env.insert(tk.END, "=== UNIFIED ENVIRONMENT DIFF (-Bash, +Minishell) ===\n", "info")
            if res["env_diff_text"]:
                for line in res["env_diff_text"].splitlines(keepends=True):
                    if line.startswith("+"):
                        self.txt_env.insert(tk.END, line, "add")
                    elif line.startswith("-"):
                        self.txt_env.insert(tk.END, line, "sub")
                    elif line.startswith("@"):
                        self.txt_env.insert(tk.END, line, "info")
                    else:
                        self.txt_env.insert(tk.END, line)
            else:
                self.txt_env.insert(tk.END, "✔ Final Environment Variables match Bash perfectly.\n")

            self.txt_env.insert(tk.END, "\n=== MINISHELL ENVIRONMENT ===\n", "info")
            self.txt_env.insert(tk.END, ms_env_norm + "\n")

            self.txt_env.insert(tk.END, "\n=== BASH ENVIRONMENT ===\n", "info")
            self.txt_env.insert(tk.END, bash_env_norm + "\n")

        self._write_read_only_text(self.txt_env, write_env)

        # 5. Valgrind Tab
        self._write_read_only_text(self.txt_valgrind, lambda: self.txt_valgrind.insert(tk.END, res["valgrind_log"]))

        # 6. Malloc Tab
        self._write_read_only_text(self.txt_malloc, lambda: self.txt_malloc.insert(tk.END, res["malloc_log"]))

        # 7. Signal Handling Tab
        self._write_read_only_text(self.txt_signals, lambda: self.txt_signals.insert(tk.END, res["signal_log"]))

    def run_tests(self):
        if self.is_running:
            return

        ms_path = os.path.abspath(self.ms_path_var.get())
        if not os.path.exists(ms_path):
            messagebox.showerror("Error", f"Minishell binary '{ms_path}' not found.")
            return

        bash_path = self.bash_path_var.get().strip()
        if self.chk_bash.get() and not os.path.exists(bash_path) and not shutil.which(bash_path):
            messagebox.showerror("Error", f"Bash executable '{bash_path}' not found.")
            return

        selected_tests = [t for t in self.tests_data if t["selected"]]

        if not selected_tests:
            messagebox.showwarning("Warning", "No tests selected.")
            return

        cleanup_test_artifacts()

        for t in selected_tests:
            t["status"] = "PENDING"
            t["result"] = None
        self._populate_tree()

        self.is_running = True
        self.btn_run.config(state=tk.DISABLED)
        self.progress_var.set(0)

        opts = {
            "skip_bash": not self.chk_bash.get(),
            "skip_valgrind": not self.chk_valgrind.get(),
            "skip_malloc": not self.chk_malloc.get(),
            "skip_signals": not self.chk_signals.get()
        }

        threading.Thread(
            target=self._worker_thread,
            args=(selected_tests, ms_path, bash_path, self.env_mgr.hook_so, self.env_mgr.supp_file, opts, self.jobs_var.get()),
            daemon=True
        ).start()

    def _worker_thread(self, tests, ms_path, bash_path, hook_so_path, supp_file_path, opts, num_threads):
        total = len(tests)
        completed = 0

        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            future_to_test = {
                executor.submit(execute_single_test, t, ms_path, bash_path, hook_so_path, supp_file_path, opts): t
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
                    cleanup_test_artifacts()
        except queue.Empty:
            pass

        self.root.after(100, self._poll_queue)


def main():
    root = tk.Tk()
    app = MinishellTestGUI(root)
    root.mainloop()

if __name__ == "__main__":
    main()
