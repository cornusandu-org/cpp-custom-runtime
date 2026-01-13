#include "include/_crt_structs.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

void main(struct exitData* exit_data) {
    printf("Test\n");

    exit_data->error_message="Hello!";
    return;
}

// gcc ./src/_crt_start.c ./main.c -o main -nostartfiles
