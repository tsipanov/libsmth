/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-fragment-parser.h: interprets binary fragments stream (public header)
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

#ifndef __SMTH_FRAGMENT_PARSER__
#define __SMTH_FRAGMENT_PARSER__

/**
 * \internal
 * \file   smth-fragment-parser.h
 * \brief  Exported definitions for smth-fragment-parser.c
 * \author Stefano Sanfilippo
 * \date   27th-30th June 2010
 */

#include <smth-common-defs.h>

/** \brief The encryption system used by the samples */
/* To avoid coding issues, add new encodings ONLY between AES_CBC and NEW	  */
typedef enum {  NONE,    /**< non encrypted sample							  */
				AES_CTR, /**< AES 128bit CTR encrypted sample				  */
				AES_CBC, /**< AES 128bit CBC encrypted sample				  */
				UNSET	 /**< Unknown encryption method. MUST be the last one */
			 } EncryptionType ;

/** \brief Holds the extension data parsed from UUIDBoxes */
typedef struct
{   uuid_t  uuid;   /**< The UUID of the extension block	*/
	byte_t *data;   /**< The body of the Box				*/
	lenght_t size;  /**< The size of the Box				*/
} Extension;

/** \brief Holds the encryption metadata for the samples
 *  parsed from SampleEncryptionBox.
 */
typedef struct
{	/** The algorithm used to encrypt each Sample. Filled from AlgorithmID */
	EncryptionType type;
	/** A UUID that identifies the key used to encrypt Samples. */
	uuid_t id;
	/** The size of the InitializationVector field, in bytes.
	    Allowed values are 0x08 and 0x10 */
	byte_t vectorsize;
	/** The number of instances of the InitializationVector field in
	 *  the SampleEncryptionBox field, filled from
	 *  SampleEncryptionBoxSampleCount field
	 */
	count_t vectorno;
	/** The Initialization Vector for each Sample.
	 *  MUST be repeated exactly SampleEncryptionBoxSampleCount times.
	 *  Filled from InitializationVector field.
	 *  There is no way to tell where one vector ends and another one starts but
	 *  relying on the vectorsize and vectorno fields.
	 */
	byte_t *vectors;
} Encryption;

/** \brief Holds the default sample metadata parsed from the TfhdBox */
typedef struct
{	/** The offset, in bytes, from the beginning of the MdatBox field to the
	 *  Sample field in the MdatBox field. Filled from BaseDataOffset field.
	 */
	offset_t dataoffset;
	/** The ordinal of the Sample description for the Track that is
	 *  applicable to this Fragment. This field \e should be omitted.
	 *  Filled from SampleDescriptionIndex field.
	 */
	count_t index;
	/** The default duration of each Sample, in increments defined by
	 *  the TimeScale for the Track. Filled from DefaultSampleDuration field
	 */
	tick_t duration;
	/** The default size of each Sample, in bytes. Filled from
	 *  DefaultSampleSize field.
	 */
	bitrate_t size;
	/** The default value of the SampleFlags field for each Sample. If it is
	 *  zero, its implicit value is Fragment::settings. Filled from
	 *  DefaultSampleFlags field.
	 */
	flags_t settings;
} SampleDefault;

/** \brief Sample settings. */
typedef struct
{	/** The Sample degradation priority. */
	unit_t priority;
	/** Whether the Sample is a difference between two states. */
	bool isdifference;
	/** The Sample padding value (first 3 bits). */
	byte_t padding;
	/** Whether the Sample depends on another */
	state_t dependson;
	/** Whether other samples depends on this */
	state_t isdependedon;
	/** Whether the sample uses redundant coding */
	state_t redundant;
} SampleSettings;

/** \brief Per-sample metadata.
 *
 *  Note that the first three fields are equal to the ones in SampleDefault
 */
typedef struct
{	/** The duration of each Sample, in increments defined by Manifest::tick.
	 *  If this field is not present, its implicit value is the value of
	 *  SampleDefault::duration. Filled from SampleDuration field.
	 */
	bitrate_t duration;
	/** The size of each Sample, in bytes. If this field is zero its implicit
	 *  value is SampleDefault::size. Filled from SampleSize field.
	 */
	bitrate_t size;
	/** The Sample flags. If this field is not present, its implicit value is
	 *  SampleDefault::settings.
	 */
	flags_t settings;
	/** The Sample Composition Time offset of each Sample.
	 *  This field MUST be present if and only if the
	 *  SampleCompositionTimeOffsetPresent flag is set. Filled from
	 *  SampleCompositionTimeOffset field.
	 */
	bitrate_t timeoffset;
} Sample;

/** \brief Will hold the parsed fragment data */
typedef struct
{   /** \brief An ordinal number for the Fragment in the Track timeline.
	 *
	 *  The value for a Fragment later in the timeline must be greater than for
	 *  a Fragment earlier in the timeline, but ordinal values for consecutive
	 *  Fragments are not required to be consecutive.
	 */
	count_t index;
	/** The number of Samples in the Fragment, filled from Trun::SampleCount */
	count_t sampleno;
	/** The value of the SampleFlags field for the first Sample.
	 *  This field may be 0x00000000: in that case, program should check
	 *  per-sample settings and default settings. Filled from FirstSampleFlags
	 */ 
	flags_t settings;
	/** The absolute timestamp of the first sample of the fragment, in time
	 *  scale increments for the track. FIXME FIRST*/
	tick_t timestamp;
	/** The total duration of all samples in the fragment, in time scale
	 *  increments for the track. */
	tick_t duration;
	/** The encryption data for ciphered streams */
	Encryption armor;
	/** The default metadata for samples in the stream */
	SampleDefault defaults;
	/** Per-field settings from TrunBox, repeated exactly SampleCount times. */
	Sample *samples;
	/** Vendor-specific boxes, as a NULL terminated array */
	Extension *extensions;
	/** The size of the allocated data block [synthetic] */
	lenght_t size;
	/** The fragment data. Sample boundaries in the MdatBox are defined
	 *  by the values of the DefaultSampleSize and SampleSize fields
	 *  in the TrunBox. */
	byte_t *data;
} Fragment;

/** The fragment was successfully parsed */
#define FRAGMENT_SUCCESS			  ( 0)
/** The parser encountered an i/o error on the SmoothStream */
#define FRAGMENT_IO_ERROR			  (-1)
/** An unknown Box was encountered */
#define FRAGMENT_UNKNOWN			  (-2)
/** No more memory to allocate for data sections */ 
#define FRAGMENT_NO_MEMORY			  (-3)
/** A malformed Box was encountered */
#define FRAGMENT_PARSE_ERROR		  (-4)
/** A fragment that should not be in the current section was parsed */
#define FRAGMENT_INAPPROPRIATE		  (-5)
/** The fragment is smaller than declared. This often means a parse error */
#define FRAGMENT_OUT_OF_BOUNDS		  (-6)
/** The fragment is encrypted in a new, non implemented, algorithm */
#define FRAGMENT_UNKNOWN_ENCRYPTION	  (-7)
/** There are trailing bytes after a MdatBox that will not be parsed */
#define FRAGMENT_BIGGER_THAN_DECLARED (-8)

/** Sample priority (first 2 bytes)	*/
#define	SAMPLE_PRIORITY(S)		 ((unit_t) ((S) & 0xffff))
/** Sample difference flag (bit 17)					*/
#define SAMPLE_IS_DIFFERENCE(S)  ((bool) (((S) >> 16) & 1))
/** Sample padding values (bits 18,19 and 20)		*/
#define SAMPLE_PADDING(S) 		 ((byte_t) (((S) >> 17) & 7))
/** Sample redundancy (bits 21 and 22)				*/
#define SAMPLE_REDUNDANCY(S)	 ((state_t) (((S) >> 20) & 3))
/** Sample is depended on (bits 23 and 24)			*/
#define SAMPLE_IS_DEPENDED_ON(S) ((state_t) (((S) >> 22) & 3))
/** Sample depends on others (bits 25 and 26)		*/
#define SAMPLE_DEPENDS_ON(S)	 ((state_t) (((S) >> 24) & 3))

/** 
 *  \brief Get last byte, containing SampleSettings::dependson,
 *		   SampleSettings::isdependedon and SampleSettings::redundant.
 *
 *  For compatibility with MS specifications.
 */
#define SAMPLE_GET_SIMPLE_FLAGS(S) ((byte_t) ((S >> 20) & 0x3f))

#if 0
/** Fills a SampleSettings struct with data parsed from flagfield settings */
inline void parsesampleflags(SampleSettings *s, flags_t settings)
{
	s->priority 	= SAMPLE_PRIORITY (settings);
	s->isdifference = SAMPLE_IS_DIFFERENCE (settings);
	s->padding		= SAMPLE_PADDING (settings);
	s->dependson	= SAMPLE_REDUNDANCY (settings);
	s->isdependedon = SAMPLE_IS_DEPENDED_ON (settings);
	s->redundant	= SAMPLE_DEPENDS_ON (settings);
}
#endif

error_t parsefragment(Fragment *f, FILE *stream);
void disposefragment(Fragment *f);

#endif /* __SMTH_FRAGMENT_PARSER__ */

/* vim: set ts=4 sw=4 tw=0: */
