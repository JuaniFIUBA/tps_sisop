#include "exec.h"

#define ERROR -1
// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	// Your code here
	char key[BUFLEN];
	char value[BUFLEN];
	int idx;
	for (int i = 0; i < eargc; i++) {
		idx = block_contains(eargv[i], '=');
		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, idx);
		setenv(key, value, 0);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
// Pasarle O_CLOEXEC
static int
open_redir_fd(char *file, int flags)
{
	// Your code here
	return open(file, flags, S_IWUSR | S_IRUSR);
}

static void
exec_command(struct execcmd *e)
{
	set_environ_vars(e->eargv, e->eargc);
	execvp(e->argv[0], e->argv);
	_exit(ERROR);
}

static void
dup_file(char file[FNAMESIZE], int oldfd, int flags)
{
	if (strlen(file) > 0) {
		if (dup2(open_redir_fd(file, flags), oldfd) == ERROR) {
			_exit(ERROR);
		}
	}
}

static void
exec_redir(struct execcmd *r)
{
	dup_file(r->in_file, 0, O_CLOEXEC | O_RDONLY);

	dup_file(r->out_file, 1, O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC);

	if (strlen(r->err_file) > 0) {
		if (block_contains(r->err_file, '&') > ERROR) {
			if (dup2(1, 2) == ERROR)
				_exit(ERROR);
		} else {
			dup_file(r->err_file, 2, O_CREAT | O_WRONLY | O_CLOEXEC);
		}
	}

	r->type = EXEC;
	exec_cmd((struct cmd *) r);
	_exit(-1);
}

static void
exec_pipe(struct pipecmd *p)
{
	pid_t pid_r;
	pid_t pid_l;
	int pipefd[2];

	if (pipe(pipefd) == ERROR) {
		perror("Error al abrir la pipe");
		_exit(ERROR);
	}

	if ((pid_r = fork()) < 0) {
		perror("Error al hacer fork");
		_exit(ERROR);
	}

	if (pid_r == 0) {
		// hijo, ejecuto el cmd derecho
		close(pipefd[WRITE]);

		if (dup2(pipefd[READ], 0) < 0) {
			_exit(ERROR);
		}

		close(pipefd[READ]);

		exec_cmd(p->rightcmd);

	} else {
		// padre, ejecuto el cmd izquierdo

		if ((pid_l = fork()) == 0) {
			close(pipefd[READ]);

			if (dup2(pipefd[WRITE], 1) < 0) {
				printf("Fallo el dup");
				_exit(ERROR);
			}

			close(pipefd[WRITE]);

			exec_cmd(p->leftcmd);

		} else {
			close(pipefd[READ]);
			close(pipefd[WRITE]);
			waitpid(pid_r, NULL, 0);
			waitpid(pid_l, NULL, 0);
			_exit(0);
		}
	}
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	switch (cmd->type) {
	case EXEC: {
		// spawns a command
		//
		// Your code here
		setpgid(0, 0);
		exec_command((struct execcmd *) cmd);
		break;
	}

	case BACK: {
		// runs a command in background
		//
		// Your code here
		struct backcmd *b;
		b = (struct backcmd *) cmd;
		exec_command((struct execcmd *) b->c);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		// Your code here

		exec_redir((struct execcmd *) cmd);
		break;
	}

	case PIPE: {
		// free the memory allocated
		// for the pipe tree structure
		// free_command(parsed_pipe);

		exec_pipe((struct pipecmd *) cmd);
		break;
	}
	}
}
