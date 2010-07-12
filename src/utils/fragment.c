/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * dissector.c: relying on the internal API, verbosely analizes the structure
 *              of a given Fragment and dumps its content to a file.
 *
 * 30th June 2010
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <unistd.h>
#include <smth.h>

void dumpt(Fragment* vc, char* ifile);

int main(int argc, char **argv)
{
	char* ifile = argv[1];

	if (!ifile)
	{	fprintf(stderr, "SMTH dissector v0.1\nusage: ismc filename\n");
		return 0;
	}
	if (access(ifile, R_OK))
	{	fprintf(stderr, "File specified does not exist or it is not readable.\n");
		return 0;
	}

	FILE *input  = fopen(ifile, "rb");

	Fragment vc;

	error_t exitcode = parsefragment(&vc, input);
	if (exitcode != FRAGMENT_SUCCESS)
	{	printf("Error no.%d!\n", exitcode);
		return 1;
	}

	SMTH_dumpfragment(&vc, stdout);

	printf("Dumping data to file...\n", vc.size);
	SMTH_dumppayload(&vc, ifile); //dumpt

	disposefragment(&vc);

	fclose(input);
	return 0;
}

#include <string.h>

void dumpt(Fragment* vc, char* ifile)
{
	char ofile[strlen(ifile)+4];
	sprintf(ofile, "%s.wmv", ifile);
	FILE *output = fopen(ofile, "wb");
	fwrite(vc->data, sizeof (byte_t), vc->size, output);
	fclose(output);
}
