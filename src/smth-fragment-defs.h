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
 * \date   27th-30th June 2010
 */

#include <endian.h>
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
				SDTP,    /**< ???									 */
				UNKNOWN	 /**< unknown box type. MUST be the last	 */
			 } BoxType;

/** \brief Holds stream and metadata of currently parsed Box */
typedef struct
{   signedlenght_t bsize;	/**< size of the incoming block	body			*/
	signedlenght_t tsize;	/**< size of the incoming block (total)			*/
	count_t	allocext;	    /**< number of allocated Extension slots		*/
	BoxType  type;			/**< type of the incoming block					*/
	FILE *stream;			/**< input stream								*/
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

/** Version of the TFHD box structure */
static const byte_t tfhdVersion = 0x00;

/** Version of the SampleEncryption box structure */
static const byte_t encryptionVersion = 0x00;

/** The signature of a SampleEncryptionBox, namely a specific UUIDBox */
static const uuid_t encryptionuuid = { 0xa2, 0x39, 0x4f, 0x52,
                                       0x5a, 0x9b, 0x4f, 0x14,
                                       0xa2, 0x44, 0x6c, 0x42,
                                       0x7c, 0x64, 0x8d, 0xf4 };

static const uuid_t emptyuuid = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/************************START ENDIAN DEPENDENT SECTION*************************
 * All data is initialised with little endian values, as most people using this
 * library will compile it on a x86 platform. Anyway, it should not change
 * much if using a big endian CPU, only a few assembler istructions more...
 * The code is packed with le32toh.
 ******************************************************************************/

/** If BoxSize is equal to boxishuge, then a LongBoxSize section is present.  */
static const word_t boxishuge = 0x01000000;

/** Names of Boxes encoded as 32bit unsigned integer, used for type detection.*/

/*  Used this Python snippet to build each row:
 *		for c in namestring: print '%x' % ord(c)
 */
static const word_t BoxTypeMask[] = { 0x666f6f6d, /**< "moof" */
									  0x6468666d, /**< "mfhd" */
								   	  0x66617274, /**< "traf" */
								 	  0x64697575, /**< "uuid" */
								 	  0x64686674, /**< "tfhd" */
								 	  0x6e757274, /**< "trun" */
								 	  0x7461646d, /**< "mdat" */
									  0x70746473  /**< "sdtp" */ };

/** The signature of different encryption methods. [First byte is keysize] */
static const word_t EncryptionTypeMask[] = { 0x00010000,   /**< AES 128-bit CTR */
               		                         0x00020000};  /**< AES 128-bit CBC */

/*************************END ENDIAN DEPENDED SECTION**************************/

static error_t  parsebox(Box* root);
static error_t parsemoof(Box* root);
static error_t parsemdat(Box* root);
static error_t parsemfhd(Box* root);
static error_t parsetraf(Box* root);
static error_t parsetfhd(Box* root);
static error_t parsetrun(Box* root);
static error_t parseuuid(Box* root);
static error_t parseencr(Box* root);
static error_t parsesdtp(Box* root);
static error_t  scanuuid(Box* root, signedlenght_t boxsize);
static bool isencrbox(Box* root);
static bool readbox(void *dest, size_t size, Box* root);

/**
 * \brief If there are less than 8 bytes remaining in the Box, skips 4B:
 *        they are certainly trailing or unknown bytes.
 *
 * No box is shorter than 9B.
 * This is an orrible quirk, specifically crafted for parsemfhd and parsetfhd,
 * and should been removed as soon as the function of undocumented fields is
 * discovered.
 */
#define XXX_SKIP_4B_QUIRK \
	if (boxsize < 9) \
	{   fseek(root->stream, sizeof(word_t), SEEK_CUR); \
		boxsize -= sizeof(word_t); \
	}

#endif /* __SMTH_MANIFEST_FRAGMENT_H__ */

/* vim: set ts=4 sw=4 tw=0: */
