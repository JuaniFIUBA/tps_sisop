#ifndef HIST_H
#define HIST_H

#include "defs.h"
struct history {
	char *hist_path;
	int cmd_count;  // cant de comandos que guarda el file
	int actual_cmd;
};

struct history *init_hist(void);
void add_command(struct history *h, char *cmd);
void end_hist(struct history *h);
void get_next_cmd(struct history *h, char *buf);
void get_previous_cmd(struct history *h, char *buf);
void actualizar_cmd_actual(struct history *h);

// int print_hist(void);

#endif