#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_LARGO_TOKENS 1024
pid_t pid_hijo;

void
timer_handler()
{
	printf("Se termino el tiempo, eliminando el comando.\n");

	if (pid_hijo > 0) {
		kill(pid_hijo, SIGTERM);
	}
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Uso: %s <tiempo> <comando>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	long timeout_seconds = strtol(argv[1], NULL, 10);
	char *comando = argv[2];


	struct sigevent sev;
	timer_t timerid;
	struct itimerspec its;

	struct sigaction sa;
	sa.sa_handler = timer_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGALRM;
	sev.sigev_value.sival_ptr = &timerid;
	if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) {
		perror("timer_create");
		exit(EXIT_FAILURE);
	}

	its.it_value.tv_sec = timeout_seconds;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if (timer_settime(timerid, 0, &its, NULL) == -1) {
		perror("error en timer_settime");
		exit(EXIT_FAILURE);
	}

	if ((pid_hijo = fork()) == -1) {
		perror("error en fork");
		exit(EXIT_FAILURE);
	}

	if (pid_hijo == 0) {
		// HIJO
		printf("Ejecutando comando: %s\n", comando);
		char *tokens[MAX_LARGO_TOKENS];
		int token_count = 0;

		char *token = strtok(comando, " ");
		while (token != NULL) {
			tokens[token_count++] = token;
			token = strtok(NULL, " ");
		}

		tokens[token_count] = NULL;

		execvp(comando, tokens);

		perror("execvp fallo");
		exit(EXIT_FAILURE);
	} else {
		// Padre
		int status;
		if (waitpid(pid_hijo, &status, 0) == -1) {
			perror("waitpid");
			exit(EXIT_FAILURE);
		}

		if (timer_delete(timerid) == -1) {
			perror("error en timer_delete");
			exit(EXIT_FAILURE);
		}

		if (WIFEXITED(status)) {
			printf("El comando se completo antes del timeout.\n");
			return WEXITSTATUS(status);
		} else if (WIFSIGNALED(status)) {
			printf("El comando no termino en tiempo.\n");
			return EXIT_FAILURE;
		} else {
			printf("La ejecucion del comando fallo.\n");
			return EXIT_FAILURE;
		}
	}
}
