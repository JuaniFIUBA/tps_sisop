#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>

#define BUFLEN 1024

int
es_numero(const char *str)
{
	char *final;
	strtol(str, &final, 10);

	return (*str != '\0' && *final == '\0');
}

int
main(void)
{
	DIR *dir;
	struct dirent *entrada;
	char direccion[BUFLEN] = "/proc/";

	if ((dir = opendir(direccion)) == NULL) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	while ((entrada = readdir(dir)) != NULL) {
		char *nombre = entrada->d_name;
		if (es_numero(nombre)) {
			char comm_path[BUFLEN];
			snprintf(comm_path,
			         sizeof(comm_path),
			         "%s%s",
			         direccion,
			         nombre);
			FILE *comm_data = fopen(strcat(comm_path, "/comm"), "r");
			char linea[BUFLEN];
			while (fgets(linea, sizeof(linea), comm_data) != NULL) {
			}
			printf("%s %s", entrada->d_name, linea);
		}
	}

	closedir(dir);
	return EXIT_SUCCESS;
}