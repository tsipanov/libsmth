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
 * \date   27th June 2010
 */

#include <smth-common-defs.h>

/** \brief The encryption system used by the samples */
/* To avoid coding issues, add new encodings ONLY between AES_CBC and NEW	  */
typedef enum {  NONE,    /**< non encrypted sample							  */
				AES_CTR, /**< AES 128bit CTR encrypted sample				  */
				AES_CBC, /**< AES 128bit CBC encrypted sample				  */
				NEW		 /**< Unknown encryption method. MUST be the last one */
			 } EncryptionType ;

/** \brief Holds the encryption metadata for the samples
 *  parsed from SampleEncryptionBox.
 */
typedef struct
{	/** The algorithm used to encrypt each Sample. Filled from AlgorithmID */
	EncryptionType type;
	/** A UUID that identifies the key used to encrypt Samples. */
	ID id;
	/** The size of the InitializationVector field, in bytes.
	    Allowed values are 0x08 and 0x10 */
	byte vectorsize;
	/** The number of instances of the InitializationVector field in
	 *  the SampleEncryptionBox field, filled from
	 *  SampleEncryptionBoxSampleCount field
	 */
	count vectorno;
	/** The Initialization Vector for each Sample.
	 *  MUST be repeated exactly SampleEncryptionBoxSampleCount times.
	 *  Filled from InitializationVector field.
	 *  There is no way to tell where one vector ends and another one starts but
	 *  relying on the vectorsize and vectorno fields.
	 */
	byte* vectors;
} Encryption; // MUST BE FREED.
//TODO cerchiamo di unificare le due strutture sottostanti
/** \brief Holds the default sample metadata parsed from the TfhdBox */
typedef struct
{	/** The offset, in bytes, from the beginning of the MdatBox field to the
	 *  Sample field in the MdatBox field. Filled from BaseDataOffset field.
	 */
	offset dataoffset;
	/** The ordinal of the Sample description for the Track that is
	 *  applicable to this Fragment. This field \e should be omitted.
	 *  Filled from SampleDescriptionIndex field.
	 */
	count index;
	/** The default duration of each Sample, in increments defined by
	 *  the TimeScale for the Track. Filled from DefaultSampleDuration field
	 */
	tick duration;
	/** The default size of each Sample, in bytes. Filled from
	 *  DefaultSampleSize field.
	 */
	bitrate size;
	/** The default value of the SampleFlags field for each Sample. Filled from
	 *  DefaultSampleFlags field.
	 */
	flags settings;
} SampleDefaultFields;

/** Note that the first three fields are equal to the ones in
 *  SampleDefaultFields */
typedef struct
{	/** The duration of each Sample, in increments defined by the TimeScale
	 *  for the Track. If this field is not present, its implicit value is
     *  the value of the DefaultSampleDuration field. This field MUST be present
	 *  if and only if SampleDurationPresent flag is set. Filled from
	 *  SampleDuration field.
	 */
	bitrate duration;
	/** The size of each Sample, in bytes. This field MUST be present
	 *  if and only if the flag SampleSizePresent is set. If this field is
	 *  not present, its implicit value is the value of the DefaultSampleSize
	 *  field. Filled from SampleSize field.
	 */
	bitrate size;
	/** The Sample flags of each Sample. This field MUST be present if and
	 *  only if SampleFlagsPresent flag is set. If this field is not
	 *  present, its implicit value is the value of the DefaultSampleDuration
	 *  field. If the FirstSampleFlags field is present and this field is
	 *  omitted, this field's implicit value for the first Sample in the
	 *  Fragment MUST be the value of the FirstSampleFlags field.
	 *  Retrieved from SampleFlags field.
	 */
	flags settings;
	/* The Sample Composition Time offset of each Sample, as defined in [ISOFF].
	 * This field MUST be present if and only if the
	 * SampleCompositionTimeOffsetPresent flag is set. Filled from
	 * SampleCompositionTimeOffset field.
	 */
	bitrate timeoffset;
} SampleFields;

/** \brief Fragment will hold the parsed fragment data */
typedef struct
{   /** \brief An ordinal number for the Fragment in the Track timeline.
	 *
	 *  The value for a Fragment later in the timeline must be greater than for
	 *  a Fragment earlier in the timeline, but ordinal values for consecutive
	 *  Fragments are not required to be consecutive.
	 */
	count ordinal;
	/** The number of Samples in the Fragment, filled from Trun::SampleCount */
	count sampleno;
	/** The value of the SampleFlags field for the first Sample.
	 *  This field may be 0x00000000: in that case, program should check
	 *  per-sample settings and default settings. Filled from FirstSampleFlags
	 */ 
	flags settings;
	/** The encryption data for ciphered streams */
	Encryption armor;
	/** The default metadata for samples in the stream */
	SampleDefaultFields defaults;
	/** Per-field settings from TrunBox, repeated exactly SampleCount times. */
	SampleFields *samples;
	/** The size of the allocated data block [synthetic] */
	lenght size;
	/** The fragment data. Sample boundaries in the MdatBox are defined
	 *  by the values of the DefaultSampleSize and SampleSize fields
	 *  in the TrunBox. */
	byte *data;
} Fragment;

/** The fragment was successfully parsed */
#define FRAGMENT_SUCCESS			( 1)
/** The parser encountered an i/o error on the SmoothStream */
#define FRAGMENT_IO_ERROR			(-1)
/** An unknown Box was encountered */
#define FRAGMENT_UNKNOWN			(-2)
/** No more memory to allocate for data sections */ 
#define FRAGMENT_NO_MEMORY			(-3)
/** A malformed Box was encountered */
#define FRAGMENT_PARSE_ERROR		(-4)
/** A fragment that should not be in the current section was parsed */
#define FRAGMENT_INAPPROPRIATE		(-5)
/** The fragment is smaller than declared. This often means a parse error */
#define FRAGMENT_OUT_OF_BOUNDS		(-6)
/** The fragment is encrypted in a new, non implemented, algorithm */
#define FRAGMENT_UNKNOWN_ENCRYPTION (-7)
/** There are trailing bytes after a MdatBox that will not be parsed */
#define FRAGMENT_BIGGER_THAN_DECLARED (-8)

int parsefragment(SmoothStream *stream, Fragment *f);
void disposefragment(Fragment *f);

#endif /* __SMTH_FRAGMENT_PARSER__ */

/* vim: set ts=4 sw=4 tw=0: */
