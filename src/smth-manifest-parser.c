/*
 * Copyright (C) 2010 Stefano Sanfilippo
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
 * \file   smth-fragment-parser.c
 * \brief  XML manifest parser
 * \author Stefano Sanfilippo
 * \date   30th June 2010
 */

#include <expat.h>
#include <stdbool.h>
#include <smth-manifest-defs.h>

#if 0
static void XMLCALL startblock(void *data, const char *el, const char **attr)
{
	int i;
	for (i = 0; i < Depth; i++) printf("  ");
	printf("%s", el);
	for (i = 0; attr[i]; i += 2) printf(" %s ='%s'", attr[i], attr[i + 1]);
	printf("\n");
	Depth++;
}

static void XMLCALL endblock(void *data, const char *el)
{
	Depth--;
}

static void XMLCALL textblock(void *data, const char *text, int lenght)
{
	printf("\n%4d: Text - ", Eventcnt++);
	fwrite(txt, txtlen, sizeof(char), stdout);
}
#endif

/**
 * \brief Parses a SmoothStreamingMedia.
 *
 * A SmoothStreamingMedia is an XML Element containing all metadata
 * required by the client to play back the content. The parser scans for
 * known attributes and sets corresponding fields in Manifest. Attributes may
 * appear in any order, but MajorVersion, MinorVersion and Duration must be
 * present. We take advantage of atol implementation, which sets to 0 all
 * invalid fields (i.e. containing non-numeric characters).
 *
 * \param m    The manifest to be filled with data parsed.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or MANIFEST_WRONG_VERSION
 */
static error_t parsemedia(Manifest *m, const char **attr)
{
	count_t i;
	/* we rely on expat for attr to have an even element number: if a[2*i] is
	 * not NULL, we safely assume that neither a[2*i+1] is. */
	for (i = 0; attr[i]; i += 2)
	{
		/* The specifications require that Major is set to 2 and Minor to 0 */
		if (!strcmp(attr[i], MANIFEST_MEDIA_MAJOR_VERSION))
		{   if (strcmp(attr[i+1]), MANIFEST_MEDIA_DEFAULT_MAJOR)
			{	return MANIFEST_WRONG_VERSION;
			}
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_MINOR_VERSION))
		{   if (strcmp(attr[i+1]), MANIFEST_MEDIA_DEFAULT_MINOR)
			{	return MANIFEST_WRONG_VERSION;
			}
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_TIME_SCALE))
		{   m->tick = (tick_t) atol(attr[i + 1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_DURATION))
		{   m->duration = (tick_t) atol(attr[i + 1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_IS_LIVE))
		{   /* we can safely assume that if it is not true, it is false */
			//m->islive = (tolower(attr[i + 1]) == "true")? true: false; FIXME
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_LOOKAHEAD))
		{   m->lookahead = (count_t) atoi(attr[i + 1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_DVR_WINDOW))
		{   m->dvrwindow = (lenght_t) atol(attr[i + 1]);
			continue;
		}
		//TODO aggiungere frame custom
	}
	/* if the field is null, set it to default, as required by specs */
	if (!m->tick) m->tick = MANIFEST_MEDIA_DEFAULT_TICKS;

	return MANIFEST_SUCCESS;
}

/**
 * \brief Parses a ProtectionElement
 *
 * This XML element holds metadata required by to play back protected content.
 *
 * \param m    The manifest to be filled with data parsed.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or MANIFEST_INAPPROPRIATE_ATTRIBUTE if an
 *			   attribute different from MANIFEST_PROTECTION_ID was encountered.
 */
static error_t parsearmor(Manifest *m, const char **attr)
{
	if(strcmp(attr[0], MANIFEST_PROTECTION_ID))
		return MANIFEST_INAPPROPRIATE_ATTRIBUTE;
	//m->armorID = ; // attr[1]; FIXME (4-2-2-8)
	//ProtectionHeaderContent = STRING_BASE64 TODO char handler
}

static error_t parseelement(Manifest *m, const char **attr)
{
	
}

/* vim: set ts=4 sw=4 tw=0: */
