#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHELL_LINE_MAX 128


void shell_run(void);

void shell_init(void);


int shell_readline(char *buf, size_t buflen);

#ifdef __cplusplus
}
#endif

#endif 
