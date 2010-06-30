/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-manifest-defs.h: Parses SMTH xml Manifests (private header)
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

#ifndef __SMTH_MANIFEST_DEFS_H__
#define __SMTH_MANIFEST_DEFS_H__

/**
 * \internal
 * \file   smth-manifest-parser.h
 * \brief  XML manifest parser (private header)
 * \author Stefano Sanfilippo
 * \date   30th June 2010
 */

#include <smth-common-defs.h>
#include <smth-manifest-parser.h>

/**
 * \brief Manifest Response (see chapter 2.2.2 of specification)
 *
 * According to the specifications, the Manifest MUST be a Well-Formed XML
 * Document [XML] subject to the following constraints:
 *
 *   +The Document's XML Declaration's major version is 1.
 *   +The Document’s XML Declaration's minor version is 0.
 *   +The Document does not use a Document Type Definition (DTD).
 *   +The Document uses an encoding that is supported by the Client.
 *   +The XML Elements specified in this document do not use XML Namespaces.
 */

#define MANIFEST_STREAM_ELEMENT			 ("SmoothStreamingMedia")
	#define MANIFEST_MEDIA_TIME_SCALE		 ("TimeScale")
	#define MANIFEST_MEDIA_DURATION			 ("Duration")
	#define MANIFEST_MEDIA_IS_LIVE			 ("IsLive")
	#define MANIFEST_MEDIA_LOOKAHEAD		 ("LookaheadCount")
	#define MANIFEST_MEDIA_MAJOR_VERSION	 ("MajorVersion")
	#define MANIFEST_MEDIA_MINOR_VERSION	 ("MinorVersion")
	#define MANIFEST_MEDIA_DVR_WINDOW	     ("DVRWindowLength")
#define MANIFEST_ARMOR_ELEMENT			 ("Protection")
	#define MANIFEST_PROTECTION_ID			 ("SystemID")

#define MANIFEST_MEDIA_DEFAULT_TICKS	 (10000000)
#define MANIFEST_MEDIA_DEFAULT_MAJOR	 ("2")
#define MANIFEST_MEDIA_DEFAULT_MINOR	 ("0")

#define MANIFEST_SUCCESS				 ( 0)
#define MANIFEST_WRONG_VERSION			 (-1)
#define MANIFEST_INAPPROPRIATE_ATTRIBUTE (-2)

static error_t parsemedia(Manifest *m, const char **attr);
static error_t parsearmor(Manifest *m, const char **attr);

#if 0
typedef enum {VIDEO, AUDIO, TEXT} Type;
char TypeNames[3][6] = {"video", "audio", "text"};

typedef enum { 	SCMD, /* Triggers for actions by the higher-layer
				       * implementation on the Client */
   				CHAP, /* Chapter markers */
  				SUBT, /* Subtitles used for foreign-language audio */
				CAPT, /* Closed captions for the hearing-impaired */
   				DESC, /* Media descriptions for the hearing-impaired */
 				CTRL, /* Events the control application business logic */
  				DATA  /* Application data that does not fall
				       * into any of the above categories */
} Subtype;
char SubtypeNames[7][5] =  { "SCMD", "CHAP", "SUBT", "CAPT",
							 "DESC", "CTRL", "DATA" };

typedef enum {	PCM,  /* Linear 8 or 16 bit Pulse Code Modulation */
				WMA,  /* +Microsoft Windows Media Audio v7, v8
				       * and v9.x Standard (WMA Standard)
			           * +Microsoft Windows Media Audio v9.x
				       * and v10 Professional (WMA Professional)*/
				MP3,  /* ISO MPEG-1 Layer III */
				AAC,  /* ISO Advanced Audio Coding */
				VEN   /* SYNTHETIC Vendor-extensible format. */
} Codec;
char CodecNames[4][6] = {"353", "85", "255","65534"};

typedef enum {	H264, /* Advanced Video Coding, as specified in [AVCFF] */
				WVC1, /* VC-1, as specified in [VC-1] */
				AACL, /* AAC (Low Complexity), as specified in [AAC] */
				WMAP, /* WMA Professional */
				CUST  /* SYNTHETIC A vendor extension value containing a
				       * registered with MPEG4-RA, as specified in [ISOFF] */
} Container;
char ContainerNames[4][5] = { "H264", "WVC1", "AACL", "WMAP" };

/*
 * /SmoothStreamingMedia/StreamIndex/c/f
 *
 * specify metadata pertaining to a Fragment for a specific Track,
 * rather than all versions of a Fragment for a Stream.
 *
 * MUST contain: TrackFragmentIndex
 */
typedef struct
{	/* An ordinal that MUST match the value the Index field for the
	 * Track to which this TrackFragment field pertains. */
	count TrackFragmentIndex; // i
	/* A string of custom data containing no XML Elements. */
	chardata VendorExtensionTrackData;
} TrackFragmentElement;

/*
 * /SmoothStreamingMedia/StreamIndex/c
 *
 * specify metadata for one set of Related Fragments in a Stream.
 *
 * IMPORTANT!
 * The order of repeated StreamFragmentElement fields in a containing
 * StreamElement is significant for the correct function of the IIS
 * Smooth Streaming Transport Protocol.
 *
 * MUST contain: FragmentDuration, FragmentTime (either or both)
 */
typedef struct
{	/* The ordinal of the StreamFragmentElement in the Stream. If
  	 * FragmentNumber is specified, its value MUST monotonically
	 * increase with the value of FragmentTime. */
	count FragmentNumber; // n
	/* The duration of the Fragment, specified as a number of increments
  	 * defined by the implicit or explicit value of the containing
	 * StreamElement's StreamTimeScale field.
	 *
	 * IMPORTANT!
	 * If the FragmentDuration field is omitted, its implicit value MUST be
	 * computed by the Client by subtracting the value of the preceding
	 * StreamFragmentElement's FragmentTime field from the value of this
	 * StreamFragmentElement's FragmentTime field. If no subsequent
	 * StreamFragmentElement exists, the implicit value of FragmentTime is 0.
	 */
	tick FragmentDuration; // d
	/* The tick of the Fragment, specified as a number of increments defined
 	 * by the implicit or explicit value of the containing StreamElement's
	 * StreamTimeScale field.
	 *
	 * IMPORTANT!
	 * If the FragmentDuration field is omitted, its implicit value MUST
	 * be computed by the Client by adding the value of the preceding
	 * StreamFragmentElement's FragmentTime field to the value of this
	 * StreamFragmentElement's FragmentDuration field. If no preceding
	 * StreamFragmentElement exists, the implicit value of FragmentTime is 0.
	 */
	tick FragmentTime; // t
	/**/
	TrackFragmentElement*  TrackFragment;
} StreamFragmentElement;

/*
 * /SmoothStreamingMedia/StreamIndex/QualityLevel/CustomAttribute
 *
 * specify metadata that disambiguates Tracks in a Stream.
 */
typedef struct
{	chardata Name, Value;
} CustomAttributeElement;

/*
 * /SmoothStreamingMedia/StreamIndex/QualityLevel
 *
 * encapsulate metadata required to play a specific Track in the Stream.
 *
 * MUST contain: IndexAttribute, BitrateAttribute
 * MUST contain (if it is a video track): MaxWidth, MaxHeight, CodecPrivateData
 * MUST contain (if it is an audio track):
 *     MaxWidth, MaxHeight, CodecPrivateData, SamplingRate, Channels,
 *     BitsPerSample, PacketSize, AudioTag, FourCC
 */
typedef struct
{	/* An ordinal that identifies the Track and MUST be unique
	 * for each Track in the Stream. */
	count Index;
	/* The average bandwidth consumed by the track, in bits-per-second (bps).
	 * The value 0 MAY be used for Tracks whose Bit Rate is negligible
	 * relative to other Tracks in the Presentation. */
	bitrate Bitrate;
	/* The maximum width of a video Sample, in pixels. */
	metric MaxWidth;
	/* The maximum height of a video Sample, in pixels. */
	metric MaxHeight;
	/* The Sampling Rate of an audio Track, as defined in [ISOFF] */
	bitrate SamplingRate;
	/* The Channel Count of an audio Track, as defined in [ISOFF] */
	unit Channels;
	/* A numeric code that identifies which Media Format
	 * and variant of the Media
	 */
	tag AudioTag;
	/* */
	Codec Format;
	/* The Sample Size of an audio Track, as defined in [ISOFF] */
	unit BitsPerSample;
	/* The size of each audio Packet, in bytes */
	metric PacketSize;
	/* A four-character code that identifies which Media Format
	 * is used for each Sample.
	 */
	Container FourCC;
	/* Data that specifies parameters specific to the Media Format and
	 * common to all Samples in the Track, represented as a string
	 * of hex-coded bytes.
	 *
	 * 	FourCC == H264
	 * 		The CodecPrivateData field contains a hex-coded string
	 * 		representation of the following byte sequence:
	 * 			%x00 %x00 %x00 %x01 SPSField %x00 %x00 %x00 %x01 SPSField
 	 * 			SPSField contains the Sequence Parameter Set (SPS)
 	 *     		PPSField contains the Slice Parameter Set (PPS)
 	 * 	FourCC == WVC1
 	 * 		The CodecPrivateData field contains a hex-coded string
 	 * 		representation of the VIDEOINFOHEADER structure, specified in [VIH].
 	 * 	FourCC == AACL
 	 * 		The CodecPrivateData field SHOULD be empty.
 	 * 	FourCC == WMAP
 	 * 		The CodecPrivateData field contains the WAVEFORMATEX structure,
 	 * 		specified in [WFEX], if the AudioTag field equals "65534" equals
 	 * 		and SHOULD be empty otherwise.
 	 * 	FourCC is a vendor extension value
 	 * 		The format of the CodecPrivateData field is also vendor-extensible.
 	 * 		Registration of the FourCC field value with MPEG4-RA, as specified
 	 * 		in [ISOFF], to can be used to avoid collision between extensions.
 	 */
	hexdata *CodecPrivateData;
	/* The number of bytes that specify the length of each Network
	 * Abstraction Layer (NAL) unit. This field SHOULD be omitted
	 * unless the value of the FourCC field is "H264".
	 * The default value is 4.
	 */
	unit NALUnitLengthField;
	/* specify metadata that disambiguates Tracks in a Stream. */
	CustomAttributeElement *CustomAttribute;
	/* */
	StreamFragmentElement *StreamFragment;
} TrackElement;

/* /SmoothStreamingMedia/StreamIndex
 *
 * encapsulate metadata required to play a specific Stream in the Presentation.
 *
 * MUST contain: TypeAttribute
 * MUST contain, unless it is an embedded track:
 *     NumberOfFragments, NumberOfTracks, Url
 */
typedef struct
{	/* The type of the Stream: video, audio, or text. */
	Type TypeAttribute;
	/* 1. The number of Fragments available for this Stream.
	 * 2. The number of Tracks available for this Stream. */
	count NumberOfFragments, NumberOfTracks;
	/* A pattern used by the Client to generate Fragment Request messages. */
	url_t *Url;
	/* The time scale for duration and time values in this Stream,
	 * specified as the number of increments in one second. */
	tick TimeScale;
	/* Metadata describing available Tracks and Fragments. */
	TrackElement *Track;
	/* A four-character code that identifies the intended use
	 * category for each Sample in a text Track. */
	Subtype SubContent;
	/* The name of the Stream. */
	chardata *Name;
	/* Control events for applications on the Client. */
	//SubtypeControlEvents //FIXME NOT DEFINED!!
	/* The maximum width of a video Sample, in pixels. */
	metric StreamMaxWidth; //TODO: sistemare con struct
	/* The maximum height of a video Sample, in pixels. */
	metric StreamMaxHeight;
	/* The suggested display width of a video Sample, in pixels. */
	metric DisplayWidth;
	/* The suggested display height of a video Sample, in pixels. */
	metric DisplayHeight;
	/**/
	StreamFragmentElement *StreamFragment;
} StreamIndexElement;

#endif

#endif /* __SMTH_MANIFEST_DEFS_H__ */

/* vim: set ts=4 sw=4 tw=0: */
