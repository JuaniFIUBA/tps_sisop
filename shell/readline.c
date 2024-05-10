#include <unistd.h>
#include <termios.h>
#include "defs.h"
#include "readline.h"
#include <assert.h>
#include <ctype.h>
#define CHAR_NL '\n'
#define CHAR_EOF '\004'
#define CHAR_BACK '\b'
#define CHAR_DEL 127
#define CHAR_ESC '\033'

const int PRINCIPIO_PANTALLA = 0;

static char buffer[BUFLEN];

struct termios saved_attributes;

void eliminar_caracter_de_pantalla(int *line_pos);
void borrar_linea_actual(int *line_pos);
void flecha_arriba(int *line_pos);
void flecha_abajo(int *line_pos);
void flecha_derecha(int *line_pos);
void flecha_izquierda(int *line_pos);
void escribir_caracter(int *line_pos, char c);
void flecha_derecha_con_control(int *line_pos);
void flecha_izquierda_con_control(int *line_pos);

void
reset_input_mode(void)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}

void
set_input_mode(void)
{
	struct termios tattr;

	/* Make sure stdin is a terminal. */
	if (!isatty(STDIN_FILENO)) {
		fprintf(stderr, "Not a terminal.\n");
		exit(EXIT_FAILURE);
	}

	/* Save the terminal attributes so we can restore them later. */
	tcgetattr(STDIN_FILENO, &saved_attributes);

	/* Set the funny terminal modes. */
	tcgetattr(STDIN_FILENO, &tattr);
	/* Clear ICANON and ECHO. We'll do a manual echo! */
	tattr.c_lflag &= ~(ICANON | ECHO);
	/* Read one char at a time */
	tattr.c_cc[VMIN] = 1;
	tattr.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
}


void
eliminar_caracter_de_pantalla(int *line_pos)
{
	assert(write(STDOUT_FILENO, "\b \b", 3) > 0);
	if ((size_t) *line_pos == strlen(buffer)) {
		(*line_pos)--;
		buffer[*line_pos] = '\0';
		return;
	}
	char nuevo_buffer[BUFLEN] = { 0 };
	assert(write(STDOUT_FILENO, nuevo_buffer, BUFLEN) > 0);
	strcpy(nuevo_buffer, buffer + *line_pos);
	nuevo_buffer[strlen(nuevo_buffer)] = ' ';
	(*line_pos)--;
	assert(write(STDOUT_FILENO, nuevo_buffer, strlen(nuevo_buffer)) > 0);
	nuevo_buffer[strlen(nuevo_buffer) - 1] = '\0';
	strcpy(buffer + *line_pos, nuevo_buffer);

	for (int i = strlen(buffer); i > *line_pos - 1; i--)
		assert(write(STDOUT_FILENO, "\033[D", 3) > 0);
}

void
borrar_linea_actual(int *line_pos)
{
	while (true) {
		if (*line_pos == PRINCIPIO_PANTALLA) {
			// estamos al comienzo de la pantalla
			return;
		}

		eliminar_caracter_de_pantalla(line_pos);
	}
}

void
flecha_arriba(int *line_pos)
{
	borrar_linea_actual(line_pos);

	get_previous_cmd(h, buffer);
	int len_comando = strlen(buffer) - 1;

	if (len_comando < 0) {
		return;
	}

	buffer[len_comando] = '\0';
	assert(write(STDOUT_FILENO, buffer, len_comando) > 0);
	*line_pos = len_comando;
}


void
flecha_abajo(int *line_pos)
{
	borrar_linea_actual(line_pos);

	get_next_cmd(h, buffer);
	int len_comando = strlen(buffer) - 1;

	if (len_comando < 0) {
		return;
	}

	buffer[len_comando] = '\0';  // elimina el '/n'
	assert(write(STDOUT_FILENO, buffer, len_comando) > 0);
	*line_pos = len_comando;
}

void
flecha_derecha(int *line_pos)
{
	if ((size_t) *line_pos >= strlen(buffer)) {
		return;
	}
	assert(write(STDOUT_FILENO, &buffer[*line_pos], 1) > 0);
	(*line_pos)++;
}

void
flecha_izquierda(int *line_pos)
{
	if (*line_pos == PRINCIPIO_PANTALLA) {
		return;
	}
	assert(write(STDOUT_FILENO, "\033[D", 3) > 0);
	(*line_pos)--;
}

void
escribir_caracter(int *line_pos, char c)
{
	if (((size_t) (*line_pos) == strlen(buffer)) ||
	    (strlen(buffer) == PRINCIPIO_PANTALLA)) {
		assert(write(STDOUT_FILENO, &c, 1) > 0);
		buffer[*line_pos] = c;
		buffer[*line_pos + 1] = '\0';
		(*line_pos)++;

	} else {
		memmove(buffer + *line_pos + 1,
		        buffer + *line_pos,
		        strlen(buffer) - *line_pos);
		buffer[*line_pos] = c;
		assert(write(STDOUT_FILENO,
		             &buffer[*line_pos],
		             strlen(&buffer[*line_pos])) > 0);
		(*line_pos)++;
		for (int i = strlen(buffer); i > *line_pos; i--)
			assert(write(STDOUT_FILENO, "\033[D", 3) > 0);
	}
}


void
flecha_derecha_con_control(int *line_pos)
{
	bool encontro_palabra = false;
	while (*line_pos < (int) strlen(buffer)) {
		if (buffer[*line_pos] == ' ' && encontro_palabra)
			break;
		assert(write(STDOUT_FILENO, &buffer[*line_pos], 1) > 0);
		(*line_pos)++;
		if (buffer[*line_pos] != ' ')
			encontro_palabra = true;
	}
}

void
flecha_izquierda_con_control(int *line_pos)
{
	bool encontro_palabra = false;
	while (*line_pos > PRINCIPIO_PANTALLA) {
		if (buffer[*line_pos - 1] == ' ' && encontro_palabra)
			break;
		(*line_pos)--;
		assert(write(STDOUT_FILENO, "\033[D", 3) > 0);
		if (buffer[*line_pos] != ' ')
			encontro_palabra = true;
	}
}

// reads a line from the standard input
// and prints the prompt
char *
read_line(const char *prompt)
{
#ifndef SHELL_NO_INTERACTIVE
	if (isatty(1)) {
		fprintf(stdout, "%s %s %s\n", COLOR_RED, prompt, COLOR_RESET);
		fprintf(stdout, "%s", "$ ");
	}
#endif
	memset(buffer, 0, BUFLEN);
	char c;
	int line_pos = 0;
	while (true) {
		assert(read(STDIN_FILENO, &c, 1) > 0);
		switch (c) {
		case CHAR_NL:
			// tecla "enter"
			assert(write(STDOUT_FILENO, &c, 1) > 0);
			reset_input_mode();
			return buffer;
		case CHAR_EOF:
			// teclas "Ctrl-D"
			return NULL;
		case CHAR_DEL:
			// tecla "Backspace"
			if (line_pos == PRINCIPIO_PANTALLA) {
				// estamos al comienzo de la pantalla
				continue;
			}
			actualizar_cmd_actual(h);
			eliminar_caracter_de_pantalla(&line_pos);
			break;
		case CHAR_ESC:
			// comienzo de una sequencia
			// de escape
			char esc_seq;
			assert(read(STDIN_FILENO, &esc_seq, 1) > 0);

			if (esc_seq != '[')
				continue;

			assert(read(STDIN_FILENO, &esc_seq, 1) > 0);
			if (esc_seq == 'A') {
				// flecha "arriba"
				flecha_arriba(&line_pos);
				continue;
			}
			if (esc_seq == 'B') {
				// flecha "abajo"
				flecha_abajo(&line_pos);
				continue;
			}
			if (esc_seq == 'C') {
				// flecha "derecha"
				flecha_derecha(&line_pos);
				continue;
			}
			if (esc_seq == 'D') {
				// flecha "izquierda"
				flecha_izquierda(&line_pos);
				continue;
			}
			if (esc_seq == '1') {
				assert(read(STDIN_FILENO, &esc_seq, 1) > 0);  // ;
				assert(read(STDIN_FILENO, &esc_seq, 1) > 0);  // 5
				assert(read(STDIN_FILENO, &esc_seq, 1) >
				       0);  // D o C (D para izquierda, C para derecha)
				if (esc_seq == 'D') {
					// flecha "izquierda"
					flecha_izquierda_con_control(&line_pos);
					continue;
				}
				if (esc_seq == 'C') {
					// flecha "derecha"
					flecha_derecha_con_control(&line_pos);
					continue;
				}
			}
			break;

		default:
			break;
		}

		if (isprint(c)) {  // si es visible
			actualizar_cmd_actual(h);
			escribir_caracter(&line_pos, c);
		}
	}
}
