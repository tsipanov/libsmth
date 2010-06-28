/*
 * Copyright (C) 2010 Stefano Sanfilippo
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

/**
 * \internal
 * \file   smth-fragment-parser.c
 * \brief  Parses binary Fragments
 * \author Stefano Sanfilippo
 * \date   27th June 2010
 */

#include <stdlib.h>
#include <stdio.h>
#include <endian.h>
#include <smth-fragment-defs.h>

/*------------------------- HIC SUNT LEONES (CODICIS) ------------------------*/

/**
 * \brief        Parses a fragment opened as SmoothStream and fills all the 
 *               details in a Fragment structure.
 * \param stream pointer to the stream from which to read the fragment.
 * \param f      pointer to the Fragment structure to be filled with data
 *               extracted from the stream.
 * \return       FRAGMENT_SUCCESS on successful parse, or an appropriate error
 *               code.
 */
int parsefragment(SmoothStream *stream, Fragment *f)
{   Box root;
	root.stream = stream;
	root.f = f;
	int result;

	if(parsebox(&root) != FRAGMENT_SUCCESS) return FRAGMENT_PARSE_ERROR;
	result = parsemoof(&root);
	if(result != FRAGMENT_SUCCESS) return result;

	if(parsebox(&root) != FRAGMENT_SUCCESS) return FRAGMENT_PARSE_ERROR;
	result = parsemdat(&root);
	if(result != FRAGMENT_SUCCESS) return result;

	return FRAGMENT_SUCCESS; //FIXME se il file non e` finito, UUIDBox
}

/**
 * \brief    Disposes properly of a Fragment. These days, it only calls
 *           free() on the dinamically allocated fields, but programmers
 *           are advised to use it instead of freeing memory by themselves,
 *           as the internal data structure may vary heavily in the future.
 * \param f  The fragment  to be destroyed.
 */
void disposefragment(Fragment *f)
{
	free(f->data);
	free(f->samples);
}

/*
 * 1. Packet (`Box`) structure
 * ===========================
 *
 * Each `Fragment` contains a chunk of audio, video or text data, with a
 * metadata header. Each functional block of the fragment was named `Box`.
 * Each `Box` has a fixed structure:
 *
 *    ~-----------+------+-----------------+------------+-------------~
 *    ~ BoxLenght | Name | [BoxLongLenght] | BoxFields  | BoxChildren ~
 *    ~    4B     |  4B  |  8B (optional)  | {variable} |  {variable} ~
 *    ~-----------+------+-----------------+------------+-------------~
 *
 * where:
 *  + `Length`     is the lenght of the box in bytes, encoded in network format.
 *                 If the value of the field is "\0\0\0\1", the BoxLongLength
 *                 field must be present. Otherwise, we assert that it is not
 *                 present.
 *  + `Name`       is a 4B non null-terminated string identifying the type of
 *                 the block. There are seven different types, as it will be
 *                 explained in the appropriate section.
 *  + `LongLenght` is the lenght of the Box in bytes, encoded in network format. 
 *                 This field is present only if the size of the `Box` is larger
 *                 than BoxLenght max size (32B).
 *  + `Fields` &   are respectively the attributes and the content of the `Box`
 *    `Children`   and may vary accordingly to the role of the respective `Box`.
 *
 * The syntax of each `Box` is a strict subset of the syntax of the respective
 * Fragment Box defined in [ISOFF].
 *
 * 2. Fragment Structure
 * =====================
 *
 * As stated before, a Fragment Response is composed of various `Boxes`,
 *
 *  ~------------+------------~
 *  ~   MoofBox  |  MdatBox   ~
 *  ~ {variable} | {variable} ~
 *  ~------------+------------~
 *
 * Each `Box` must contain certain sub-Boxes, and it may contain others.
 * Any block may contain a `VendorExtensionUUIDBox`, as described later.
 *
 */

/** Version of the TFHD box structure */
static const byte tfhdVersion = 0;

/** Version of the SampleEncryption box structure */
static const byte encryptionVersion = 0;

/** If BoxSize is equal to boxishuge, then a LongBoxSize section is present. */
static const byte boxishuge[4] = {0, 0, 0, 1};

/** The signature of a SampleEncryptionBox, namely a specific UUIDBox */
static const byte encryptionuuid[16] = { 0xa2, 0x39, 0x4f, 0x52,
                                         0x5a, 0x9b, 0x4f, 0x14,
                                         0xa2, 0x44, 0x6c, 0x42,
                                         0x7c, 0x64, 0x8d, 0xf4 };

/** Names of Boxes encoded as 32bit unsigned integer, used for type detection.*/

/*  Use this Python snippet to build each row:
 *		for c in namestring: print '%x' % ord(c)
 */
static const word BoxTypeMask[7] = {   0x6d6f6f66,  /**< "moof" */
								        0x6d666864, /**< "mfhd" */
								   		0x74726166, /**< "traf" */
								        0x75756964, /**< "uuid" */
								        0x74666864, /**< "tfhd" */
								        0x7472756e, /**< "trun" */
								        0x6d646174  /**< "mdat" */ };

/** The signature of different encryption methods. Last byte is keysize */
static const word EncryptionTypeBytes[3] = { 0x00000000,   /**< Not encrypted   */
                                             0x00000100,   /**< AES 128-bit CTR */
                                             0x00000200};  /**< AES 128-bit CBC */

/**
 * \brief      Read size bytes from root->stream, and stores them into dest.
 * \param dest Pointer to the destination buffer. Note that readbox will not
 *             check for buffer overflow.
 * \param size Number of bytes to read from the input stream
 * \param root Pointer to a box structure holding the stream pointer.
 * \return     true if no error was encountered, otherwise false
 */
static bool readbox(void *dest, size_t size, Box* root)
{
	if((fread(dest, sizeof(byte), size, root->stream) < (size*sizeof(byte))) &&
	   (feof(root->stream) || ferror(root->stream)))
		return false;
	return true;
}

/**
 * \brief      Checks if a UUIDBox is a SampleEncryptionBox
 * \param root Pointer to the structure holding the Box to be parsed
 * \return     true if the Box is a SampleEncryptionBox, otherwise false.
 */
static bool isencrbox(Box* root)
{   byte signature[16];
	if(!readbox(signature, sizeof(signature), root)) return false;
	fseek(root->stream, -sizeof(signature), SEEK_CUR); /* rewind */
	return !memcmp(signature, encryptionuuid, sizeof(signature));
}

/**
 * \brief             Get flags&version field from the stream
 * \param defultflags Pointer to the buffer that will hold the flags
 * \return            true if the operation was successfull, otherwise false
 */
static bool getflags(flags *defaultflags, Box *root)
{	if(!readbox(defaultflags, sizeof(flags), root))
		return false;
	*defaultflags = (flags)be64toh(*defaultflags); /* endian-safe */
}

/**
 * \brief  prepares the first Box found on box->stream for parsing.
 *
 * parsebox will fill box structure with the size and the type of the incoming
 * block and position the input stream to the first byte of box data.
 * A parsing function would receive its size and type with the box structure,
 * then it may call parsebox to identify children Boxes and so on.
 * Obviously, it cannot be called by the parsing function itself, as the caller
 * needs to know in advance which parser to invoke.
 *
 * \param  root the box to be prepared.
 * \return FRAGMENT_SUCCESS if the box was successfully prepared,
 *         FRAGMENT_IO_ERROR in case of read/write error and FRAGMENT_UNKNOWN 
 *         if an unknown Box type was encountered (it should never happen).
 */
static int parsebox(Box* root) //FIXME attenzione! alle dimensioni! è big endian!!
{
	lenght tmpsize;
	word name;
	Boxtype element;
	shortlenght offset = sizeof(shortlenght)+sizeof(name);

	if(!readbox(&tmpsize, sizeof(shortlenght), root)) return FRAGMENT_IO_ERROR;
	if(!readbox(&name, sizeof(name), root)) return FRAGMENT_IO_ERROR;
	for(element = 0, root->type = UNKNOWN; element < UNKNOWN; element++)
		if( name & BoxTypeMask[element])
		{   root->type = element;
			break;
		}
	if(root->type == UNKNOWN) return FRAGMENT_UNKNOWN; //element non puo` mai essere UNKNOWN...
	if(!memcmp(boxishuge, &tmpsize, sizeof(boxishuge)))
	{
		if(!readbox(&root->size, sizeof(lenght),root)) return FRAGMENT_IO_ERROR;
		offset += sizeof(lenght);
	}
	else root->size = (lenght)tmpsize;

	root->size = (lenght)be64toh(root->size) - offset;
	return FRAGMENT_SUCCESS;
}

/**
 * \brief      MoofBox (metadata container) parser
 * 
 * MoofBox has two compulsory children, one MfhdBox and one TrafBox,
 * and no fields. Hence, parsemoof will parse two Boxes, determine their type
 * and pass them to the appropriate parser.
 *
 * \param root pointer to the Box structure to be parsed
 * \return     FRAGMENT_SUCCESS on successful parse, FRAGMENT_PARSE_ERROR if
 *             subparsers failed to decode Boxes, or FRAGMENT_INAPPROPRIATE if
 *             a Box that should not stay into a MoofBox was encountered.
 */

/* FIXME do they have to appear in a fixed order? It would be much simpler... */

static int parsemoof(Box* root)
{
	int i, result;
	signedlenght boxsize = root->size;

	for( i = 0; i < 2; i++)
	{   
		result = parsebox(root);
		if(result == FRAGMENT_SUCCESS)
		{	switch(root->type)
			{	case MFHD: result = parsemfhd(root); break;
				case TRAF: result = parsetraf(root); break;
				default: return FRAGMENT_INAPPROPRIATE;
			}
			if(result != FRAGMENT_SUCCESS) return result;
			boxsize -= root->size; /* size of the newly parsed Box */
		}
		else return result;
	}
	if(boxsize > 0) /*if there is something more, it is a UUIDBox */
	{
		result = parseuuid(root);
		if(result != FRAGMENT_SUCCESS) return result;
		boxsize -= root->size;
	}
	if(boxsize < 0) return FRAGMENT_OUT_OF_BOUNDS; /* should never happen */
	return FRAGMENT_SUCCESS;
}

/**
 * \brief      MfhdBox (position in the stream) parser
 *
 * Each MfhdBox contains a SequenceNumber field, representing the fragment's
 * position in the sequence for the Track. The SequenceNumber value for a
 * Fragment must be greater than for a Fragment earlier in the timeline, but
 * it is not required to be consecutive.
 *
 * \param  box pointer to the Box structure to be parsed
 * \return     FRAGMENT_SUCCESS on successful parse, or FRAGMENT_IO_ERROR.
 */

static int parsemfhd(Box* root)
{
	count tmp;
	signedlenght boxsize = root->size;

	if(!readbox(&tmp, sizeof(tmp), root)) return FRAGMENT_IO_ERROR;
	root->f->ordinal = (count)be32toh(tmp);

	if(boxsize > sizeof(tmp)) /* if there is something more */
	{	int result = parseuuid(root);
		if(result != FRAGMENT_SUCCESS) return result;
		boxsize -= root->size;
	}
	if(boxsize < 0) return FRAGMENT_OUT_OF_BOUNDS;
	return FRAGMENT_SUCCESS;
}

/**
 * \brief      TrafBox (track-specific metadata) parser
 *
 * MoofBox has two compulsory children, one TfhdBox and one TrunBox,
 * and no fields. Hence, parsemoof will parse two Boxes, determine their type
 * and pass them to the appropriate parser.
 *
 * \param root pointer to the Box structure to be parsed
 * \return     FRAGMENT_SUCCESS on successful parse, or an appropriate error
 *             code.
 * \sa         parsemoof
 */
static int parsetraf(Box* root)
{
	int i, result;
	signedlenght boxsize = root->size;

	for( i = 0; i < 2; i++)
	{   
		result = parsebox(root);
		if(result == FRAGMENT_SUCCESS)
		{	switch(root->type)
			{	case TFHD: result = parsetfhd(root); break;
				case TRUN: result = parsetrun(root); break;
				default: return FRAGMENT_INAPPROPRIATE;
			}
			if(result != FRAGMENT_SUCCESS) return result;
			boxsize -= root->size; /* size of the newly parsed Box */
		}
		else return result;
	}
	if(boxsize > 0) /* if there is something more, it is a UUIDBox */
	{
		result = parseuuid(root);
		if(result != FRAGMENT_SUCCESS) return result;
		boxsize -= root->size;
	}
	if(boxsize < 0) return FRAGMENT_OUT_OF_BOUNDS; /* should never happen */
	return FRAGMENT_SUCCESS;
}

/**
 * \brief        Sets target reading an appropriate number of bytes from stream
 *               if flag marked by mask is set and decrements boxsize
 *				 accordingly. You may add a else statement after this block,
 *               but it is highly risky: this macro is intended for internal
 *               use only, and implemention may vary without notice.
 * \param target The target to be set
 * \param mask   The mask to select the appropriate flag bit
 */
#define SETBYFLAG(target, mask)							\
	if(boxflags & (mask))								\
	{   if(!readbox(&(target), sizeof(target), root))   \
			return FRAGMENT_IO_ERROR;					\
		boxsize -= sizeof(target);						\
	}

/**
 * \brief TfhdBox (per-sample defaults metadata) parser
 *
 * A Tfhd Box has no children and a variable number of default settings, whose
 * presence is specified by the TfhdBoxFlags bitfield, a 3*BYTE field heading
 * the Box.
 *
 * \param root pointer to the Box structure to be parsed
 * \return     FRAGMENT_SUCCESS on successful parse, or an appropriate error
 *             code.
 * \sa         parsetrun
 */
static int parsetfhd(Box* root)
{
	signedlenght boxsize = root->size;
	flags boxflags;

	if(!getflags(&boxflags, root)) return FRAGMENT_IO_ERROR;

	SETBYFLAG(root->f->defaults.dataoffset, TFHD_BASE_DATA_OFFSET_PRESENT);
	SETBYFLAG(root->f->defaults.index, TFHD_SAMPLE_DESCRIPTION_INDEX_PRESENT);
	SETBYFLAG(root->f->defaults.duration, TFHD_DEFAULT_SAMPLE_DURATION_PRESENT);
	SETBYFLAG(root->f->defaults.size, TFHD_DEFAULT_SAMPLE_SIZE_PRESENT);
	SETBYFLAG(root->f->defaults.settings, TFHD_DEFAULT_SAMPLE_FLAGS_PRESENT);

	if(boxsize > 0) /* if there is something more, it is a UUIDBox */
	{
		int result = parseuuid(root);
		if(result != FRAGMENT_SUCCESS) return result;
		boxsize -= root->size;
	}
	if(boxsize < 0) return FRAGMENT_OUT_OF_BOUNDS; /* should never happen */
	return FRAGMENT_SUCCESS;
}

/**
 * \brief TrunBox (per-sample metadata) parser
 *
 * A Trun Box has no children and a variable number of default settings, whose
 * presence is specified by the TrunBoxFlags bitfield, a 3*BYTE field heading
 * the Box.
 *
 * \param root pointer to the Box structure to be parsed
 * \return     FRAGMENT_SUCCESS on successful parse, or an appropriate error
 *             code.
 * \sa         parsetfhd
 * \sa		   SETBYFLAG
 */
static int parsetrun(Box* root)
{
	signedlenght boxsize = root->size;
	flags boxflags;
	if(!getflags(&boxflags, root)) return FRAGMENT_IO_ERROR;

	count samplecount, i;
	if(!readbox(&samplecount, sizeof(count), root))
		return FRAGMENT_IO_ERROR;
	root->f->sampleno = (count)be32toh(samplecount); /* endian-safe */
	SETBYFLAG(root->f->settings, TRUN_FIRST_SAMPLE_FLAGS_PRESENT);

	if(root->f->sampleno > 0)
	{	SampleFields* tmp = malloc(root->f->sampleno * sizeof(SampleFields));
		for( i = 0; i < root->f->sampleno; i++)  //FIXME e` corretto?? FIXME memory leak!! torna senza liberare
		{
			SETBYFLAG(tmp[i].duration,  TRUN_SAMPLE_DURATION_PRESENT);
			SETBYFLAG(tmp[i].size,		TRUN_SAMPLE_SIZE_PRESENT);
			SETBYFLAG(tmp[i].settings,  TRUN_SAMPLE_FLAGS_PRESENT);
			SETBYFLAG(tmp[i].timeoffset,TRUN_SAMPLE_COMPOSITION_TIME_OFFSET_PRESENT);
		}
		root->f->samples = tmp;
		boxsize -= root->f->sampleno * sizeof(SampleFields);
	}
	if(boxsize > 0) /* if there is something more, it is a UUIDBox */
	{
		int result = parseuuid(root);
		if(result != FRAGMENT_SUCCESS) return result;
		boxsize -= root->size;
	}
	if(boxsize < 0) return FRAGMENT_OUT_OF_BOUNDS; /* should never happen */
	return FRAGMENT_SUCCESS;
}

/**
 * \brief SampleEncryptionBox (content protection metadata) parser
 *
 * A SampleEncryptionBox is a VendorUUIDBox with a particulare uuid, no
 * compulsory children and a few optional fields, whose presence is determined
 * using a flag field.
 *
 * \param root pointer to the Box structure to be parsed
 * \return     FRAGMENT_SUCCESS on successful parse, or an appropriate error
 *             code.
 * \sa         encryptionuuid, isencrbox
 */
static int parseencr(Box* root)
{
	EncryptionType enc;
	signedlenght boxsize = root->size;
	fseek(root->stream, sizeof(encryptionuuid), SEEK_CUR); /* skip signature */ //FIXME controllare che il salto sia ok
	flags boxflags;
	if(!getflags(&boxflags, root)) return FRAGMENT_IO_ERROR;

	if(boxflags & ENCR_SAMPLE_ENCRYPTION_BOX_OPTIONAL_FIELDS_PRESENT)
	{
		if(!readbox(&boxflags, sizeof(flags), root)) return FRAGMENT_IO_ERROR;

///////////////////////////////////////FIXME////////////////////////////////////

		for(enc = NONE, root->f->armor.type = NEW; enc < NEW; enc++) //puo` essere null
			if(boxflags & EncryptionTypeMask[enc]) //anche qui, fare una maschera
			{   root->f->armor.type = enc;  // attenzione all'ultimo byte
				break;
			}
		if(root->f->armor.type == NEW) return FRAGMENT_UNKNOWN_ENCRYPTION;
		boxsize -= sizeof(flags)+sizeof(byte)+sizeof(keyID); 
	}
//	encryptiontype algorithm;
//	byte vectorsize;
//	keyID kID;
//	count vectorno;
//	byte* vectors;
/*
 * |         | [AlgorithmID               |3*UINT8
 * |         |  InitializationVectorSize  |UINT8 = \x8 | \x10
 * |         |  KID]                      |UINT16
 * |         | SampleEncryptionBoxSampleCount | UINT32
 * |         | InitializationVector       |BYTE*
 * FREED
 */
/*	if(boxflags & (mask))								\*/
/*	{   if(!readbox(&(target), sizeof(target), root))   \*/
/*			return FRAGMENT_IO_ERROR;					\*/
/*		boxsize -= sizeof(target);						\*/
/*	}*/
	return FRAGMENT_SUCCESS;
}

/* +---------------------------------------------------------------------------+
 * |'MdatBox': data container                                                  |
 * +---------------------------------------------------------------------------+
 * |Name     | 'mdat'
 * |Fields   | SampleData |BYTE*
 * |         |            | A single Sample of Media. Sample boundaries in 
 * |         |            | the MdatBox are defined by the values of the
 * |         |            | `DefaultSampleSize` and `SampleSize` field in the
 * |         |            | `TrunBox`
 * |Children | none (tutti assieme)
 * +---------+------------------------------------------------------------------
 */
static int parsemdat(Box* root)
{
	byte* tmp = malloc(root->size);
	if(!tmp) return FRAGMENT_NO_MEMORY;
	if(!readbox(tmp, root->size, root))
	{   free(tmp);
		return FRAGMENT_IO_ERROR;
	}
	root->f->data = tmp;
	return FRAGMENT_SUCCESS;
}

///////////////////////////////////////TODO/////////////////////////////////////
// questo must mi preoccupa: devo controllare le dimensioni??
// Ma la dimensione include anche quella dei figli??
// delle UUID facciamo un arrray, e il tipo == il tipo dell'enum
// controllare che la parsetfhd non possa essere semplificata
// scrivere che se fallisce l'analisi, lo stato del fragment potrebbe essere
// indefinito
//FIXME controllare che non ci voglia +1
// sostituire i for sulle enumerazioni con un &. togliere max e mettere < UNKNOWN
// attenzione alla deallocazione dinamica (al top-level)
// inizializzare tutti i campi del Frammento a zero.
// aggiungere le strutture per uuiddata
// * il test per boxsize < 0 fallisce: lenght è uint!!

/* +---------+-----------------------------------------------------------------+
 * |'VendorExtensionUUIDBox': variable content                                 |
 * +---------+-----------------------------------------------------------------+
 * |Name     | 'uuid'
 * |Fields   | UUIDBoxUUID   |BYTE[16]
 * |         |               | uuid signature
 * |         | UUIDBoxData   | *BYTE
 * |         |               | variable content
 * |Children | none
 * +---------+-----------------------------------------------------------------+
 */

static int parseuuid(Box* root)
{
	
}

/* vim: set ts=4 sw=4 tw=0: */
