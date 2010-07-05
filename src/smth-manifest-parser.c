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
 * \date   30th June - 1st July 2010
 */

#include <stdbool.h>
#include <expat.h>
#include <string.h>
#include <smth-manifest-defs.h>

/** \brief expat tag start event callback   */
static void XMLCALL startblock(void *data, const char *el, const char **attr)
{
}

/** \brief expat tag end event callback		*/
static void XMLCALL endblock(void *data, const char *el)
{
}

/** \brief expat text event callback		*/
static void XMLCALL textblock(void *data, const char *text, int lenght)
{
	base64data *tmp = malloc(lenght+1);
	if (!tmp)
	{   //TODO
	}
	memcpy(tmp, text, lenght);
	tmp[lenght] = (base64data) 0;
	m->armor = tmp;
}

#define stolower(str) \
 int i; for (i = 0; str[i]; i++) str[i] = tolower(str[i]);


//TODO aggiungere uno stato per il parser
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
 * \return     MANIFEST_SUCCESS
 */
static error_t parsemedia(Manifest *m, const char **attr)
{
	count_t i;
	/* we rely on expat for attr[] to have an even element number: if a[2*i] is
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
			m->islive = (tolower(attr[i + 1]) == 't');
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
//TODO controllare adeguatamente la codifica dei caratteri (o lo fa expat??)
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
static error_t parseelement(Manifest *m, const char **attr)
{
	count_t i;
	for (i = 0; attr[i]; i += 2)
	{
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
FIXME nella struttura, bisogna inserire quale è il frammento attualmente in uso
TODO  le funzioni della serie parse pensano stupidamente ad inserire i dati,
      la funzione start controlla che sia tutto OK.

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

#if 0
The UrlPattern and related fields define a pattern that can be used by the client to make
semantically valid Fragment Requests for the presentation.
UrlPattern (variable): Encapsulates a pattern for constructing Fragment Requests.
BitrateSubstitution (variable): A placeholder expression for the Bit rate of a track.
CustomAttributesSubstitution (variable): A placeholder expression for the Attributes used to
disambiguate a track from other tracks in the stream.
TrackName (variable): A unique identifier that applies to all tracks in a stream.
BitrateSubstitution (variable): A placeholder expression for the time of a fragment.
The syntax of the fields defined in this section, specified in ABNF [RFC5234], is as follows:
   UrlPattern = QualityLevelsPattern "/" FragmentsPattern
   QualityLevelsPattern = QualityLevelsNoun "(" QualityLevelsPredicatePattern ")"
   QualityLevelsNoun = "QualityLevels"
   QualityLevelsPredicate = BitrateSubstitution ["," CustomAttributesSubstitution ]
   Bitrate = "{bitrate}" / "{Bitrate}"
   CustomAttributesSubstitution = "{CustomAttributes}"
   FragmentsPattern = FragmentsNoun "(" FragmentsPatternPredicate ")";
   FragmentsNoun = "Fragments"
   FragmentsPatternPredicate = TrackName "=" StartTimeSubstitution;
   TrackName = URISAFE_IDENTIFIER_NONNUMERIC
   StartTimeSubstitution = "{start time}" / "{start_time}"

P. 20 [24]
#endif

/* vim: set ts=4 sw=4 tw=0: */
