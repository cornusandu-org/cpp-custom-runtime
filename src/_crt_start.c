#include "../include/_crt_structs.h"
#include <stdint.h>
#include <stdlib.h>

#define MANGLE_CCRT_SYMBOL(x) __asm__(".globl __X" #x "__CCRT\n\t" \
"__X" #x "__CCRT:\n\t" \
"call " #x "\n\t" \
"ret");
#define HIDE_SYMBOL __attribute__((visibility("hidden")))
#define FAKE_SYMBOL(x) __asm__(".globl __X" #x "__CCRT\n\t" "__X" #x "__CCRT:\n\t" "ret");
#define FAKE_SYMBOL_PLAIN(x) __asm__(".globl " #x "\n\t" #x ":\n\t" "ret");

struct exitData exit_data;

extern int __libc_start_main(
    int (*main)(int, char**, char**),
    int argc,
    char** argv,
    void (*init)(void),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void* stack_end
);

HIDE_SYMBOL
void* __dso_handle = &__dso_handle;

__attribute__((noreturn))
void _start() {
    asm volatile(
        "xor %%rbp, %%rbp\n"
        "andq $-16, %%rsp\n"   // align stack
        "mov %%rsp, %%rdi\n"
        "call _crt_runtime\n"
        ::: "memory"
    );
}

extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);
extern void (*__fini_array_start[])(void);
extern void (*__fini_array_end[])(void);

HIDE_SYMBOL
void run_init_array() {
    void (**fn)(void);
    for (fn = __init_array_start; fn < __init_array_end; ++fn) {
        (*fn)();
    }
}

HIDE_SYMBOL
void run_fini_array() {
    void (**fn)(void);
    for (fn = __fini_array_start; fn < __fini_array_end; ++fn) {
        (*fn)();
    }
}

extern int user_main_trampoline(int argc, char** argv, char** envp);

void _crt_runtime(void* stack) {
    long argc = *(long*)stack;
    char** argv = (char**)((long*)stack + 1);
    char** envp = argv + argc + 1;

    exit_data.argc = argc;
    exit_data.argv = argv;
    exit_data.envp = envp;
    exit_data.exit_code = 0;
    exit_data.error_message = NULL;

    // 1. Run global constructors
   // run_init_array();

    __libc_start_main(
        user_main_trampoline,
        (int)argc,
        argv,
        NULL,   // init (libc will handle)
        NULL,   // fini
        NULL,   // rtld_fini
        stack
    );

    __builtin_unreachable();
}


uint64_t strlen(const char* const str) {
    if (!str) return 0x0;
    uint64_t counter = 0;
    while (*(str + counter) != '\0') {
        counter++;
    }
    return counter;
}

void  __attribute__((visibility("hidden"))) sys_write(const char *str, uint64_t len) {
    if (!str) return;
    asm volatile (
        "syscall"
        :
        : "a"(1),          // SYS_write = 1
          "D"(1),          // fd = 1 (stdout)
          "S"(str),        // buf
          "d"(len)         // count
        : "rcx", "r11", "memory"
    );
    asm volatile("nop" :::"rax");
}

__attribute__((weak))
void main(struct exitData *exit_data) {
    sys_write("Hello, world!", strlen("Hello, world!"));
}

__attribute__((noreturn)) __attribute__((visibility("hidden")))
void sys_exit(int code) {
    asm volatile (
        "syscall"
        :
        : "a"(231),   // SYS_exit_group
          "D"(code)
        : "rcx", "r11", "memory"
    );
    __builtin_unreachable();
}

void _exitccrt();

int user_main_trampoline(int argc, char** argv, char** envp) {
    atexit(_exitccrt);

    main(&exit_data);

    //run_fini_array();

    return exit_data.exit_code;

    __builtin_unreachable();
}

__attribute__((visibility("hidden")))
void _exitccrt() {
    sys_write(exit_data.error_message, strlen(exit_data.error_message));
}

MANGLE_CCRT_SYMBOL(_exitccrt);
MANGLE_CCRT_SYMBOL(__fini_array_end);
MANGLE_CCRT_SYMBOL(__fini_array_start);
MANGLE_CCRT_SYMBOL(strlen);

FAKE_SYMBOL(_crt_start);
FAKE_SYMBOL(_crt_runtime);
FAKE_SYMBOL_PLAIN(_crt_start);
FAKE_SYMBOL_PLAIN(_crt_runtime_start);
FAKE_SYMBOL_PLAIN(ccrt__strlen);

FAKE_SYMBOL_PLAIN(_crt_onexit);
FAKE_SYMBOL(crt_atexit);
FAKE_SYMBOL(ccrt_build);
FAKE_SYMBOL_PLAIN(__ccrt_build_Z__);
