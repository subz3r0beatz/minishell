#!/usr/bin/env python3
"""
Minishell Universal Modern Dark GUI Test Harness
Single-file self-contained Tkinter GUI test harness with stack backtrace symbol
resolution for silent malloc failures, readline valgrind suppressions,
recompile/reset controls, file-isolated execution, automatic artifact cleanup,
stderr mismatch validation, null-byte normalization, and dark UI theme.
"""

import os
import sys
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

def cleanup_test_artifacts():
    """Removes leftover temporary files and directories created during test runs."""
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

# --- Full Predefined Test Suite ---
DEFAULT_TESTS = [
    # --- Base Commands ---
    {"cat": "Normal Cmds", "cmd": "ls", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "ls -la", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "whoami", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "uname -s", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "cat /etc/passwd | grep -E 'root|nobody'", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "head -n 5 /etc/passwd", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "tail -n 3 /etc/passwd", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "wc -l /etc/passwd", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "sort /etc/passwd | head -n 10", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "uniq -c /etc/passwd", "bash_cmp": True},

    # --- Builtin: cd ---
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
    {"cat": "Builtin: cd", "cmd": "cd //", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd ///", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd /tmp/.", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd /tmp/..", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd /tmp/././.", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd /tmp/../../..", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd -- /tmp", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "unset HOME && cd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "unset OLDPWD && cd -", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd /tmp && pwd && cd - && pwd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "export CDPATH=/tmp && cd ms_test_dir", "bash_cmp": False},
    {"cat": "Builtin: cd", "cmd": "cd ''", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd ' '", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd arg1 arg2", "bash_cmp": True},

    # --- Builtin: pwd ---
    {"cat": "Builtin: pwd", "cmd": "pwd", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "pwd -L", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "pwd -P", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "pwd -LLLLPPPPLLLLPPPP", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "pwd arg1", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "pwd -L -P", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "pwd -P -L", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "export PWD=/fake/path && pwd", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "export PWD=/fake/path && pwd -L", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "export PWD=/fake/path && pwd -P", "bash_cmp": True},

    # --- Builtin: echo ---
    {"cat": "Builtin: echo", "cmd": "echo", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo hello world", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -n hello world", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -nnnn hello", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -n -n -n -n hello", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e 'hello\\nworld\\t!'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -E 'hello\\nworld\\t!'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -ne 'test\\n'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e '\\x41\\x42\\x43'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e 'Before\\cAfter'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e '\\\\\\\\'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -n", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -n -e", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -nx hello", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo '-n' hello", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo \"-n\" hello", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo $USER $HOME", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo \"$USER\" '$USER'", "bash_cmp": True},

    # --- Builtin: export ---
    {"cat": "Builtin: export", "cmd": "export", "bash_cmp": False},
    {"cat": "Builtin: export", "cmd": "export -p", "bash_cmp": False},
    {"cat": "Builtin: export", "cmd": "export VAR_TEST=123", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR_TEST+=456", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export BAD-VAR=123", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export _VALID=1 2INVALID=2 ALSO_VALID=3", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export NULL_VAR EMPTY_VAR=", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export WEIRD_VAR=\"hello=world=test=123\"", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export A=1 B=2 C=3 && echo $A $B $C", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export =INVALID", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export +=INVALID", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export 123NUM=val", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR_NAME_@=val", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export MY_VAR=\"  spaces  \" && echo \"$MY_VAR\"", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR1=\"$USER\" && echo $VAR1", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export FOO=bar && export FOO+=baz && echo $FOO", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export NEW_VAR && env | grep NEW_VAR", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export NEW_VAR= && env | grep NEW_VAR", "bash_cmp": True},

    # --- Builtin: unset ---
    {"cat": "Builtin: unset", "cmd": "unset PATH", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset DOES_NOT_EXIST", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset BAD-NAME", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset -v PATH", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "export VAR2=val2 && unset -v VAR2 && echo $VAR2", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "export A=1 B=2 C=3 && unset A B C && echo \"$A$B$C\"", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset PWD && pwd", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset OLDPWD && cd -", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset 123VAR", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset =", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset VAR1 VAR2 VAR3", "bash_cmp": True},

    # --- Builtin: env ---
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
    {"cat": "Builtin: env", "cmd": "env FOO=bar echo $FOO", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -i pwd", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -i ls", "bash_cmp": True},

    # --- Builtin: exit ---
    {"cat": "Builtin: exit", "cmd": "exit 0", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 42", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit -42", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 255", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 256", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 9223372036854775807", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 9223372036854775808", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit -9223372036854775808", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit -9223372036854775809", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 42 42", "bash_cmp": False},
    {"cat": "Builtin: exit", "cmd": "exit hello", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 42hello", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit -- -42", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit +10", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit \"  42  \"", "bash_cmp": True},

    # --- Syntax Errors ---
    {"cat": "Syntax Errors", "cmd": ";;", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": ";&", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": ";;&", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo 1; ; echo 2", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "| echo hello", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello |", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello ||", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello &&", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "&& echo hello", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "|| echo hello", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello | | echo world", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello >", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello <", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello >>", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello <<", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello > < world", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello > | world", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "(echo hello", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello)", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "(echo hello)", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "()", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "( )", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo (hello)", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello (world)", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello;", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": ";", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "; ;", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello;;", "bash_cmp": True},

    # --- Redirections & Alone ---
    {"cat": "Redir Alone", "cmd": "> /tmp/ms_empty.txt && ls -l /tmp/ms_empty.txt; rm -f /tmp/ms_empty.txt", "bash_cmp": True},
    {"cat": "Redir Alone", "cmd": ">> /tmp/ms_empty.txt && ls -l /tmp/ms_empty.txt; rm -f /tmp/ms_empty.txt", "bash_cmp": True},
    {"cat": "Redir Alone", "cmd": "> /tmp/ms_empty.txt && < /tmp/ms_empty.txt; rm -f /tmp/ms_empty.txt", "bash_cmp": True},
    {"cat": "Redir Alone", "cmd": "> /tmp/ms_out1 > /tmp/ms_out2 && ls -l /tmp/ms_out*; rm -f /tmp/ms_out*", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "echo hello > /tmp/ms_test_out.txt && cat /tmp/ms_test_out.txt; rm -f /tmp/ms_test_out.txt", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "echo hello > /tmp/ms_test_out.txt && echo append >> /tmp/ms_test_out.txt && cat /tmp/ms_test_out.txt; rm -f /tmp/ms_test_out.txt", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "echo hello > /tmp/ms_test_out.txt && cat < /tmp/ms_test_out.txt; rm -f /tmp/ms_test_out.txt", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "cat << EOF\nline 1\nline 2\nEOF", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "cat << 'EOF'\n$USER\nline 2\nEOF", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "cat << \"EOF\"\n$USER\nline 2\nEOF", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "cat << EOF1 << EOF2\nfirst\nEOF1\nsecond\nEOF2", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "echo hello 2> /tmp/ms_err.txt; cat /tmp/ms_err.txt; rm -f /tmp/ms_err.txt", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "ls /does_not_exist 2> /tmp/ms_err.txt; cat /tmp/ms_err.txt; rm -f /tmp/ms_err.txt", "bash_cmp": False},
    {"cat": "Redirections", "cmd": "ls /does_not_exist 2>> /tmp/ms_err.txt; cat /tmp/ms_err.txt; rm -f /tmp/ms_err.txt", "bash_cmp": False},

    # --- Pipes ---
    {"cat": "Pipes", "cmd": "echo hello | cat", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "cat /etc/hostname | grep -o a | wc -l", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "ls -la | grep srcs | wc -l", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "export TEST_PIPE=42 | echo hello; echo $TEST_PIPE", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "ls /does_not_exist | wc -l", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "echo hello | cat | cat | cat | grep h", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "echo 1 | echo 2 | echo 3", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "cat /etc/passwd | head -n 10 | tail -n 5 | wc -l", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "false | true", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "true | false", "bash_cmp": True},

    # --- Logic Operators ---
    {"cat": "Logic Operators", "cmd": "true && echo yes", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "false || echo no", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "false && echo no", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "true || echo no", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "echo 1 && echo 2 || echo 3", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "ls /does_not_exist && echo success || echo failed", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "false || false || echo third_time_charm", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "true && true && true && echo all_good", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "false && false && false || echo recovered", "bash_cmp": True},

    # --- Subshells ---
    {"cat": "Subshells", "cmd": "(echo inside subshell)", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(export SUB_VAR=sub); echo $SUB_VAR", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "( (echo nested) )", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "( ( (echo deep_nested) ) )", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(echo hello) > /tmp/ms_sub_out.txt; cat /tmp/ms_sub_out.txt; rm -f /tmp/ms_sub_out.txt", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(cd /tmp && pwd); pwd", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(exit 42); echo $?", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(echo a && false) || echo b", "bash_cmp": True},

    # --- Complex Mix ---
    {"cat": "Complex Mix", "cmd": "(echo a; echo b) | grep a && echo found || echo missing", "bash_cmp": True},
    {"cat": "Complex Mix", "cmd": "(cd /tmp && pwd) && pwd", "bash_cmp": True},
    {"cat": "Complex Mix", "cmd": "echo 1; (echo 2 && echo 3) | cat; echo 4", "bash_cmp": True},
    {"cat": "Complex Mix", "cmd": "(echo sub1 && (echo sub2 || echo sub3)) | cat > /tmp/ms_mix.txt && cat /tmp/ms_mix.txt; rm -f /tmp/ms_mix.txt", "bash_cmp": True},
    {"cat": "Complex Mix", "cmd": "false || (echo failed_first && echo recovering) | grep recovering", "bash_cmp": True},
    {"cat": "Complex Mix", "cmd": "echo background_job &", "bash_cmp": False},

    # --- Expansions & Quotes ---
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
    {"cat": "Expansions", "cmd": "echo \"$USER$HOME\"", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo \"$USER $HOME\"", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo '$USER$HOME'", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo \"$$USER\"", "bash_cmp": False},
    {"cat": "Expansions", "cmd": "echo \"$? $? $?\"", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "export A=1; echo \"$A$A$A\"", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo $\"USER\"", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo $'USER'", "bash_cmp": True},

    # --- Quotes & Parsing ---
    {"cat": "Quotes & Parsing", "cmd": "echo '' '' '   ' ''", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"\" \"   \" \"\"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo   a    b      c  ", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo 'a   b   c'", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"a   b   c\"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"'hello'\"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo '\"hello\"'", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"$USER's laptop\"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo 'nested \"double\" quotes'", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"nested 'single' quotes\"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"cat\"\"ls\"\"pwd\"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo 'cat''ls''pwd'", "bash_cmp": True},

    # --- Persistent State ---
    {"cat": "Persistent State", "cmd": "export A=10\nexport B=20\necho \"A=$A B=$B\"\nunset A\necho \"A=$A B=$B\"", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "cd /tmp\npwd\ncd ..\npwd", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "export VAR=hello\nexport VAR+=_world\necho $VAR", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "export X=1\n(export X=2; echo \"subshell X=$X\")\necho \"parent X=$X\"", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "cd /tmp\n(cd /var; echo \"subshell pwd=\"; pwd)\necho \"parent pwd=\"; pwd", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "export FOO=bar\nenv | grep FOO\nunset FOO\nenv | grep FOO", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "ls /does_not_exist\necho \"Status 1: $?\"\nls -d /tmp\necho \"Status 2: $?\"", "bash_cmp": True},

    # --- Rug Pull ---
    {"cat": "Rug Pull", "cmd": "mkdir -p /tmp/ms_rugpull && cd /tmp/ms_rugpull && rm -rf /tmp/ms_rugpull && pwd", "bash_cmp": True},
    {"cat": "Rug Pull", "cmd": "mkdir -p /tmp/ms_rugpull && cd /tmp/ms_rugpull && rm -rf /tmp/ms_rugpull && cd .", "bash_cmp": True},
    {"cat": "Rug Pull", "cmd": "mkdir -p /tmp/ms_rugpull && cd /tmp/ms_rugpull && rm -rf /tmp/ms_rugpull && cd ..", "bash_cmp": True},

    # --- Path Resolution & Exec ---
    {"cat": "Path & Exec", "cmd": "env /tmp", "bash_cmp": True},
    {"cat": "Path & Exec", "cmd": "/does_not_exist_mini_bin", "bash_cmp": True},
    {"cat": "Path & Exec", "cmd": "''", "bash_cmp": True},
    {"cat": "Path & Exec", "cmd": "..", "bash_cmp": True},

    # --- State Corruption ---
    {"cat": "State Corruption", "cmd": "export PWD=/completely/fake/path; pwd", "bash_cmp": True},
    {"cat": "State Corruption", "cmd": "export PWD=/completely/fake/path; pwd -L", "bash_cmp": True},
    {"cat": "State Corruption", "cmd": "export PWD=/completely/fake/path; pwd -P", "bash_cmp": True},

    # --- Flag Parsing Errors ---
    {"cat": "Flag Errors", "cmd": "cd -Z /tmp", "flag_error": True},
    {"cat": "Flag Errors", "cmd": "pwd -Z", "flag_error": True},
    {"cat": "Flag Errors", "cmd": "export -Z", "flag_error": True},
    {"cat": "Flag Errors", "cmd": "unset -Z", "flag_error": True},
    {"cat": "Flag Errors", "cmd": "env -Z", "flag_error": True},
    {"cat": "Flag Errors", "cmd": "exit -Z", "flag_error": True},

    # =========================================================================
    # --- 200 NEW TEST CASES ---
    # =========================================================================

    # --- Group 1: Extended CD Edge Cases ---
    {"cat": "Builtin: cd", "cmd": "cd /usr/bin/../bin/../../etc", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd /var/tmp/../tmp", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd ///usr///bin///", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "export HOME=/tmp && cd && pwd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "export HOME=/tmp/ && cd && pwd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "export OLDPWD=/usr && cd - && pwd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd -P /usr/bin && pwd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd -L /usr/bin && pwd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd -LP /usr/bin && pwd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd -PL /usr/bin && pwd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd /dev/null", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd /etc/passwd", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd ''", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd non_existent_directory_12345", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd . . .", "bash_cmp": True},

    # --- Group 2: Extended PWD & Env Manipulation ---
    {"cat": "Builtin: pwd", "cmd": "unset PWD && pwd", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "unset PWD && pwd -P", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "unset PWD && pwd -L", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "export PWD=\"\" && pwd", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "export PWD=\"/nonexistent\" && pwd", "bash_cmp": True},

    # --- Group 3: Extended Echo & Escape Parsing ---
    {"cat": "Builtin: echo", "cmd": "echo -n -n -n", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -nnnnn -nnnn -n hello", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -n-n hello", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo --n hello", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -n- hello", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e 'a\\tb\\nc\\rd'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e 'a\\vb\\fc'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e '\\0101\\0102\\0103'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e '\\x41\\x42\\x43'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e '\\a\\b\\e'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e 'hello\\cworld'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -E 'hello\\nworld'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -nE 'hello\\nworld'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -En 'hello\\nworld'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e -n 'hello\\nworld'", "bash_cmp": True},

    # --- Group 4: Extended Export Edge Cases ---
    {"cat": "Builtin: export", "cmd": "export A= B= C=", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export A=1 B=2 C=3 && echo \"$A $B $C\"", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export _123=test && echo $_123", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR+=1 && export VAR+=2 && echo $VAR", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR=\"line1\nline2\" && echo \"$VAR\"", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR=\"a b c\" && echo $VAR", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR=\"a  b  c\" && echo \"$VAR\"", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export @VAR=123", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR@=123", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR#=123", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export VAR%=123", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export EXPORT_TEST_123=456", "bash_cmp": True},

    # --- Group 5: Extended Unset Edge Cases ---
    {"cat": "Builtin: unset", "cmd": "unset", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset \"\"", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset 123", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset @VAR", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset VAR@", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset HOME && cd", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset PATH && ls", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "export A=1 && unset A && echo $A", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "export A=1 B=2 && unset A B && echo \"$A$B\"", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset IFS && echo hello world", "bash_cmp": True},

    # --- Group 6: Extended Env Edge Cases ---
    {"cat": "Builtin: env", "cmd": "env -i pwd", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -i echo hello", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -u PATH ls", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -u HOME pwd", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env A=1 B=2 env | grep -E 'A=|B='", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env --null", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -0", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env --ignore-environment pwd", "bash_cmp": True},
    {"cat": "Builtin: env", "cmd": "env -i FOO=bar printenv FOO", "bash_cmp": True},

    # --- Group 7: Extended Exit Edge Cases ---
    {"cat": "Builtin: exit", "cmd": "exit 000", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 0001", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit +0", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit +42", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit -0", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 1000", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit -1000", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 9223372036854775806", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit -9223372036854775807", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit \"42\"", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit \"  -42  \"", "bash_cmp": True},

    # --- Group 8: Advanced Syntax & Parsing Errors ---
    {"cat": "Syntax Errors", "cmd": "echo hello | | world", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello ||| world", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello &&& world", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello >>> file", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello <<< word", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello <<<< word", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello > > file", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello < < file", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "(echo 1; echo 2", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo 1; echo 2)", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "((echo 1)", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "(echo 1))", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo 1; ; ; echo 2", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo 1 | ; echo 2", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo 1 && ; echo 2", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo 1 || ; echo 2", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo 'unclosed single quote", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo \"unclosed double quote", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo 'unclosed double inside single\"", "bash_cmp": True},

    # --- Group 9: Complex Redirections ---
    {"cat": "Redirections", "cmd": "cat < /dev/null", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "echo test > /tmp/ms_r1 > /tmp/ms_r2 > /tmp/ms_r3 && cat /tmp/ms_r3; rm -f /tmp/ms_r*", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "echo line1 > /tmp/ms_a && echo line2 >> /tmp/ms_a && cat /tmp/ms_a; rm -f /tmp/ms_a", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "< /etc/passwd grep root | wc -l", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "grep root < /etc/passwd | wc -l", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "cat < /etc/passwd > /tmp/ms_copy && wc -l < /tmp/ms_copy; rm -f /tmp/ms_copy", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "echo 1 > /tmp/ms_file && < /tmp/ms_file cat && rm -f /tmp/ms_file", "bash_cmp": True},
    {"cat": "Redirections", "cmd": "echo hello 2>&1 | cat", "bash_cmp": True},

    # --- Group 10: Multi-stage Pipelines ---
    {"cat": "Pipes", "cmd": "echo a | echo b | echo c", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "cat /etc/passwd | grep -v root | head -n 5 | wc -l", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "echo \"line1\nline2\nline3\" | grep line | wc -l", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "false | false | false | true", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "true | true | true | false", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "echo test | cat | cat | cat | cat | cat", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "ls -l /tmp | head -n 2", "bash_cmp": True},

    # --- Group 11: Boolean Logical Operators & Precedence ---
    {"cat": "Logic Operators", "cmd": "true && true || false", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "false || true && true", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "false || false && true", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "true || false && false", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "echo 1 && false || echo 2 && echo 3", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "false && echo 1 || echo 2 || echo 3", "bash_cmp": True},

    # --- Group 12: Subshell Grouping & Isolation ---
    {"cat": "Subshells", "cmd": "(echo 1; echo 2; echo 3)", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(cd /tmp && pwd); pwd", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(export A=1); echo $A", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(unset PATH); ls", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "((echo subshell_1) && (echo subshell_2))", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(echo a && (echo b || echo c)) | cat", "bash_cmp": True},

    # --- Group 13: Quotes, Backslashes & Concatenation ---
    {"cat": "Quotes & Parsing", "cmd": "echo 'a'\"b\"'c'\"d\"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"'\"'\"'", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo '\"'\"'\"'", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"\"\"\"\"\"hello\"\"\"\"\"\"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo ''''''hello''''''", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"hello \"\"world\"", "bash_cmp": True},

    # --- Group 14: Variable Expansion Combinations ---
    {"cat": "Expansions", "cmd": "export A=a B=b C=c && echo $A$B$C", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "export A=a B=b C=c && echo \"$A $B $C\"", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "export A=a && echo \"$A_B\"", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "export A_B=ab && echo \"$A_B\"", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo \"$UNKNOWN_VAR\"", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo '$UNKNOWN_VAR'", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "echo $UNKNOWN_VAR", "bash_cmp": True},

    # --- Group 15: PATH & Command Execution Resolution ---
    {"cat": "Path & Exec", "cmd": "/bin/echo hello", "bash_cmp": True},
    {"cat": "Path & Exec", "cmd": "/bin/ls -d /tmp", "bash_cmp": True},
    {"cat": "Path & Exec", "cmd": "./non_existent_binary", "bash_cmp": True},
    {"cat": "Path & Exec", "cmd": "../non_existent_binary", "bash_cmp": True},
    {"cat": "Path & Exec", "cmd": "/usr/bin/touch /tmp/ms_touch && ls /tmp/ms_touch; rm -f /tmp/ms_touch", "bash_cmp": True},

    # --- Group 16: Additional Builtin Combinations ---
    {"cat": "Builtin: export", "cmd": "export VAR=value && env | grep VAR=", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "export VAR=value && unset VAR && env | grep VAR=", "bash_cmp": True},
    {"cat": "Builtin: cd", "cmd": "cd /usr && cd share && pwd", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo \"line 1\nline 2\"", "bash_cmp": True},
    {"cat": "Builtin: pwd", "cmd": "cd /tmp && pwd", "bash_cmp": True},

    # --- Group 17: Environment Edge Cases ---
    {"cat": "Persistent State", "cmd": "export A=1 && export B=$A && echo $B", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "export A=1 && (export A=2) && echo $A", "bash_cmp": True},
    {"cat": "Persistent State", "cmd": "export A=1 && (unset A) && echo $A", "bash_cmp": True},

    # --- Group 18: File Descriptor Leak / Stress Checks ---
    {"cat": "Redirections", "cmd": "echo 1 > /tmp/f1 && echo 2 > /tmp/f2 && cat /tmp/f1 /tmp/f2; rm -f /tmp/f1 /tmp/f2", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "echo 1 | cat | cat | cat | cat", "bash_cmp": True},

    # --- Group 19: Additional Syntax Edge Cases ---
    {"cat": "Syntax Errors", "cmd": "echo hello > > world", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello < < world", "bash_cmp": True},
    {"cat": "Syntax Errors", "cmd": "echo hello | | world", "bash_cmp": True},

    # --- Group 20: Exit Status Verification ($?) ---
    {"cat": "Expansions", "cmd": "true; echo $?", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "false; echo $?", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "ls /does_not_exist; echo $?", "bash_cmp": True},
    {"cat": "Expansions", "cmd": "expr 1 + 1; echo $?", "bash_cmp": True},

    # --- Group 21: Quotes with Dollar Signs ---
    {"cat": "Quotes & Parsing", "cmd": "echo \"$\"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo '$'", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo \"$ \"", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo '$ '", "bash_cmp": True},

    # --- Group 22: Mixed Whitespace Handling ---
    {"cat": "Quotes & Parsing", "cmd": "echo\thello\tworld", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "echo   \t   hello   \t   world   ", "bash_cmp": True},

    # --- Group 23: Double Operators & Separators ---
    {"cat": "Control & Semicolons", "cmd": "echo 1 ; echo 2 ; echo 3 ;", "bash_cmp": True},
    {"cat": "Control & Semicolons", "cmd": "echo 1; echo 2; echo 3;", "bash_cmp": True},

    # --- Group 24: Complex Subshell Piping ---
    {"cat": "Subshells", "cmd": "(echo 1; echo 2) | (grep 1)", "bash_cmp": True},
    {"cat": "Subshells", "cmd": "(echo 1) | (echo 2) | (echo 3)", "bash_cmp": True},

    # --- Group 25: Special Escape Sequence Stress ---
    {"cat": "Builtin: echo", "cmd": "echo -e 'a\\bb'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e 'a\\rb'", "bash_cmp": True},
    {"cat": "Builtin: echo", "cmd": "echo -e 'a\\\\b'", "bash_cmp": True},

    # --- Group 26: Export Identifier Validation ---
    {"cat": "Builtin: export", "cmd": "export _=valid", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export _A_B_C=123", "bash_cmp": True},
    {"cat": "Builtin: export", "cmd": "export A123=123", "bash_cmp": True},

    # --- Group 27: Unset Special Variables ---
    {"cat": "Builtin: unset", "cmd": "unset PWD && echo $PWD", "bash_cmp": True},
    {"cat": "Builtin: unset", "cmd": "unset SHLVL && echo $SHLVL", "bash_cmp": True},

    # --- Group 28: Path Execution Edge Cases ---
    {"cat": "Path & Exec", "cmd": "/bin/pwd", "bash_cmp": True},
    {"cat": "Path & Exec", "cmd": "/bin/whoami", "bash_cmp": True},

    # --- Group 29: Redirection Appending Verification ---
    {"cat": "Redirections", "cmd": "echo 1 >> /tmp/ms_app && echo 2 >> /tmp/ms_app && cat /tmp/ms_app; rm -f /tmp/ms_app", "bash_cmp": True},

    # --- Group 30: Nested Logic Chains ---
    {"cat": "Logic Operators", "cmd": "true && false || true && echo ok", "bash_cmp": True},
    {"cat": "Logic Operators", "cmd": "false || true && false || echo ok", "bash_cmp": True},

    # --- Group 31: Quote Concatenation with Variables ---
    {"cat": "Quotes & Parsing", "cmd": "export X=hello && echo \"$X\"world", "bash_cmp": True},
    {"cat": "Quotes & Parsing", "cmd": "export X=hello && echo '$X'world", "bash_cmp": True},

    # --- Group 32: Environment Clean Slate ---
    {"cat": "Builtin: env", "cmd": "env -i HOME=/tmp pwd", "bash_cmp": True},

    # --- Group 33: Exit Code Overflow Limits ---
    {"cat": "Builtin: exit", "cmd": "exit 257", "bash_cmp": True},
    {"cat": "Builtin: exit", "cmd": "exit 512", "bash_cmp": True},

    # --- Group 34: Variable Expansion inside Redirections ---
    {"cat": "Redirections", "cmd": "export FILE=/tmp/ms_var_file && echo test > $FILE && cat $FILE; rm -f /tmp/ms_var_file", "bash_cmp": True},

    # --- Group 35: Pipeline Exit Status ---
    {"cat": "Pipes", "cmd": "true | false; echo $?", "bash_cmp": True},
    {"cat": "Pipes", "cmd": "false | true; echo $?", "bash_cmp": True},

    # --- Group 36: Subshell Redirection Inheritance ---
    {"cat": "Subshells", "cmd": "(echo 1; echo 2) > /tmp/ms_sub_out && cat /tmp/ms_sub_out; rm -f /tmp/ms_sub_out", "bash_cmp": True},

    # --- Group 37: Empty Commands and Newlines ---
    {"cat": "Normal Cmds", "cmd": "\n\n", "bash_cmp": True},
    {"cat": "Normal Cmds", "cmd": "   \n   ", "bash_cmp": True},

    # --- Group 38: Multiple Environment Assignments ---
    {"cat": "Builtin: export", "cmd": "export A=1 B=2 C=3 D=4 E=5 && echo $A$B$C$D$E", "bash_cmp": True},

    # --- Group 39: Deep Directory Creation and Navigation ---
    {"cat": "Builtin: cd", "cmd": "mkdir -p /tmp/ms_d1/d2/d3 && cd /tmp/ms_d1/d2/d3 && pwd && cd ../../..; rm -rf /tmp/ms_d1", "bash_cmp": True},

    # --- Group 40: Final Miscellaneous Stress Cases ---
    {"cat": "Complex Mix", "cmd": "echo 1 | grep 1 && (echo 2 || echo 3) > /tmp/ms_final && cat /tmp/ms_final; rm -f /tmp/ms_final", "bash_cmp": True},
]

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

def run_bash(cmd_str, cwd=None):
    try:
        proc = subprocess.run(
            ["bash", "--posix"],
            input=cmd_str,
            capture_output=True,
            text=True,
            cwd=cwd,
            timeout=5
        )
        return proc.stdout, proc.stderr, proc.returncode
    except subprocess.TimeoutExpired:
        return "", "TIMEOUT", -1

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

def normalize_stderr(raw_stderr, is_bash=False):
    if not raw_stderr:
        return ""
    clean = re.sub(r'sh: [0-9]+: getcwd\(\) failed.*\n?', '', raw_stderr)
    if is_bash:
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
    root_dir = os.path.dirname(ms_path)
    cmd_raw = test_item["cmd"]

    # Rewrite /tmp/ms_ paths per shell to isolate Minishell and Bash file mutations
    ms_cmd_str = re.sub(r'/tmp/ms_', f'/tmp/ms_t{test_item["id"]}_ms_', cmd_raw)
    bash_cmd_str = re.sub(r'/tmp/ms_', f'/tmp/ms_t{test_item["id"]}_bash_', cmd_raw)

    stdin_input = f"{ms_cmd_str}\nexit $?\n"

    # Automatically transform unset to unset -v for bash so bad names are handled as variables
    bash_cmd_str = re.sub(r'(^|;|&&|\|\||\||\(|\n)(\s*)unset\b(?!\s+-)', r'\1\2unset -v', bash_cmd_str)
    bash_stdin_input = f"{bash_cmd_str}\nexit $?\n"

    result = {
        "id": test_item["id"],
        "cmd": cmd_raw,
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

    raw_ms_out, ms_err, ms_code = run_shell(stdin_input, ms_path, cwd=root_dir)
    raw_bash_out, bash_err, bash_code = run_bash(bash_stdin_input, cwd=root_dir)

    ms_out = normalize_stdout(raw_ms_out)
    bash_out = normalize_stdout(raw_bash_out)

    clean_ms_err = normalize_stderr(strip_hook_output(ms_err), is_bash=False)
    clean_bash_err = normalize_stderr(bash_err, is_bash=True)

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
            norm_ms = normalize_env_output(ms_out)
            norm_bash = normalize_env_output(bash_out)
            out_match = (norm_ms == norm_bash)
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
            proc = subprocess.run(valgrind_cmd, input=stdin_input, capture_output=True, text=True, cwd=root_dir, timeout=10)
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
        env = os.environ.copy()
        env["LD_PRELOAD"] = hook_so_path
        env["LOG_ALLOC_COUNT"] = "1"

        _, ms_err_log, _ = run_shell(stdin_input, ms_path, env=env, cwd=root_dir)
        
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

                _, err_m, code_m = run_shell(stdin_input, ms_path, env=fail_env, cwd=root_dir)
                
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

        ttk.Label(header_card, text="Minishell Executable:", style="Panel.TFrame").pack(side=tk.LEFT, padx=(0, 6))
        self.ms_path_var = tk.StringVar(value="./minishell")
        ttk.Entry(header_card, textvariable=self.ms_path_var, width=22).pack(side=tk.LEFT, padx=(0, 6))
        ttk.Button(header_card, text="Browse", command=self._browse_binary).pack(side=tk.LEFT, padx=(0, 10))

        ttk.Button(header_card, text="🔨 Recompile", command=self._recompile_minishell).pack(side=tk.LEFT, padx=(0, 16))

        ttk.Label(header_card, text="Threads:", style="Panel.TFrame").pack(side=tk.LEFT, padx=(0, 6))
        self.jobs_var = tk.IntVar(value=os.cpu_count() or 4)
        ttk.Spinbox(header_card, from_=1, to=32, textvariable=self.jobs_var, width=3).pack(side=tk.LEFT, padx=(0, 16))

        self.chk_bash = tk.BooleanVar(value=True)
        self.chk_valgrind = tk.BooleanVar(value=True)
        self.chk_malloc = tk.BooleanVar(value=True)

        ttk.Checkbutton(header_card, text="Bash Compare", variable=self.chk_bash, style="TCheckbutton").pack(side=tk.LEFT, padx=6)
        ttk.Checkbutton(header_card, text="Valgrind / FDs", variable=self.chk_valgrind, style="TCheckbutton").pack(side=tk.LEFT, padx=6)
        ttk.Checkbutton(header_card, text="Malloc Faults", variable=self.chk_malloc, style="TCheckbutton").pack(side=tk.LEFT, padx=6)

        self.btn_run = ttk.Button(header_card, text="▶  Run Selected (F5)", style="Accent.TButton", command=self.run_tests)
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

        custom_card = ttk.LabelFrame(right_frame, text="Add Custom Command Test", padding=8)
        custom_card.pack(fill=tk.X, side=tk.TOP, pady=(0, 8))

        self.custom_cmd_var = tk.StringVar()
        self.custom_cmd_entry = ttk.Entry(custom_card, textvariable=self.custom_cmd_var)
        self.custom_cmd_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 6))
        ttk.Button(custom_card, text="➕ Add Test", command=self._add_custom_test).pack(side=tk.RIGHT)

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

        self.txt_stderr.tag_config("add", background=COLOR_DIFF_ADD_BG, foreground=COLOR_DIFF_ADD_FG)
        self.txt_stderr.tag_config("sub", background=COLOR_DIFF_SUB_BG, foreground=COLOR_DIFF_SUB_FG)
        self.txt_stderr.tag_config("info", foreground=COLOR_ACCENT)

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

        selected_tests = [t for t in self.tests_data if t["selected"]]

        if not selected_tests:
            messagebox.showwarning("Warning", "No tests selected.")
            return

        cleanup_test_artifacts()

        # Unconditionally reset all selected tests to PENDING state and clear old results
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
