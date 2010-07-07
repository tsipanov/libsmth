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
	ManifestBox root = {m, false, NULL, MANIFEST_SUCCESS};
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
			mb->m->islive = (tolower(attr[i + 1][0]) == 't');
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
	if (!attr[0]) return MANIFEST_MALFORMED_ARMOR_UUID; /* no uuid */

	if (strcmp(attr[0], MANIFEST_PROTECTION_ID)) //FIXME fortify
		return MANIFEST_INAPPROPRIATE_ATTRIBUTE;
	if (strlen(attr[1]) != MANIFEST_ARMOR_UUID_LENGHT)
		return MANIFEST_MALFORMED_ARMOR_UUID;
//	mb->m->armorID = attr[1]; TODO 9A04F079-9840-4286-AB92E65BE0885F95
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

	Stream *tmp = calloc(1, sizeof(Stream));
	if (!tmp) return MANIFEST_NO_MEMORY;

	for (i = 0; attr[i]; i += 2)
	{
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
		{   tmp->tick = (tick_t) atol(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_NAME))
		{   tmp->name = malloc(strlen(attr[i+1])); //FIXME sanitize ALPHA *( ALPHA / DIGIT / UNDERSCORE / DASH )
			if(!tmp->name) return MANIFEST_NO_MEMORY;
			strcpy(tmp->name, attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_CHUNKS_NO))
		{   tmp->chunksno = (count_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_QUALITY_LEVELS_NO))
		{   tmp->tracksno = (count_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_MAX_WIDTH))
		{   tmp->maxsize.width = (metric_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_MAX_HEIGHT))
		{   tmp->maxsize.height = (metric_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_DISPLAY_WIDTH))
		{   tmp->bestsize.width = (metric_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_DISPLAY_HEIGHT))
		{   tmp->bestsize.height = (metric_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_OUTPUT))
		{   /* we can safely assume that if it is not true, it is false */
			tmp->isembedded = (tolower(attr[i+1][0]) == 't');
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_STREAM_SUBTYPE))
		{   if (strlen(attr[i+1]) == MANIFEST_STREAM_SUBTYPE_SIZE)
			{   strcpy(tmp->subtype, attr[i+1]);
				continue;
			}
			if (strlen(attr[i+1]) != 0) return MANIFEST_MALFORMED_SUBTYPE;
			/* else (NULL) keep it NULL */
		}
//////////////////////////////////////TODO//////////////////////////////////////
#if 0
Url (variable): A pattern used by the client to generate Fragment Request messages.
#endif
		if (!strcmp(attr[i], MANIFEST_STREAM_URL)) //UrlPattern
		{
			continue;
		}
#if 0
ParentStream (variable): Specifies the non-sparse stream that is used to transmit timing
information for this stream. If the ParentStream field is present, it indicates that the stream
described by the containing StreamElement field is a sparse stream. If present, the value of this
field MUST match the value of the Name field for a non-sparse stream in the presentation.
#endif
		if (!strcmp(attr[i], MANIFEST_STREAM_PARENT)) //FIXME sanitize ALPHA *( ALPHA / DIGIT / UNDERSCORE / DASH )
		{
			continue;
		}
#if 0
SubtypeControlEvents (variable): Control events for applications on the client.
#endif
		/* TODO else */
	}

	free(tmp->name); //XXX
	free(tmp); //XXX

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

	Track *tmp = calloc(1, sizeof(Track));
	if (!tmp) return MANIFEST_NO_MEMORY;

	for (i = 0; attr[i]; i += 2)
	{
		if (!strcmp(attr[i], MANIFEST_TRACK_INDEX))
		{   tmp->index = (count_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_BITRATE))
		{   tmp->bitrate = (bitrate_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_MAXWIDTH))
		{   tmp->maxsize.width = (metric_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_MAXHEIGHT))
		{   tmp->maxsize.height = (metric_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_PACKETSIZE))
		{   tmp->packetsize = (bitrate_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_SAMPLERATE))
		{   tmp->samplerate = (bitrate_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_AUDIOTAG))
		{   tmp->audiotag = (flags_t) atoi(attr[i+1]);
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
			tmp->header = malloc(strlen(attr[i+1]));
			if(!tmp->header) return MANIFEST_NO_MEMORY;
			/* data is not unhexlified because vendor extensions could put
			 * here anything, even text. */
			strcpy(tmp->header, attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_CHANNELS))
		{   tmp->channelsno = (unit_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_BITSPERSAMPLE))
		{   tmp->bitspersample = (unit_t) atoi(attr[i+1]);
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_TRACK_NAL_LENGHT))
		{   tmp->nalunitlenght = (unit_t) atoi(attr[i+1]);
			if (!tmp->nalunitlenght) tmp->nalunitlenght = NAL_DEFAULT_LENGHT;
			continue;
		}
		/* TODO else */
	}

	mb->fillwithattrs = tmp->attributes;
	//if (tmp->header) free(tmp->header);  XXX
	if (tmp) free(tmp); //XXX

	return MANIFEST_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Attribute (metadata that disambiguates tracks in a stream) parser.
 *
 * Tags different from Name and Value will be graciously ignored.
 *
 * \param m    The Manifest struct wrapper to be filled with parsed data.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or MANIFEST_INAPPROPRIATE_ATTRIBUTE if an
 *			   attribute different from MANIFEST_PROTECTION_ID was encountered.
 */
// <CustomAttributesElementName>
static error_t parseattr(ManifestBox* mb, const char **attr)
{
	count_t i;

	if (!mb->fillwithattrs) return MANIFEST_UNEXPECTED_ATTRS;

	Attribute *tmp = malloc(sizeof (Attribute));
	if (!tmp) return MANIFEST_NO_MEMORY;

	for (i = 0; attr[i]; i += 2)
	{
		chardata *tmpvalue = malloc(strlen(attr[i+1]));
		if (!tmpvalue) return MANIFEST_NO_MEMORY;

		/* the lenght must not change (_const_ char **)*/
		strcpy(tmpvalue, attr[i+1]);

		if (!strcmp(attr[i], MANIFEST_ATTRS_KEY))
		{   tmp->key = tmpvalue;
			continue;
		}
		if (!strcmp(attr[i], MANIFEST_ATTRS_VALUE))
		{   tmp->value = tmpvalue;
			continue;
		}
	}

//	mb->fillwithattrs = tmp; FIXME lista dinamica.
	free(tmp); //XXX
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

	return MANIFEST_SUCCESS;
}

//TODO mettere dove servono i calloc

#if 0
  <c n=STRING_UINT32 //number d=STRING_UINT64 //duration t=STRING_UINT64 //time>
    *(<f i=STRING_UINT32 VendorExtensionAttribute>
          XML_CHARDATA
      </f>)
  </c>
#endif

typedef struct
{   /** The ordinal of the StreamFragmentElement in the stream.
	 *  If FragmentNumber is specified, its value must monotonically increase
	 *  with the value of the FragmentTime field.
	 */
	count_t index;
	/** The duration of the fragment, specified as a number of increments
	 *  defined by the implicit or explicit value of the containing
	 *  StreamElements StreamTimeScale field. If the FragmentDuration field
	 *  is omitted, its implicit value MUST be computed by the client by
	 *  subtracting the value of the preceding StreamFragmentElements
	 *  FragmentTime field from the value of this StreamFragmentElement's
	 *  FragmentTime field. If no subsequent StreamFragmentElement exists,
	 *  the implicit value of the FragmentTime field is 0.
	 */
	tick_t duration;
	/** The time of the fragment, specified as a number of increments defined
	 *  by the implicit or explicit value of the containing StreamElement's
	 *  StreamTimeScale field. If the FragmentDuration field is omitted, its
	 *  implicit value MUST be computed by the client by adding the value of
	 *  the preceding StreamFragmentElement's FragmentTime field to the value
	 *  of this StreamFragmentElement's FragmentDuration field. If no preceding
	 *  StreamFragmentElement exists, the implicit value of the FragmentTime
	 *  field is 0.
	 */
	tick_t time;
} Chunk;



//aggiungere tipo di box aperto come controllo sulle chiusuere

/**
 * \brief TrackFragmentElement (per-fragment specific metadata) parser.
 *
 * The TrackFragmentIndex attribute field is required and MUST be present.
 *
 * \param m    The Manifest struct wrapper to be filled with parsed data.
 * \param attr The attributes to parse.
 * \return     MANIFEST_SUCCESS or MANIFEST_INAPPROPRIATE_ATTRIBUTE if an
 *			   attribute different from MANIFEST_PROTECTION_ID was encountered.
 */
static error_t parsefrag(ManifestBox* mb, const char** attr)
{
}
#if 0
  TrackFragmentElement = <f i=STRING_UINT32 //index Vendor>
							BASE64_STRING
                         </f>
#endif

typedef struct
{   /**  An ordinal that MUST match the value of the Index field for the track
	 *   to which this TrackFragment field pertains.
	 */
	count_t index;
	/** A string that contains the Base64-encoded representation of the raw
	 *  bytes of the sample data for this fragment. This field MUST be omitted
	 *  unless the ManifestOutput field for the corresponding stream contains
	 *  a CASEINSENSITIVE_TRUE value.
	 */
	base64data* content; //FIXME inizializzare a NULL
} FragmentElement;


//TODO inserire un puntatore allo stream attivo e annullarlo ad ogni chiusura.

/** \brief expat tag start event callback. */
static void XMLCALL startblock(void *data, const char *el, const char **attr)
{
	ManifestBox *manbox = data;

	if (!strcmp(el, MANIFEST_ELEMENT))
	{   manbox->state = parsemedia(manbox, attr);
		return;
	}
	if (!strcmp(el, MANIFEST_STREAM_ELEMENT))
	{   manbox->state = parsestream(manbox, attr);
		return;
	}
	if (!strcmp(el, MANIFEST_TRACK_ELEMENT))
	{   manbox->state = parsetrack(manbox, attr);
		return;
	}
	if (!strcmp(el, MANIFEST_ARMOR_ELEMENT))
	{   manbox->state = parsearmor(manbox, attr);
		return;
	}

// FIXME case CHUNK: manbox->state = parsemedia(manbox, attr); break;

//	manbox->state = MANIFEST_UNKNOWN_BLOCK; /* it should never arrive here */
}

/** \brief expat tag end event callback. */
static void XMLCALL endblock(void *data, const char *el)
{   ManifestBox *manbox = data;
}

/** \brief expat text event callback. */
static void XMLCALL textblock(void *data, const char *text, int lenght)
{
	ManifestBox *manbox = data;
	if (manbox->armorwaiting)
	{
//FIXME e se unserissi anche la lunghezza??
		if (lenght > 0)
		{	
			base64data *tmp = malloc(lenght+1);
			if (!tmp) manbox->state = MANIFEST_NO_MEMORY;
			memcpy(tmp, text, lenght);
			tmp[lenght] = (base64data) 0;
			manbox->m->armor = tmp;
		}
		else manbox->m->armor = NULL;
	}
	manbox->armorwaiting = false;
}

/* vim: set ts=4 sw=4 tw=0: */
