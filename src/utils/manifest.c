/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * manifest.c: relying on the internal API, verbosely analizes the Manifest
 *             of a given SmoothStream.
 *
 * 10th-12th July 2010
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
#include <smth-dump.h>

int main()
{
	FILE *f = fopen("/home/Sanfi/Progetti/libsmth/.junk/rai.Manifest.xml", "r");

	Manifest m;

	error_t r = SMTH_parsemanifest(&m, f);

	if (r != MANIFEST_SUCCESS)
	{	fprintf(stderr, "Error n.%d\n", r);
		return 1;
	}

	SMTH_dumpmanifest(&m, stdout);
	SMTH_disposemanifest(&m);

	fclose(f);
}
