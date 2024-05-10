#include "entrada_enlace_simbolico.h"


int
crear_enlace_simbolico(entrada_enlace_simbolico_t *enl_simbolico,
                       char *nombre,
                       const char *enlace)
{
	enl_simbolico->tipo = ENLACE_SIMBOLICO;
	enl_simbolico->st.st_uid = getuid();
	enl_simbolico->st.st_gid = getgid();
	enl_simbolico->st.st_mode = __S_IFLNK | 0777;

	enl_simbolico->st.st_mtime = time(NULL);
	enl_simbolico->st.st_atime = time(NULL);
	enl_simbolico->st.st_ctime = time(NULL);
	enl_simbolico->st.st_nlink = 1;
	enl_simbolico->st.st_ino = 0;
	enl_simbolico->st.st_blocks = 0;
	enl_simbolico->st.st_blksize = 0;

	if (strlen(nombre) < MAXIMO_PATH) {
		strcpy(enl_simbolico->nombre, nombre);
	} else {
		return -EINVAL;
	}

	enl_simbolico->st.st_size = strlen(enlace);

	if (strlen(enlace) < MAXIMO_PATH) {
		strcpy(enl_simbolico->enlace, enlace);
	} else {
		return -EINVAL;
	}

	return 0;
}