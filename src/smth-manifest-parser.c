/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-manifest-parser.c: parses a SmoothStream XML manifest.
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
 * \date   30th June - 1st July/5th July/11th-12th July 2010
 */

#include <stdbool.h>
#include <expat.h>
#include <string.h>
#include <stdio.h>
#include <smth-manifest-defs.h>

/** \brief Converts a string into a 32bit integer. */
#define atoint32(x) atoi(x)
/** \brief Converts a string into a 64bit integer. */
#define atoint64(x) atoll(x)
/** \brief Converts a string into a boolean value.
 *
 *  \note We can safely assume that if a token is not \c (t)rue, it is \c false.
 */
#define atobool(x) (tolower(attr[i + 1][0]) == 't')

/**
 * \brief Checks whether a string is a valid identifier.
 *
 * \note This is a paranoid check to prevent buffer based pointer injections.
 *
 * \param s The string to check.
 * \return  true if the string is sane.
 */
static bool stringissane(const char* s)
{   count_t i;
	/* we can save a few cpu cycles if it is not alpha nor \0 */
	if (s[0] && !isalpha(s[0])) return false;
	for (i = 0; i < strlen(s); i++)
		if (!(isalpha(s[i]) || isdigit(s[i]) || (s[i] == '_') || (s[i] == '-')))
			return false;
	return true;
}

/**
 * \brief Parses a manifest from file stream and fills a Manifest struct.
 *
 * If a parse error was detected, only the last error code is reported.
 *
 * \param m      Pointer to the manifest struct to be filled
 * \param stream The stream containing the manifest to be parsed.
 * \return       MANIFEST_SUCCESS or an appropriate error code.
 */
error_t SMTH_parsemanifest(Manifest *m, FILE *stream)
{
	chardata chunk[MANIFEST_XML_BUFFER_SIZE];
	ManifestBox root;
	error_t result;

	memset(&root, 0x00, sizeof(ManifestBox)); /* reset memory */
	root.m = m;
	root.state = MANIFEST_SUCCESS;

	bool done = false;

	memset(m, 0x00, sizeof (Manifest)); /* reset memory */

	if (feof(stream)) return MANIFEST_EMPTY;

	XML_Parser parser = XML_ParserCreate(NULL);
	if (!parser) return MANIFEST_NO_MEMORY;
	XML_SetElementHandler(parser, startblock, endblock);
	XML_SetCharacterDataHandler(parser, textblock);
	XML_SetUserData(parser, &root);

	root.parser = parser;

	while (!done)
	{
		length_t len;
		len = (length_t) fread(chunk, sizeof(byte_t), sizeof(chunk), stream);
		done = feof(stream);

		if (ferror(stdin))
		{   result = MANIFEST_IO_ERROR;
			done = true;
		}
		(void) XML_Parse(parser, chunk, len, done);
	}

	XML_ParserFree(parser);
	return root.state;
}

/**
 * \brief Disposes properly of a Manifest struct.
 *
 * Programmers are advised to use this function to free a Manifest struct,
 * avoiding the direct use of \c free(). NEVER TRY TO DISPOSE OF A FRAGMENT if
 * \c parsemanifest() did not return \c MANIFEST_SUCCESS.
 *
 * \param m The manifest to be destroyed.
 */
void SMTH_disposemanifest(Manifest* m)
{   
	if (m->armor)
	{   disposeembedded(m->armor);
		free(m->armor);
	}
	if (m->streams)
	{   count_t i;
		for (i = 0; m->streams[i]; i++)
		{
			Stream *tmpstream = m->streams[i];

			if (tmpstream->name) free(tmpstream->name);
			if (tmpstream->parent) free(tmpstream->parent);
			/* if the pointer and its content are not NULL */
			if (tmpstream->tracks)
			{   count_t j;
				for (j = 0; tmpstream->tracks[j]; j++)
				{   Track *tmptrack = tmpstream->tracks[j];
					if (tmptrack->header) free(tmptrack->header);
					if (tmptrack->attributes)
					{	count_t n;
						for (n = 0; tmptrack->attributes[n]; n++)
						{   free(tmptrack->attributes[n]);
						}
						free(tmptrack->attributes);
					}
					disposevendorattrs(tmptrack->vendorattrs);
					free(tmptrack);
				}
				free(tmpstream->tracks);
			}
			if (tmpstream->chunks)
			{	count_t j;
				for (j = 0; tmpstream->chunks[j]; j++)
				{   Chunk *tmpchunk = tmpstream->chunks[j];
					count_t n;
					if (tmpchunk->fragments)
					{	for (n = 0; tmpchunk->fragments[n]; n++)
						{   ChunkIndex *tmpfragment = tmpchunk->fragments[n];
							if (tmpfragment->embedded)
								disposeembedded(tmpfragment->embedded);
							disposevendorattrs(tmpfragment->vendorattrs);
							free(tmpfragment);
						}
						free(tmpchunk->fragments);
					}
					free(tmpchunk);
				}
				free(tmpstream->chunks);
			}
			disposevendorattrs(tmpstream->vendorattrs);
			disposeurlpattern(tmpstream->url);
			free(tmpstream);
		}
		free(m->streams);
	}
	disposevendorattrs(m->vendorattrs);

	/* destroy even the reference. */
	m->armor = NULL;
	m->streams = (Stream**) NULL;
	m->vendorattrs = (chardata**) NULL;
}

/*--------------------- HIC QUOQUE SUNT LEONES (CODICIS) ---------------------*/

/** \brief             Destroys a vendor data attrs sequence.
 *  \param vendorattrs The structure to be destroyed.
 */
static void disposevendorattrs(chardata **vendorattrs)
{   if (vendorattrs)
	{   int i;
		for (i = 0; vendorattrs[i]; i++) free(vendorattrs[i]);
	}
	free(vendorattrs);
}

/** \brief Add a tuple key/value to the specifiedd list.
 *
 *  \note The tuple is passed as a two element chardata* array
 *  \param vendordata The list to which add the parameters
 *  \param attr       The tuple to add
 */
static bool addvendorattrs(DynList *vendordata, const char **attr)
{	chardata *key = malloc(strlen(attr[0]) + sizeof(chardata));
	chardata *value = malloc(strlen(attr[1]) + sizeof(chardata));
	strcpy(key, attr[0]);
	strcpy(value, attr[1]);
	if (!SMTH_addtolist(key, vendordata) || !SMTH_addtolist(value, vendordata))
	{   SMTH_disposelist(vendordata);
		free(key);
		free(value);
		return false;
	}
	return true;
}

/** \brief Appropriately destroy an embedded content struct
 *
 *  \param ed Pointer to the \c EmbeddedData struct to free.
 */
static void inline disposeembedded(EmbeddedData *ed)
{   free(ed->content);
}

/** \brief expat tag start event callback.
 *
 * Every time a block is opened, the current dynamic list for the appropriate
 * element is reset. Each called parser should assign the dynamic field to the
 * correct \c DynList::list
 */
static void XMLCALL startblock(void *data, const char *el, const char **attr)
{
	ManifestBox *mb = data;

	//fprintf(stderr, ">: %s\n", el); //DEBUG

	if (mb->manifestparsed)
	{   mb->state = MANIFEST_UNEXPECTED_TRAILING;
		return;
	}
	if (!strcmp(el, MANIFEST_ELEMENT))
	{   SMTH_preparelist(&mb->tmpstreams);
		mb->state = parsemedia(mb, attr);
		return;
	}
	if (!strcmp(el, MANIFEST_ARMOR_ELEMENT))
	{   mb->state = parsearmor(mb, attr);
		return;
	}
	if (!strcmp(el, MANIFEST_STREAM_ELEMENT))
	{   SMTH_preparelist(&mb->tmpchunks);
		SMTH_preparelist(&mb->tmptracks);
		mb->previoustime = mb->previousduration = 0;
		mb->state = parsestream(mb, attr);
		return;
	}
	if (!strcmp(el, MANIFEST_TRACK_ELEMENT))
	{   SMTH_preparelist(&mb->tmpattributes);
		mb->state = parsetrack(mb, attr);
		return;
	}
	if (!strcmp(el, MANIFEST_ATTRS_ELEMENT))
	{   mb->state = parseattr(mb, attr);
		return;
	}
	if (!strcmp(el, MANIFEST_CHUNK_ELEMENT))
	{   SMTH_preparelist(&mb->tmpfragments);
		mb->state = parsechunk(mb, attr);
		return;
	}
	if (!strcmp(el, MANIFEST_FRAGMENT_ELEMENT))
	{   mb->state = parsefragindex(mb, attr);
		return;
	}

	mb->state = MANIFEST_UNKNOWN_BLOCK; /* it should never arrive here */
}

/** \brief expat tag end event callback. */
static void XMLCALL endblock(void *data, const char *el)
{  
	ManifestBox *mb = data;

	//fprintf(stderr, "<: %s\n", el); //DEBUG

	if (mb->state != MANIFEST_SUCCESS)
	{   SMTH_disposemanifest(mb->m);
		XML_StopParser(mb->parser, XML_FALSE);
	}

	if (!strcmp(el, MANIFEST_ELEMENT))
	{	if(!SMTH_finalizelist(&mb->tmpstreams)) mb->state = MANIFEST_NO_MEMORY;
		mb->m->streams = (Stream**) mb->tmpstreams.list;
		mb->manifestparsed = true;
		return;
	}
	if (!strcmp(el, MANIFEST_ARMOR_ELEMENT))
	{   mb->activearmor = NULL;
	}
	if (!strcmp(el, MANIFEST_STREAM_ELEMENT))
	{   if(!SMTH_finalizelist(&mb->tmptracks)) mb->state = MANIFEST_NO_MEMORY;
		if(!SMTH_finalizelist(&mb->tmpchunks)) mb->state = MANIFEST_NO_MEMORY;
		mb->activestream->tracks = (Track**) mb->tmptracks.list;
		mb->activestream->chunks = (Chunk**) mb->tmpchunks.list;
		mb->activestream = NULL;
		return;
	}
	if (!strcmp(el, MANIFEST_TRACK_ELEMENT))
	{   if(!SMTH_finalizelist(&mb->tmpattributes)) mb->state = MANIFEST_NO_MEMORY;
		mb->activetrack->attributes = (chardata**) mb->tmpattributes.list;
		mb->activetrack = NULL;
		return;
	}
	//if (!strcmp(el, MANIFEST_ATTRS_ELEMENT)) not used.
	if (!strcmp(el, MANIFEST_CHUNK_ELEMENT))
	{   if(!SMTH_finalizelist(&mb->tmpfragments)) mb->state = MANIFEST_NO_MEMORY;
		mb->activechunk->fragments = (ChunkIndex**) mb->tmpfragments.list;
		mb->activechunk = NULL;
		return;
	}
	if (!strcmp(el, MANIFEST_FRAGMENT_ELEMENT))
	{   if (mb->embedded)
		{   mb->activefragment->embedded = mb->embedded;
			mb->embedded = NULL;
			mb->activefragment = NULL;
		}
		return;
	}
}

/** \brief expat text event callback
 *
 *  Searches and stores embedded data, decoding base64
 */
static void XMLCALL textblock(void *data, const char *text, int length)
{
	ManifestBox *mb = data;
	size_t i;

	if (length < 1) return;

	for (i = 0; i < length && isspace(text[i]); i++); /* skip blanks */
	if (length == i) return; /* no text, only blanks */
	const char *sanetext = &text[i];
	int sanelength = length - i;

	//fwrite(sanetext, 1, sanelength, stderr); //DEBUG

	if (mb->activearmor)
	{   mb->state = parsepayload(mb->activearmor, sanetext, sanelength);
		return;
	}
	if (mb->activefragment)
	{   mb->state = parsepayload(mb->activefragment->embedded, sanetext, sanelength);
		return;
	}
	mb->state = MANIFEST_UNEXPECTED_TRAILING;
}

/**
 * \brief Parses a SmoothStreamingMedia.
 *
 * A SmoothStreamingMedia is an XML Element containing all metadata
 * required by the client to play back the content. The parser scans for
 * known attributes and sets corresponding fields in Manifest. Attributes may
 * appear in any order, but MajorVersion, MinorVersion and Duration must be
 * present. We take advantage of \c atoint32|64 implementation, which sets to 0
 * all invalid fields (i.e. containing non-numeric characters).
 *
 * \param m    The manifest to be filled with parsed data.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or an appropriate error code.
 */
static error_t parsemedia(ManifestBox *mb, const char **attr)
{
	count_t i;

	DynList vendordata;
	SMTH_preparelist(&vendordata);
		
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
		{   mb->m->tick = (tick_t) atoint64(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_DURATION))
		{   mb->m->duration = (tick_t) atoint64(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_IS_LIVE))
		{   mb->m->islive = atobool(a[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_LOOKAHEAD))
		{   mb->m->lookahead = (count_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_MEDIA_DVR_WINDOW))
		{   mb->m->dvrwindow = (length_t) atoint64(attr[i+1]);
			continue;
		}
		/* else */
		if(!addvendorattrs(&vendordata, &attr[i])) return MANIFEST_NO_MEMORY;
	}
	/* if the field is null, set it to default, as required by specs. */
	if (!mb->m->tick) mb->m->tick = MANIFEST_MEDIA_DEFAULT_TICKS;

	if (!SMTH_finalizelist(&vendordata)) return MANIFEST_NO_MEMORY;
	mb->m->vendorattrs = (chardata**) vendordata.list;

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
	if (!attr[0]) return MANIFEST_MALFORMED_ARMOR_UUID; /* no uuid */

	if (strcmp(attr[0], MANIFEST_PROTECTION_ID))
		return MANIFEST_INAPPROPRIATE_ATTRIBUTE;
	if (strlen(attr[1]) != MANIFEST_ARMOR_UUID_LENGTH)
		return MANIFEST_MALFORMED_ARMOR_UUID;

//  mb->m->armorID = attr[1]; TODO 9A04F079-9840-4286-AB92E65BE0885F95

	if (mb->m->armor) free(mb->m->armor); /* only one armor, please... */
	
	/* build new armor struct */
	EmbeddedData *tmparmor = calloc(1, sizeof (EmbeddedData));
	if (!tmparmor) return MANIFEST_NO_MEMORY;
	mb->m->armor = tmparmor;
	mb->activearmor = tmparmor; /* This is not really needed, but may help in *
								 * case MS decides to add more than 1 armor.  */
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
 * This parser is not paranoid safe, as it will deduce some parameters from
 * the initial letter of a tag, without analizing the complete string.
 *
 * \param mb   The Manifest struct wrapper to be filled with parsed data.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or MANIFEST_INAPPROPRIATE_ATTRIBUTE if an
 *			   attribute different from MANIFEST_PROTECTION_ID was encountered.
 */
static error_t parsestream(ManifestBox *mb, const char **attr)
{
	count_t i;

	DynList vendordata;
	SMTH_preparelist(&vendordata);

	Stream *tmp = calloc(1, sizeof(Stream));
	if (!tmp) return MANIFEST_NO_MEMORY;

	for (i = 0; attr[i]; i += 2)
	{
		if (!attr[i+1]) return MANIFEST_PARSER_ERROR;

		if (!strcmp(attr[i], MANIFEST_STREAM_TYPE))
		{
			switch (tolower(attr[i+1][0]))
			{   case 'v': tmp->type = VIDEO; break; /* "video" */
				case 'a': tmp->type = AUDIO; break; /* "audio" */
				case 't': tmp->type = TEXT;  break; /* "text"  */
				default:
				{	free(tmp);
					return MANIFEST_UNKNOWN_STREAM_TYPE;
				}
			}
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_TIME_SCALE))
		{   tmp->tick = (tick_t) atoint64(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_NAME))
		{   if (!stringissane(attr[i+1]))
			{   free(tmp);
				return MANIFEST_INVALID_IDENTIFIER;
			}
			tmp->name = malloc(strlen(attr[i+1]) + sizeof (chardata)); /* including a \0 sigil */
			if (!tmp->name)
			{   free(tmp);
				return MANIFEST_NO_MEMORY;  
			}
			strcpy(tmp->name, attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_CHUNKS_NO))
		{   tmp->chunksno = (count_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_QUALITY_LEVELS_NO))
		{   tmp->tracksno = (count_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_MAX_WIDTH))
		{   tmp->maxsize.width = (metric_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_MAX_HEIGHT))
		{   tmp->maxsize.height = (metric_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_DISPLAY_WIDTH))
		{   tmp->bestsize.width = (metric_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_DISPLAY_HEIGHT))
		{   tmp->bestsize.height = (metric_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_OUTPUT))
		{	tmp->isembedded = atobool(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_SUBTYPE))
		{   /* overflow safe */
			if (strlen(attr[i+1]) == MANIFEST_STREAM_SUBTYPE_SIZE)
			{   strcpy(tmp->subtype, attr[i+1]);
				continue;
			}
			if (strlen(attr[i+1]) != 0)
			{   free(tmp);
				return MANIFEST_MALFORMED_SUBTYPE;
			}
			/* else (NULL) keep it NULL */
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_PARENT))
		{   if (!stringissane(attr[i+1]))
			{   free(tmp);
				return MANIFEST_INVALID_IDENTIFIER;
			}
			tmp->parent = malloc(strlen(attr[i+1])+1); /* including a \0 sigil */
			if (!tmp->parent)
			{   free(tmp);
				return MANIFEST_NO_MEMORY;
			}
			strcpy(tmp->parent, attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_URL))
		{   if (!parseurlpattern(&tmp->url, attr[i+1]))
			{   free(tmp);
				return MANIFEST_MALFORMED_URL;
			}
			continue;
		}
		//TODO SubtypeControlEvents: Control events for applications on the client.
		/* else */
		if(!addvendorattrs(&vendordata, &attr[i])) return MANIFEST_NO_MEMORY;
	}

	if (!SMTH_finalizelist(&vendordata)) return MANIFEST_NO_MEMORY;
	tmp->vendorattrs = (chardata**) vendordata.list;

	mb->activestream = tmp;
	if (!SMTH_addtolist(tmp, &mb->tmpstreams)) return MANIFEST_NO_MEMORY;

	return MANIFEST_SUCCESS;
}

/**
 * \brief TrackElement parser.
 *
 * Per-track metadata.
 *
 * Attributes can appear in any order. However, some are required:
 *
 * by   ANY: Index | Bitrate
 * by VIDEO: MaxWidth | MaxHeight | CodecPrivateData
 * by AUDIO: MaxWidth | MaxHeight | CodecPrivateData | SamplingRate |
 *           Channels | BitsPerSample | PacketSize | AudioTag | FourCC
 *
 * \param mb   The Manifest struct wrapper to be filled with parsed data.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or MANIFEST_INAPPROPRIATE_ATTRIBUTE if an
 *			   attribute different from MANIFEST_PROTECTION_ID was encountered.
 */
static error_t parsetrack(ManifestBox *mb, const char **attr)
{
	count_t i;

	DynList vendordata;
	SMTH_preparelist(&vendordata);

	Track *tmp = calloc(1, sizeof(Track));
	if (!tmp) return MANIFEST_NO_MEMORY;

	for (i = 0; attr[i]; i += 2)
	{
		if (!attr[i+1]) return MANIFEST_PARSER_ERROR;

		if (!strcmp(attr[i], MANIFEST_TRACK_INDEX))
		{   tmp->index = (count_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_BITRATE))
		{   tmp->bitrate = (bitrate_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_MAXWIDTH))
		{   tmp->maxsize.width = (metric_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_MAXHEIGHT))
		{   tmp->maxsize.height = (metric_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_PACKETSIZE))
		{   tmp->packetsize = (bitrate_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_SAMPLERATE))
		{   tmp->samplerate = (bitrate_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_AUDIOTAG))
		{   tmp->audiotag = (flags_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_FOURCC))
		{   /* overflow safe */
			if (strlen(attr[i+1]) == MANIFEST_TRACK_FOURCC_SIZE)
			{   strcpy(tmp->fourcc, attr[i+1]);
				continue;
			}

			if (strlen(attr[i+1]) != 0) return MANIFEST_MALFORMED_FOURCC;
			/* else (not null, not 4 letters) keep it NULL */
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_HEADER))
		{	
			tmp->header = malloc(strlen(attr[i+1]) + sizeof (chardata));
			if(!tmp->header) return MANIFEST_NO_MEMORY;
			/* data is not unhexlified because vendor extensions could put
			 * here anything, even text. */
			strcpy(tmp->header, attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_CHANNELS))
		{   tmp->channelsno = (unit_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_BITSPERSAMPLE))
		{   tmp->bitspersample = (unit_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_NAL_LENGTH))
		{   tmp->nalunitlength = (unit_t) atoint32(attr[i+1]);
			if (!tmp->nalunitlength) tmp->nalunitlength = NAL_DEFAULT_LENGTH;
			continue;
		}
		/* else */
		if(!addvendorattrs(&vendordata, &attr[i])) return MANIFEST_NO_MEMORY;
	}

	if (!SMTH_finalizelist(&vendordata)) return MANIFEST_NO_MEMORY;
	tmp->vendorattrs = (chardata**) vendordata.list;

	mb->activetrack = tmp;
	if (!SMTH_addtolist(tmp, &mb->tmptracks)) return MANIFEST_NO_MEMORY;

	return MANIFEST_SUCCESS;
}

/**
 * \brief Attribute (metadata that disambiguates tracks in a stream) parser.
 *
 * \warning Tags different from Name and Value will be graciously ignored.
 *
 * \param m    The Manifest struct wrapper to be filled with parsed data.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or MANIFEST_INAPPROPRIATE_ATTRIBUTE if an
 *			   attribute different from MANIFEST_PROTECTION_ID was encountered.
 */
static error_t parseattr(ManifestBox* mb, const char **attr)
{
	count_t i;

	chardata *key, *value;

	for (i = 0; attr[i]; i += 2)
	{	if (!attr[i+1]) return MANIFEST_PARSER_ERROR;

		/* including a \0 sigil */
		chardata *tmpvalue = malloc(strlen(attr[i+1]) + sizeof (char));
		if (!tmpvalue) return MANIFEST_NO_MEMORY;

		/* the length must not change (_const_ char **) */
		strcpy(tmpvalue, attr[i+1]);

		if (!strcmp(attr[i], MANIFEST_ATTRS_KEY))
		{   key = tmpvalue;
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_ATTRS_VALUE))
		{   value = tmpvalue;
			continue;
		}
	}

	if (!SMTH_addtolist(key, &mb->tmpattributes) || !SMTH_addtolist(value, &mb->tmpattributes))
	{   free(key);
		free(value);
		return MANIFEST_NO_MEMORY;
	}

	return MANIFEST_SUCCESS;
}

/**
 * \brief StreamFragment (metadata for a set of Related fragments) parser.
 *
 * Attributes may appear in any order, but at least one of FragmentDuration and
 * FragmentTime must be present.
 *
 * \param m    The Manifest struct wrapper to be filled with parsed data.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or MANIFEST_INAPPROPRIATE_ATTRIBUTE if an
 *			   attribute different from MANIFEST_PROTECTION_ID was encountered.
 */
static error_t parsechunk(ManifestBox *mb, const char **attr)
{
	count_t i;

	Chunk *tmp = calloc(1, sizeof (Chunk));
	if (!tmp) return MANIFEST_NO_MEMORY;

	for (i = 0; attr[i]; i += 2)
	{
		if (!attr[i+1]) return MANIFEST_PARSER_ERROR;

		if (!strcmp(attr[i], MANIFEST_CHUNK_INDEX))
		{   tmp->index = (count_t) atoint32(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_CHUNK_DURATION))
		{   tmp->duration = (tick_t) atoint64(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_CHUNK_TIME))
		{   tmp->time = (tick_t) atoint64(attr[i+1]);
			continue;
		}
	}

	if (!tmp->duration)
		tmp->duration = mb->previoustime? tmp->time - mb->previoustime: 0;
	if (!tmp->time)
		tmp->time = mb->previousduration? tmp->duration + mb->previousduration: 0;

	mb->previousduration = tmp->duration;
	mb->previoustime = tmp->time;

	if (!SMTH_addtolist(tmp, &mb->tmpchunks))
	{   free(tmp);
		return MANIFEST_NO_MEMORY;
	}

	mb->activechunk = tmp;

	return MANIFEST_SUCCESS;
}

/**
 * \brief TrackFragmentElement (per-fragment specific metadata) parser.
 *
 * The Track::index attribute field is required and MUST be present.
 *
 * \param m    The Manifest struct wrapper to be filled with parsed data.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or MANIFEST_INAPPROPRIATE_ATTRIBUTE if an
 *			   attribute different from MANIFEST_PROTECTION_ID was encountered.
 */
static error_t parsefragindex(ManifestBox *mb, const char **attr)
{
	count_t i;

	DynList vendordata;
	SMTH_preparelist(&vendordata);

	ChunkIndex *tmp = calloc(1, sizeof (Chunk));

	if (!tmp) return MANIFEST_NO_MEMORY;

	for (i = 0; attr[i]; i += 2)
	{
		if (!attr[i+1]) return MANIFEST_PARSER_ERROR;

		if (!strcmp(attr[i], MANIFEST_FRAGMENT_INDEX))
		{	tmp->index = (count_t) atoint32(attr[i+1]);
			continue;
		}
		/* else */
		if(!addvendorattrs(&vendordata, &attr[i]))
		{   free(tmp);
			return MANIFEST_NO_MEMORY;
		}
	}

	if (!SMTH_finalizelist(&vendordata)) return MANIFEST_NO_MEMORY;
	tmp->vendorattrs = (chardata**) vendordata.list;

	if (!SMTH_addtolist(tmp, &mb->tmpfragments)) return MANIFEST_NO_MEMORY;

	/* content may be present only if it is flagged into active Stream */
	if (mb->activestream->isembedded)
	{   EmbeddedData *tmpembedded = calloc(1, sizeof (EmbeddedData));
		if (!tmpembedded)
		{   free(tmp);
			return MANIFEST_NO_MEMORY;
		}
		tmp->embedded = tmpembedded;
		mb->activefragment = tmp;
	}
	
	return MANIFEST_SUCCESS;
}

/**
 * \brief Decodes (base64) the given \c text of length \c length and appends
 *        it to a \c EmbeddedData.
 *
 * \param dest   The EmbeddedData to fill
 * \param text   The text to be decoded.
 * \param length The length of \c text. If \c text is longer, only first
 *               \c length bytes will be decoded, if it is shorter, a memory
 *               error will be likely (never attempt!).
 */
static error_t parsepayload(EmbeddedData *ed, const char *text, int length)
{
	if (length > 0)
	{
		length_t newlength =  ed->length + length;
		/* In case this is the first time, it will be equivalent to a malloc */
		base64data *tmp = realloc(ed->content, newlength);
		if (!tmp) return MANIFEST_NO_MEMORY;
		/* Append */
		memcpy(&tmp[ed->length], text, length); //FIXME decode

		ed->content = tmp;
		ed->length = newlength;
	}

	return MANIFEST_SUCCESS;
}

/* vim: set ts=4 sw=4 tw=0: */
