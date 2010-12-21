/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-test.c : test program from docs
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
#include <smth.h>

int main(int argc, char** argv)
{
	SMTHh h;
	char buffer[8192];
	int i, fragnum = 0;
	size_t streamsno;  /* total number of streams */
	char *name;

	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s url\n", argv[0]);
		return -1;
	}

	h = SMTH_open(argv[1], 0);

	if (!h) return -1;

	SMTH_getinfo(SMTH_STREAMS_NO, h, &streamsno);

	for (i = 0; i < streamsno; ++i)
	{
		SMTH_getinfo(SMTH_NAME, h, i, &name);
		fprintf(stderr, "Downloading chunks from stream %s\n", name);
		free(name); // this is very important! it's up to you to free strings

		for (fragnum = 0; !SMTH_EOS(h, i); ++fragnum)
		{
			size_t written = -1;

			char filename[100];
			snprintf(filename, 100, "fragment-%d-%d.vc1", i, fragnum);
			FILE *output = fopen(filename, "w");

			while (written)
			{
				written = SMTH_read(buffer, sizeof (buffer), i, h);
				fwrite(buffer, written, 1, output);
			}

			fclose(output);
		}
	}

	SMTH_close(h);

	return 0;
}

