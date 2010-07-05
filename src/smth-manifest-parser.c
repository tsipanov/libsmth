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
 * \file   smth-manifest-parser.c
 * \brief  XML manifest parser
 * \author Stefano Sanfilippo
 * \date   30th June - 1st July/5th July 2010
 */

#include <stdbool.h>
#include <expat.h>
#include <string.h>
#include <stdio.h>
#include <smth-manifest-defs.h>

/**
 * \brief Parses a manifest from file stream and fills a Manifest struct.
 *
 * If a parse error was detected, only the last error code is reported.
 *
 * \param m      Pointer to the manifest struct to be filled
 * \param stream The stream containing the manifest to be parsed.
 * \return       MANIFEST_SUCCESS or an appropriate error code.
 */
error_t parsemanifest(Manifest *m, FILE *stream)
{
	chardata chunk[MANIFEST_XML_BUFFER_SIZE];
	ManifestBox root = {m, false, MANIFEST_SUCCESS};
	error_t result;
	bool done = false;

	memset(m, 0x00, sizeof (Manifest)); /* reset memory */

	if (feof(stream)) return MANIFEST_EMPTY;

	XML_Parser parser = XML_ParserCreate(NULL);
	if (!parser) return MANIFEST_NO_MEMORY;
	XML_SetElementHandler(parser, startblock, endblock);
	XML_SetCharacterDataHandler(parser, textblock);
	XML_SetUserData(parser, &root);

	while (!done)
	{
		lenght_t len;
		len = (lenght_t) fread(chunk, sizeof(byte_t), sizeof(chunk), stream);
		done = feof(stream);

		if (ferror(stdin))
		{   result = MANIFEST_IO_ERROR;
			done = true;
		}
		if (XML_Parse(parser, chunk, len, done) == XML_STATUS_ERROR)
		{	result = MANIFEST_PARSE_ERROR;
			break; /* here we could put a done = true statement, but it would *
			        * be ignored by XML_Parse, as `while` would trigger off.  */
		}
		if (root.state != MANIFEST_SUCCESS)
		{   result = root.state;
			break; /* same as above. */
		}
	}

	XML_ParserFree(parser);
	return result;
}

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
 * \param m    The manifest to be filled with parsed data.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or an appropriate error code.
 */
static error_t parsemedia(ManifestBox *mb, const char **attr)
{
	count_t i;
	for (i = 0; attr[i]; i += 2)
	{
		if (!attr[i+1]) return MANIFEST_PARSER_ERROR;

		/* The specifications require that Major is set to 2 and Minor to 0 */
		if (!strcmp(attr[i], MANIFEST_MEDIA_MAJOR_VERSION))
		{   if (strcmp(attr[i+1], MANIFEST_MEDIA_DEFAULT_MAJOR))
			{	return MANIFEST_WRONG_VERSION;
			}
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_MINOR_VERSION))
		{   if (strcmp(attr[i+1], MANIFEST_MEDIA_DEFAULT_MINOR))
			{	return MANIFEST_WRONG_VERSION;
			}
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_TIME_SCALE))
		{   mb->m->tick = (tick_t) atol(attr[i + 1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_DURATION))
		{   mb->m->duration = (tick_t) atol(attr[i + 1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_IS_LIVE))
		{   /* we can safely assume that if it is not true, it is false */
			mb->m->islive = (tolower(attr[i + 1]) == 't');
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_LOOKAHEAD))
		{   mb->m->lookahead = (count_t) atoi(attr[i + 1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_DVR_WINDOW))
		{   mb->m->dvrwindow = (lenght_t) atol(attr[i + 1]);
			continue;
		}
		/* TODO else */
	}
	/* if the field is null, set it to default, as required by specs */
	if (!mb->m->tick) mb->m->tick = MANIFEST_MEDIA_DEFAULT_TICKS;

	return MANIFEST_SUCCESS;
}

/**
 * \brief Parses a ProtectionHeader element.
 *        
 * This XML element holds metadata required by to play back protected content.
 * Funnily enough, its container `Protection` is pointless.
 *
 * \param m    The manifest to be filled with parsed attributes.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or MANIFEST_INAPPROPRIATE_ATTRIBUTE if an
 *			   attribute different from MANIFEST_PROTECTION_ID was encountered.
 */
static error_t parsearmor(ManifestBox *mb, const char **attr)
{
	if (strcmp(attr[0], MANIFEST_PROTECTION_ID))
		return MANIFEST_INAPPROPRIATE_ATTRIBUTE;
	if (strlen(attr[1]) != MANIFEST_ARMOR_UUID_LENGHT)
		return MANIFEST_MALFORMED_ARMOR_UUID;
//////////////////////////////////TODO//////////////////////////////////////////
//	mb->m->armorID =; // attr[1];  9A04F079-9840-4286-AB92E65BE0885F95
////////////////////////////////////////////////////////////////////////////////
	mb->armorwaiting = true; /* waiting for the body to be parsed. */

	return MANIFEST_SUCCESS;
}

/**
 * \brief A StreamElement contains all metadata needed to play a specific stream.
 *
 * Attributes may appear in any order, however Type must always be present.
 * If the track is not an embedded content, also NumberOfFragments, and Url must
 * appear. If the track carries text, also the Subtype must be present.
 * Unless the type specified is video, StreamMaxWidth, StreamMaxHeight,
 * DisplayWidth, and DisplayHeight must not appear.
 *
 * \param m    The Manifest struct to be filled with parsed data.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or MANIFEST_INAPPROPRIATE_ATTRIBUTE if an
 *			   attribute different from MANIFEST_PROTECTION_ID was encountered.
 */
static error_t parseelement(ManifestBox *mb, const char **attr)
{
	count_t i;
	for (i = 0; attr[i]; i += 2)
	{

	tick_t duration;
	tick_t tick;
	bool islive;
	count_t lookahead;
	lenght_t dvrwindow;
	uuid_t armorID;

		if (!strcmp(attr[i], MANIFEST_STREAM_TYPE)) //"video" / "audio" / "text"
		{   
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_SUBTYPE)) //4*4 ALPHA
		{
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_TIME_SCALE)) //STRING_UINT64
		{
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_NAME)) //ALPHA *( ALPHA / DIGIT / UNDERSCORE / DASH )
		{
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_CHUNKS)) //STRING_UINT32
		{
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_QUALITY_LEVELS)) //STRING_UINT32
		{
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_URL)) //UrlPattern
		{
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_MAX_WIDTH)) //STRING_UINT32
		{
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_MAX_HEIGHT)) //STRING_UINT32
		{
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_DISPLAY_WIDTH)) // STRING_UINT32
		{
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_DISPLAY_HEIGHT)) //STRING_UINT32
		{
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_PARENT)) //ALPHA *( ALPHA / DIGIT / UNDERSCORE / DASH )
		{
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_OUTPUT)) //CASEINSENTITIVE_TRUE / CASEINSENSITIVE_FALSE
		{
			continue;
		}
		//FIXME	VendorExtensionAttribute
	}
}

#if 0

Type (variable): The type of the stream: video, audio, or text.  
StreamTimeScale (variable): The time scale for duration and time values in this stream, specified
as the number of increments in one second.
Name (variable): The name of the stream.
NumberOfFragments (variable): The number of fragments available for this stream.
NumberOfTracks (variable): The number of tracks available for this stream.
Subtype (variable): A four-character code that identifies the intended use category for each
sample in a text track. However, the FourCC field, specified in section 2.2.2.5, is used to identify
the media format for each sample. The following range of values is reserved, with the following
semantic meanings:
"SCMD": Triggers for actions by the higher-layer implementation on the client
"CHAP": Chapter markers
"SUBT": Subtitles used for foreign-language audio
"CAPT": Closed captions for the hearing-impaired
"DESC": Media descriptions for the hearing-impaired
"CTRL": Events the control the application business logic
"DATA": Application data that does not fall into any of the above categories
Url (variable): A pattern used by the client to generate Fragment Request messages.
SubtypeControlEvents (variable): Control events for applications on the client.
StreamMaxWidth (variable): The maximum width of a video sample, in pixels.
StreamMaxHeight (variable): The maximum height of a video sample, in pixels.
DisplayWidth (variable): The suggested display width of a video sample, in pixels.
DisplayHeight (variable): The suggested display height of a video sample, in pixels.
ParentStream (variable): Specifies the non-sparse stream that is used to transmit timing
information for this stream. If the ParentStream field is present, it indicates that the stream
described by the containing StreamElement field is a sparse stream. If present, the value of this
field MUST match the value of the Name field for a non-sparse stream in the presentation.
ManifestOutput (variable): Specifies whether sample data for this stream appears directly in the
Manifest as part of the ManifestOutputSample field, specified in section 2.2.2.6.1, if this field
contains a CASEINSENTITIVE_TRUE value. Otherwise, the ManifestOutputSample field for
fragments that are part of this stream MUST be omitted.

#endif

////////////////////////////////XML FIXME///////////////////////////////////////

/** \brief expat tag start event callback. */
static void XMLCALL startblock(void *data, const char *el, const char **attr)
{
	ManifestBox *manbox = data;

	if (!strcmp(el, MANIFEST_ELEMENT))
	{   manbox->state = parsemedia(manbox, attr);
		return;
	}
// FIXME
//		case LEVEL: manbox->state = parsemedia(manbox, attr); break;
//		case TRACK: manbox->state = parsemedia(manbox, attr); break;
//		case CHUNK: manbox->state = parsemedia(manbox, attr); break;
//		case PROTECT: manbox->state = parsemedia(manbox, attr); break;

//	manbox->state = MANIFEST_UNKNOWN_BLOCK; /* it should never arrive here */
}

/** \brief expat tag end event callback. */
static void XMLCALL endblock(void *data, const char *el)
{   ManifestBox *manbox = data;
}

//TODO implementare UnknownEncoding

//FIXME e se unserissi anche la lunghezza??
/** \brief expat text event callback. */
static void XMLCALL textblock(void *data, const char *text, int lenght)
{
	ManifestBox *manbox = data;
	if (manbox->armorwaiting)
	{
		if (lenght > 0)
		{	
			base64data *tmp = malloc(lenght+1);
			if (!tmp)
			{   free(tmp);
				manbox->state = MANIFEST_NO_MEMORY;
			}
			memcpy(tmp, text, lenght);
			tmp[lenght] = (base64data) 0;
			manbox->m->armor = tmp;
		}
		else manbox->m->armor = NULL;
	}
	manbox->armorwaiting = false;
}

/* vim: set ts=4 sw=4 tw=0: */
