/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * dissector.c: relying on the internal API, verbosely analizes the structure
 *              of a given Fragment and dumps it to a specified file.
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
#include <smth-fragment-parser.h>

void examine(Fragment *vc);

int main(int argc, char **argv)
{	FILE *input  = fopen(argv[1], "rb");
	FILE *output = fopen(argv[2], "wb");

	Fragment vc;

	error_t exitcode = parsefragment(input, &vc);
	if (exitcode != FRAGMENT_SUCCESS)
	{	printf("Error no.%d!\n", exitcode);
		return 1;
	}

	examine(&vc);

	printf("Dumping data to file...\n", vc.size);
	fwrite(vc.data, sizeof (byte_t), vc.size, output);

	disposefragment(&vc);
	fclose(output);
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
	printf(" | +-id: 0x%032lx\n", vc->armor.id);
	printf(" | +-size of init vectors: %x\n", vc->armor.vectorsize);
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
		char bar = (i == vc->sampleno - 1)? ' ': '|';
		char corner = (i == vc->sampleno - 1)? '`': '+';
		printf("   %c-sample #%d\n", corner, i + 1);
		printf("   %c +-duration: %d ticks\n", bar, vc->samples[i].duration);
		printf("   %c +-size: %d bytes\n", bar, vc->samples[i].size);
		printf("   %c +-settings: 0x%08lx\n", bar, vc->samples[i].settings);
		printf("   %c `-offset: 0x%x\n", bar, vc->samples[i].timeoffset);
	}
	printf("\n============================================================\n\n");
}
