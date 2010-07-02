/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * -- smth-fragment.h --
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

#ifndef __SMTH_FRAGMENT_H__
#define __SMTH_FRAGMENT_H__

/**
 * \internal
 * \file   smth-fragment-defs.h
 * \brief  private header for smth-fragment.c
 * \author Stefano Sanfilippo
 * \date   27th-30th June 2010
 */


/** \brief Sample settings. */
typedef struct
{	/** The Sample degradation priority. */ //UNSIGNED_INT16
	unit_t priority;
	/** Whether the Sample is a difference between two states. */ //BIT
	bool isdifference;
	/** The Sample padding value (first 3 bits). */ //3BITS
	byte_t padding;
	/** Whether the Sample depends on another */
	state_t dependson;
	/** Whether other samples depends on this */
	state_t isdependedon;
	/** Whether the sample uses redundant coding */
	state_t redundant;
} SampleSettings;

/** Sample priority (first 2 bytes)	*/
#define	SAMPLE_PRIORITY(S)	(S) & 0xffff
/** Sample difference flag (bit 17)					*/
#define SAMPLE_IS_DIFFERENCE(S) 	(((S) >> 16) & 1)
/** Sample padding values (bits 18,19 and 20)		*/
#define SAMPLE_PADDING(S) 			(((S) >> 17) & 7)
/** Sample redundancy (bits 21 and 22)				*/
#define SAMPLE_REDUNDANCY_MASK(S)	(((S) >> 20) & 3)
/** Sample is depended on (bits 23 and 24)			*/
#define SAMPLE_IS_DEPENDED_ON(S) 	(((S) >> 22) & 3)
/** Sample depends on others (bits 25 and 26)		*/
#define SAMPLE_DEPENDS_ON(S)		(((S) >> 24) & 3)

/** Get last byte, containing SampleSettings::dependson,
 *  SampleSettings::isdependedon and SampleSettings::redundant.
 *  For compatiility with MS specifications.
 */
#define SAMPLE_GET_SIMPLE_FLAGS (S) ((S >> 24) & 0xff)

/** Fills a SampleSettings struct with data parsed from flagfield settings */
inline void parsesampleflags(SampleSettings *s, flags_t settings)
{
	s->priority 	= (unit_t) 	SAMPLE_PRIORITY (settings);
	s->isdifference = (bool) 	SAMPLE_IS_DIFFERENCE (settings);
	s->padding		= (byte_t)	SAMPLE_PADDING (settings);
	s->dependson	= (state_t)	SAMPLE_REDUNDANCY_MASK (settings);
	s->isdependedon = (state_t)	SAMPLE_IS_DEPENDED_ON (settings);
	s->redundant	= (state_t)	SAMPLE_DEPENDS_ON (settings);
}

#endif /* __SMTH_FRAGMENT_H__ */

/* vim: set ts=4 sw=4 tw=0: */
