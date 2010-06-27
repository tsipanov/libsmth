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

#include <smth-common-defs.h>
#include <stdbool.h>

/**
 * \file   smth-fragment-defs.h
 * \brief  private header for smth-fragment-parser.c
 * \author Stefano Sanfilippo
 * \date   27th June 2010
 */

/** The type of the Box being parsed. */
typedef enum {  MOOF,    /**< main metadata container                */
				MFHD,    /**< fragment's position into the track     */
				TRAF,    /**< track specific metadata                */
				SPECIAL, /**< vendor specific or encryption metadata */
				TFHD,    /**< per-sample defaults metadata           */
				TRUN,    /**< per-sample metadata                    */ 
				MDAT,    /**< data container                         */
				UNKNOWN  /**< unknown box type						 */
			 } boxtype;

/** The encryption system used by the samples */
typedef enum {  NONE,    /**< non encrypted sample            */
				AES_CTR, /**< AES 128bit CTR encrypted sample */
				AES_CBC  /**< AES 128bit CBC encrypted sample */
			 } encryptiontype ;

/** Holds the stream and metadata of currently parsed Box */
typedef struct
{   lenght  size;		 /**< size of the incoming block				*/
	boxtype type;		 /**< type of the incoming block				*/
	SmoothStream stream; /**< input stream								*/
	Fragment* fragment;  /**< Fragment to be filled with extracted data */
} box;

/** The number of Box types, UNKNOWN excluded       */
#define N_OF_BOXES 7

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

static  int  parsebox(box *root);
static  int parsemoof(box* root);
static  int parsemdat(box* root);
static  int parsemfhd(box* root);
static  int parsetraf(box* root);
static  int parsetfhd(box* root);
static  int parsetrun(box* root);
static  int parseuuid(box* root);
static  int parseencr(box* root);
static bool isencrbox(box* root);
static bool readbox(void *dest, size_t size, box* root);

////////////////////////////////////////////////////////////////////////////////
  
typedef struct
{	/* A single Sample of Media. Sample boundaries in the MdatBox are defined
	 * by the values of the DefaultSampleSize and SampleSize fields
	 * in the TrunBox. */
	byte* SampleData;
} MdatBox;

#if 0
typedef struct
{	/* The number of Samples in the Fragment (4 bytes) */
	count SampleCount;
	/* The value of the SampleFlags field for the first Sample.
	 * This field MUST be present if and only if FirstSampleFlagsPresent
	 * takes the value %b1. (4 bytes) */
	bitrate FirstSampleFlags;
	/* The size of each Sample, in bytes. This field MUST be present
	 * if and only if SampleSizePresent takes the value %b1. If this field is
	 * not present, its implicit value is the value of the DefaultSampleSize
	 * field. (4 bytes)
	 */
	bitrate SampleSize;
	/* The duration of each Sample, in increments defined by the TimeScale
	 * for the Track. This field MUST be present if and only if
	 * SampleDurationPresent takes the value %b1.
	 * If this field is not present, its implicit value is the value
	 * of the DefaultSampleDuration field. (4 bytes)
	 */
	bitrate SampleDuration;
	/* The Sample flags of each Sample. This field MUST be present if and
	 * only if SampleFlagsPresent takes the value %b1. If this field is not
	 * present, its implicit value is the value of the DefaultSampleDuration
	 * field. If the FirstSampleFlags field is present and this field is
	 * omitted, this field's implicit value for the first Sample in the
	 * Fragment MUST be the value of the FirstSampleFlags field. (4 bytes)
	 */
	bitrate SampleFlags;
	/* The Sample Composition Time offset of each Sample, as defined in [ISOFF].
	 * This field MUST be present if and only if
	 * SampleCompositionTimeOffsetPresent takes the value %b1. (4 bytes) */
	bitrate SampleCompositionTimeOffset;
} TrunBox;

struct PerSampleFields
{	/* The duration of each Sample, in increments defined by the TimeScale
	 * for the Track. If this field is not present, its implicit value is
     * the value of the DefaultSampleDuration field. (4 bytes)
	 */
	bitrate SampleDuration;
	/* The size of each Sample, in bytes. This field MUST be present
	 * if and only if SampleSizePresent takes the value %b1. If this field is
	 * not present, its implicit value is the value of the DefaultSampleSize
	 * field. (4 bytes)
	 */
	bitrate SampleSize;
	/* The Sample flags of each Sample. This field MUST be present if and
	 * only if SampleFlagsPresent takes the value %b1. If this field is not
	 * present, its implicit value is the value of the DefaultSampleDuration
	 * field. If the FirstSampleFlags field is present and this field is
	 * omitted, this field's implicit value for the first Sample in the
	 * Fragment MUST be the value of the FirstSampleFlags field. (4 bytes)
	 */
	bitrate SampleFlags;
	/* The Sample Composition Time offset of each Sample, as defined in [ISOFF].
	 * This field MUST be present if and only if
	 * SampleCompositionTimeOffsetPresent takes the value %b1. (4 bytes) */
	bitrate SampleCompositionTimeOffset;
};
#endif
#if 0
ExtendedSampleFlags (4 bytes): A comprehensive Sample flags field.
SampleFlags (1 byte): A compact Sample flags field useful to choose Samples to discard.
SampleDependsOn (2 bits): Specifies whether this Sample depends on another Sample.
SampleDependsOnUnknown (2 bits): Unknown whether this Sample depends on other Samples.
SampleDependsOnOthers (2 bits): This Sample depends on other Samples.
SampleDoesNotDependOnOthers (2 bits): This Sample does not depend on other Samples.
SampleIsDependedOn (2 bits): Specifies whether other Samples depend on this Sample.
SampleIsDependedOnUnknown (2 bits): Unknown whether other Samples depend on this Sample.
SampleIsNotDisposable (2 bits): Other Samples depend on this Sample.
SampleIsDisposable (2 bits): No other Samples depend on this Sample.
SampleHasRedundancy (2 bits): Specifies whether this Sample uses redundant coding.
RedundantCodingUnknown (2 bits): Unknown whether this Sample uses redundant coding.
RedundantCoding (2 bits): This Sample uses redundant coding.
NoRedundantCoding (2 bits): This Sample does not use redundant coding.
SampleIsDifferenceValue (1 bit): Specifies whether the Sample is a difference between two states.
SamplePaddingValue (3 bits): The Sample padding value, as specified in [ISOFF].
SampleDegradationPriority (2 bytes): The Sample degradation priority, as specified in [ISOFF].

ExtendedSampleFlags = 6*6 RESERVED_BIT
                            SampleDependsOn
                            SampleIsDependedOn
                            SampleHasRedundancy
                            SamplePaddingValue
                            SampleIsDifferenceValue
                            SampleDegradationPriority

SampleFlags = 2*2 RESERVED_BIT
                  SampleDependsOn
                  SampleIsDependedOn
                  SampleHasRedundancy

SampleDependsOn = SampleDependsOnUnknown / SampleDependsOnOthers  / SampleDoesNotDependsOnOthers
SampleDependsOnUnknown = %b0 %b0
SampleDependsOnOthers = %b0 %b1
SampleDoesNotDependOnOthers = %b1 %b0
SampleIsDependedOn = SampleIsDependedOnUnknown / SampleIsNotDisposable / SampleIsDisposable
SampleIsDependedOnUnknown = %b0 %b0
SampleIsNotDisposable = %b0 %b1
SampleIsDisposable = %b1 %b0
SampleHasRedundancy = RedundantCodingUnknown / RedundantCoding / NoRedundantCoding
RedundantCodingUnknown = %b0 %b0
RedundantCoding = %b0 %b1
NoRedundantCoding = %b1 %b0
SamplePaddingValue = 3*3 BIT
SampleIsDifferenceValue = BIT
SampleDegradationPriority = UNSIGNED_INT16
RESERVED_UNSIGNED_INT64 = %x00 %x00 %x00 %x00 %x00 %x00 %x00 %x00
UNSIGNED_INT64 = 8*8 BYTE
RESERVED_UNSIGNED_INT32 = %x00 %x00 %x00 %x00
UNSIGNED_INT32 = 4*4 BYTE
RESERVED_UNSIGNED_INT16 = %x00 %x00
UNSIGNED_INT16 = 2*2 BYTE
RESERVED_BYTE = %x00
BYTE = 8*8 BIT
RESERVED_BIT = %b0
BIT = %b0 / %b1
#endif

#endif /* __SMTH_MANIFEST_FRAGMENT_H__ */

/* vim: set ts=4 sw=4 tw=0: */
