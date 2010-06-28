/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-fragment-defs.h: interprets binary fragments stream (private header)
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

#ifndef __SMTH_MANIFEST_FRAGMENT_H__
#define __SMTH_MANIFEST_FRAGMENT_H__

/**
 * \internal
 * \file   smth-fragment-defs.h
 * \brief  private header for smth-fragment-parser.c
 * \author Stefano Sanfilippo
 * \date   27th June 2010
 */

#include <stdbool.h>
#include <smth-fragment-parser.h>
#include <smth-common-defs.h>

/** The type of the Box being parsed. */
typedef enum {  MOOF,    /**< main metadata container                */
				MFHD,    /**< fragment's position into the track     */
				TRAF,    /**< track specific metadata                */
				SPECIAL, /**< vendor specific or encryption metadata */
				TFHD,    /**< per-sample defaults metadata           */
				TRUN,    /**< per-sample metadata                    */ 
				MDAT,    /**< data container                         */
				UNKNOWN	 /**< unknown box type. MUST be the last	 */
			 } Boxtype;

/** Holds the stream and metadata of currently parsed Box */
typedef struct
{   lenght  size;			/**< size of the incoming block					*/
	Boxtype type;			/**< type of the incoming block					*/
	SmoothStream *stream;	/**< input stream								*/
	Fragment *f;			/**< Fragment to be filled with extracted data  */
} Box;

/** The tfhd Box has a BaseDataOffset field			*/
#define TFHD_BASE_DATA_OFFSET_PRESENT						(1<<0)
/** The tfhd Box has a SampleDescriptionIndex field */
#define TFHD_SAMPLE_DESCRIPTION_INDEX_PRESENT				(1<<1)
/** The tfhd Box has a DefaultSampleDuration field  */
#define TFHD_DEFAULT_SAMPLE_DURATION_PRESENT				(1<<3)
/** The tfhd Box has a DefaultSampleSize field		*/
#define TFHD_DEFAULT_SAMPLE_SIZE_PRESENT					(1<<4)
/** The tfhd Box has a DefaultSampleFlags field		*/
#define TFHD_DEFAULT_SAMPLE_FLAGS_PRESENT					(1<<5)

/** The TrunBox has a FirstSampleFlags field		*/
#define TRUN_FIRST_SAMPLE_FLAGS_PRESENT						(1<<2)
/** The TrunBox has a SampleDuration field			*/
#define TRUN_SAMPLE_DURATION_PRESENT						(1<<8)
/** The TrunBox has a SampleSize field				*/
#define TRUN_SAMPLE_SIZE_PRESENT							(1<<9)
/** The TrunBox has SampleFlags field(s)			*/
#define TRUN_SAMPLE_FLAGS_PRESENT							(1<<10)
/** The TrunBox has a CompositionTimeOffset field   */
#define TRUN_SAMPLE_COMPOSITION_TIME_OFFSET_PRESENT			(1<<11)

/** The SampleEncryptionBox has the optional fields */
#define ENCR_SAMPLE_ENCRYPTION_BOX_OPTIONAL_FIELDS_PRESENT  (1<<0)

/** Mask for extracting keysize value from a 32bit word containing both flags
 *  and KeySizeValue. Beware of endianess */
#define ENCRYPTION_KEY_TYPE_MASK (0xffffff00)
/** Mask for extracting flags value from a 32bit word containing both flags
 *  and KeySizeValue. Beware of endianess */
#define ENCRYPTION_KEY_SIZE_MASK (0x000000ff)

static  int  parsebox(Box* root);
static  int parsemoof(Box* root);
static  int parsemdat(Box* root);
static  int parsemfhd(Box* root);
static  int parsetraf(Box* root);
static  int parsetfhd(Box* root);
static  int parsetrun(Box* root);
static  int parseuuid(Box* root);
static  int parseencr(Box* root);
static bool isencrbox(Box* root);
static bool readbox(void *dest, size_t size, Box* root);

#endif /* __SMTH_MANIFEST_FRAGMENT_H__ */

/* vim: set ts=4 sw=4 tw=0: */
