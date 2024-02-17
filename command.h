#ifndef _COMMAND_H_
#define _COMMAND_H_

typedef struct {
  const char *name;
  struct option *options;

} command_t;

#endif