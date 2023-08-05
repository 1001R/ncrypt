#ifndef _UTIL_H_
#define _UTIL_H_

#include <stddef.h>

enum tty_echo {
  RS_ECHO_OFF  = 0x00,		/* Turn off echo (default). */
  RS_ECHO_ON   = 0x01,		/* Leave echo on. */
  RS_ECHO_MASK = 0x02,
};

// #include <stddef.h>

int readall(int fd, void *buf, size_t bufsz);

int readsecret(void *secret, size_t *slen);
// char *tty_readsecret(const char *prompt, char *buf, size_t bufsiz, int echo);

#endif