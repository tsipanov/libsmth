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
 * \file   smth-manifest-parser.h
 * \brief  XML manifest parser (public header)
 * \author Stefano Sanfilippo
 * \date   30th June 2010
 */

#ifndef __SMTH_MANIFEST_PARSER_H__
#define __SMTH_MANIFEST_PARSER_H__

#include <stdio.h>
#include <smth-common-defs.h>

/** Screen size metadata. */
typedef struct
{   metric_t width, height;
} ScreenMetrics;

/** The Stream content type. */
typedef enum {VIDEO, AUDIO, TEXT} StreamType;

typedef struct
{	/** The type of the stream: video, audio, or text. */
	StreamType type;
	/** The time scale for duration and time values in this stream,
	 *  specified as the number of increments in one second. */
	tick_t tick;
	/** The name of the stream. */
	chardata* name;
	/** The number of fragments available for this stream. */
	count_t chunksno;
	/** The number of tracks available for this stream. */
	count_t tracksno;
	/** The maximum size of a video sample, in pixels. */
	ScreenMetrics maxsize;
	/** The suggested display size of a video sample, in pixels. */
	ScreenMetrics bestsize;
	/** Whether sample data for this stream are embedded in the Manifest as
	 *  part of the ManifestOutputSample field.
	 *  Otherwise, the ManifestOutputSample field for fragments that are part
	 *  of this stream MUST be omitted.
	 */
	bool isembedded;
	/** A four-character code that identifies the intended use category for
	 *  each sample in a text track. However, the FourCC field, is used to
	 *  identify the media format for each sample.
	 */
	char subtype[4]; //XXX
} Stream;

/** \brief Holds the manifest data of the opened stream. */
typedef struct
{
	/** The duration of the content, measured in ticks, as indicated by the
	 *  value of the Manifest::tick field.
	 */
	tick_t duration;
	/** The time scale of the Manifest::duration attribute, specified as the
	 *  number of increments per second. The default value is 10000000.
	 */
	tick_t tick;
	/** Whether the presentation is a live streaming or an on-demand. */
	bool islive;
	/** Size of the server buffer, in number of fragments.
	 *  This field is set to 0 for on-demand content.
	 */
	count_t lookahead;
	/** The length of the DVR window, measured in ticks. If this field is
	 *  omitted for a live presentation or set to 0, the DVR window is
	 *  effectively infinite.
	 *  This field MUST be omitted for on-demand presentations.
	 */
	lenght_t dvrwindow;
	/** A UUID that uniquely identifies the Content Protection System to
	 *  which this ProtectionElement pertains.
	 *
	 *  For instance: {9A04F079-9840-4286-AB92E65BE0885F95}
	 */
	uuid_t armorID;
	/** Opaque data that can use to enable playback for authorize users,
	 *  encoded using Base64.
	 */
	base64data *armor;
	/** Pointer to the streams array (NULL terminated). */
	Stream* streams;
} Manifest;

#define MANIFEST_SUCCESS				 ( 0)
/** Wrong Manifest version. */
#define MANIFEST_WRONG_VERSION			 (-1)
/** An out-of-context attribute was parsed.	*/
#define MANIFEST_INAPPROPRIATE_ATTRIBUTE (-2)
/** There was no memory to istantiate the parser. */
#define MANIFEST_NO_MEMORY				 (-3)
/** There was an i/o error with the manifest file. */
#define MANIFEST_IO_ERROR				 (-4)
/** The parser encountered a malformed xml manifest. */
#define MANIFEST_PARSE_ERROR			 (-5)
/** The manifest is empty... */
#define MANIFEST_EMPTY					 (-6)
/** The xml backend behaved badly and it was blocked.	*/
#define MANIFEST_PARSER_ERROR			 (-7)
/** The manifest parser encountered an unknown element.	*/
#define MANIFEST_UNKNOWN_BLOCK			 (-8)
/** A text block was encountered where it was not expected.	*/
#define MANIFEST_UNEXPECTED_TRAILING	 (-9)
/** The armor UUID is malformed. */
#define MANIFEST_MALFORMED_ARMOR_UUID    (-10)
/** A stream type different from audio, video and text was encountered.	*/
#define MANIFEST_UNKNOWN_STREAM_TYPE	 (-11)
/** A malformed subtype string was encountered */
#define MANIFEST_MALFORMED_SUBTYPE		 (-12)

error_t parsemanifest(Manifest *m, FILE *manifest);

#endif /* __SMTH_MANIFEST_PARSER_H__ */

/* vim: set ts=4 sw=4 tw=0: */
