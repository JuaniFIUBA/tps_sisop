#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1


void
pasamanos(int left_pipe_readfd)
{
	int num_filtro;
	ssize_t b_read = read(left_pipe_readfd, &num_filtro, sizeof(num_filtro));

	// Checkeo que no haya fallado o que lea 0 bytes para cortar
	if (b_read == -1) {
		fprintf(stderr, "Error al leer de la tuberia izquierda\n");
		close(left_pipe_readfd);
		exit(EXIT_FAILURE);

	} else if (b_read == 0) {
		close(left_pipe_readfd);
		exit(EXIT_SUCCESS);
	}

	printf("primo %d\n", num_filtro);
	int right_pipefd[2];

	if (pipe(right_pipefd) == -1) {
		fprintf(stderr,
		        "Error al abrir la pipe en el proceso <%d>\n",
		        getpid());
		close(left_pipe_readfd);
		exit(EXIT_FAILURE);
	}

	int f = fork();

	if (f < 0) {
		fprintf(stderr,
		        "Error al hacer fork en el proceso <%d>\n",
		        getpid());

		close(left_pipe_readfd);
		close(right_pipefd[READ]);
		close(right_pipefd[WRITE]);
		exit(EXIT_FAILURE);

	} else if (f > 0) {
		//----PADRE----
		close(right_pipefd[READ]);
		int num;
		ssize_t bytes_read = 0;

		while (1) {
			bytes_read = read(left_pipe_readfd, &num, sizeof(num));
			if (bytes_read == -1) {
				fprintf(stderr, "Error al leer\n");
				close(left_pipe_readfd);
				close(right_pipefd[WRITE]);
				exit(EXIT_FAILURE);
			} else if (bytes_read == 0)
				break;

			else {
				if (num % num_filtro != 0) {
					if (write(right_pipefd[WRITE],
					          &num,
					          sizeof(num)) == -1) {
						perror("Error al "
						       "escribir en la "
						       "pipe");
						break;
					}
				}
			}
		}

		close(left_pipe_readfd);
		close(right_pipefd[WRITE]);
		wait(NULL);
		exit(EXIT_SUCCESS);
	} else {
		//----HIJO----
		close(left_pipe_readfd);
		close(right_pipefd[WRITE]);
		pasamanos(right_pipefd[READ]);
	}
}

void
generador_de_numeros(int n)
{
	int pipefd[2];

	if (pipe(pipefd) == -1) {
		fprintf(stderr, "Error al abrir la pipe principal, abortando\n");
		exit(EXIT_FAILURE);
	}

	int f = fork();
	if (f < 0) {
		// Error
		fprintf(stderr, "Error al forkear al proceso principal\n");
		close(pipefd[WRITE]);
		close(pipefd[READ]);
		exit(EXIT_FAILURE);

	} else if (f > 0) {
		// Padre
		close(pipefd[READ]);
		for (int i = 2; i <= n; i++) {
			if (write(pipefd[WRITE], &i, sizeof(i)) == -1) {
				perror("Error al escribir en la pip del "
				       "proceso generador");
				close(pipefd[WRITE]);
				close(pipefd[READ]);
				exit(EXIT_FAILURE);
			}
		}
		close(pipefd[WRITE]);
		wait(NULL);
		exit(EXIT_SUCCESS);
	} else {
		// hijo
		close(pipefd[WRITE]);
		pasamanos(pipefd[READ]);
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Error, la forma de llamar al programa es ./primes <n>, donde n es un numero natural mayor a 1\n");
		return -1;
	}
	int n;
	if ((n = atoi(argv[1])) == 0) {
		fprintf(stderr, "Error al transformar el numero a entero\n");
		return -1;
	}
	generador_de_numeros(n);

	return 0;
}