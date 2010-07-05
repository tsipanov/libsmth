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
 * \internal
 * \file   smth-manifest-defs.h
 * \brief  XML manifest parser (privare header)
 * \author Stefano Sanfilippo
 * \date   3rd July 2010
 */

/** The xml tag identifying a SmoothStream (root) section					  */
#define MANIFEST_ELEMENT					"SmoothStreamingMedia"
/** The xml attribute representing number of ticks per second				  */
	#define MANIFEST_MEDIA_TIME_SCALE		"TimeScale"
/** The xml attribute representing media duration							  */
	#define MANIFEST_MEDIA_DURATION			"Duration"
/** The xml attribute signaling whether the fragment is part of live content  */
	#define MANIFEST_MEDIA_IS_LIVE			"IsLive"
/** The xml attribute representing server cache size in Fragments			  */
	#define MANIFEST_MEDIA_LOOKAHEAD		"LookaheadCount"
/** The xml attribute representing the major version of the Manifest		  */
	#define MANIFEST_MEDIA_MAJOR_VERSION	"MajorVersion"
/** The xml attribute representing the minor version of the Manifest		  */
	#define MANIFEST_MEDIA_MINOR_VERSION	"MinorVersion"
/** The xml attribute representing the size of the DVR window				  */
	#define MANIFEST_MEDIA_DVR_WINDOW	    "DVRWindowLength"

/** The xml tag identifying a Protection (sub)section	   */
#define MANIFEST_ARMOR_ELEMENT			"Protection"
/** The xml attribute marking a 16bytes UUID				*/
	#define MANIFEST_PROTECTION_ID			"SystemID"

/**	The xml tag name for StreamElement						*/
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
	#define	MANIFEST_STREAM_CHUNKS			"Chunks"
/**	The xml attribute name for StreamElement::QualityLevels */
	#define	MANIFEST_STREAM_QUALITY_LEVELS  "QualityLevels"
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

/** Default number of ticks per minute					*/
#define MANIFEST_MEDIA_DEFAULT_TICKS	10000000
/** Major version number for the Manifest				*/
#define MANIFEST_MEDIA_DEFAULT_MAJOR	"2"
/** Minor version number for the Manifest				*/
#define MANIFEST_MEDIA_DEFAULT_MINOR	"0"

static char    StreamTypeNames[3][6] = { "video", "audio", "text"};
static char     ContainerNames[4][5] = { "H264", "WVC1", "AACL", "WMAP" };
static char     CodecTypeNames[4][6] = { "353", "85", "255", "65534"};
static char StreamSubtypeNames[7][5] = { "SCMD", "CHAP", "SUBT", "CAPT",
										 "DESC", "CTRL", "DATA" };

static void XMLCALL startblock(void *data, const char *el, const char **attr);
static void XMLCALL   endblock(void *data, const char *el);
static void XMLCALL  textblock(void *data, const char *text, int lenght);

static error_t   parsemedia(Manifest *m, const char **attr);
static error_t   parsearmor(Manifest *m, const char **attr);
static error_t parseelement(Manifest *m, const char **attr);

#endif /* __SMTH_MANIFEST_DEFS_H__ */

/* vim: set ts=4 sw=4 tw=0: */
