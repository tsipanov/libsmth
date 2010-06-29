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
 * \brief  Binary Fragments parser
 * \author Stefano Sanfilippo
 * \date   27th June 2010
 */

#include <stdlib.h>
#include <stdio.h>
#include <endian.h>
#include <string.h>
#include <smth-fragment-defs.h>

/**
 * \brief        Parses a fragment opened as SmoothStream and fills all the 
 *               details in a Fragment structure.
 * \param stream pointer to the stream from which to read the fragment.
 * \param f      pointer to the Fragment structure to be filled with data
 *               extracted from the stream.
 * \return       FRAGMENT_SUCCESS on successful parse, or an appropriate error
 *               code.
 */
error_t parsefragment(SmoothStream *stream, Fragment *f)
{   Box root;
	error_t result;
	root.stream = stream;
	root.f = f;

	result = parsebox(&root);
	if (result != FRAGMENT_SUCCESS) return result;
	result = parsemoof(&root);
	if (result != FRAGMENT_SUCCESS) return result;
	result = parsebox(&root);
	if (result != FRAGMENT_SUCCESS) return result;
	result = parsemdat(&root);
	if (result != FRAGMENT_SUCCESS) return result;
	if (!feof(stream)) return FRAGMENT_BIGGER_THAN_DECLARED;
	return FRAGMENT_SUCCESS;
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
	/* destroy even the reference */
	f->data = NULL;
	f->samples = NULL;
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
 * Any block may contain one or more `VendorExtensionUUIDBox`, as described later.
 *
 */

/*------------------------- HIC SUNT LEONES (CODICIS) ------------------------*/

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
	if ((fread(dest, sizeof (byte_t), size, root->stream) < (size*sizeof (byte_t))) &&
	   (feof(root->stream) || ferror(root->stream)))
		return false;
	return true;
}

/**
 * \brief             Get flags & version field from the stream
 * \param defultflags Pointer to the buffer that will hold the flags
 * \return            true if the operation was successfull, otherwise false
 */
static bool getflags(flags_t *defaultflags, Box *root)
{	if (!readbox(defaultflags, sizeof (flags_t), root)) return false;
	*defaultflags = (flags_t) be32toh(*defaultflags); /* endian-safe */
	return true;
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
static error_t parsebox(Box* root)
{
	lenght_t tmpsize;
	word_t name;
	BoxType element;
	shortlenght_t offset = sizeof (shortlenght_t) + sizeof (name);

	if (!readbox(&tmpsize, sizeof (shortlenght_t), root)) return FRAGMENT_IO_ERROR;
	if (!readbox(&name, sizeof (name), root)) return FRAGMENT_IO_ERROR;
	for (element = 0, root->type = UNKNOWN; element < UNKNOWN; element++)
		if (name == BoxTypeMask[element]) //FIXME endianess
		{   root->type = element;
			break;
		}
	/* if it is still unknown */
	if (root->type == UNKNOWN) return FRAGMENT_UNKNOWN;
	if (tmpsize == boxishuge)
	{
		if (!readbox(&root->size, sizeof (lenght_t), root)) return FRAGMENT_IO_ERROR;
		offset += sizeof (lenght_t);
		root->size = (lenght_t) be64toh(root->size);
	}
	else root->size = (lenght_t) be32toh(tmpsize);

	root->size -= offset;
	fprintf(stderr, "size: 0x%04lx\n", root->size); //DEBUG
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

/* TODO do they have to appear in a fixed order? It would be much simpler... */

static error_t parsemoof(Box* root)
{
	int i;
	error_t result;
	signedlenght_t boxsize = (signedlenght_t) root->size;

	for ( i = 0; i < 2; i++)
	{   
		result = parsebox(root);
		if (result == FRAGMENT_SUCCESS)
		{	switch (root->type)
			{	case MFHD: result = parsemfhd(root); break;
				case TRAF: result = parsetraf(root); break;
				case SDTP: result = parsesdtp(root); break; //DEBUG
				default: return FRAGMENT_INAPPROPRIATE;
			}
			if (result != FRAGMENT_SUCCESS) return result;
			boxsize -= root->size; /* size of the newly parsed Box */
		}
		else return result;
	}

	LOOK_FOR_UUIDBOXES_AND_RETURN;
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

static error_t parsemfhd(Box* root)
{
	signedlenght_t boxsize = (signedlenght_t) root->size;
	count_t tmp;

	XXX_SKIP_TRAILING_QUIRK;

	if (!readbox(&tmp, sizeof (tmp), root)) return FRAGMENT_IO_ERROR;
	root->f->ordinal = (count_t) be32toh(tmp);
	boxsize -= sizeof (tmp);

	LOOK_FOR_UUIDBOXES_AND_RETURN;
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
static error_t parsetraf(Box* root)
{
	int i;
	error_t result;
	signedlenght_t boxsize = (signedlenght_t) root->size;

	for ( i = 0; i < 2; i++)
	{   
		result = parsebox(root);
		if (result == FRAGMENT_SUCCESS)
		{	switch (root->type)
			{	case TFHD: result = parsetfhd(root); break;
				case TRUN: result = parsetrun(root); break;
				default: return FRAGMENT_INAPPROPRIATE;
			}
			if (result != FRAGMENT_SUCCESS) return result;
			boxsize -= root->size; /* size of the newly parsed Box */
		}
		else return result;
	}

	LOOK_FOR_UUIDBOXES_AND_RETURN;
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
static error_t parsetfhd(Box* root)
{
	signedlenght_t boxsize = (signedlenght_t) root->size;
	flags_t boxflags;
	if (!getflags(&boxflags, root)) return FRAGMENT_IO_ERROR;
	boxsize -= sizeof (boxflags);

	uint64_t doubleword = 0;
	uint32_t singleword = 0;

	GET_IF_FLAG_SET(doubleword, TFHD_BASE_DATA_OFFSET_PRESENT);
	root->f->defaults.dataoffset = (offset_t) be64toh(doubleword);

	GET_IF_FLAG_SET(singleword, TFHD_SAMPLE_DESCRIPTION_INDEX_PRESENT);
	root->f->defaults.index = (count_t) be32toh(singleword);

	GET_IF_FLAG_SET(doubleword, TFHD_DEFAULT_SAMPLE_DURATION_PRESENT);
	root->f->defaults.duration = (tick_t) be64toh(doubleword);

	GET_IF_FLAG_SET(singleword, TFHD_DEFAULT_SAMPLE_SIZE_PRESENT);
	root->f->defaults.size = (bitrate_t) be32toh(singleword);

	GET_IF_FLAG_SET(singleword, TFHD_DEFAULT_SAMPLE_FLAGS_PRESENT);
	root->f->defaults.settings = (flags_t) be32toh(singleword);

	XXX_SKIP_TRAILING_QUIRK;

	LOOK_FOR_UUIDBOXES_AND_RETURN;
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
static error_t parsetrun(Box* root)
{
	signedlenght_t boxsize = (signedlenght_t) root->size;
	flags_t boxflags;
	if (!getflags(&boxflags, root)) return FRAGMENT_IO_ERROR;

	count_t samplecount;
	if (!readbox(&samplecount, sizeof (count_t), root))
		return FRAGMENT_IO_ERROR;
	root->f->sampleno = (count_t) be32toh(samplecount); /* endian-safe */
	GET_IF_FLAG_SET(root->f->settings, TRUN_FIRST_SAMPLE_FLAGS_PRESENT);

	if(root->f->sampleno > 0)
	{	
		int i;
		uint32_t singleword;
		SampleFields* tmp = malloc(root->f->sampleno * sizeof (SampleFields));
		if(!tmp) return FRAGMENT_NO_MEMORY;
		for( i = 0; i < root->f->sampleno; i++)
		{
			GET_IF_FLAG_SET(singleword, TRUN_SAMPLE_DURATION_PRESENT);
			tmp[i].duration = (bitrate_t) be32toh(singleword);
			GET_IF_FLAG_SET(singleword, TRUN_SAMPLE_SIZE_PRESENT);
			tmp[i].size = (bitrate_t) be32toh(singleword);
			GET_IF_FLAG_SET(singleword, TRUN_SAMPLE_FLAGS_PRESENT);
			tmp[i].settings = (flags_t) singleword; /* this is not to be converted */
			GET_IF_FLAG_SET(singleword, TRUN_SAMPLE_COMPOSITION_TIME_OFFSET_PRESENT);
			tmp[i].timeoffset = (bitrate_t) be32toh(singleword);
		}
		root->f->samples = tmp;
	}

	LOOK_FOR_UUIDBOXES_AND_RETURN;
}

/**
 * \brief MdatBox (data container) parser
 *
 * A MdatBox has no children (not even a UUIDBox) and no fields: simply reads
 * it in Fragment::data. There may be only one per fragment, so we do not
 * worry about data concatenation.
 * Sample boundaries in the MdatBox are defined by the values of the
 * TrunBox::DefaultSampleSize and TrunBox::SampleSize fields. Individual sample
 * sizes are stored into SampleFields::size and the overall number of samples
 * in Fragment::sampleno.
 *
 * \param root pointer to the Box structure to be parsed
 * \return     FRAGMENT_SUCCESS on successful parse, or an appropriate error
 *             code.
 */
static error_t parsemdat(Box* root)
{
	byte_t *tmp = malloc(root->size);
	if (!tmp) return FRAGMENT_NO_MEMORY;
	if (!readbox(tmp, root->size, root))
	{   free(tmp);
		return FRAGMENT_IO_ERROR;
	}
	root->f->data = tmp;
	root->f->size = root->size;
	return FRAGMENT_SUCCESS;
}

/**
 * \brief SampleEncryptionBox (content protection metadata) parser
 *
 * A SampleEncryptionBox is a particularly crafted VendorUUIDBox, no
 * compulsory children and a few optional fields, whose presence is determined
 * using a flag field. It assumes that the signature has already been stripped
 * by a call to parseuuid.
 *
 * \param root pointer to the Box structure to be parsed
 * \return     FRAGMENT_SUCCESS on successful parse, or an appropriate error
 *             code.
 * \sa         encryptionuuid, parseuuid
 */
static error_t parseencr(Box* root)
{
	EncryptionType enc;
	signedlenght_t boxsize = (signedlenght_t) root->size;
	flags_t boxflags; /* first used to retrieve box flags, then crypt flags */
	if (!getflags(&boxflags, root)) return FRAGMENT_IO_ERROR;

	if (boxflags & ENCR_SAMPLE_ENCRYPTION_BOX_OPTIONAL_FIELDS_PRESENT)
	{
		if(!readbox(&boxflags, sizeof (flags_t), root)) return FRAGMENT_IO_ERROR;
		boxflags = (flags_t) be32toh(boxflags); /* endian-safe */
		if(boxflags & ENCRYPTION_KEY_TYPE_MASK) /* if it is encrypted */
		{	for (enc = AES_CTR, root->f->armor.type = UNSET; enc < UNSET; enc++)
				if (boxflags & EncryptionTypeMask[enc])
				{   root->f->armor.type = enc;
					break;
				}
			if (root->f->armor.type == UNSET) return FRAGMENT_UNKNOWN_ENCRYPTION;
		}
		else root->f->armor.type = NONE;

		root->f->armor.vectorsize = (byte_t)(boxflags & ENCRYPTION_KEY_SIZE_MASK);
		
		if (!readbox(&root->f->armor.id, sizeof(uuid_t), root))
			return FRAGMENT_IO_ERROR;
		/* WARNING: if you change type size, it will break!! */
		boxsize -= sizeof (flags_t) + sizeof (byte_t) + sizeof (uuid_t); 
	}
	else
	{
		root->f->armor.type = UNSET;
		memset(root->f->armor.id, 0, sizeof(uuid_t));
		root->f->armor.vectorsize = (byte_t) 0;
		root->f->armor.vectors    = NULL; // no vectors if no vectorsize?
	}

	if (!readbox(&root->f->armor.vectorno, sizeof(count_t), root))
		return FRAGMENT_IO_ERROR;

	lenght_t vectorlenght = root->f->armor.vectorsize * root->f->armor.vectorno;
	byte_t *tmp = malloc(vectorlenght);
	if (!tmp) return FRAGMENT_NO_MEMORY;
	if (!readbox(tmp, vectorlenght, root))
	{   free(tmp);
		return FRAGMENT_IO_ERROR;
	}
	boxsize -= vectorlenght;

	LOOK_FOR_UUIDBOXES_AND_RETURN;
}

/**
 * \brief VendorExtensionUUIDBox (variable content) box.
 *
 * Parses a 16byte UUID and a bytestream content to a Variable structure, and
 * adds it to the list dereferenced by Fragment::extensions. If the Box is a 
 * SampleEncryptionBox, it calls parseencr.
 *
 * \param root pointer to the Box structure to be parsed
 * \return     FRAGMENT_SUCCESS on successful parse, or an appropriate error
 *             code.
 * \sa         parseencr
 */
///////////////////////////////////////TODO/////////////////////////////////////
static error_t parseuuid(Box* root)
{
	uuid_t uuid;
	error_t result;
	fprintf(stderr, "Mi hai chiamato?\n"); //DEBUG
	if (!readbox(uuid, sizeof (uuid), root)) return FRAGMENT_IO_ERROR;
	if (!memcmp(uuid, encryptionuuid, sizeof (uuid_t)))
		result = parseencr(root);
/* UUIDBoxUUID | UUIDBoxData  *
 * BYTE[16]    | *BYTE        */
// aggiungere un campo type.
// fseek(root->stream, sizeof(word), SEEK_CUR);
/* skip signature */
// fseek(root->stream, sizeof(word)-1, SEEK_CUR); boxsize -= sizeof(word); //HORRIBLETEST
	return FRAGMENT_SUCCESS;
}

/**
 * \brief   Unknown Box: skip it... So sad :(
 */
static error_t parsesdtp(Box* root)
{
	fseek(root->stream, root->size, SEEK_CUR);
	return FRAGMENT_SUCCESS;
}

/* vim: set ts=4 sw=4 tw=0: */
