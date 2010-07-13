/*
 * \internal
 * \file base64.c
 * \date 13th July 2010
 * \author Stefano Sanfilippo
 *
 * Inspired from:
 * http://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
 */

#include <string.h>
#include <stdbool.h>

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
 *
 * \param dest 
 */
int unbase64(unsigned char *dest, const unsigned char *src, int srclen)
{
	*dest = 0;
	if (*src == 0) return 0;

	if (srclen % 4 - 1) puts("error"); //FIXME

	unsigned char *p = dest;

	while (srclen-= 4)
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

  		while (*src && (*src == 13 || *src == 10)) src++;
	}

	*p = 0;

	return p - dest;
}

////////////////////////////////////////////////////////////////////////////////

static const char decoded[] = "Man is distinguished, not only by his reason, \
but by this singular passion from other animals, which is a lust of the mind, \
that by a perseverance of delight in the continued and indefatigable generation \
of knowledge, exceeds the short vehemence of any carnal pleasure.";

static const char encoded[] = "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhp\
cyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB\
3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaW\
dodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZ\
GdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";

int main()
{	char comp[strlen(encoded)+10];
	unbase64(comp, encoded, strlen(encoded)+1); //attenzione...
	if (strcmp(comp, decoded) == 0) puts("ok");
}
