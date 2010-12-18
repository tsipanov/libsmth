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

#ifndef __SMTH_ERROR_H__
#define __SMTH_ERROR_H__

/**
 * \internal
 * \file   smth-error.h
 * \brief  prints a readable error message for each error code.
 * \author Stefano Sanfilippo
 * \date   14th December 2010
 */
#include <smth-fragment-parser.h>
#include <smth-manifest-parser.h>
#include <smth-http.h>
#include <smth-defs.h>

/**
 * \brief Prints a readable error message for each error code.
 *
 * \param code   The error code
 * \param output The file stream to write to (tipically, \c stderr)
 * \return       The error code itself
 */
error_t SMTH_error(error_t code, FILE *output)
{
	switch (code)
	{
		case FRAGMENT_SUCCESS:
			break;
		case FRAGMENT_IO_ERROR:
			fputs("The parser encountered an i/o error in the "
				"SmoothStream.", output);
			break;
		case FRAGMENT_UNKNOWN:
			fputs("An unknown Box was encountered in a ISMV fragment.", output);
			break;
		case FRAGMENT_NO_MEMORY:
			fputs("No more memory to allocate data sections.", output);
			break;
		case FRAGMENT_PARSE_ERROR:
			fputs("A malformed Box was encountered.", output);
			break;
		case FRAGMENT_INAPPROPRIATE:
			fputs("An element that should not be in the current section was "
				"parsed.", output);
			break;
		case FRAGMENT_OUT_OF_BOUNDS:
			fputs("The fragment is smaller than declared. This often means a "
				"parse error.", output);
			break;
		case FRAGMENT_UNKNOWN_ENCRYPTION:
			fputs("The fragment is encrypted with a new, non implemented, "
				"algorithm.", output);
			break;
		case FRAGMENT_BIGGER_THAN_DECLARED:
			fputs("There are trailing bytes after a MdatBox that will not be "
				"parsed.", output);
			break;
		case MANIFEST_WRONG_VERSION:
			fputs("Wrong Manifest version.", output);
			break;
		case MANIFEST_INAPPROPRIATE_ATTRIBUTE:
			fputs("An out-of-context attribute was parsed.", output);
			break;
		case MANIFEST_NO_MEMORY:
			fputs("There was no memory to istantiate the parser.", output);
			break;
		case MANIFEST_IO_ERROR	:
			fputs("There was an i/o error with the manifest file.", output);
			break;
		case MANIFEST_PARSE_ERROR:
			fputs("The parser encountered a malformed xml manifest.", output);
			break;
		case MANIFEST_EMPTY:
			fputs("The manifest is... ehm... empty?", output);
			break;
		case MANIFEST_PARSER_ERROR	:
			fputs("The xml backend behaved badly and it was blocked.", output);
			break;
		case MANIFEST_UNKNOWN_BLOCK:
			fputs("The manifest parser encountered an unknown element.", output);
			break;
		case MANIFEST_UNEXPECTED_TRAILING:
			fputs("A text block was encountered where it was not expected in the "
				"manifest.", output);
			break;
		case MANIFEST_MALFORMED_ARMOR_UUID:
			fputs("The armor UUID is malformed.", output);
			break;
		case MANIFEST_UNKNOWN_STREAM_TYPE:
			fputs("A stream type different from audio, video and text was "
				"encountered.", output);
			break;
		case MANIFEST_MALFORMED_SUBTYPE:
			fputs("A malformed subtype string was encountered.", output);
			break;
		case MANIFEST_MALFORMED_FOURCC	:
			fputs("A malformed fourcc string was encountered.", output);
			break;
		case MANIFEST_UNEXPECTED_ATTRS	:
			fputs("An attribute block was encountered out of a Track "
				"element.", output);
			break;
		case MANIFEST_INVALID_IDENTIFIER:
			fputs("A string identifier contained characters other than "
				"[A-Za-z/-] .", output);
			break;
		case MANIFEST_MALFORMED_URL:
			fputs("A malformed request URI was encountered.", output);
			break;
		case FETCHER_FAILED_FDSET:
			fputs("Could not set fd_set descriptors for cache.", output);
			break;
		case FETCHER_CONNECTION_TIMEOUT:
			fputs("Connection to server has reached timeout latency.", output);
			break;
		case FETCHER_NO_MULTIPLEX:
			fputs("Could not multiplex fd_set with select().", output);
			break;
		case FECTHER_FAILED_INIT:
			fputs("Could not initialize libcurl backend.", output);
			break;
		case FECTHER_NO_MEMORY:
			fputs("There was no memory enough to instantiate another."
				"handle.", output);
			break;
		case FECTHER_HANDLE_NOT_ADDED:
			fputs("Could not add a Fragment handler to the global."
				"handle", output);
			break;
		case FETCHER_HANDLE_NOT_CLEANED:
			fputs("Could not destroy global fetch handle.", output);
			break;
		case FETCHER_HANDLE_NOT_INITIALISED:
			fputs("Could not properly initialise an handle.", output);
			break;
		case FETCHER_TRANFER_FAILED:
			fputs("Could not transfer the file from the Network.", output);
			break;
		case FETCHER_NOT_CLEANED:
			fputs("Could not destroy a fetcher.", output);
			break;
		case FETCHER_NO_FILE:
			fputs("Could not create a cache file for a chunk.", output);
			break;
		case FECTHER_NO_URL: /* -37 */
			fputs("An appropriate url for chunk retrieval was not "
				"specified.", output);
			break;
		case SMTH_NO_FILE_HANDLE:
			fputs("Could not open a blocking file handle for the Manifest",
				output);
			break;
		case SMTH_NO_MEMORY:
			fputs("No more memory to allocate data", output);
			break;
		default:
			fputs("Unknown error code.", output);
			break;
	}

	return code;
}

#endif /* __SMTH_ERROR_H__ */

/* vim: set ts=4 sw=4 tw=0: */
