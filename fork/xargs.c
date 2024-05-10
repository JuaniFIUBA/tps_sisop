#ifndef NARGS
#define NARGS 4
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void
call_execv(char *path, char *comandos[])
{
	int f = fork();
	if (f < 0) {
		perror("Error al forkear\n");
		exit(EXIT_FAILURE);
	} else if (f > 0)
		wait(NULL);
	else {
		if (execvp(path, comandos) == -1)
			exit(EXIT_FAILURE);
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "La forma de invocar al programa es ./xargs.c <comando>\n");
		return -1;
	}
	char *input = NULL;
	size_t n = 0;

	int count = 1;
	char *cmds[NARGS + 2] = {
		argv[1]
	};  // + 2 para poder ubicar el NULL y el primer argumento que es ingorado por execvp

	while (getline(&input, &n, stdin) != -1) {
		cmds[count] = (char *) malloc(strlen(input) + 1);
		strcpy(cmds[count], input);
		count++;
		if (count == NARGS + 1) {
			cmds[count] = NULL;
			call_execv(argv[1], cmds);
			for (int i = 1; i < count; i++)
				free(cmds[i]);
			count = 1;
		}
	}

	// check que no hayan quedado inputs sin enviar
	if (count > 1) {
		cmds[count] = NULL;
		call_execv(argv[1], cmds);
		for (int i = 1; i < count; i++)
			free(cmds[i]);
	}

	free(input);


	return 0;
}