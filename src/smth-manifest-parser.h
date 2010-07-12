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
 * \brief  XML manifest parser - public header
 * \author Stefano Sanfilippo
 * \date   30th June 2010
 */

#ifndef __SMTH_MANIFEST_PARSER_H__
#define __SMTH_MANIFEST_PARSER_H__

#include <stdio.h>
#include <smth-common-defs.h>
#include <smth-http.h>

/** The size of a Track::fourcc attribute string. */
#define MANIFEST_TRACK_FOURCC_SIZE   4
/** The size of a Stream::subtype attribute string. */
#define MANIFEST_STREAM_SUBTYPE_SIZE 4

/** \brief Screen size metadata. */
typedef struct
{   /** A coords couple describing a screensize. */
	metric_t width, height;
} ScreenMetrics;

/** \brief Holds embedded track data. */
typedef struct
{   byte_t* data;    /**< Data. */
	length_t length; /**< Lenght of the data. */
} Embedded; //XXX unisci a quelle sotto....
//TODO una funzione che toglie base64 e stipa....

/** \brief Holds index metadata for a Fragment. */
typedef struct
{   /**  An ordinal that must match the value of the Index field for the track
	 *   to which this TrackFragment field pertains.
	 */
	count_t index;
	/** The raw bytes of the sample data for this fragment, only if
	 *  Stream::isembedded is true.
	 */
	base64data* content;
} ChunkIndex;

/** \brief Holds metadata for a single chunk. */
typedef struct
{   /** The ordinal of the StreamFragmentElement in the stream.
	 *  The value must monotonically increase.
	 */
	count_t index;
	/** The duration of the fragment in ticks. If the FragmentDuration field
	 *  is omitted, its implicit value must be computed by the client by
	 *  subtracting the value of the preceding Chunk::time to the current
	 *  Chunk::time. If the fragment is the first in the stream, the implicit
	 *  value is 0.
	 */
	tick_t duration;
	/** The time of the fragment, in ticks. If it is omitted, its implicit
	 *  value must be computed by the client by adding the value of
	 *  the preceding StreamFragmentElement's FragmentTime field to the value
	 *  of this StreamFragmentElement's FragmentDuration field. If the fragment
	 *  is the first in the stream, the implicit value is 0.
	 */
	tick_t time;
	/** The subfragments of a \c Chunk. */
	ChunkIndex **fragments;
} Chunk;

/** \brief Metadata expressed as key/value pairs that disambiguates tracks. */
typedef struct
{   /** The name of a custom Attribute for a track. */
	chardata *key;
	/** The value of a custom Attribute for a track. */
	chardata *value;
} Attribute;

/** \brief Track specific metadata. */
typedef struct
{   /** An ordinal that identifies the track and MUST be unique for each track
	 *  in the stream. The Index should start at 0 and increment by 1 for each
	 *  subsequent track in the stream.
	 */
	count_t index;
	/** The average bandwidth consumed by the track, in bits per second (bps).
	 *  The value 0 may be used for tracks whose bit rate is negligible relative
	 *  to other tracks in the presentation.
	 */
	bitrate_t bitrate;
	/** The maximum size of a video sample, in pixels. */
	ScreenMetrics maxsize;
	/** The size of each audio Packet, in bytes. */
	bitrate_t packetsize;
	/** The Sampling Rate of an audio track */
	bitrate_t samplerate;
	/** A numeric code that identifies which media format and variant of the
	 *  media format is used for each sample in an audio track. */
	flags_t audiotag;
	/** A four-character code that identifies which media format is used for
	 *  each sample. */
	char fourcc[MANIFEST_TRACK_FOURCC_SIZE+1];
	/** Data that specifies parameters specific to the media format and common
	 *  to all samples in the track.
	 *  The meaning of each sequence is correlated to the FourCC type, as follows:
	 *    + "H264": hex-coded string of the following byte sequence:
	 *           %x00 %x00 %x00 %x01 SPSField %x00 %x00 %x00 %x01 SPSField
	 *              SPSField contains the Sequence Parameter Set (SPS).
	 *              PPSField contains the Slice Parameter Set (PPS).
	 *    + "WVC1": hex-coded string of the VIDEOINFOHEADER structure.
	 *    + "AACL": should be empty.
	 *    + "WMAP": WAVEFORMATEX structure, if the AudioTag field equals "65534",
	 *				and should be empty otherwise.
	 *    +  other: the format of the CodecPrivateData field is vendor-extensible.
	 *              Registration of the FourCC field value with MPEG4-RA, can be
	 *              used to avoid collision between extensions.
	 */
	hexdata *header;
	/** The Channel Count of an audio track */
	unit_t channelsno;
	/** The sample Size of an audio track */
	unit_t bitspersample;
	/** The number of bytes that specify the length of each Network Abstraction
	 *  Layer (NAL) unit. This field SHOULD be omitted unless the value of the
	 *  FourCC field is "H264". The default value is 4.
	 */
	unit_t nalunitlength;
	/** A set of attributes as a NULL terminated array that identify the Track. */
	Attribute **attributes;
} Track;

/** The Stream content type. */
typedef enum {VIDEO, AUDIO, TEXT} StreamType;

/** \brief Holds stream metadata. */
typedef struct
{	/** The type of the stream: video, audio, or text. */
	StreamType type;
	/** The time scale for duration and time values in this stream,
	 *  specified as the number of increments in one second. */
	tick_t tick;
	/** The name of the stream. */
	chardata *name;
	/** The number of fragments available for this stream. */
	count_t chunksno;
	/** The number of tracks available for this stream. */
	count_t tracksno;
	/** The maximum size of a video sample, in pixels. */
	ScreenMetrics maxsize;
	/** The suggested display size of a video sample, in pixels. */
	ScreenMetrics bestsize;
	/** A pattern used by the client to generate Fragment Request messages. */
	UrlPattern url;
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
	chardata subtype[MANIFEST_STREAM_SUBTYPE_SIZE+1];
	/**
	 * Specifies the non-sparse stream that is used to transmit timing
	 * information for this stream. If the ParentStream field is present, it
	 * indicates that the stream described is a sparse stream. The value of
	 * this field must match the value of Stream::name for a non-sparse stream
	 * in the presentation.
	 */
	chardata *parent;
	/** Pointer to a NULL terminated array of child tracks. */
	Track **tracks;
	/** Pointer to a NULL terminated array of child chunks. */
	Chunk **chunks;
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
	 *  effectively infinite. This field must not appear in on-demand
	 *  presentations.
	 */
	length_t dvrwindow;
	/** A UUID that uniquely identifies the Content Protection System.
	 *  For instance: \c {9A04F079-9840-4286-AB92E65BE0885F95}
	 */
	uuid_t armorID;
	/** Opaque data that can use to enable playback for authorize users,
	 *  encoded using Base64.
	 */
	base64data *armor;
	/** Pointer to the streams array (NULL terminated). */
	Stream **streams;
} Manifest;

/** The manifest was successfully parsed. */
#define MANIFEST_SUCCESS				 ( 0)
/** Wrong Manifest version. */
#define MANIFEST_WRONG_VERSION			 (-9)
/** An out-of-context attribute was parsed.	*/
#define MANIFEST_INAPPROPRIATE_ATTRIBUTE (-10)
/** There was no memory to istantiate the parser. */
#define MANIFEST_NO_MEMORY				 (-11)
/** There was an i/o error with the manifest file. */
#define MANIFEST_IO_ERROR				 (-12)
/** The parser encountered a malformed xml manifest. */
#define MANIFEST_PARSE_ERROR			 (-13)
/** The manifest is empty... */
#define MANIFEST_EMPTY					 (-14)
/** The xml backend behaved badly and it was blocked.	*/
#define MANIFEST_PARSER_ERROR			 (-15)
/** The manifest parser encountered an unknown element.	*/
#define MANIFEST_UNKNOWN_BLOCK			 (-16)
/** A text block was encountered where it was not expected.	*/
#define MANIFEST_UNEXPECTED_TRAILING	 (-17)
/** The armor UUID is malformed. */
#define MANIFEST_MALFORMED_ARMOR_UUID    (-18)
/** A stream type different from audio, video and text was encountered.	*/
#define MANIFEST_UNKNOWN_STREAM_TYPE	 (-19)
/** A malformed subtype string was encountered */
#define MANIFEST_MALFORMED_SUBTYPE		 (-20)
/** A malformed fourcc string was encountered */
#define MANIFEST_MALFORMED_FOURCC		 (-21)
/** An attribute block was encountered out of a Track element. */
#define MANIFEST_UNEXPECTED_ATTRS		 (-22)
/** A string identifier contained characters other than [A-Za-z/-] . */
#define MANIFEST_INVALID_IDENTIFIER		 (-23)

error_t SMTH_parsemanifest(Manifest *m, FILE *stream);
void  SMTH_disposemanifest(Manifest *m);

#endif /* __SMTH_MANIFEST_PARSER_H__ */

/* vim: set ts=4 sw=4 tw=0: */
