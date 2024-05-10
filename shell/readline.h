#ifndef READLINE_H
#define READLINE_H
#include "hist.h"

extern struct history *h;
char *read_line(const char *prompt);
void reset_input_mode(void);
void set_input_mode(void);
#endif  // READLINE_H
