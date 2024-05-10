#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"
#include "hist.h"
#include <termios.h>

char prompt[PRMTLEN] = { 0 };
stack_t ss;
struct sigaction sa;
struct history *h;

static void
handler()
{
	pid_t pgid = waitpid(0, &status, WNOHANG);
	if (pgid > 0) {
		char buf[20];

		if (snprintf(buf, 20, "%d\n$ ", pgid) < 0)
			perror("Error en snprintf, handler");

		if (write(1, buf, strlen(buf)) < 0)
			perror("Error al escribir, handler");
	}
	return;
}
static void
init_handler()
{
	sa.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESTART;

	if (sigfillset(&sa.sa_mask) < 0) {
		perror("Error al bloquear las signals");
		exit(EXIT_FAILURE);
	}

	sa.sa_sigaction = handler;

	ss.ss_sp = malloc(SIGSTKSZ);
	if (ss.ss_sp == NULL) {
		perror("Error al reservar memoria para el stack secundario");
		exit(EXIT_FAILURE);
	}

	ss.ss_size = SIGSTKSZ;

	ss.ss_flags = 0;
	if (sigaltstack(&ss, NULL)) {
		perror("sigaltstack");
		exit(EXIT_FAILURE);
	}

	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		perror("Error en sigaction");
		exit(EXIT_FAILURE);
	}
}

// runs a shell command
static void
run_shell()
{
	char *cmd;
	size_t size;
	char *path;
	set_input_mode();
	while ((cmd = read_line(prompt)) != NULL) {
		path = getcwd(NULL, size);
		add_command(h, cmd);
		snprintf(prompt, sizeof prompt, "(%s)", path);
		free(path);
		if (run_cmd(cmd) == EXIT_SHELL) {
			return;
		}
		set_input_mode();
	}
}
// initializes the shellz
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		init_handler();
		snprintf(prompt, sizeof prompt, "(%s)", home);
		h = init_hist();
	}
}

int
main(void)
{
	init_shell();

	run_shell();
	free(ss.ss_sp);
	end_hist(h);
	reset_input_mode();
	return 0;
}
