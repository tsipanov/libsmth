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
 * \file   smth-fragment-parser.h
 * \brief  Exported definitions for smth-fragment-parser.c
 * \author Stefano Sanfilippo
 * \date   27th June 2010
 */

#include <smth-common-defs.h>

/** Holds the default sample metadata parsed from the TfhdBox */
typedef struct
{	/** The offset, in bytes, from the beginning of the MdatBox field to the
	 *  Sample field in the MdatBox field. Filled from BaseDataOffset field
	 */
	offset dataoffset;
	/** The ordinal of the Sample description for the Track that is
	 *  applicable to this Fragment. This field \e should be omitted.
	 *  Filled from SampleDescriptionIndex field
	 */
	count index;
	/** The default duration of each Sample, in increments defined by
	 *  the TimeScale for the Track. Filled from DefaultSampleDuration field
	 */
	tick duration;
	/** The default size of each Sample, in bytes. Filled from
	 *  DefaultSampleSize field
	 */
	bitrate samplesize;
	/** The default value of the SampleFlags field for each Sample. Filled from
	 *  DefaultSampleFlags field
	 */
	flags settings;
} defaultsample;

/** Holds the encryption metadata for the samples
 *  parsed from SampleEncryptionBox. MUST BE FREED.
 */
typedef struct
{	/** The algorithm used to encrypt each Sample. Filled from AlgorithmID */
	encryptiontype algorithm;
	/** The size of the InitializationVector field, in bytes.
	    Allowed values are 0x08 and 0x10 */
	byte vectorsize;
	/** A UUID that identifies the key used to encrypt Samples. */
	keyID kID;
	/** The number of instances of the InitializationVector field in
	 *  the SampleEncryptionBox field, filled from
	 *  SampleEncryptionBoxSampleCount field
	 */
	count vectorno;
	/** The Initialization Vector for each Sample
	 *  MUST be repeated exactly SampleEncryptionBoxSampleCount times
	 *  Filled from InitializationVector field
	 */
	byte* vectors;
} encryption;

/** Fragment will hold the parsed fragment data */
typedef struct
{   /** \brief An ordinal number for the Fragment in the Track timeline.
	 *
	 *  The value for a Fragment later in the timeline must be greater than for
	 *  a Fragment earlier in the timeline, but ordinal values for consecutive
	 *  Fragments are not required to be consecutive.
	 */
	count ordinal;
	/** The number of Samples in the Fragment */
	count samples;
	/** The default metadata for samples in the stream */
	defaultsample defaults;
	/** The encryption data for ciphered streams */
	encryption armor;
	/** The fragment data */
	byte  *data;
} fragment;

/** the fragment was successfully parsed */
#define FRAGMENT_SUCCESS		( 1)
/** the parser encountered an i/o error on the SmoothStream */
#define FRAGMENT_IO_ERROR		(-1)
/** an unknown Box was encountered */
#define FRAGMENT_UNKNOWN		(-2)
/** no more memory to allocate for data sections */ 
#define FRAGMENT_NO_MEMORY		(-3)
/** a malformed Box was encountered */
#define FRAGMENT_PARSE_ERROR	(-4)
/** a fragment that should not be in the current section was parsed */
#define FRAGMENT_INAPPROPRIATE  (-5)

int parsefragment(SmoothStream stream, fragment *f);

#endif /* __SMTH_FRAGMENT_PARSER__ */

/* vim: set ts=4 sw=4 tw=0: */
