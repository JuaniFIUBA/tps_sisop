#include "builtin.h"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	// Your code here
	if (strcmp(cmd, "exit") == 0)
		return 1;
	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	// Your code here
	if (strstr(cmd, "cd")) {
		int result;
		char *actual_path;
		size_t size;
		if (block_contains(cmd, ' ') > 0) {
			char *dir = split_line(cmd, ' ');
			result = chdir(dir);
		} else
			result = chdir(getenv("HOME"));

		if (result < 0)
			return 0;  // no se encontro el dir
		else {
			actual_path = getcwd(NULL, size);
			snprintf(prompt, sizeof prompt, "(%s)", actual_path);
			free(actual_path);
			return 1;
		}
	}
	return 0;
}
// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	// Your code here
	if (strstr(cmd, "pwd")) {
		size_t size;
		char *actual_path = getcwd(NULL, size);
		if (actual_path != NULL) {
			printf("%s\n", actual_path);
			free(actual_path);
			return 1;
		}
	}
	return 0;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	if (!strstr(cmd, "history"))
		return 0;

	FILE *fp;
	char buf[BUFLEN] = { 0 };
	// if(block_contains(cmd, ' ')) // fijarse como llegar al numero sin preguntar esto
	int n = atoi(split_line(cmd, ' '));
	int not_n = h->cmd_count - n;

	int count = 0;
	fp = fopen(h->hist_path, "r");
	if (fp == NULL)
		perror("No se encontro el directorio\n");


	while (fgets((buf), sizeof(buf), fp) != NULL) {
		if (n == 0) {
			printf("%s", buf);
			continue;
		}

		if (count < not_n) {
			count++;
		} else {
			printf("%s", buf);
		}
	}

	fclose(fp);
	return 1;
}
