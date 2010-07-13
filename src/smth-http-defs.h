/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-http-defs.h : web transfer glue (private header)
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

#ifndef __SMTH_HTTP_DEFS_H__
#define __SMTH_HTTP_DEFS_H__

/**
 * \internal
 * \file   smth-http-defs.h
 * \brief  Web transfer glue (private header).
 * \author Stefano Sanfilippo
 * \date   13th June 2010
 */

//1. scarica il Manifest
//2. scopri quanto dura un frammento
//3. fai buffer a sufficienza
//4. scarica continuamente audio e video
//5. apri un folder temporaneo

$presentation   = "/path/$name.(ism|[\w]{1}[\w\d]*)";
$manifest       = "$presentation/Manifest"; //mettere i punti di domanda dopo

bitrate_t $bitrate; /* The bit rate of the Requested fragment. */
tick_t $time;       /* The time of the Requested fragment.     */

/* An Attribute of the Requested fragment used to disambiguate tracks. */
$attribute		= "$key=$value"
$key            = URISAFE_IDENTIFIER_NONNUMERIC;
$value          = URISAFE_IDENTIFIER;
/* The name of the requested stream */
$name           = URISAFE_IDENTIFIER_NONNUMERIC;
/* The type of response expected by the client. */
$noun           = (	"Fragments"    |  /* FragmentsNounFullResponse */
                    "FragmentInfo" |  /* FragmentsNounMetadataOnly */
                    "RawFragments" |  /* FragmentsNounDataOnly */
                    "KeyFrames"    ); /* FragmentsNounIndependentOnly */
$fragment       = "$presentation/QualityLevels($bitrate(,$attribute)*)/$noun($name=$time)";

/*  The SparseStreamPointer and related fields contain data required to locate
 *  the latest fragment of a sparse stream. This message is used in conjunction
 *  with a Fragment Response message.
 */

/* The timestamp of the latest timestamp for a fragment for the SparseStream
 * that occurs at the same point in time or earlier than the presentation
 * than the requested fragment.
 */
$timestamp = STRING_UINT64
/* The stream Name of the related Sparse Name. The value of this field MUST
 * match the Name field of the StreamElement field that describes the stream,
 * specified in section 2.2.2.3, in the Manifest Response.
 */
$name = CHARDATA
/* The latest fragment pointer for a single related sparse stream. */
$sparse = "$name=$timestamp"
/* The set of latest fragment pointer for all sparse streams related to a
 * single requested fragment.
 */
$sparseset = "$sparse(,$sparse)*"
$header = 1*CHAR
/* A set of data that indicates the latest fragment for all related sparse streams. */
$sparsepointer = "($header;)?ChildTrack=\"SparseStreamSet (; SparseStreamSet )*\""

/*  The Fragment Not Yet Available message is an HTTP Response with an empty
 *  message body field and the HTTP Status Code 412 Precondition Failed.
 */

standard = "QualityLevels({bitrate},{CustomAttributes})/Fragments(video={start_time})"
#endif /* __SMTH_HTTP_DEFS__ */

/* vim: set ts=4 sw=4 tw=0: */
