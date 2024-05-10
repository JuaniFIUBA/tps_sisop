#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#define BUFLEN 1024


void
printear_tipo(struct stat sb)
{
	if (S_ISDIR(sb.st_mode)) {
		printf("d");
	} else if (S_ISREG(sb.st_mode)) {
		printf("-");
	} else {
		printf("l");
	}
}
void
printear_permisos(struct stat sb)
{
	if (sb.st_mode & S_IRUSR) {
		printf("r");
	} else {
		printf("-");
	}
	if (sb.st_mode & S_IWUSR) {
		printf("w");
	} else {
		printf("-");
	}
	if (sb.st_mode & S_IXUSR) {
		printf("x");
	} else {
		printf("-");
	}
	if (sb.st_mode & S_IRGRP) {
		printf("r");
	} else {
		printf("-");
	}
}

void
printear_link_simbolico(char *ruta)
{
	char linkname[BUFLEN];
	ssize_t r;
	r = readlink(ruta, linkname, BUFLEN);
	if (r == -1) {
		perror("readlink");
		exit(EXIT_FAILURE);
	}
	linkname[r] = '\0';
	printf("-> %s\n", linkname);
}

int
main(int argc, char *argv[])
{
	DIR *dir;
	struct dirent *entrada;
	char direccion[BUFLEN] = { 0 };
	if (argc > 2) {
		printf("Uso: %s [directorio]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	if (argc > 1) {
		strcpy(direccion, argv[1]);

	} else {
		direccion[0] = '.';
	}
	if ((dir = opendir(direccion)) == NULL) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	while ((entrada = readdir(dir)) != NULL) {
		struct stat sb;
		char ruta[BUFLEN] = { 0 };
		strcpy(ruta, direccion);
		strcat(ruta, "/");
		strcat(ruta, entrada->d_name);
		if (lstat(ruta, &sb) == -1) {
			perror("stat");
			exit(EXIT_FAILURE);
		}
		printear_tipo(sb);
		printear_permisos(sb);

		printf(" ");
		printf("%d ", (int) sb.st_uid);
		char *nombre = entrada->d_name;

		printf("%s ", nombre);

		if (S_ISLNK(sb.st_mode)) {
			printear_link_simbolico(ruta);
		} else {
			printf("\n");
		}
	}
	closedir(dir);
	return EXIT_SUCCESS;
}