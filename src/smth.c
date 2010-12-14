/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth.c : public API
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

/**
 * \internal
 * \file   smth.c
 * \brief  Main loop and public API
 * \author Stefano Sanfilippo
 * \date   14th December 2010
 */

#include <smth-http.h>
#include <smth-manifest-parser.h>
#include <smth-common-defs.h>
#include <smth-dynlist.h>

#include <stdlib.h>

/*
 * 1. Packet (`Box`) structure
 * ===========================
 *
 * Each `Fragment` contains a chunk of audio, video or text data, with a
 * metadata header. Each functional block of the fragment was named `Box`.
 * Each `Box` has a fixed structure:
 *
 *    ~-----------+------+-----------------+------------+-------------~
 *    ~ BoxLenght | Name | [BoxLongLenght] | BoxFields  | BoxChildren ~
 *    ~    4B     |  4B  |  8B (optional)  | {variable} |  {variable} ~
 *    ~-----------+------+-----------------+------------+-------------~
 *
 * where:
 *  + `Length`     is the length of the box in bytes, encoded in network format.
 *                 If the value of the field is 1, the BoxLongLength
 *                 field must be present. Otherwise, we assert that it is not
 *                 present. Size includes all the fields, even BoxLenght itself.
 *  + `Name`       is a 4B non null-terminated string identifying the type of
 *                 the block. There are seven different types, as it will be
 *                 explained in the appropriate section.
 *  + `LongLenght` is the length of the Box in bytes, encoded in network format. 
 *                 This field is present only if the size of the `Box` is larger
 *                 than BoxLenght max size (32B).
 *  + `Fields` &   are respectively the attributes and the content of the `Box`
 *    `Children`   and may vary accordingly to the role of the respective `Box`.
 *
 * The syntax of each `Box` is a strict subset of the syntax of the respective
 * Fragment Box defined in [ISOFF].
 *
 * 2. Fragment Structure
 * =====================
 *
 * As stated before, a Fragment Response is composed of various `Boxes`,
 *
 *  ~------------+------------~
 *  ~   MoofBox  |  MdatBox   ~
 *  ~ {variable} | {variable} ~
 *  ~------------+------------~
 *
 * Each `Box` must contain certain sub-Boxes, and it may contain others.
 * Any block may contain one or more `VendorExtensionUUIDBox`, as described later.
 *
 * 3. Manifest Response
 * ====================
 *
 * According to the specifications, the Manifest must be a Well-Formed XML
 * Document subject to the following constraints:
 *
 *   +The Document's XML Declaration's major version is 1.
 *   +The Document’s XML Declaration's minor version is 0.
 *   +The Document does not use a Document Type Definition (DTD).
 *   +The Document uses an encoding that is supported by the Client.
 *   +The XML Elements specified in this document do not use XML Namespaces.
 */

/**
 * \brief Opens an url for a Smooth Stream and
 *
 * \params url    The url from which to retrieve the Smooth Stream
 * \params params Optional GET params to make the request (e.g. authentication
 *                codes, pages... etc.
 * \return
 */
int SMTH_open(const char *url, const char *params)
{
	FILE *mfile = SMTH_fetchmanifest(url, params);
	Manifest manifest;
	DynList cachedirslist;
	char **cachedirs;
	count_t i;

	SMTH_parsemanifest(&manifest, mfile);
	fclose(mfile);

	SMTH_preparelist(&cachedirslist);

	for (i = 0; manifest.streams[i]; ++i) /* if possible find something more efficient */
		SMTH_addtolist(SMTH_fetch(url, manifest.streams[i], 0), &cachedirslist);

	SMTH_finalizelist(&cachedirslist);
	cachedirs = cachedirslist.list;

	for (i = 0; i < cachedirslist.index; ++i)
	{
		puts(cachedirs[i]); //TODO pipe
		free(cachedirs[i]);
	}

	SMTH_disposelist(&cachedirslist);
	SMTH_disposemanifest(&manifest);

	return 0;
}

/* vim: set ts=4 sw=4 tw=0: */
