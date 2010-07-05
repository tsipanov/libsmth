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
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <smth-fragment-parser.h>

void examine(Fragment *vc);
void dumps(Fragment* vc, char* ifile);
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

	examine(&vc);

	printf("Dumping data to file...\n", vc.size);
	dumps(&vc, ifile); //dumpt

	disposefragment(&vc);

	fclose(input);
	return 0;
}

void examine(Fragment *vc)
{
	int i;

	printf("\n==========================DATA SUMMARY======================\n\n");
	printf("Fragment no.%d [%d bytes of data]\n", vc->ordinal, vc->size);
	printf(" +-first settings: 0x%08lx\n", vc->settings);
	printf(" +-armor\n");
	printf(" | +-type: %d (0 = NONE)\n", vc->armor.type);
	printf(" | +-id: ");
	fwrite(vc->armor.id, sizeof(byte_t), sizeof(uuid_t), stdout);
	printf("\n | +-size of init vectors: %x\n", vc->armor.vectorsize);
	printf(" | `-number of init vectors: %d\n", vc->armor.vectorno);
	printf(" +-fragment defaults\n");
	printf(" | +-offset: 0x%x\n", vc->defaults.dataoffset);
	printf(" | +-index: %d\n", vc->defaults.index);
	printf(" | +-duration: %d ticks\n", vc->defaults.duration);
	printf(" | +-size: %d\n", vc->defaults.size);
	printf(" | `-settings: 0x%08lx\n", vc->defaults.settings);
	printf(" `-samples (%d in total)\n", vc->sampleno);
	for ( i = 0; i < vc->sampleno; i++)
	{
		char corner = (i == vc->sampleno - 1)? '`': '+';
		char bar = (i == vc->sampleno - 1)? ' ': '|';
		printf("   %c-sample #%d\n", corner, i + 1);
		printf("   %c +-duration: %d ticks\n", bar, vc->samples[i].duration);
		printf("   %c +-size: %d bytes\n", bar, vc->samples[i].size);
		printf("   %c +-settings: 0x%08lx\n", bar, vc->samples[i].settings);
		printf("   %c `-offset: 0x%x\n", bar, vc->samples[i].timeoffset);
	}
	printf("\n============================================================\n\n");
}

void dumps(Fragment* vc, char* ifile)
{
	char ofile[strlen(ifile)+16];
	int i, offset = 0;
	//SECURE
	for( i = 0; i < vc->sampleno; i++)
	{
		sprintf(ofile, "%s.d", ifile);
		mkdir(ofile, 0755);
		sprintf(ofile, "%s.d/%04d.vc1", ifile, i);
		int size = vc->samples[i].size;
		FILE *output = fopen(ofile, "wb");
		fwrite(&(vc->data[offset]), sizeof (byte_t), size, output);
		offset += size;
		fclose(output);
	}
}

void dumpt(Fragment* vc, char* ifile)
{
	char ofile[strlen(ifile)+4];
	sprintf(ofile, "%s.wmv", ifile);
	FILE *output = fopen(ofile, "wb");
	fwrite(vc->data, sizeof (byte_t), vc->size, output);
	fclose(output);
}


