/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * downloader.c: relying on the internal API, downloads all chunks in a Manifest
 *
 * 10th-12th July 2010 ~ 12th December 2010
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
#include <smth-dump.h>
#include <smth-http.h>

int usage(const char *name)
{
	fprintf(stderr, "SMTH downloader v0.1\nusage: %s url [-m manifestname "
		"| -d [authentication]]\n", name);
	return 0;
}

int main(int argc, char **argv)
{
	FILE *f;

	if (argc < 3) return usage(argv[0]);

	char *urlname = argv[1];
	char *option  = argv[2];

	if (!strcmp(option, "-m"))
	{
		char *filename = argv[3];

		if (access(filename, R_OK))
		{
			fprintf(stderr, "File specified does not exist or it is not readable.\n");
			return 0;
		}

		if (argc < 4) return usage(argv[0]);

		f = fopen(filename, "r");
	}
	else if (!strcmp(option, "-d"))
	{
		char *params;

		switch (argc)
		{
			case 3:
				params = NULL; break;
			case 4:
				params = argv[3];break;
			default:
				usage(argv[0]);
		}

		f = SMTH_fetchmanifest(urlname, params);
	}
	else return usage(argv[0]);

	Manifest m;
	error_t r = SMTH_parsemanifest(&m, f);

	if (r != MANIFEST_SUCCESS)
	{	fprintf(stderr, "Error n.%d\n", r);
		return 1;
	}

	int i;

	for (i = 0; m.streams[i]; ++i)
		SMTH_fetch(urlname, m.streams[i], 0); //FIXME track (deve essere adattivo)

	SMTH_disposemanifest(&m);

	fclose(f);
}
