#ifndef _CRT_STRUCTS_H
#define _CRT_STRUCTS_H

#ifdef __cplusplus
extern "C" {
#endif

struct exitData {
    int argc;
    char** argv;
    char** envp;
    int exit_code;
    const char* error_message;
};

extern struct exitData exit_data;

extern void main(struct exitData*);

#ifdef __cplusplus
};
#endif

#endif
