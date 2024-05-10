#ifndef ENTRADA_ENLACE_SIMBOLICO_H
#define ENTRADA_ENLACE_SIMBOLICO_H

#include "fisopfs.h"


typedef struct entrada_enlace_simbolico {
	char nombre[MAXIMO_PATH];
	struct stat st;
	char tipo;
	char enlace[MAXIMO_PATH];
} entrada_enlace_simbolico_t;


// Crear un enlace simbolico en enl_simbolico. Si hay un error lo devuleve.
// Caso de exito devuelve 0.
int crear_enlace_simbolico(entrada_enlace_simbolico_t *enl_simbolico,
                           char *nombre,
                           const char *enlace);

#endif