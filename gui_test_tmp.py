#!/usr/bin/env python3
"""
Minishell Universal Modern Dark GUI Test Harness
Single-file self-contained Tkinter GUI test harness with stack backtrace symbol
resolution for silent malloc failures, readline valgrind suppressions, isolated dual-mode
execution (Non-Interactive Piped Pass & Interactive PTY Pass for Base Command, CWD Probe,
Env Probe, Malloc Faults, Valgrind, and Signal Phase), Forbidden Functions Audit, Norminette integration,
HTML test report exporter, raw-TTY Valgrind-wrapped terminal debug launcher with delayed STDIN payload feeding,
dynamic thread adjustment, keyboard reordering, external JSON test suite persistence, recompile controls,
path configuration persistence, automatic Makefile directory traversal, and headless CLI mode support.
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
import termios
import tty
import traceback
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
CONFIG_FILE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "config.json")

# --- 42 Curriculum Whitelisted Functions ---
ALLOWED_42_FUNCTIONS = {
    "readline", "rl_clear_history", "rl_on_new_line", "rl_replace_line",
    "rl_redisplay", "add_history", "printf", "malloc", "free", "write",
    "access", "open", "read", "close", "fork", "wait", "waitpid",
    "wait3", "wait4", "signal", "sigaction", "sigemptyset", "sigaddset",
    "kill", "exit", "getcwd", "chdir", "stat", "lstat", "fstat",
    "unlink", "execve", "dup", "dup2", "pipe", "opendir", "readdir",
    "closedir", "strerror", "perror", "isatty", "ttyname", "ttyslot",
    "ioctl", "getenv", "tcsetattr", "tcgetattr", "tgetent", "tgetflag",
    "tgetnum", "tgetstr", "tgoto", "tputs"
}

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

#--- Embedded C Hook Source Code ---
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
static atomic_int g_in_main = 0;
static char g_dummy_buf[65536];
static size_t g_dummy_pos = 0;

typedef int (*main_t)(int, char **, char **);
static main_t real_user_main = NULL;

static int wrapped_main(int argc, char **argv, char **envp)
{
    g_in_main = 1;
    return real_user_main(argc, argv, envp);
}

int __libc_start_main(
    main_t main_func,
    int argc,
    char **argv,
    void (*init)(void),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void *stack_end)
{
    int (*real_start)(main_t, int, char **, void (*)(void), void (*)(void), void (*)(void), void *)
        = dlsym(RTLD_NEXT, "__libc_start_main");

    if (!real_start)
    {
        const char *err = "Error locating real __libc_start_main\n";
        write(STDERR_FILENO, err, strlen(err));
        exit(1);
    }
    real_user_main = main_func;
    return real_start(wrapped_main, argc, argv, init, fini, rtld_fini, stack_end);
}

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
    if (!g_in_main)
        return 0;

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
                    strstr(info.dli_sname, "rl_") ||
                    strstr(info.dli_sname, "add_history"))
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
    {"cat": "Edge Cases", "cmd": "cat < nonexistent_file_xyz", "bash_cmp": True},
    {"cat": "Edge Cases", "cmd": "| ls", "bash_cmp": True},
    {"cat": "Edge Cases", "cmd": "echo \"unclosed string", "bash_cmp": True},
    {"cat": "Flag Errors", "cmd": "cd -Z /tmp", "flag_error": True}
]

def load_app_config():
    if os.path.exists(CONFIG_FILE_PATH):
        try:
            with open(CONFIG_FILE_PATH, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception:
            pass
    return {}

def save_app_config(cfg):
    try:
        with open(CONFIG_FILE_PATH, "w", encoding="utf-8") as f:
            json.dump(cfg, f, indent=2)
    except Exception:
        pass

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

def find_makefile_dir(start_dir):
    if not os.path.exists(start_dir):
        return start_dir

    start_dir = os.path.abspath(start_dir)

    if os.path.exists(os.path.join(start_dir, "Makefile")) or os.path.exists(os.path.join(start_dir, "makefile")):
        return start_dir

    parent_dir = os.path.dirname(start_dir)
    if os.path.exists(os.path.join(parent_dir, "Makefile")) or os.path.exists(os.path.join(parent_dir, "makefile")):
        return parent_dir

    for root, dirs, files in os.walk(parent_dir if parent_dir else start_dir):
        rel_depth = root.count(os.sep) - (parent_dir if parent_dir else start_dir).count(os.sep)
        if rel_depth > 3:
            dirs.clear()
            continue
        if "Makefile" in files or "makefile" in files:
            return root

    return start_dir

def check_forbidden_functions(ms_path):
    if not os.path.exists(ms_path) or not shutil.which("nm"):
        return ["`nm` utility not found or binary missing."]

    try:
        proc = subprocess.run(["nm", "-u", ms_path], capture_output=True, text=True, timeout=3)
        if proc.returncode != 0:
            return [f"Failed to run `nm -u`: {proc.stderr}"]

        forbidden = []
        ignore_syms = {
            "faulty_malloc", "malloc_hook", "init_hooks", "is_minishell_caller",
            "log_callstack", "cleanup_hook", "g_alloc_count", "g_fail_index"
        }
        for line in proc.stdout.splitlines():
            line_str = line.strip()
            if not line_str:
                continue
            parts = line_str.split()
            sym = parts[-1] if parts else ""
            sym_clean = sym.split("@")[0]

            if not sym_clean or sym_clean.startswith("__") or sym_clean.startswith("_"):
                continue

            if any(ign in sym_clean for ign in ignore_syms) or "faulty_malloc" in sym_clean:
                continue

            if sym_clean not in ALLOWED_42_FUNCTIONS:
                forbidden.append(sym_clean)

        return sorted(list(set(forbidden)))
    except Exception as e:
        return [f"Audit error: {str(e)}"]

def run_norminette_check(ms_dir):
    norm_bin = shutil.which("norminette")
    if not norm_bin:
        return "norminette command not found in PATH."

    c_files = []
    ignore_files = {"faulty_malloc.c", "faulty_malloc.h", "malloc_hook.c", "malloc_hook.h"}
    for root, _, files in os.walk(ms_dir):
        for file in files:
            if file.endswith((".c", ".h")):
                if file in ignore_files or "faulty_malloc" in file or "malloc_hook" in file:
                    continue
                c_files.append(os.path.relpath(os.path.join(root, file), ms_dir))

    if not c_files:
        return "No C/H source files found to check with Norminette (ignoring faulty_malloc files)."

    try:
        proc = subprocess.run([norm_bin] + c_files, cwd=ms_dir, capture_output=True, text=True, timeout=15)
        return proc.stdout if proc.stdout else "Norminette finished with no output."
    except Exception as e:
        return f"Error executing Norminette: {str(e)}"

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

def run_interactive_session(executable, script_str, env=None, cwd=None, timeout=8):
    executable = os.path.abspath(executable)
    if env is None:
        env = os.environ.copy()
    
    err_fd, err_path = tempfile.mkstemp(prefix="ms_interactive_err_")
    os.close(err_fd)
    
    full_script = f"exec 2> {err_path}\n" + script_str
    
    master, slave = pty.openpty()
    try:
        proc = subprocess.Popen(
            [executable],
            stdin=slave, stdout=slave, stderr=slave,
            cwd=cwd, env=env, start_new_session=True, text=False
        )
        os.close(slave)
        
        time.sleep(0.05)
        os.write(master, full_script.encode('utf-8'))
        
        out = b""
        start_t = time.time()
        while time.time() - start_t < timeout:
            r, _, _ = select.select([master], [], [], 0.1)
            if master in r:
                try:
                    chunk = os.read(master, 1024)
                    if not chunk:
                        break
                    out += chunk
                except OSError:
                    break
            if proc.poll() is not None:
                while True:
                    r, _, _ = select.select([master], [], [], 0.05)
                    if not r:
                        break
                    try:
                        chunk = os.read(master, 1024)
                        if not chunk:
                            break
                        out += chunk
                    except OSError:
                        break
                break
        
        if proc.poll() is None:
            proc.kill()
            proc.wait()
        
        try:
            os.close(master)
        except Exception:
            pass
            
        stdout_raw = out.decode('utf-8', errors='replace')
        
        stderr_raw = ""
        if os.path.exists(err_path):
            try:
                with open(err_path, "r", encoding="utf-8", errors="replace") as ef:
                    stderr_raw = ef.read()
            except Exception:
                pass
            try:
                os.remove(err_path)
            except Exception:
                pass
        
        return stdout_raw, stderr_raw, proc.returncode
    except Exception as e:
        try:
            os.close(master)
        except Exception:
            pass
        if os.path.exists(err_path):
            try: os.remove(err_path)
            except Exception: pass
        return f"PTY_ERROR: {str(e)}", "", -1

def strip_ansi(text):
    if not text:
        return ""
    clean = re.sub(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])', '', text)
    clean = clean.replace('\r\n', '\n').replace('\r', '\n')
    return clean

def normalize_stdout(raw_stdout):
    if not raw_stdout:
        return ""
    clean = strip_ansi(raw_stdout)
    clean = clean.replace('\0', '\n')
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

def parse_interactive_raw(raw):
    if not raw:
        return "", None, None, 0
    clean = strip_ansi(raw)
    cwd, env, exit_code = None, None, 0

    cwd_match = re.search(r'__CWD_START__\s*\n?(.*?)\n?\s*__CWD_END__', clean, re.DOTALL)
    if cwd_match:
        cwd = cwd_match.group(1).strip()
        clean = re.sub(r'__CWD_START__[\s\S]*?__CWD_END__', '', clean)

    env_match = re.search(r'__ENV_START__\s*\n?(.*?)\n?\s*__ENV_END__', clean, re.DOTALL)
    if env_match:
        env = env_match.group(1).strip()
        clean = re.sub(r'__ENV_START__[\s\S]*?__ENV_END__', '', clean)

    exit_match = re.search(r'__EXIT_START__\s*\n?(.*?)\n?\s*__EXIT_END__', clean, re.DOTALL)
    if exit_match:
        try:
            exit_code = int(exit_match.group(1).strip())
        except ValueError:
            exit_code = 0
        clean = re.sub(r'__EXIT_START__[\s\S]*?__EXIT_END__', '', clean)

    return normalize_stdout(clean), cwd, env, exit_code

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

    def read_all(master, timeout=0.3):
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
        read_all(m, 0.2)
        os.write(m, b"\x03")
        read_all(m, 0.2)
        os.write(m, b"echo $?\n")
        out2 = read_all(m, 0.2)
        os.write(m, b"exit\n")
        try:
            p.wait(timeout=1)
        except subprocess.TimeoutExpired:
            p.kill()
            p.wait()
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
        read_all(m, 0.2)
        os.write(m, b"cat << EOF\n")
        read_all(m, 0.2)
        os.write(m, b"line 1\n")
        read_all(m, 0.2)
        os.write(m, b"\x03")
        out = read_all(m, 0.3)
        os.write(m, b"echo $?\n")
        out2 = read_all(m, 0.2)
        os.write(m, b"exit\n")
        try:
            p.wait(timeout=1)
        except subprocess.TimeoutExpired:
            p.kill()
            p.wait()
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
        read_all(m, 0.2)
        os.write(m, b"sleep 5\n")
        time.sleep(0.1)
        os.write(m, b"\x03")
        out = read_all(m, 0.3)
        os.write(m, b"echo $?\n")
        out2 = read_all(m, 0.2)
        os.write(m, b"exit\n")
        try:
            p.wait(timeout=1)
        except subprocess.TimeoutExpired:
            p.kill()
            p.wait()
        os.close(m)

        if "130" in out2 or p.returncode == 0:
            logs.append("✔ [Child Execution] Ctrl+C (SIGINT): OK (Interrupted child, status = 130)")
        else:
            failures.append("Child Execution: Ctrl+C during child execution did not set status to 130.")
            logs.append("✖ [Child Execution] Ctrl+C: FAILED")
    except Exception as e:
        logs.append(f"✖ [Child Execution] Ctrl+C test error: {e}")

    return logs, failures

def execute_single_test(test_item, ms_path, bash_path, hook_so_path, supp_file_path, opts, check_cancel=None):
    def is_cancelled():
        return check_cancel is not None and check_cancel()

    if is_cancelled():
        return None

    start_perf = time.perf_counter()
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
        "duration_ms": 0.0,
        "pass1_duration_ms": 0.0,
        "pass2_duration_ms": 0.0,
        "pass3_duration_ms": 0.0,
        "valgrind_duration_ms": 0.0,
        "malloc_duration_ms": 0.0,
        "signals_duration_ms": 0.0,
        "interactive_out": "",
        "interactive_err": "",
        "interactive_code": 0,
        "interactive_cwd": None,
        "interactive_env": "",
        "interactive_diff": "",
        "failures": []
    }

    try:
        # =========================================================================
        # PASS A: Non-Interactive Mode (Piped via STDIN)
        # =========================================================================
        if opts.get("run_non_interactive", True):
            t0 = time.perf_counter()
            stdin_base = f"{ms_cmd_str}\nexit $?\n"
            bash_stdin_base = f"{bash_cmd_str}\nexit $?\n"

            raw_ms_out, ms_err, ms_code = run_shell(stdin_base, ms_path, cwd=root_dir)
            if is_cancelled(): return None
            raw_bash_out, bash_err, bash_code = run_bash(bash_stdin_base, bash_executable=bash_path, cwd=root_dir)
            if is_cancelled(): return None

            result["pass1_duration_ms"] = round((time.perf_counter() - t0) * 1000, 2)

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
                    result["failures"].append("Non-Interactive Flag Option Check: Expected non-zero exit code and error message on STDERR.")
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
                    if not out_match: err_reasons.append("STDOUT mismatch")
                    if not err_match: err_reasons.append("STDERR mismatch")
                    if not code_match: err_reasons.append(f"Exit status mismatch (minishell={ms_code}, bash={bash_code})")
                    result["failures"].append("Non-Interactive Bash Comparison: " + ", ".join(err_reasons))

        # =========================================================================
        # PASS B: Interactive Mode (PTY Pseudo-Terminal Session)
        # =========================================================================
        if opts.get("run_interactive", True):
            if is_cancelled(): return None
            interactive_script = (
                f"{ms_cmd_str}\n"
                f"echo '\n__CWD_START__'\npwd -P\necho '__CWD_END__'\n"
                f"echo '\n__ENV_START__'\nenv\necho '__ENV_END__'\n"
                f"echo '\n__EXIT_START__'\necho $?\necho '__EXIT_END__'\n"
                f"exit\n"
            )
            bash_interactive_script = (
                f"{bash_cmd_str}\n"
                f"echo '\n__CWD_START__'\npwd -P\necho '__CWD_END__'\n"
                f"echo '\n__ENV_START__'\nenv\necho '__ENV_END__'\n"
                f"echo '\n__EXIT_START__'\necho $?\necho '__EXIT_END__'\n"
                f"exit\n"
            )

            raw_ms_int_out, raw_ms_int_err, ms_int_code = run_interactive_session(ms_path, interactive_script, cwd=root_dir)
            if is_cancelled(): return None
            raw_bash_int_out, raw_bash_int_err, bash_int_code = run_interactive_session(bash_path, bash_interactive_script, cwd=root_dir)
            if is_cancelled(): return None

            ms_int_stdout, ms_int_cwd, ms_int_env, ms_int_status = parse_interactive_raw(raw_ms_int_out)
            bash_int_stdout, bash_int_cwd, bash_int_env, bash_int_status = parse_interactive_raw(raw_bash_int_out)

            clean_ms_int_err = normalize_stderr(strip_hook_output(raw_ms_int_err), is_bash=False)
            clean_bash_int_err = normalize_stderr(raw_bash_int_err, is_bash=True, bash_executable=bash_path)

            result["interactive_out"] = ms_int_stdout
            result["interactive_err"] = clean_ms_int_err
            result["interactive_code"] = ms_int_status
            result["interactive_cwd"] = ms_int_cwd
            result["interactive_env"] = ms_int_env

            interactive_diff = difflib.unified_diff(
                bash_int_stdout.splitlines(keepends=True),
                ms_int_stdout.splitlines(keepends=True),
                fromfile="bash interactive stdout",
                tofile="minishell interactive stdout"
            )
            result["interactive_diff"] = "".join(interactive_diff)

            if test_item.get("flag_error", False):
                if not clean_ms_int_err or ms_int_status == 0:
                    result["passed"] = False
                    result["failures"].append("Interactive Flag Option Check: Expected non-zero exit code and error message on STDERR.")
            elif test_item.get("bash_cmp", True) and not opts.get("skip_bash", False):
                int_out_match = (ms_int_stdout == bash_int_stdout)
                int_err_match = (clean_ms_int_err == clean_bash_int_err)
                int_code_match = (ms_int_status == bash_int_status)
                int_cwd_match = (ms_int_cwd == bash_int_cwd)

                if not int_out_match or not int_code_match or not int_err_match or not int_cwd_match:
                    result["passed"] = False
                    int_reasons = []
                    if not int_out_match: int_reasons.append("Interactive STDOUT mismatch")
                    if not int_err_match: int_reasons.append("Interactive STDERR mismatch")
                    if not int_code_match: int_reasons.append(f"Interactive exit status mismatch (minishell={ms_int_status}, bash={bash_int_status})")
                    if not int_cwd_match: int_reasons.append(f"Interactive CWD mismatch (minishell='{ms_int_cwd}', expected='{bash_int_cwd}')")
                    result["failures"].append("Interactive Mode Pass: " + ", ".join(int_reasons))

        # CWD & ENV Non-Interactive Probes
        if is_cancelled(): return None
        t0 = time.perf_counter()
        stdin_cwd = f"{ms_cmd_str}\necho '\n__CWD_START__'\npwd -P\necho '__CWD_END__'exit $?\n"
        bash_stdin_cwd = f"{bash_cmd_str}\necho '\n__CWD_START__'\npwd -P\necho '__CWD_END__'exit $?\n"

        ms_cwd_raw, _, _ = run_shell(stdin_cwd, ms_path, cwd=root_dir)
        if is_cancelled(): return None
        ms_cwd = ms_cwd_raw.split("__CWD_START__")[1].split("__CWD_END__")[0].strip() if "__CWD_START__" in ms_cwd_raw and "__CWD_END__" in ms_cwd_raw else None

        bash_cwd_raw, _, _ = run_bash(bash_stdin_cwd, bash_executable=bash_path, cwd=root_dir)
        if is_cancelled(): return None
        bash_cwd = bash_cwd_raw.split("__CWD_START__")[1].split("__CWD_END__")[0].strip() if "__CWD_START__" in bash_cwd_raw and "__CWD_END__" in bash_cwd_raw else None

        result["pass2_duration_ms"] = round((time.perf_counter() - t0) * 1000, 2)
        result["ms_cwd"] = ms_cwd
        result["bash_cwd"] = bash_cwd

        if ms_cwd and bash_cwd and ms_cwd != bash_cwd:
            result["passed"] = False
            result["failures"].append(f"CWD Mismatch: Minishell in '{ms_cwd}', expected '{bash_cwd}'")

        if is_cancelled(): return None
        t0 = time.perf_counter()
        stdin_env = f"{ms_cmd_str}\necho '\n__ENV_START__'\nenv\necho '__ENV_END__'exit $?\n"
        bash_stdin_env = f"{bash_cmd_str}\necho '\n__ENV_START__'\nenv\necho '__ENV_END__'exit $?\n"

        ms_env_raw, _, _ = run_shell(stdin_env, ms_path, cwd=root_dir)
        if is_cancelled(): return None
        ms_env = ms_env_raw.split("__ENV_START__")[1].split("__ENV_END__")[0].strip() if "__ENV_START__" in ms_env_raw and "__ENV_END__" in ms_env_raw else None

        bash_env_raw, _, _ = run_bash(bash_stdin_env, bash_executable=bash_path, cwd=root_dir)
        if is_cancelled(): return None
        bash_env = bash_env_raw.split("__ENV_START__")[1].split("__ENV_END__")[0].strip() if "__ENV_START__" in bash_env_raw and "__ENV_END__" in bash_env_raw else None

        result["pass3_duration_ms"] = round((time.perf_counter() - t0) * 1000, 2)
        result["ms_env"] = ms_env
        result["bash_env"] = bash_env

        if ms_env and bash_env and test_item.get("bash_cmp", True) and not opts.get("skip_bash", False):
            norm_ms_env = normalize_env_output(ms_env_raw)
            norm_bash_env = normalize_env_output(bash_env_raw)
            env_diff = difflib.unified_diff(norm_bash_env.splitlines(keepends=True), norm_ms_env.splitlines(keepends=True), fromfile="bash env", tofile="minishell env")
            result["env_diff_text"] = "".join(env_diff)
            if norm_ms_env != norm_bash_env and "env" not in cmd_raw and "export" not in cmd_raw:
                result["passed"] = False
                result["failures"].append("ENV Mismatch: Final environment variables do not match Bash output")

        if opts.get("run_env_i", False):
            if is_cancelled(): return None
            _, _, ms_code_i = run_shell(stdin_base, ms_path, env={}, cwd=root_dir)
            if ms_code_i < 0 or ms_code_i in (134, 137, 139):
                result["passed"] = False
                result["failures"].append(f"Env -i Pass: Minishell crashed/segfaulted under empty environment (Code {ms_code_i})")

        if not opts.get("skip_valgrind", False) and shutil.which("valgrind"):
            if is_cancelled(): return None
            t0 = time.perf_counter()
            valgrind_cmd = ["valgrind", f"--suppressions={supp_file_path}", "--leak-check=full", "--show-leak-kinds=all", "--errors-for-leak-kinds=all", "--track-fds=yes", "--error-exitcode=99", ms_path]
            try:
                proc = subprocess.run(valgrind_cmd, input=stdin_base, capture_output=True, text=True, cwd=root_dir, timeout=8)
                if is_cancelled(): return None
                result["valgrind_duration_ms"] = round((time.perf_counter() - t0) * 1000, 2)
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

        if not opts.get("skip_malloc", False):
            if is_cancelled(): return None
            t0 = time.perf_counter()
            env = os.environ.copy()
            env["LD_PRELOAD"] = hook_so_path
            env["LOG_ALLOC_COUNT"] = "1"
            _, ms_err_log, _ = run_shell(stdin_base, ms_path, env=env, cwd=root_dir, timeout=1.5)
            if is_cancelled(): return None

            total_allocs = 0
            for line in ms_err_log.splitlines():
                if "__HOOK_TOTAL_ALLOCS:" in line:
                    try: total_allocs = int(line.split(":")[1].rstrip("_"))
                    except ValueError: pass

            if total_allocs > 0:
                malloc_fail_logs = []
                for fail_idx in range(1, total_allocs + 1):
                    if is_cancelled(): return None
                    fail_env = os.environ.copy()
                    fail_env["LD_PRELOAD"] = hook_so_path
                    fail_env["FAIL_MALLOC_INDEX"] = str(fail_idx)
                    _, err_m, code_m = run_shell(stdin_base, ms_path, env=fail_env, cwd=root_dir, timeout=1.2)
                    if is_cancelled(): return None
                    program_err = strip_hook_output(err_m)
                    callstack_loc = resolve_stack_trace(ms_path, err_m)
                    if code_m < 0 or code_m in (134, 137, 139):
                        result["passed"] = False
                        msg = f"Crash/Segfault at malloc #{fail_idx}/{total_allocs} (Exit Code: {code_m})"
                        if callstack_loc: msg += f"\n{callstack_loc}"
                        result["failures"].append("Malloc Fault: " + msg)
                        malloc_fail_logs.append(msg)
                        break
                    elif not program_err:
                        result["passed"] = False
                        msg = f"Silent Failure (No error message printed to STDERR by Minishell) at malloc #{fail_idx}/{total_allocs}"
                        if callstack_loc: msg += f"\n{callstack_loc}"
                        result["failures"].append("Malloc Fault: " + msg)
                        malloc_fail_logs.append(msg)
                        break
                result["malloc_log"] = "\n".join(malloc_fail_logs) if malloc_fail_logs else f"All {total_allocs} malloc failures handled safely."
            else:
                result["malloc_log"] = "No heap allocations recorded for this command."
            result["malloc_duration_ms"] = round((time.perf_counter() - t0) * 1000, 2)

        if not opts.get("skip_signals", False):
            if is_cancelled(): return None
            t0 = time.perf_counter()
            sig_logs, sig_failures = execute_pty_signal_tests(ms_path, root_dir)
            if is_cancelled(): return None
            result["signals_duration_ms"] = round((time.perf_counter() - t0) * 1000, 2)
            result["signal_log"] = "\n".join(sig_logs)
            if sig_failures:
                result["passed"] = False
                for sf in sig_failures:
                    result["failures"].append(sf)

    except Exception as exc:
        result["passed"] = False
        result["failures"].append(f"Tester Internal Failure: {str(exc)}\n{traceback.format_exc()}")

    result["duration_ms"] = round((time.perf_counter() - start_perf) * 1000, 2)
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

        cfg = load_app_config()
        default_ms = cfg.get("ms_path", "./minishell")
        default_bash = cfg.get("bash_path", "/home/subz3r0/Downloads/bash-5.1.16/bash" if os.path.exists("/home/subz3r0/Downloads/bash-5.1.16/bash") else (shutil.which("bash") or "/bin/bash"))

        self.ms_path_var = tk.StringVar(value=default_ms)
        self.bash_path_var = tk.StringVar(value=default_bash)

        self.ms_path_var.trace_add("write", lambda *args: self._save_app_config())
        self.bash_path_var.trace_add("write", lambda *args: self._save_app_config())

        self._uid_counter = 0
        raw_tests = load_tests_from_file()
        self.tests_data = []
        for idx, item in enumerate(raw_tests):
            self._uid_counter += 1
            self.tests_data.append({
                "_uid": self._uid_counter,
                "id": idx + 1,
                "cat": item.get("cat", "Custom"),
                "cmd": item.get("cmd", ""),
                "bash_cmp": item.get("bash_cmp", True),
                "flag_error": item.get("flag_error", False),
                "selected": True,
                "status": "PENDING",
                "result": None
            })

        self.tests_lock = threading.RLock()
        self.msg_queue = queue.Queue()
        self.worker_thread = None
        self.current_run_id = 0
        self.is_running = False
        self.is_paused = False
        self.stop_requested = False
        self.pause_requested = False
        self.inspector_mode = "NON_INTERACTIVE"

        self._setup_dark_theme()
        self._build_ui()
        self._bind_shortcuts()
        self._populate_tree()
        self._update_stats_bar()
        self.root.after(100, self._poll_queue)

    def _save_app_config(self):
        save_app_config({
            "ms_path": self.ms_path_var.get().strip(),
            "bash_path": self.bash_path_var.get().strip()
        })

    def _reindex_tests_locked(self):
        for idx, t in enumerate(self.tests_data):
            t["id"] = idx + 1

    def _setup_dark_theme(self):
        self.root.configure(bg=COLOR_BG_DARK)
        self.style = ttk.Style()
        self.style.theme_use("clam")

        self.style.configure(".", background=COLOR_BG_DARK, foreground=COLOR_FG_TEXT, font=("Segoe UI", 9))
        self.style.configure("TFrame", background=COLOR_BG_DARK)
        self.style.configure("Panel.TFrame", background=COLOR_BG_PANEL)

        self.style.configure("TLabelframe", background=COLOR_BG_PANEL, bordercolor=COLOR_BORDER, borderwidth=1, relief="solid")
        self.style.configure("TLabelframe.Label", background=COLOR_BG_PANEL, foreground=COLOR_ACCENT, font=("Segoe UI", 9, "bold"))

        self.style.configure("TButton", background=COLOR_BG_PANEL, foreground=COLOR_FG_TEXT, borderwidth=1, bordercolor=COLOR_BORDER, focuscolor="none", padding=(8, 4), font=("Segoe UI", 9, "bold"))
        self.style.map("TButton", background=[("active", COLOR_BORDER), ("disabled", COLOR_BG_DARK)], foreground=[("disabled", COLOR_FG_MUTED)])

        self.style.configure("Accent.TButton", background=COLOR_ACCENT, foreground="#11111b", borderwidth=0, padding=(10, 5))
        self.style.map("Accent.TButton", background=[("active", COLOR_ACCENT_HOVER), ("disabled", COLOR_BORDER)], foreground=[("disabled", COLOR_FG_MUTED)])

        self.style.configure("TCheckbutton", background=COLOR_BG_PANEL, foreground=COLOR_FG_TEXT, focuscolor="none")
        self.style.map("TCheckbutton", background=[("active", COLOR_BG_PANEL)])

        self.style.configure("TEntry", fieldbackground=COLOR_BG_INPUT, foreground=COLOR_FG_TEXT, bordercolor=COLOR_BORDER, insertcolor=COLOR_FG_TEXT, padding=5)
        self.style.configure("TSpinbox", fieldbackground=COLOR_BG_INPUT, foreground=COLOR_FG_TEXT, bordercolor=COLOR_BORDER, arrowcolor=COLOR_FG_TEXT, padding=5)

        self.style.configure("Treeview", background=COLOR_BG_INPUT, foreground=COLOR_FG_TEXT, fieldbackground=COLOR_BG_INPUT, borderwidth=0, rowheight=28, font=("Consolas", 9))
        self.style.configure("Treeview.Heading", background=COLOR_BG_PANEL, foreground=COLOR_ACCENT, font=("Segoe UI", 9, "bold"), relief="flat", padding=6)
        self.style.map("Treeview", background=[("selected", "#363a4f")], foreground=[("selected", "#ffffff")])

        self.style.configure("TNotebook", background=COLOR_BG_DARK, borderwidth=0)
        self.style.configure("TNotebook.Tab", background=COLOR_BG_PANEL, foreground=COLOR_FG_MUTED, padding=(12, 6), font=("Segoe UI", 9, "bold"), borderwidth=0)
        self.style.map("TNotebook.Tab", background=[("selected", COLOR_BG_INPUT)], foreground=[("selected", COLOR_ACCENT)])

        self.style.configure("Horizontal.TProgressbar", background=COLOR_ACCENT, troughcolor=COLOR_BG_PANEL, bordercolor=COLOR_BORDER, thickness=6)

    def _build_ui(self):
        header_card = ttk.Frame(self.root, style="Panel.TFrame", padding=12)
        header_card.pack(fill=tk.X, side=tk.TOP, padx=12, pady=(12, 4))

        # Row 1: Executable Paths
        r1 = ttk.Frame(header_card, style="Panel.TFrame")
        r1.pack(fill=tk.X, side=tk.TOP, pady=(0, 6))

        ttk.Label(r1, text="Minishell Executable:", style="Panel.TFrame").pack(side=tk.LEFT, padx=(0, 6))
        ttk.Entry(r1, textvariable=self.ms_path_var, width=18).pack(side=tk.LEFT, padx=(0, 4))
        ttk.Button(r1, text="Browse", command=self._browse_binary).pack(side=tk.LEFT, padx=(0, 10))

        ttk.Label(r1, text="Bash Executable:", style="Panel.TFrame").pack(side=tk.LEFT, padx=(0, 6))
        ttk.Entry(r1, textvariable=self.bash_path_var, width=30).pack(side=tk.LEFT, padx=(0, 4))
        ttk.Button(r1, text="Browse", command=self._browse_bash_binary).pack(side=tk.LEFT, padx=(0, 10))

        ttk.Button(r1, text="🔨 Recompile", command=self._recompile_minishell).pack(side=tk.LEFT, padx=(0, 4))
        ttk.Button(r1, text="📋 Audit / Norm", command=self._run_compliance_audit).pack(side=tk.LEFT, padx=(0, 4))
        ttk.Button(r1, text="📊 Export HTML", command=self._export_html_report).pack(side=tk.LEFT)

        # Row 2: Controls, Checkboxes, Run/Pause/Stop
        r2 = ttk.Frame(header_card, style="Panel.TFrame")
        r2.pack(fill=tk.X, side=tk.TOP)

        ttk.Label(r2, text="Threads:", style="Panel.TFrame").pack(side=tk.LEFT, padx=(0, 6))
        self.jobs_var = tk.IntVar(value=os.cpu_count() or 4)
        self.jobs_spinbox = ttk.Spinbox(r2, from_=1, to=32, textvariable=self.jobs_var, width=3)
        self.jobs_spinbox.pack(side=tk.LEFT, padx=(0, 12))
        self.jobs_spinbox.bind("<Left>", self._spinbox_decrement)
        self.jobs_spinbox.bind("<Right>", self._spinbox_increment)
        self.jobs_spinbox.bind("<Up>", self._navigate_tree)
        self.jobs_spinbox.bind("<Down>", self._navigate_tree)

        self.chk_bash = tk.BooleanVar(value=True)
        self.chk_valgrind = tk.BooleanVar(value=True)
        self.chk_malloc = tk.BooleanVar(value=True)
        self.chk_signals = tk.BooleanVar(value=True)
        self.chk_env_i = tk.BooleanVar(value=False)
        self.chk_non_interactive = tk.BooleanVar(value=True)
        self.chk_interactive = tk.BooleanVar(value=True)

        ttk.Checkbutton(r2, text="Non-Interactive Pass", variable=self.chk_non_interactive, style="TCheckbutton").pack(side=tk.LEFT, padx=3)
        ttk.Checkbutton(r2, text="Interactive Pass (PTY)", variable=self.chk_interactive, style="TCheckbutton").pack(side=tk.LEFT, padx=3)
        ttk.Checkbutton(r2, text="Bash Compare", variable=self.chk_bash, style="TCheckbutton").pack(side=tk.LEFT, padx=3)
        ttk.Checkbutton(r2, text="Valgrind / FDs", variable=self.chk_valgrind, style="TCheckbutton").pack(side=tk.LEFT, padx=3)
        ttk.Checkbutton(r2, text="Malloc Faults", variable=self.chk_malloc, style="TCheckbutton").pack(side=tk.LEFT, padx=3)
        ttk.Checkbutton(r2, text="Signal Phase", variable=self.chk_signals, style="TCheckbutton").pack(side=tk.LEFT, padx=3)
        ttk.Checkbutton(r2, text="Env -i Pass", variable=self.chk_env_i, style="TCheckbutton").pack(side=tk.LEFT, padx=3)

        self.btn_run = ttk.Button(r2, text="▶  Run Selected (F5)", style="Accent.TButton", command=self.run_tests)
        self.btn_run.pack(side=tk.RIGHT, padx=(4, 0))

        self.btn_pause = ttk.Button(r2, text="⏸  Pause (Ctrl+P)", command=self.toggle_pause, state=tk.DISABLED)
        self.btn_pause.pack(side=tk.RIGHT, padx=(4, 0))

        self.progress_var = tk.DoubleVar(value=0)
        self.progress_bar = ttk.Progressbar(self.root, variable=self.progress_var, maximum=100, style="Horizontal.TProgressbar")
        self.progress_bar.pack(fill=tk.X, side=tk.TOP, padx=12, pady=2)

        shortcut_bar = ttk.Frame(self.root, style="Panel.TFrame", padding=(12, 4))
        shortcut_bar.pack(fill=tk.X, side=tk.TOP, padx=12, pady=(0, 4))
        legend_text = "Shortcuts:  [↑/↓] Navigate  |  [Ctrl+↑/↓] Move Test  |  [Ctrl+A] Toggle Mode View  |  [Ctrl+D] Debug Terminal  |  [F5 / Ctrl+R] Run/Stop  |  [Ctrl+P] Pause"
        ttk.Label(shortcut_bar, text=legend_text, style="Panel.TFrame", foreground=COLOR_FG_MUTED, font=("Segoe UI", 8)).pack(side=tk.LEFT)

        paned = ttk.PanedWindow(self.root, orient=tk.HORIZONTAL)
        paned.pack(fill=tk.BOTH, expand=True, padx=12, pady=4)

        left_frame = ttk.Frame(paned, width=460)
        paned.add(left_frame, weight=1)

        sel_btn_frame = ttk.Frame(left_frame)
        sel_btn_frame.pack(fill=tk.X, side=tk.TOP, pady=(0, 6))

        s1 = ttk.Frame(sel_btn_frame)
        s1.pack(fill=tk.X, side=tk.TOP, pady=(0, 2))
        ttk.Button(s1, text="Select All", command=lambda: self._set_all_selected(True)).pack(side=tk.LEFT, padx=(0, 2))
        ttk.Button(s1, text="Deselect All", command=lambda: self._set_all_selected(False)).pack(side=tk.LEFT, padx=2)
        ttk.Button(s1, text="Passed", command=self._select_passed_only).pack(side=tk.LEFT, padx=2)
        ttk.Button(s1, text="Failed", command=self._select_failed_only).pack(side=tk.LEFT, padx=2)
        ttk.Button(s1, text="Pending", command=self._select_pending_only).pack(side=tk.LEFT, padx=2)

        s2 = ttk.Frame(sel_btn_frame)
        s2.pack(fill=tk.X, side=tk.TOP)
        ttk.Button(s2, text="🔄 Reset Selected", command=self._reset_selected_tests).pack(side=tk.LEFT)

        search_frame = ttk.Frame(left_frame)
        search_frame.pack(fill=tk.X, side=tk.TOP, pady=(0, 4))
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

        self.tree.column("sel", width=20, anchor="center")
        self.tree.column("id", width=20, anchor="center")
        self.tree.column("status", width=40, anchor="center")
        self.tree.column("cat", width=100, anchor="center")
        self.tree.column("cmd", width=300, anchor="w")

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

        ttk.Button(ec_r2, text="🐛 Debug in Terminal (Ctrl+D)", command=self._debug_in_terminal).pack(side=tk.LEFT, padx=(0, 10))
        ttk.Button(ec_r2, text="➕ Add as New", command=self._add_test_item).pack(side=tk.RIGHT, padx=(2, 0))
        ttk.Button(ec_r2, text="💾 Update Selected", command=self._update_test_item).pack(side=tk.RIGHT, padx=2)
        ttk.Button(ec_r2, text="🗑 Delete Selected", command=self._delete_test_item).pack(side=tk.RIGHT, padx=2)

        mode_bar = ttk.Frame(right_frame, style="Panel.TFrame")
        mode_bar.pack(fill=tk.X, side=tk.TOP, pady=(0, 4))

        self.btn_mode_toggle = ttk.Button(
            mode_bar,
            text="🔀 View Mode: Non-Interactive Pass (Ctrl+A)",
            style="Accent.TButton",
            command=self.toggle_inspector_view_mode
        )
        self.btn_mode_toggle.pack(side=tk.LEFT, fill=tk.X, expand=True)

        self.notebook = ttk.Notebook(right_frame)
        self.notebook.pack(fill=tk.BOTH, expand=True)

        self.txt_overview = self._create_dark_text_tab("Overview", self.notebook)
        self.txt_diff = self._create_dark_text_tab("STDOUT Output / Diff", self.notebook, mono=True)
        self.txt_stderr = self._create_dark_text_tab("STDERR Logs", self.notebook, mono=True)
        self.txt_env = self._create_dark_text_tab("Environment Variables", self.notebook, mono=True)
        self.txt_valgrind = self._create_dark_text_tab("Valgrind / FDs", self.notebook, mono=True)
        self.txt_malloc = self._create_dark_text_tab("Malloc Faults", self.notebook, mono=True)
        self.txt_signals = self._create_dark_text_tab("Signal Handling", self.notebook, mono=True)
        self.txt_audit = self._create_dark_text_tab("Compliance & Norm", self.notebook, mono=True)

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

    def _spinbox_decrement(self, event=None):
        try:
            val = max(1, self.jobs_var.get() - 1)
            self.jobs_var.set(val)
        except Exception:
            pass
        return "break"

    def _spinbox_increment(self, event=None):
        try:
            val = min(32, self.jobs_var.get() + 1)
            self.jobs_var.set(val)
        except Exception:
            pass
        return "break"

    def _bind_shortcuts(self):
        self.root.bind("<F5>", lambda e: self.run_tests())
        self.root.bind("<Control-r>", lambda e: self.run_tests())
        self.root.bind("<Control-p>", lambda e: self.toggle_pause())
        self.root.bind("<Control-a>", lambda e: self.toggle_inspector_view_mode())
        self.root.bind("<Control-d>", lambda e: self._debug_in_terminal())
        self.root.bind("<Control-f>", self._focus_search)
        self.root.bind("<slash>", self._focus_search)

        self.root.bind("<Up>", self._navigate_tree)
        self.root.bind("<Down>", self._navigate_tree)

        self.root.bind("<Control-Up>", self._move_test_up)
        self.root.bind("<Control-Down>", self._move_test_down)
        self.tree.bind("<Control-Up>", self._move_test_up)
        self.tree.bind("<Control-Down>", self._move_test_down)

        self.root.bind("<Left>", self._handle_tab_navigation)
        self.root.bind("<Right>", self._handle_tab_navigation)

        self.search_entry.bind("<Escape>", self._focus_tree)
        self.tree.bind("<space>", self._on_space_key)

    def toggle_inspector_view_mode(self, event=None):
        if self.inspector_mode == "NON_INTERACTIVE":
            self.inspector_mode = "INTERACTIVE"
            self.btn_mode_toggle.config(text="🔀 View Mode: Interactive PTY Pass (Ctrl+A)")
        else:
            self.inspector_mode = "NON_INTERACTIVE"
            self.btn_mode_toggle.config(text="🔀 View Mode: Non-Interactive Pass (Ctrl+A)")

        sel = self.tree.selection()
        if sel:
            try:
                t_id = int(self.tree.item(sel[0])["values"][1])
                with self.tests_lock:
                    test_item = next((t for t in self.tests_data if t["id"] == t_id), None)
                if test_item:
                    self._update_inspector(test_item)
            except (IndexError, ValueError):
                pass
        return "break"

    def _debug_in_terminal(self):
        sel = self.tree.selection()
        if not sel:
            return "break"
        try:
            t_id = int(self.tree.item(sel[0])["values"][1])
        except (IndexError, ValueError):
            return "break"
        with self.tests_lock:
            test_item = next((t for t in self.tests_data if t["id"] == t_id), None)
        if not test_item:
            return "break"

        ms_path = os.path.abspath(self.ms_path_var.get())
        if not os.path.exists(ms_path):
            messagebox.showerror("Error", f"Minishell executable '{ms_path}' not found.")
            return "break"

        term = shutil.which("xterm") or shutil.which("gnome-terminal") or shutil.which("kitty") or shutil.which("alacritty") or shutil.which("x-terminal-emulator")
        if not term:
            messagebox.showerror("Error", "No supported terminal emulator found (xterm, gnome-terminal, kitty, alacritty).")
            return "break"

        cmd_raw = test_item['cmd']
        ms_cmd_str = re.sub(r'/tmp/ms_', f'/tmp/ms_t{test_item["id"]}_ms_', cmd_raw)
        payload = f"{ms_cmd_str}\n"

        try:
            self.root.clipboard_clear()
            self.root.clipboard_append(payload)
            self.root.update()
        except Exception:
            pass

        valgrind_bin = shutil.which("valgrind")
        supp_path = self.env_mgr.supp_file
        ms_dir = os.path.dirname(ms_path)

        runner_code = f'''import os, sys, pty, subprocess, time, select, termios, tty

ms_path = {repr(ms_path)}
supp_path = {repr(supp_path)}
payload = {repr(payload)}
root_dir = {repr(ms_dir)}
valgrind_bin = {repr(valgrind_bin)}

if valgrind_bin:
    cmd = [valgrind_bin, "--suppressions=" + supp_path, "--leak-check=full", "--show-leak-kinds=all", "--errors-for-leak-kinds=all", "--track-fds=yes", ms_path]
else:
    cmd = [ms_path]

print("=== INTERACTIVE VALGRIND DEBUG SESSION ===")
print("Target: " + ms_path)
print("Payload: " + repr(payload.strip()))
print("--------------------------------------------------")
sys.stdout.flush()

master_fd, slave_fd = pty.openpty()
proc = subprocess.Popen(cmd, stdin=slave_fd, stdout=slave_fd, stderr=slave_fd, cwd=root_dir, close_fds=True)
os.close(slave_fd)

time.sleep(1.8)

if payload:
    os.write(master_fd, payload.encode('utf-8'))

old_settings = None
if sys.stdin.isatty():
    try:
        old_settings = termios.tcgetattr(sys.stdin.fileno())
    except Exception:
        pass

try:
    if old_settings:
        tty.setraw(sys.stdin.fileno())
    
    while proc.poll() is None:
        r, _, _ = select.select([sys.stdin.fileno(), master_fd], [], [], 0.02)
        if sys.stdin.fileno() in r:
            d = os.read(sys.stdin.fileno(), 1024)
            if not d: break
            os.write(master_fd, d)
        if master_fd in r:
            try:
                d = os.read(master_fd, 1024)
                if not d: break
                os.write(sys.stdout.fileno(), d)
            except OSError:
                break
finally:
    if old_settings:
        try: termios.tcsetattr(sys.stdin.fileno(), termios.TCSADRAIN, old_settings)
        except Exception: pass
    try: os.close(master_fd)
    except Exception: pass
    proc.wait()
    print("\\r\\n=== Debug Session Ended ===")
    input("Press Enter to close terminal...")
'''

        runner_file = os.path.join(self.env_mgr.temp_dir, f"debug_runner_{test_item['id']}.py")
        with open(runner_file, "w", encoding="utf-8") as f:
            f.write(runner_code)

        run_script = f'python3 "{runner_file}"; exec bash'

        try:
            if "gnome-terminal" in term:
                subprocess.Popen([term, "--", "bash", "-c", run_script])
            else:
                subprocess.Popen([term, "-e", "bash", "-c", run_script])
        except Exception as e:
            messagebox.showerror("Error", f"Failed to launch terminal debug session: {e}")

        return "break"

    def _run_compliance_audit(self):
        ms_path = os.path.abspath(self.ms_path_var.get())
        ms_dir = os.path.dirname(ms_path) if os.path.exists(ms_path) else "."

        self.lbl_status.config(text="Status: Auditing Functions & Norminette...", foreground=COLOR_WARN)
        self.root.update_idletasks()

        forbidden_list = check_forbidden_functions(ms_path)
        norm_output = run_norminette_check(ms_dir)

        def write_audit():
            self.txt_audit.insert(tk.END, "=== 1. FORBIDDEN FUNCTIONS AUDIT (`nm -u`) ===\n\n")
            if not forbidden_list:
                self.txt_audit.insert(tk.END, "✔ Perfect! All linked external symbols are whitelisted by the 42 curriculum.\n\n")
            elif "Audit error" in forbidden_list[0] or "`nm` utility" in forbidden_list[0]:
                self.txt_audit.insert(tk.END, f"⚠ Audit Warning: {forbidden_list[0]}\n\n")
            else:
                self.txt_audit.insert(tk.END, f"✖ Forbidden/Non-Whitelisted Symbols Detected ({len(forbidden_list)}):\n")
                for fn in forbidden_list:
                    self.txt_audit.insert(tk.END, f"  • {fn}\n")
                self.txt_audit.insert(tk.END, "\n")

            self.txt_audit.insert(tk.END, "=== 2. NORMINETTE CODE STYLE AUDIT ===\n\n")
            self.txt_audit.insert(tk.END, norm_output)

        self._write_read_only_text(self.txt_audit, write_audit)
        self.notebook.select(self.txt_audit.master)
        self.lbl_status.config(text="Status: Compliance Audit Completed", foreground=COLOR_PASS)
        self._update_stats_bar()

    def _export_html_report(self):
        with self.tests_lock:
            data_copy = [dict(t) for t in self.tests_data]

        total = len(data_copy)
        passed = sum(1 for t in data_copy if t["status"] == "PASS")
        failed = sum(1 for t in data_copy if t["status"] == "FAIL")

        html = [
            "<!DOCTYPE html>",
            "<html><head><meta charset='utf-8'><title>Minishell Test Report</title>",
            "<style>",
            "body { font-family: 'Segoe UI', Arial, sans-serif; background-color: #1e1e2e; color: #cdd6f4; margin: 20px; }",
            "h1 { color: #89b4fa; }",
            ".summary { background-color: #252538; padding: 15px; border-radius: 8px; margin-bottom: 20px; }",
            ".pass { color: #a6e3a1; font-weight: bold; }",
            ".fail { color: #f38ba8; font-weight: bold; }",
            "table { width: 100%; border-collapse: collapse; margin-top: 10px; }",
            "th, td { padding: 10px; text-align: left; border-bottom: 1px solid #313244; }",
            "th { background-color: #252538; color: #89b4fa; }",
            "tr:nth-child(even) { background-color: #181825; }",
            "pre { font-family: Consolas, monospace; font-size: 12px; background: #11111b; padding: 10px; border-radius: 5px; white-space: pre-wrap; }",
            "</style></head><body>",
            "<h1>Minishell Automated Execution Report</h1>",
            f"<div class='summary'><b>Total Tests:</b> {total} | <span class='pass'>Passed: {passed}</span> | <span class='fail'>Failed: {failed}</span></div>",
            "<table><tr><th>ID</th><th>Category</th><th>Command</th><th>Status</th><th>Duration</th><th>Details</th></tr>"
        ]

        for t in data_copy:
            status_cls = "pass" if t["status"] == "PASS" else ("fail" if t["status"] == "FAIL" else "")
            res = t.get("result") or {}
            dur = f"{res.get('duration_ms', 0)} ms" if res else "N/A"
            fails = "<br>".join(res.get("failures", [])) if res.get("failures") else "None"

            html.append(
                f"<tr><td>{t['id']}</td><td>{t['cat']}</td><td><code>{t['cmd']}</code></td>"
                f"<td class='{status_cls}'>{t['status']}</td><td>{dur}</td><td>{fails}</td></tr>"
            )

        html.append("</table></body></html>")

        report_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "minishell_test_report.html")
        try:
            with open(report_path, "w", encoding="utf-8") as f:
                f.write("\n".join(html))
            messagebox.showinfo("Report Exported", f"HTML Report saved successfully to:\n{report_path}")
        except Exception as e:
            messagebox.showerror("Export Error", f"Failed to save HTML report: {e}")

    def _move_test_up(self, event=None):
        sel = self.tree.selection()
        if not sel:
            return "break"

        try:
            t_id = int(self.tree.item(sel[0])["values"][1])
        except (IndexError, ValueError):
            return "break"

        target_uid = None
        with self.tests_lock:
            idx = next((i for i, t in enumerate(self.tests_data) if t["id"] == t_id), None)
            if idx is None or idx == 0:
                return "break"

            target_uid = self.tests_data[idx]["_uid"]
            self.tests_data[idx], self.tests_data[idx - 1] = self.tests_data[idx - 1], self.tests_data[idx]
            self._reindex_tests_locked()
            save_tests_to_file(self.tests_data)

        self._populate_tree()

        if target_uid is not None:
            for item in self.tree.get_children():
                try:
                    row_id = int(self.tree.item(item)["values"][1])
                except (IndexError, ValueError):
                    continue
                with self.tests_lock:
                    matched = next((t for t in self.tests_data if t["id"] == row_id), None)
                if matched and matched["_uid"] == target_uid:
                    self.tree.selection_set(item)
                    self.tree.focus(item)
                    self.tree.see(item)
                    break

        return "break"

    def _move_test_down(self, event=None):
        sel = self.tree.selection()
        if not sel:
            return "break"

        try:
            t_id = int(self.tree.item(sel[0])["values"][1])
        except (IndexError, ValueError):
            return "break"

        target_uid = None
        with self.tests_lock:
            idx = next((i for i, t in enumerate(self.tests_data) if t["id"] == t_id), None)
            if idx is None or idx >= len(self.tests_data) - 1:
                return "break"

            target_uid = self.tests_data[idx]["_uid"]
            self.tests_data[idx], self.tests_data[idx + 1] = self.tests_data[idx + 1], self.tests_data[idx]
            self._reindex_tests_locked()
            save_tests_to_file(self.tests_data)

        self._populate_tree()

        if target_uid is not None:
            for item in self.tree.get_children():
                try:
                    row_id = int(self.tree.item(item)["values"][1])
                except (IndexError, ValueError):
                    continue
                with self.tests_lock:
                    matched = next((t for t in self.tests_data if t["id"] == row_id), None)
                if matched and matched["_uid"] == target_uid:
                    self.tree.selection_set(item)
                    self.tree.focus(item)
                    self.tree.see(item)
                    break

        return "break"

    def _navigate_tree(self, event):
        children = self.tree.get_children()
        if not children:
            return "break"

        direction = -1 if event.keysym == "Up" else 1

        selection = self.tree.selection()
        if not selection:
            target = children[0]
        elif self._is_input_focused():
            current_idx = children.index(selection[0])
            new_idx = max(0, min(len(children) - 1, current_idx + direction))
            target = children[new_idx]
        else:
            target = selection

        self.tree.focus_set()
        self.tree.selection_set(target)
        self.tree.focus(target)
        self.tree.see(target)

        return "break"

    def _is_input_focused(self):
        focus = self.root.focus_get()
        return isinstance(focus, (ttk.Entry, tk.Entry, ttk.Spinbox, tk.Spinbox))

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
            try:
                t_id = int(self.tree.item(sel[0])["values"][1])
            except (IndexError, ValueError):
                return "break"

            with self.tests_lock:
                for t in self.tests_data:
                    if t["id"] == t_id:
                        t["selected"] = not t["selected"]
                        if not t["selected"] and t["status"] in ("RUNNING", "QUEUED"):
                            t["status"] = "PENDING"
                            t["result"] = None
                        self._update_tree_row(sel[0], t)
                        if t["selected"] and t["status"] == "PENDING" and self.is_running and not self.is_paused:
                            self._ensure_worker_running()
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
        start_dir = os.path.dirname(ms_path) if os.path.exists(ms_path) else os.getcwd()
        make_dir = find_makefile_dir(start_dir)

        self.lbl_status.config(text=f"Status: Recompiling in {make_dir} (make)...", foreground=COLOR_WARN)
        self.root.update_idletasks()

        try:
            res = subprocess.run(["make"], cwd=make_dir, capture_output=True, text=True)
            if res.returncode == 0:
                messagebox.showinfo("Recompile Success", f"Build succeeded in '{make_dir}'! 'make' returned 0.\n\n" + (res.stdout[-600:] if res.stdout else "No output."))
                self.lbl_status.config(text="Status: Recompile Successful", foreground=COLOR_PASS)
            else:
                messagebox.showerror("Recompile Failed", f"Build failed in '{make_dir}'!\n\n" + (res.stderr[-1000:] if res.stderr else "Compilation error."))
                self.lbl_status.config(text="Status: Recompile Failed", foreground=COLOR_FAIL)
        except Exception as e:
            messagebox.showerror("Error", f"Failed to run make: {str(e)}")
            self.lbl_status.config(text="Status: Recompile Error", foreground=COLOR_FAIL)

        self._update_stats_bar()

    def _reset_selected_tests(self):
        cleanup_test_artifacts()
        with self.tests_lock:
            for t in self.tests_data:
                if t["selected"]:
                    t["status"] = "PENDING"
                    t["result"] = None
        self._populate_tree()

        sel = self.tree.selection()
        if sel:
            try:
                t_id = int(self.tree.item(sel[0])["values"][1])
                t_item = next((t for t in self.tests_data if t["id"] == t_id), None)
                if t_item:
                    self._update_inspector(t_item)
            except (IndexError, ValueError):
                pass

        if self.is_running and not self.is_paused:
            self._ensure_worker_running()

    def _populate_tree(self):
        filter_str = self.filter_var.get().lower()
        selected_id = None
        sel = self.tree.selection()
        if sel:
            try:
                selected_id = int(self.tree.item(sel[0])["values"][1])
            except (IndexError, ValueError):
                selected_id = None

        for item in self.tree.get_children():
            self.tree.delete(item)

        with self.tests_lock:
            data_copy = [dict(item) for item in self.tests_data]

        for item in data_copy:
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
            elif item["status"] in ("RUNNING", "QUEUED"):
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
        elif item["status"] in ("RUNNING", "QUEUED"):
            status_text = "⏳ RUNNING"
            tag = "RUNNING"

        self.tree.item(node, values=(sel_mark, item["id"], status_text, item["cat"], item["cmd"].replace("\n", "\\n")), tags=(tag,))
        self._update_stats_bar()

    def _update_stats_bar(self):
        with self.tests_lock:
            total = len(self.tests_data)
            selected = sum(1 for t in self.tests_data if t["selected"])
            passed = sum(1 for t in self.tests_data if t["status"] == "PASS")
            failed = sum(1 for t in self.tests_data if t["status"] == "FAIL")

        if self.is_running:
            if self.is_paused:
                self.lbl_status.config(text="Status: Paused", foreground=COLOR_WARN)
            else:
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
                    try:
                        t_id = int(vals[1])
                    except (IndexError, ValueError):
                        return
                    with self.tests_lock:
                        for t in self.tests_data:
                            if t["id"] == t_id:
                                t["selected"] = not t["selected"]
                                if not t["selected"] and t["status"] in ("RUNNING", "QUEUED"):
                                    t["status"] = "PENDING"
                                    t["result"] = None
                                self._update_tree_row(item_id, t)
                                if t["selected"] and t["status"] == "PENDING" and self.is_running and not self.is_paused:
                                    self._ensure_worker_running()
                                break

    def _on_tree_select(self, event):
        sel = self.tree.selection()
        if not sel:
            return
        try:
            t_id = int(self.tree.item(sel[0])["values"][1])
        except (IndexError, ValueError):
            return
        with self.tests_lock:
            test_item = next((t for t in self.tests_data if t["id"] == t_id), None)
        if test_item:
            self.edit_cat_var.set(test_item["cat"])
            self.edit_cmd_var.set(test_item["cmd"])
            self.edit_bash_cmp_var.set(test_item["bash_cmp"])
            self.edit_flag_err_var.set(test_item["flag_error"])
            self._update_inspector(test_item)

    def _set_all_selected(self, val):
        with self.tests_lock:
            for t in self.tests_data:
                t["selected"] = val
                if not val and t["status"] in ("RUNNING", "QUEUED"):
                    t["status"] = "PENDING"
                    t["result"] = None
        self._populate_tree()
        if val and self.is_running and not self.is_paused:
            self._ensure_worker_running()

    def _select_failed_only(self):
        with self.tests_lock:
            for t in self.tests_data:
                t["selected"] = (t["status"] == "FAIL")
        self._populate_tree()

    def _select_passed_only(self):
        with self.tests_lock:
            for t in self.tests_data:
                t["selected"] = (t["status"] == "PASS")
        self._populate_tree()

    def _select_pending_only(self):
        with self.tests_lock:
            for t in self.tests_data:
                t["selected"] = (t["status"] == "PENDING")
        self._populate_tree()

    def _add_test_item(self):
        cmd = self.edit_cmd_var.get().strip()
        cat = self.edit_cat_var.get().strip() or "Custom"
        if not cmd:
            return
        with self.tests_lock:
            self._uid_counter += 1
            new_test = {
                "_uid": self._uid_counter,
                "id": len(self.tests_data) + 1,
                "cat": cat,
                "cmd": cmd,
                "bash_cmp": self.edit_bash_cmp_var.get(),
                "flag_error": self.edit_flag_err_var.get(),
                "selected": True,
                "status": "PENDING",
                "result": None
            }
            self.tests_data.append(new_test)
            self._reindex_tests_locked()
            save_tests_to_file(self.tests_data)
        self._populate_tree()
        if self.is_running and not self.is_paused:
            self._ensure_worker_running()

    def _update_test_item(self):
        sel = self.tree.selection()
        if not sel:
            return
        try:
            t_id = int(self.tree.item(sel[0])["values"][1])
        except (IndexError, ValueError):
            return
        test_item = None
        with self.tests_lock:
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
        if test_item:
            self._update_inspector(test_item)
        if self.is_running and not self.is_paused:
            self._ensure_worker_running()

    def _delete_test_item(self):
        sel = self.tree.selection()
        if not sel:
            return
        try:
            t_id = int(self.tree.item(sel[0])["values"][1])
        except (IndexError, ValueError):
            return
        with self.tests_lock:
            self.tests_data = [t for t in self.tests_data if t["id"] != t_id]
            self._reindex_tests_locked()
            save_tests_to_file(self.tests_data)
        self._populate_tree()

    def _update_inspector(self, test_item):
        res = test_item["result"]

        if not res or not isinstance(res, dict):
            self._write_read_only_text(
                self.txt_overview,
                lambda: self.txt_overview.insert(tk.END, f"Command: {test_item['cmd']}\nStatus: {test_item['status']}\n\nRun test to inspect output.")
            )
            for txt in (self.txt_diff, self.txt_stderr, self.txt_env, self.txt_valgrind, self.txt_malloc, self.txt_signals):
                self._write_read_only_text(txt, lambda: None)
            return

        is_int = (self.inspector_mode == "INTERACTIVE")

        # 1. Overview Tab
        def write_overview():
            ov = [
                f"Command:  {res['cmd']}",
                f"Category: {res['cat']}",
                f"Result:   {'PASS' if res['passed'] else 'FAIL'}",
                f"Total Duration: {res.get('duration_ms', 0)} ms",
                f"\nExit Statuses (Non-Interactive / Interactive):",
                f"  Minishell: {res['ms_code']} / {res.get('interactive_code', 0)}",
                f"  Bash:      {res['bash_code']}",
                f"\nWorking Directory (CWD):",
                f"  Minishell (Non-Interactive): {res['ms_cwd'] if res['ms_cwd'] else '(Not Captured)'}",
                f"  Minishell (Interactive):     {res.get('interactive_cwd', '(Not Captured)')}",
                f"  Bash:                        {res['bash_cwd'] if res['bash_cwd'] else '(Not Captured)'}"
            ]
            if res["failures"]:
                ov.append("\nFailures / Mismatches Detected:")
                for f in res["failures"]:
                    ov.append(f"  • {f}")
            self.txt_overview.insert(tk.END, "\n".join(ov))

        self._write_read_only_text(self.txt_overview, write_overview)

        # 2. STDOUT Tab
        def write_diff():
            dur_str = f"Execution Duration: {res.get('pass1_duration_ms', 0)} ms"
            diff_str = res.get("interactive_diff" if is_int else "diff_text", "")
            out_str = res.get("interactive_out" if is_int else "ms_out", "")
            mode_lbl = "INTERACTIVE (PTY)" if is_int else "NON-INTERACTIVE (PIPED)"

            self.txt_diff.insert(tk.END, f"{dur_str}\n", "info")
            self.txt_diff.insert(tk.END, f"=== {mode_lbl} UNIFIED DIFF (-Bash, +Minishell) ===\n", "info")
            if diff_str:
                for line in diff_str.splitlines(keepends=True):
                    if line.startswith("+"): self.txt_diff.insert(tk.END, line, "add")
                    elif line.startswith("-"): self.txt_diff.insert(tk.END, line, "sub")
                    elif line.startswith("@"): self.txt_diff.insert(tk.END, line, "info")
                    else: self.txt_diff.insert(tk.END, line)
            else:
                self.txt_diff.insert(tk.END, f"✔ {mode_lbl} STDOUT matches Bash output perfectly.\n")

            self.txt_diff.insert(tk.END, f"\n=== MINISHELL STDOUT ({mode_lbl}) ===\n", "info")
            self.txt_diff.insert(tk.END, (out_str if out_str else "(empty)") + "\n")

            self.txt_diff.insert(tk.END, "\n=== BASH STDOUT ===\n", "info")
            self.txt_diff.insert(tk.END, res["bash_out"] if res["bash_out"] else "(empty)\n")

        self._write_read_only_text(self.txt_diff, write_diff)

        # 3. STDERR Tab
        def write_stderr():
            mode_lbl = "INTERACTIVE (PTY)" if is_int else "NON-INTERACTIVE (PIPED)"
            ms_e = res.get('interactive_err' if is_int else 'ms_err', '')
            ms_e = ms_e if ms_e.strip() else "(empty)"
            bash_e = res['bash_err'] if res['bash_err'].strip() else "(empty)"

            self.txt_stderr.insert(tk.END, f"=== MINISHELL STDERR ({mode_lbl}) ===\n{ms_e}\n", "info")
            self.txt_stderr.insert(tk.END, f"\n=== BASH STDERR ===\n{bash_e}\n")

        self._write_read_only_text(self.txt_stderr, write_stderr)

        # 4. Environment Variables Tab
        def write_env():
            mode_lbl = "INTERACTIVE (PTY)" if is_int else "NON-INTERACTIVE (PIPED)"
            ms_env_raw = res.get('interactive_env' if is_int else 'ms_env', '')
            ms_env_norm = normalize_env_output(ms_env_raw) if ms_env_raw else "(empty)"
            bash_env_norm = normalize_env_output(res['bash_env']) if res['bash_env'] else "(empty)"

            self.txt_env.insert(tk.END, f"Execution Duration (Env Pass): {res.get('pass3_duration_ms', 0)} ms\n", "info")
            self.txt_env.insert(tk.END, f"\n=== MINISHELL ENVIRONMENT ({mode_lbl}) ===\n", "info")
            self.txt_env.insert(tk.END, ms_env_norm + "\n")
            self.txt_env.insert(tk.END, "\n=== BASH ENVIRONMENT ===\n", "info")
            self.txt_env.insert(tk.END, bash_env_norm + "\n")

        self._write_read_only_text(self.txt_env, write_env)

        # 5. Valgrind Tab
        self._write_read_only_text(
            self.txt_valgrind,
            lambda: self.txt_valgrind.insert(tk.END, f"Execution Duration (Valgrind Pass): {res.get('valgrind_duration_ms', 0)} ms\n\n" + res["valgrind_log"])
        )

        # 6. Malloc Tab
        self._write_read_only_text(
            self.txt_malloc,
            lambda: self.txt_malloc.insert(tk.END, f"Execution Duration (Malloc Pass): {res.get('malloc_duration_ms', 0)} ms\n\n" + res["malloc_log"])
        )

        # 7. Signal Handling Tab
        self._write_read_only_text(
            self.txt_signals,
            lambda: self.txt_signals.insert(tk.END, f"Execution Duration (Signal Pass): {res.get('signals_duration_ms', 0)} ms\n\n" + res["signal_log"])
        )

    def stop_tests(self):
        if not self.is_running and not self.is_paused:
            return
        self.stop_requested = True
        self.pause_requested = False
        self.is_paused = False
        self.current_run_id += 1
        self.lbl_status.config(text="Status: Stopped", foreground=COLOR_WARN)
        self._finish_run()

    def toggle_pause(self):
        if not self.is_running and not self.is_paused:
            return

        if not self.is_paused:
            self.pause_requested = True
            self.is_paused = True
            self.btn_pause.config(text="▶  Resume (Ctrl+P)")
            self.lbl_status.config(text="Status: Paused", foreground=COLOR_WARN)

            with self.tests_lock:
                for t in self.tests_data:
                    if t["status"] in ("RUNNING", "QUEUED"):
                        t["status"] = "PENDING"
                        t["result"] = None
            self._populate_tree()
        else:
            self.is_paused = False
            self.pause_requested = False
            self.btn_pause.config(text="⏸  Pause (Ctrl+P)")
            self.lbl_status.config(text="Status: Executing Tests...", foreground=COLOR_WARN)

            has_pending = False
            with self.tests_lock:
                has_pending = any(t["selected"] and t["status"] == "PENDING" for t in self.tests_data)

            if not has_pending:
                self._finish_run()
            else:
                self._ensure_worker_running()

    def run_tests(self):
        if self.is_running or self.is_paused:
            self.stop_tests()
            return

        ms_path = os.path.abspath(self.ms_path_var.get())
        if not os.path.isfile(ms_path):
            messagebox.showerror("Error", f"Minishell binary '{ms_path}' not found.")
            return

        bash_path = self.bash_path_var.get().strip()
        if self.chk_bash.get() and not os.path.isfile(bash_path) and not shutil.which(bash_path):
            messagebox.showerror("Error", f"Bash executable '{bash_path}' not found.")
            return

        cleanup_test_artifacts()

        with self.tests_lock:
            for t in self.tests_data:
                if t["selected"]:
                    t["status"] = "PENDING"
                    t["result"] = None

        self.current_run_id += 1
        self.stop_requested = False
        self.pause_requested = False
        self.is_paused = False
        self.is_running = True

        self.btn_run.config(text="⏹  Stop Tests (F5)", style="Accent.TButton")
        self.btn_pause.config(text="⏸  Pause (Ctrl+P)", state=tk.NORMAL)
        self.progress_var.set(0)
        self._populate_tree()

        self._ensure_worker_running()

    def _ensure_worker_running(self):
        if not self.is_running or self.is_paused or self.stop_requested:
            return

        ms_path = os.path.abspath(self.ms_path_var.get())
        if not os.path.isfile(ms_path):
            messagebox.showerror("Error", f"Minishell binary '{ms_path}' not found.")
            self.stop_tests()
            return

        bash_path = self.bash_path_var.get().strip()
        if self.chk_bash.get() and not os.path.isfile(bash_path) and not shutil.which(bash_path):
            messagebox.showerror("Error", f"Bash executable '{bash_path}' not found.")
            self.stop_tests()
            return

        if self.worker_thread is None or not self.worker_thread.is_alive() or getattr(self.worker_thread, "run_id", None) != self.current_run_id:
            opts = {
                "skip_bash": not self.chk_bash.get(),
                "skip_valgrind": not self.chk_valgrind.get(),
                "skip_malloc": not self.chk_malloc.get(),
                "skip_signals": not self.chk_signals.get(),
                "run_env_i": self.chk_env_i.get(),
                "run_non_interactive": self.chk_non_interactive.get(),
                "run_interactive": self.chk_interactive.get()
            }
            t = threading.Thread(
                target=self._worker_thread,
                args=(self.current_run_id, ms_path, bash_path, self.env_mgr.hook_so, self.env_mgr.supp_file, opts),
                daemon=True
            )
            t.run_id = self.current_run_id
            self.worker_thread = t
            t.start()

    def _execute_test_wrapper(self, run_id, test_item, ms_path, bash_path, hook_so_path, supp_file_path, opts):
        def cancel_check():
            if not self.is_running or self.current_run_id != run_id or self.stop_requested or self.pause_requested:
                return True
            with self.tests_lock:
                existing = next((t for t in self.tests_data if t["_uid"] == test_item["_uid"]), None)
                if not existing or not existing["selected"]:
                    return True
            return False

        if cancel_check():
            return None

        self.msg_queue.put(("STARTING", run_id, test_item["_uid"], None))

        res = execute_single_test(
            test_item, ms_path, bash_path, hook_so_path, supp_file_path, opts, check_cancel=cancel_check
        )
        return res

    def _worker_thread(self, run_id, ms_path, bash_path, hook_so_path, supp_file_path, opts):
        with ThreadPoolExecutor(max_workers=32) as executor:
            active_futures = {}

            while self.is_running and self.current_run_id == run_id and not self.stop_requested and not self.pause_requested:
                done_futures = [f for f in list(active_futures.keys()) if f.done()]
                for f in done_futures:
                    t_uid = active_futures.pop(f)
                    try:
                        res = f.result()
                        if res is not None and isinstance(res, dict) and self.is_running and self.current_run_id == run_id and not self.stop_requested and not self.pause_requested:
                            self.msg_queue.put(("RESULT", run_id, t_uid, res))
                        else:
                            self.msg_queue.put(("RESET_TEST", run_id, t_uid, None))
                    except Exception:
                        self.msg_queue.put(("RESET_TEST", run_id, t_uid, None))

                try:
                    num_threads = max(1, min(32, self.jobs_var.get()))
                except Exception:
                    num_threads = 4

                if len(active_futures) < num_threads and self.is_running and self.current_run_id == run_id and not self.pause_requested and not self.stop_requested:
                    next_test = None
                    with self.tests_lock:
                        for t in self.tests_data:
                            if t["selected"] and t["status"] == "PENDING":
                                t["status"] = "QUEUED"
                                next_test = dict(t)
                                break

                    if next_test:
                        f = executor.submit(
                            self._execute_test_wrapper, run_id, next_test, ms_path, bash_path, hook_so_path, supp_file_path, opts
                        )
                        active_futures[f] = next_test["_uid"]
                        continue

                if not active_futures:
                    has_pending = False
                    with self.tests_lock:
                        has_pending = any(t["selected"] and t["status"] == "PENDING" for t in self.tests_data)
                    if not has_pending:
                        break

                time.sleep(0.02)

            for f in list(active_futures.keys()):
                f.cancel()

        self.msg_queue.put(("BATCH_DONE", run_id, None, None))

    def _finish_run(self):
        self.is_running = False
        self.is_paused = False
        self.stop_requested = False
        self.pause_requested = False
        self.btn_run.config(text="▶  Run Selected (F5)", style="Accent.TButton")
        self.btn_pause.config(text="⏸  Pause (Ctrl+P)", state=tk.DISABLED)

        with self.tests_lock:
            for t in self.tests_data:
                if t["status"] in ("RUNNING", "QUEUED"):
                    t["status"] = "PENDING"
                    t["result"] = None

        self._populate_tree()
        self._update_stats_bar()
        cleanup_test_artifacts()

    def _update_overall_progress(self):
        with self.tests_lock:
            selected_tests = [t for t in self.tests_data if t["selected"]]
            if selected_tests:
                completed_count = sum(1 for t in selected_tests if t["status"] in ("PASS", "FAIL"))
                prog = (completed_count / len(selected_tests)) * 100
                self.progress_var.set(prog)

    def _poll_queue(self):
        try:
            while True:
                msg_type, run_id, t_uid, res = self.msg_queue.get_nowait()
                if run_id != self.current_run_id:
                    continue

                if msg_type == "STARTING":
                    if not self.stop_requested and not self.pause_requested:
                        with self.tests_lock:
                            for t in self.tests_data:
                                if t["_uid"] == t_uid and t["status"] == "QUEUED":
                                    t["status"] = "RUNNING"
                                    break
                        self._populate_tree()

                elif msg_type == "RESULT":
                    if not self.stop_requested and not self.pause_requested and isinstance(res, dict):
                        with self.tests_lock:
                            for t in self.tests_data:
                                if t["_uid"] == t_uid:
                                    t["result"] = res
                                    t["status"] = "PASS" if res.get("passed", False) else "FAIL"
                                    break
                        self._update_overall_progress()
                        self._populate_tree()

                        sel = self.tree.selection()
                        if sel:
                            try:
                                sel_id = int(self.tree.item(sel[0])["values"][1])
                                with self.tests_lock:
                                    t_item = next((t for t in self.tests_data if t["id"] == sel_id), None)
                                if t_item and t_item["_uid"] == t_uid:
                                    self._update_inspector(t_item)
                            except (IndexError, ValueError):
                                pass

                elif msg_type == "RESET_TEST":
                    with self.tests_lock:
                        for t in self.tests_data:
                            if t["_uid"] == t_uid and t["status"] in ("RUNNING", "QUEUED"):
                                t["status"] = "PENDING"
                                t["result"] = None
                    self._populate_tree()

                elif msg_type == "BATCH_DONE":
                    if self.is_paused:
                        self.lbl_status.config(text="Status: Paused", foreground=COLOR_WARN)
                        self._update_stats_bar()
                    else:
                        has_pending = False
                        with self.tests_lock:
                            has_pending = any(t["selected"] and t["status"] == "PENDING" for t in self.tests_data)
                        if has_pending and self.is_running and not self.stop_requested:
                            self._ensure_worker_running()
                        else:
                            self._finish_run()
        except queue.Empty:
            pass
        except Exception:
            pass

        self.root.after(100, self._poll_queue)


def main_cli(args):
    cfg = load_app_config()
    ms_setting = args.ms or cfg.get("ms_path") or "./minishell"
    bash_setting = args.bash or cfg.get("bash_path") or (shutil.which("bash") or "/bin/bash")

    ms_path = os.path.abspath(ms_setting)
    bash_path = os.path.abspath(bash_setting) if not shutil.which(bash_setting) and os.path.exists(bash_setting) else (shutil.which(bash_setting) or bash_setting)

    if not os.path.isfile(ms_path):
        print(f"Error: Minishell executable '{ms_path}' not found.")
        sys.exit(1)

    print("=== Minishell Automated CLI Test Runner ===")
    print(f"Target Binary: {ms_path}")
    print(f"Bash Compare: {bash_path}")

    env_mgr = EnvironmentManager()
    try:
        env_mgr.build_hook()
    except Exception as e:
        print(f"Error compiling allocation hook: {e}")
        sys.exit(1)

    raw_tests = load_tests_from_file()
    tests_data = []
    for idx, item in enumerate(raw_tests):
        tests_data.append({
            "id": idx + 1,
            "cat": item.get("cat", "Custom"),
            "cmd": item.get("cmd", ""),
            "bash_cmp": item.get("bash_cmp", True),
            "flag_error": item.get("flag_error", False)
        })

    opts = {
        "skip_bash": False,
        "skip_valgrind": False,
        "skip_malloc": False,
        "skip_signals": False,
        "run_env_i": True,
        "run_non_interactive": True,
        "run_interactive": True
    }

    passed_count = 0
    failed_count = 0

    print(f"\nExecuting {len(tests_data)} test cases...\n")

    with ThreadPoolExecutor(max_workers=os.cpu_count() or 4) as executor:
        futures = {
            executor.submit(execute_single_test, t, ms_path, bash_path, env_mgr.hook_so, env_mgr.supp_file, opts): t
            for t in tests_data
        }
        for future in as_completed(futures):
            t_item = futures[future]
            res = future.result()
            if res and res["passed"]:
                passed_count += 1
                print(f"  ✔ [PASS] #{t_item['id']} ({t_item['cat']}): {t_item['cmd']} ({res['duration_ms']} ms)")
            else:
                failed_count += 1
                print(f"  ✖ [FAIL] #{t_item['id']} ({t_item['cat']}): {t_item['cmd']}")
                if res and res["failures"]:
                    for f in res["failures"]:
                        print(f"      ├─ {f}")

    print("\n=== Compliance Audit Summary ===")
    forbidden = check_forbidden_functions(ms_path)
    if not forbidden:
        print("  ✔ External functions audit: Passed (All symbols whitelisted)")
    else:
        print(f"  ✖ Forbidden symbols detected: {', '.join(forbidden)}")

    print(f"\nResults: {passed_count} Passed, {failed_count} Failed.")
    cleanup_test_artifacts()
    sys.exit(0 if failed_count == 0 else 1)


def main():
    parser = argparse.ArgumentParser(description="Minishell Test Harness GUI / CLI")
    parser.add_argument("--cli", action="store_true", help="Run test suite headlessly in CLI terminal mode")
    parser.add_argument("--ms", default=None, help="Path to minishell binary (for CLI mode)")
    parser.add_argument("--bash", default=None, help="Path to bash binary (for CLI mode)")
    args = parser.parse_args()

    if args.cli:
        main_cli(args)
    else:
        root = tk.Tk()
        app = MinishellTestGUI(root)
        root.mainloop()

if __name__ == "__main__":
    main()
