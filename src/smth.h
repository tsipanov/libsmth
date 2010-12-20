/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth.h: public API
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

#ifndef __SMTH_H__
#define __SMTH_H__

/**
 * \file   smth.h
 * \brief  libsmth public API
 * \author Stefano Sanfilippo
 * \date   14th December 2010
 */

#include <stdio.h>
#include <stdlib.h>

/** The Stream content type. \sa StreamType */
typedef enum {SMTH_VIDEO, SMTH_AUDIO, SMTH_TEXT} SMTH_type;

/** \brief Enumerates the settings that can be retrieved with \c SMTH_getinfo
 *
 *  Unless otherwise stated, all values are returned as 32 bit \c unsigned int
 *  Metadata specific to a certain type of stream (e.g. \c AUDIO), will be zeroed
 *  if active stream is not of the appropriate type.
 */
typedef enum
{
	/** The current average bitrate, in bps */
	SMTH_BITRATE,
	/** The size of each audio Packet, in bytes */
	SMTH_AUDIO_PACKET_SIZE,
	/** The Sampling Rate of an audio track */
	SMTH_AUDIO_SAMPLE_RATE,
	/** Numeric code identifying media format and variant */
	SMTH_AUDIO_TAG,
	/** The number of channels of an audio track */
	SMTH_AUDIO_CHANNELS,
	/** Size of an audio sample, in bps, as 16bit \c uint */
	SMTH_AUDIO_SAMPLE_SIZE,
	/** Length of a H264 NAL unit, as 16bit \c uint */ 
	SMTH_NAL_UNIT_LENGTH,
	/** A four characters code identifying media type. 0 terminated */ 
	SMTH_FOURCC,
	/** Max screensize allowed, as an array of two 32b \c uint {witdth, length} */
	SMTH_SCREENSIZE,
	/** Pointer to a \c malloced string containing the media header \sa Track */
	SMTH_HEADER,
	/** Type of stream, \sa SMTH_type */
	SMTH_TYPE,
	/** Pointer to a malloced string containing the stream name */
	SMTH_NAME,

} SMTH_setting;

#ifndef __COMPILING_LIBSMTH__

/** \brief Pseudofile handle, declared as an opaque pointer */
typedef void *SMTHh;

SMTHh SMTH_open(const char *url, const char *params);
size_t SMTH_read(void *buffer, size_t size, int stream, SMTHh handle);
int SMTH_EOS(SMTHh handle, int stream);
void SMTH_getinfo(SMTH_setting what, SMTHh handle, int stream, ...);
void SMTH_close(SMTHh handle);

#endif /* __COMPILING_LIBSMTH___ */

#endif /* __SMTH_H__ */

/* vim: set ts=4 sw=4 tw=0: */
