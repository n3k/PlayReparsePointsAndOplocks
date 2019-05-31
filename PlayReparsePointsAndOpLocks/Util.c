#pragma once
#include "ReparsePointAndOpLocks.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#pragma warning(disable : 4996)
WCHAR **parse_arguments(WCHAR *command_line, WCHAR arg_delim) {
	WCHAR delim[2] = { 0 };
	if (arg_delim == NULL) {
		delim[0] = ' ';
	}
	else {
		delim[0] = arg_delim;
	}
	WCHAR **args = (WCHAR **)calloc(1, 0x40);
	WCHAR *token;
	WCHAR **p = args;
	unsigned int argc = 1;

	p++;
	token = wcstok(command_line, delim, NULL);
	while (token != NULL) {
		*p = token;
		token = wcstok(NULL, delim, NULL);
		p++;
		argc++;
	}

	*(unsigned int *)&args[0] = argc;

	return args;
}

void get_user_input(WCHAR *input, int size) {
	memset(input, 0x00, size);
	fgetws(input, size, stdin);

	// clean the trailing '\n'
	WCHAR *pos;
	if ((pos = wcschr(input, '\n')) != NULL)
		*pos = '\0';
}

// Economou function
void print_memory(unsigned long address, char *buffer, unsigned int bytes_to_print)
{
	unsigned int cont;
	unsigned int i;
	const unsigned short bytes = 16;

	/* Print the lines */
	for (cont = 0; cont < bytes_to_print; cont = cont + bytes)
	{
		printf("%p | ", (void *)address);
		address = address + bytes;

		for (i = 0; i < bytes; i++)
		{
			if (i < (bytes_to_print - cont))
			{
				printf("%.2x ", (unsigned char)buffer[i + cont]);
			}
			else
			{
				printf("   ");
			}
		}

		//Space between two columns
		printf("| ");

		//Print the characters
		for (i = 0; i < bytes; i++)
		{
			if (i < (bytes_to_print - cont))
			{
				printf("%c", (isgraph(buffer[i + cont])) ? buffer[i + cont] : '.');
			}
			else
			{
				printf(" ");
			}
		}
		printf("\n");
	}
}

void error(WCHAR *msg) {
	fprintf(stderr, "Error: %S\n", msg);
	exit(-1);
}

