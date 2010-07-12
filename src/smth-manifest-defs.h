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
 * \file   smth-manifest-defs.h
 * \brief  XML manifest parser - private header
 * \author Stefano Sanfilippo
 * \date   30th June 2010
 */

#include <smth-common-defs.h>
#include <smth-manifest-parser.h>
#include <smth-dynlist.h>

/** \brief Holds data and metadata for the Manifest parser. */
typedef struct
{	/** Handle of the active XML parser (for start/stop). */
	XML_Parser parser;
	/** The manifest to be filled with parsed data. */
	Manifest *m;
	/** The parser has finished parsing, anything coming after should
		be ignored. */
	bool manifestparsed;
	/** The error code reported by a parser handler. */
	error_t state;
	/** The duration of the previous chunk in the active stream. */	 
	tick_t previousduration;
	/** The timestamp of the previous chunk in the active stream. */
	tick_t previoustime;
	/** Pointer to the active \c Stream. */
	Stream *activestream;
	/** Pointer to the active \c Track. */
	Track *activetrack;
	/** Pointer to the active \c Chunk. */
	Chunk *activechunk;
	/** Pointer to the active \c ChunkIndex. */
	ChunkIndex *activefragment;
	/** The \c Manifest::streams to fill with Stream data. */
	DynList tmpstreams;
	/** The \c Stream::tracks to be filled with \c Track metadata. */
	DynList tmptracks;
	/** The \c Stream::chunks to be filled with \c Chunk metadata. */
	DynList tmpchunks;
	/** The \c Track::attributes to be filled with key/value metadata pairs. */
	DynList tmpattributes;
	/** The \c Chunk::fragments to be filled with \c FragmentIndex(es). */
	DynList tmpfragments;
	/** Embedded data to be assigned to active \c ChunkIndex. */
	EmbeddedData *embedded;
} ManifestBox;

/** The xml tag identifying a SmoothStream (root) section */
#define MANIFEST_ELEMENT					"SmoothStreamingMedia"
	/** The xml attribute representing number of ticks per second */
	#define MANIFEST_MEDIA_TIME_SCALE		"TimeScale"
	/** The xml attribute representing media duration */
	#define MANIFEST_MEDIA_DURATION			"Duration"
	/** The xml attribute signaling whether the fragment is part of live content */
	#define MANIFEST_MEDIA_IS_LIVE			"IsLive"
	/** The xml attribute representing server cache size in Fragments */
	#define MANIFEST_MEDIA_LOOKAHEAD		"LookAheadFragmentCount"
	/** The xml attribute representing the major version of the Manifest */
	#define MANIFEST_MEDIA_MAJOR_VERSION	"MajorVersion"
	/** The xml attribute representing the minor version of the Manifest */
	#define MANIFEST_MEDIA_MINOR_VERSION	"MinorVersion"
	/** The xml attribute representing the size of the DVR window */
	#define MANIFEST_MEDIA_DVR_WINDOW	    "DVRWindowLength"

/** The xml tag identifying a Protection (sub)section */
#define MANIFEST_ARMOR_ELEMENT				"ProtectionHeader"
	/** The xml attribute marking a 16bytes UUID */
	#define MANIFEST_PROTECTION_ID			"SystemID"

/**	The xml tag name for StreamElement */
#define MANIFEST_STREAM_ELEMENT				"StreamIndex"
	/**	The xml attribute name for StreamElement::Type			*/
	#define MANIFEST_STREAM_TYPE			"Type"
	/**	The xml attribute name for StreamElement::SubType		*/
	#define MANIFEST_STREAM_SUBTYPE			"Subtype"
	/**	The xml attribute name for StreamElement::TimeScale		*/
	#define MANIFEST_STREAM_TIME_SCALE		"TimeScale"
	/**	The xml attribute name for StreamElement::StreamName	*/
	#define MANIFEST_STREAM_NAME			"Name"
	/**	The xml attribute name for StreamElement::StreamNumber  */
	#define	MANIFEST_STREAM_CHUNKS_NO		"Chunks"
	/**	The xml attribute name for StreamElement::QualityLevels */
	#define	MANIFEST_STREAM_QUALITY_LEVELS_NO "QualityLevels"
	/**	The xml attribute name for StreamElement::StreamUrl		*/
	#define	MANIFEST_STREAM_URL				"Url"
	/**	The xml attribute name for StreamElement::MaxWidth	    */
	#define	MANIFEST_STREAM_MAX_WIDTH		"MaxWidth"
	/**	The xml attribute name for StreamElement::MaxHeight		*/
	#define	MANIFEST_STREAM_MAX_HEIGHT		"MaxHeight"
	/**	The xml attribute name for StreamElement::DisplayWidth  */
	#define	MANIFEST_STREAM_DISPLAY_WIDTH   "DisplayWidth"
	/**	The xml attribute name for StreamElement::DisplayHeight	*/
	#define	MANIFEST_STREAM_DISPLAY_HEIGHT  "DisplayHeight"
	/**	The xml attribute name for StreamElement::ParentStream	*/
	#define	MANIFEST_STREAM_PARENT			"ParentStreamIndex"
	/**	The xml attribute name for StreamElement::ManifestOutput */
	#define MANIFEST_STREAM_OUTPUT			"ManifestOutput"

/** The xml tag name identifying a new track.	*/
#define MANIFEST_TRACK_ELEMENT 				"QualityLevel"
	/**	The xml attribute name for Track::Index 	*/
	#define MANIFEST_TRACK_INDEX			"Index"
	/**	The xml attribute name for Track::Bitrate	*/
	#define MANIFEST_TRACK_BITRATE			"Bitrate"
	/**	The xml attribute name for Track::MaxWidth 	*/
	#define MANIFEST_TRACK_MAXWIDTH			"MaxWidth"
	/**	The xml attribute name for Track::MaxHeight	*/
	#define MANIFEST_TRACK_MAXHEIGHT		"MaxHeight"
	/**	The xml attribute name for Track::PacketSize */
	#define MANIFEST_TRACK_PACKETSIZE  		"PacketSize"
	/**	The xml attribute name for Track::SampleRate */
	#define MANIFEST_TRACK_SAMPLERATE   	"SamplingRate"
	/**	The xml attribute name for Track::AudioTag	*/
	#define MANIFEST_TRACK_AUDIOTAG			"AudioTag"
	/** The xml attribute name for Track::FourCC */
	#define MANIFEST_TRACK_FOURCC 			"FourCC"
	/** The xml attribute name for Track::Header */
	#define MANIFEST_TRACK_HEADER   		"CodecPrivateData"
	/** The xml attribute name for Track::Channels */
	#define MANIFEST_TRACK_CHANNELS 		"Channels"
	/** The xml attribute name for Track::SampleBitRate */
	#define MANIFEST_TRACK_BITSPERSAMPLE 	"BitsPerSample"
	/** The xml attribute name for Track::NalSize */
	#define MANIFEST_TRACK_NAL_LENGTH		"NALUnitLengthField"

/** The xml tag name for a single attribute. */
#define MANIFEST_ATTRS_ELEMENT				"Attribute"
	/** The xml attribute name for a Attrs::Key. */
	#define MANIFEST_ATTRS_KEY				"Name"
	/** The xml attribute name for a Attrs::Value. */
	#define MANIFEST_ATTRS_VALUE 			"Value"

/** The xml tag name of a Chunk identifier. */
#define MANIFEST_CHUNK_ELEMENT				"c"
	/** The xml attribute name of a Chunk::index. */
	#define MANIFEST_CHUNK_INDEX			"n"
	/** The xml attribute name of a Chunk::duration. */
	#define MANIFEST_CHUNK_DURATION			"d"
	/** The xml attribute name of a Chunk::time. */
	#define MANIFEST_CHUNK_TIME				"t"

/** The xml tag name of a Fragment identifier. */
#define MANIFEST_FRAGMENT_ELEMENT			"f"
	/** The xml attribute name for FragmentIndex::index. */
	#define MANIFEST_FRAGMENT_INDEX			"i"

/** Major version number for the Manifest. */
#define MANIFEST_MEDIA_DEFAULT_MAJOR		"2"
/** Minor version number for the Manifest. */
#define MANIFEST_MEDIA_DEFAULT_MINOR		"0"

/** Default number of ticks per minute. */
#define MANIFEST_MEDIA_DEFAULT_TICKS	10000000
/** The size of the parser buffer, in bytes. */
#define MANIFEST_XML_BUFFER_SIZE		8192
/** The length of a UUID string in bytes. */
#define MANIFEST_ARMOR_UUID_LENGTH		(35 + 2)
/** The default NAL length for tracks. */
#define NAL_DEFAULT_LENGTH				4

static bool stringissane(const char* s);

static void disposevendorattrs(chardata **vendorattrs);
static bool addvendorattrs(DynList *vendordata, const char **attr);
 
static void inline disposeembedded(EmbeddedData *ed);

static error_t     parsemedia(ManifestBox *mb, const char **attr);
static error_t     parsearmor(ManifestBox *mb, const char **attr);
static error_t    parsestream(ManifestBox *mb, const char **attr);
static error_t     parsetrack(ManifestBox *mb, const char **attr);
static error_t      parseattr(ManifestBox *mb, const char **attr);
static error_t     parsechunk(ManifestBox *mb, const char **attr);
static error_t parsefragindex(ManifestBox *mb, const char **attr);

static error_t parsepayload(EmbeddedData *ed, const char *text, int length);

static void XMLCALL startblock(void *data, const char *el, const char **attr);
static void XMLCALL   endblock(void *data, const char *el);
static void XMLCALL  textblock(void *data, const char *text, int length);

#endif /* __SMTH_MANIFEST_DEFS_H__ */

/* vim: set ts=4 sw=4 tw=0: */

//XXX convenience macro.
#define insertcustomattr(...) true
