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
#include <stdlib.h>
#include <smth-dump.h>
#include <smth-http.h>

int usage(const char *name)
{
	fprintf(stderr,
		"Usage: %s url [-m manifestname | -d [authentication]]\n"
		"Options:\n"
		"\t-m manifestname\t\tThe filename of the manifest of the stream to "
			"be downloaded.\n"
		"\t-d [authentication]\tDownload manifest (with optional urlencoded "
			"GET tokens, `authentication')\n"
		"\t-v\t\t\tPrints version number and exits.\n", name);
	return 0;
}

int main(int argc, char **argv)
{
	FILE *f;

	if ((argc == 2) && (!strcmp(argv[1], "-v")))
	{	printf("SMTH downloader v0.1\n");
		return 0;
	}

	if (argc < 2) return usage(argv[0]);

	char *urlname = argv[1];

	if (argc == 4)
	{
		char *option  = argv[2];

		if (!strcmp(option, "-m"))
		{
			char *filename = argv[3];

			if (access(filename, R_OK))
			{
				fprintf(stderr, "File specified does not exist or it is not readable.\n\n");
				return usage(argv[0]);
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
	}
	else
	{	f = SMTH_fetchmanifest(urlname, NULL);
	}

	if (!f)
	{
		fprintf(stderr, "Could not retrieve the stream manifest from the Internet.\n");
		return -1;
	}

	Manifest m;
	error_t r = SMTH_parsemanifest(&m, f);

	if (r != MANIFEST_SUCCESS)
	{	fprintf(stderr, "Error n.%d\n", r);
		return 1;
	}

	int i;

	for (i = 0; m.streams[i]; ++i)
	{
		char *dir = SMTH_fetch(urlname, m.streams[i], 0);
		printf("[*] Stream %d saved to dir `%s'\n", i, dir);
		free(dir);
	}
		

	SMTH_disposemanifest(&m);

	fclose(f);

	return 0;
}
