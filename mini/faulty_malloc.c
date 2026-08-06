#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

void *malloc(size_t size) {
    static void *(*real_malloc)(size_t) = NULL;
    static int in_malloc = 0;
    static int target_alloc = -1;
    static int init_env = 0;
    static int alloc_count = 0;
    Dl_info info;

    if (!real_malloc)
        real_malloc = dlsym(RTLD_NEXT, "malloc");

    /* Prevent recursive interception */
    if (in_malloc)
        return real_malloc(size);

    in_malloc = 1;

    if (!init_env) {
        char *env = getenv("FAIL_MALLOC_AT");
        if (env)
            target_alloc = atoi(env);
        init_env = 1;
    }

    if (target_alloc > 0 && dladdr(__builtin_return_address(0), &info) && info.dli_fname) {
        if (strstr(info.dli_fname, "minishell")) {
            /* Skip glibc / readline internal symbols */
            int is_libc_internal = 0;
            if (info.dli_sname) {
                if (strstr(info.dli_sname, "tsearch") || 
                    strstr(info.dli_sname, "environ") || 
                    strstr(info.dli_sname, "readline") || 
                    strstr(info.dli_sname, "rl_"))
                    is_libc_internal = 1;
            }
            if (!is_libc_internal) {
                alloc_count++;
                if (alloc_count == target_alloc) {
                    if (getenv("TRACE_MALLOC")) {
                        raise(SIGTRAP);
                    }
                    in_malloc = 0;
                    return NULL; /* SABOTAGE! */
                }
            }
        }
    }
    in_malloc = 0;
    return real_malloc(size);
}
