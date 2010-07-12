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
#include <smth-fragment-parser.h>
#include <smth-common-defs.h>
#include <smth-dynlist.h>

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
{   signedlength_t bsize;	/**< size of the incoming block	body			*/
	signedlength_t tsize;	/**< size of the incoming block (total)			*/
	DynList	extlist;	    /**< Extension dynamic list					    */
	BoxType type;			/**< type of the incoming block					*/
	FILE *stream;			/**< input stream								*/
	Fragment *f;			/**< Fragment to be filled with extracted data. */
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

/** If BoxSize is equal to boxishuge, then a LongBoxSize section is present.  */
#define BOX_IS_HUGE				 0x00000001

/** Mask for extracting keysize value from a 32bit word containing both flags
 *  and KeySizeValue. Beware of endianess */
#define ENCRYPTION_KEY_TYPE_MASK 0xffffff00

/** Mask for extracting flags value from a 32bit word containing both flags
 *  and KeySizeValue. Beware of endianess */
#define ENCRYPTION_KEY_SIZE_MASK 0x000000ff

/** If the mask reveals a non-zero value, then the following fields are 8B each
 *  else, they are 4B each. */
#define TFXD_LONG_FIELDS_MASK    0xff000000

/** If the mask reveals a non-zero value, then the following fields are 8B each
 *  else, they are 4B each. */
#define TFRF_LONG_FIELDS_MASK    0xff000000

/** Version of the TFHD box structure */
static const byte_t tfhdVersion = 0x00;

/** Version of the SampleEncryption box structure */
static const byte_t encryptionVersion = 0x00;

/** The uuid of a SampleEncryptionBox, namely a specific UUIDBox. */
static const uuid_t encryptionuuid = {  0xa2, 0x39, 0x4f, 0x52,
										0x5a, 0x9b, 0x4f, 0x14,
										0xa2, 0x44, 0x6c, 0x42,
										0x7c, 0x64, 0x8d, 0xf4 };

/** The uuid identifying a TfxdBox, namely a specific UUIDBox. */
static const uuid_t tfxduuid = { 0x6d, 0x1d, 0x9b, 0x05,
								 0x42, 0xd5, 0x44, 0xe6,
								 0x80, 0xe2, 0x14, 0x1d,
								 0xaf, 0xf7, 0x57, 0xb2 };

/** The uuid identifying a TfrfBox, namely a specific UUIDBox. */
static const uuid_t tfrfuuid = { 0xd4, 0x80, 0x7e, 0xf2,
								 0xca, 0x39, 0x46, 0x95,
								 0x8e, 0x54, 0x26, 0xcb,
								 0x9e, 0x46, 0xa7, 0x9f };

/** Names of Boxes encoded as 32bit unsigned integer, used for type detection.*/

/*  Used this Python snippet to build each row:
 *		for c in namestring: print '%x' % ord(c)
 */
static const word_t BoxTypeID[] = { 0x6d6f6f66, /**< "moof" */
									0x6d666864, /**< "mfhd" */
								   	0x74726166, /**< "traf" */
								 	0x75756964, /**< "uuid" */
								 	0x74666864, /**< "tfhd" */
								 	0x7472756e, /**< "trun" */
								 	0x6d646174, /**< "mdat" */
									0x73647470  /**< "sdtp" */ };
/** The signature of different encryption methods. [LSB is keysize]   */
static const word_t EncryptionTypeID[] = { 0x00000100,  /**< AES 128-bit CTR */
               		                       0x00000200}; /**< AES 128-bit CBC */

static error_t  parsebox(Box* root);
static error_t parsemoof(Box* root);
static error_t parsemdat(Box* root);
static error_t parsemfhd(Box* root);
static error_t parsetraf(Box* root);
static error_t parsetfhd(Box* root);
static error_t parsetrun(Box* root);
static error_t parseuuid(Box* root);
static error_t parsetfxd(Box* root);
static error_t parseencr(Box* root);
static error_t parsesdtp(Box* root);
static error_t parsetfrf(Box* root);
static error_t  scanuuid(Box* root, signedlength_t boxsize);
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
