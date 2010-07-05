/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * -- smth-common-defs.h --
 * synthetic type definitions: though not appearing in [smth] specifications,
 * these help clarify the meaning of many symbols
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

#ifndef __SMTH_COMMON_DEFS_H__
#define __SMTH_COMMON_DEFS_H__

/**
 * \internal
 * \file   smth-common-defs.h
 * \brief  synthetic type definitions.
 * \author Stefano Sanfilippo
 * \date   27th June 2010
 *
 * Though not appearing in [smth] specifications, these may
 * help to clarify the meaning of many symbols
 */

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t tick_t;			/**< A time interval					*/
typedef uint64_t offset_t;			/**< A memory offset					*/
typedef uint64_t lenght_t;			/**< A memory lenght					*/
typedef  int64_t signedlenght_t;	/**< A lenght that can be negative. Used
									 **  for overflow check.				*/
typedef uint32_t shortlenght_t;	    /**< Short memory lenght				*/
typedef uint32_t count_t;		    /**< An ordinal number					*/
typedef uint32_t bitrate_t;		    /**< A bitrate or a samplerate			*/
typedef uint32_t metric_t;		    /**< Something concerning screen size   */
typedef uint32_t tag_t;				/**< A numeric code that identifier		*/
typedef uint32_t word_t;			/**< A 32bit word						*/
typedef uint32_t flags_t;			/**< Flags bitfield						*/
typedef uint16_t unit_t;			/**< An atomic unit						*/
typedef int error_t;				/**< An error type (returned by parser  */
typedef char byte_t;				/**< A byte (obviously)					*/
typedef char uuid_t[16];			/**< A unique id						*/
typedef char base64data, hexdata, chardata, url_t;

/** \brief State of a variable */
typedef enum { UNDEF = 0, YES = 1, NO = 2} state_t;

#endif /* __SMTH_COMMON_DEFS_H__ */

/* vim: set ts=4 sw=4 tw=0: */
