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

/** The Stream content type */
typedef enum {VIDEO, AUDIO, TEXT} StreamType;

/** The Stream subtype (for text streams) */
typedef enum { 	SCMD, /**< Triggers for actions by the higher-layer
				       *   implementation on the Client */
   				CHAP, /**< Chapter markers */
  				SUBT, /**< Subtitles used for foreign-language audio */
				CAPT, /**< Closed captions for the hearing-impaired */
   				DESC, /**< Media descriptions for the hearing-impaired */
 				CTRL, /**< Events the control application business logic */
  				DATA  /**< Application data that does not fall
				       *   into any of the above categories */
} StreamSubtype;

/** Stream codec */
typedef enum {	PCM,  /**< Linear 8 or 16 bit Pulse Code Modulation */
				WMA,  /**< +Microsoft Windows Media Audio v7, v8
				       *   and v9.x Standard (WMA Standard)
			           *   +Microsoft Windows Media Audio v9.x
				       *   and v10 Professional (WMA Professional)*/
				MP3,  /**< ISO MPEG-1 Layer III */
				AAC,  /**< ISO Advanced Audio Coding */
				VEN   /**< SYNTHETIC Vendor-extensible format. */
} CodecType;

/** Stream container type */
typedef enum {	H264, /**< Advanced Video Coding */
				WVC1, /**< Microsoft VC-1(R) */
				AACL, /**< AAC (Low Complexity) */
				WMAP, /**< WMA Professional */
				CUST  /**< A vendor extension value registered with MPEG4-RA */
} ContainerType;

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
	/** Opaque data that the Content Protection System identified in the
	 *  Manifest::armorid field can use to enable playback for authorized
	 *  users, encoded using Base64.
	 */
	base64data *armor;
} Manifest;

#define MANIFEST_SUCCESS				 ( 0)
#define MANIFEST_WRONG_VERSION			 (-1)
#define MANIFEST_INAPPROPRIATE_ATTRIBUTE (-2)

#endif /* __SMTH_MANIFEST_PARSER_H__ */

/* vim: set ts=4 sw=4 tw=0: */
