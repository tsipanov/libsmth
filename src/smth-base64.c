/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-base64.c: base64 decoder.
 *
 * 13th July 2010
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

/*
 * \internal
 * \file smth-base64.c
 * \brief Base64 decoder, for embedded data.
 * \date 13th July 2010
 * \author Stefano Sanfilippo
 *
 * Inspired from:
 * http://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
 */

#include <string.h>
#include <stdbool.h>
#include <smth-common-defs.h>

#define BASE64_INVALID_LENGTH (-25)
#define BASE64_SUCCESS        (0)

/** base64 symbols set */
static const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * \brief Tests if a \c char is a valid base64 symbol (i.e. is among base64chars)
 *
 * \param c the character to be tested.
 * \return \c true if the test was successfull, otherwise false.
 * \sa base64chars
 */
static inline bool isbase64(char c)
{	return c && strchr(base64chars, c); 
}

/** \brief Gets the value of a base64 symbol */
static inline char value(char c)
{	const char *p = strchr(base64chars, c);
	return p? p - base64chars: 0;
}

/**
 * \brief Decodes a base64 string to \c dest.
 * \bug This function is untested with EBCDIC encoded strings.
 * \param dest 
 */
error_t unbase64(unsigned char *dest, const unsigned char *src, int srclen)
{
	*dest = 0; //XXX
	if (*src == 0) return 0;

	if (srclen % 4) return BASE64_INVALID_LENGTH;
	srclen++; //FIXME

	unsigned char *p = dest;

	while (srclen -= 4)
	{
		char a = value(src[0]);
		char b = value(src[1]);
		char c = value(src[2]);
		char d = value(src[3]);
		*p++ = (a << 2) | (b >> 4);
		*p++ = (b << 4) | (c >> 2);
		*p++ = (c << 6) | d;
		if (!isbase64(src[1])) 
		{	p -= 2; break;
		} 
		if (!isbase64(src[2])) 
		{	p -= 2; break;
 		} 
		if (!isbase64(src[3])) 
		{	p--; break;
		}

  		src += 4;

  		while (*src && (*src == '\n' || *src == '\r')) src++;
	}

	*p = 0;

	//return p - dest;
	return BASE64_SUCCESS;
}

/* vim: set ts=4 sw=4 tw=0: */
