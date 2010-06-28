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
 * \file   smth-common-defs.h
 * \brief  synthetic type definitions.
 * \author Stefano Sanfilippo
 * \date   27th June 2010
 *
 * Though not appearing in [smth] specifications, these may
 * help to clarify the meaning of many symbols
 */

#include <stdint.h>
#include <stdio.h>

typedef uint64_t tick;			/**< A time interval					*/
typedef uint64_t offset;		/**< A memory offset					*/
typedef uint64_t lenght;		/**< A memory lenght					*/
typedef uint32_t shortlenght;   /**< Short memory lenght				*/
typedef uint32_t count;			/**< An ordinal number					*/
typedef uint32_t bitrate;		/**< A bitrate or a samplerate			*/
typedef uint32_t metric;		/**< Something concerning screen size   */
typedef uint32_t tag;
typedef uint32_t word;			/**< A 32bit word						*/
typedef uint32_t flags;			/**< Flags bitfield						*/
typedef uint16_t unit;			/**< An atomic unit						*/
typedef uint16_t keyID;			/**< A keyID							*/
/*typedef uint8_t  byte;*/
typedef char byte;
typedef char base64data, hexdata, chardata, url;
typedef char uuid[38];

typedef FILE SmoothStream;

#endif /* __SMTH_COMMON_DEFS_H__ */

/* vim: set ts=4 sw=4 tw=0: */
