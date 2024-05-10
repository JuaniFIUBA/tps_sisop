#include "hist.h"
#include <ctype.h>
bool esta_vacio_cmd(const char *cmd);

struct history *
init_hist()
{
	struct history *h;
	h = (struct history *) malloc(sizeof(*h));
	char *path;
	if ((path = getenv("HISTFILE")) == NULL) {
		// ruta es ~/.fisop_history
		h->hist_path = strcat(getenv("HOME"), "/.fisop_history");
	} else
		h->hist_path = path;

	FILE *f = fopen(h->hist_path, "w+");
	fclose(f);
	h->cmd_count = 0;
	h->actual_cmd = 0;
	return h;
}

// Post:actualiza el cmd actual, colocandolo en la
//       siguiente al ultimo comando escrito.
//       (el actual es la linea que se esta por
//       escribir)
void
actualizar_cmd_actual(struct history *h)
{
	h->actual_cmd = h->cmd_count + 1;
}

bool
esta_vacio_cmd(const char *cmd)
{
	size_t longitud = strlen(cmd);

	for (size_t i = 0; i < longitud; i++) {
		if (!isspace((unsigned char) cmd[i])) {
			// Si encontramos un carácter que no es un espacio en blanco, la cadena no está vacía
			return false;
		}
	}
	return true;
}

// Post: añade un nuevo comando al historial. Aumenta la
//       cantidad de comandos guardados. Actualiza el
//       cmd actual. Si el cmd esta vacio, no lo agrega.
void
add_command(struct history *h, char *cmd)
{
	if (esta_vacio_cmd(cmd))
		return;

	FILE *f = fopen(h->hist_path, "a");
	fprintf(f, "%s\n", cmd);
	fclose(f);
	h->cmd_count++;
	actualizar_cmd_actual(h);
}

// Post:Libera la memoria del historial
void
end_hist(struct history *h)
{
	free(h);
}

// Post: devulve el comando proximo al actual cmd en buf,
//       decrementado el cmd actual.
//       Si no existe cmd proximo, buf queda vacio.
void
get_next_cmd(struct history *h, char *buf)
{
	FILE *fp;

	int count = 0;
	fp = fopen(h->hist_path, "r");
	if (fp == NULL)
		perror("No se encontro el directorio\n");

	memset(buf, 0, BUFLEN);

	// el actual cmd puede ser uno mayor que la cantidad actual
	// la linea que se esta por escribir seria
	if (h->actual_cmd < h->cmd_count + 1) {
		h->actual_cmd += 1;
	}
	// si esta en el comando actual, que no haga nada
	if (h->actual_cmd == h->cmd_count + 1) {
		return;
	}

	while (fgets((buf), BUFLEN, fp) != NULL) {
		if (count < h->actual_cmd - 1) {
			count++;
		} else {
			break;
		}
	}
	fclose(fp);
}

// Post: devulve el comando previo al actual cmd en buf,
//       decrementado el cmd actual.
//       Si no existe previo cmd, buf queda vacio.
void
get_previous_cmd(struct history *h, char *buf)
{
	FILE *fp;

	if (h->actual_cmd > 1) {
		h->actual_cmd -= 1;
	}

	memset(buf, 0, BUFLEN);
	int count = 0;
	fp = fopen(h->hist_path, "r");
	if (fp == NULL)
		perror("No se encontro el directorio\n");

	while (fgets((buf), BUFLEN, fp) != NULL) {
		if (count < h->actual_cmd - 1) {
			count++;
		} else {
			break;
		}
	}
	fclose(fp);
}
