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
 * \brief  XML manifest parser
 * \author Stefano Sanfilippo
 * \date   30th June 2010
 */

#include <expat.h>

void XMLCALL startblock(void *data, const char *el, const char **attr) //alternati nome valore
{
  int i;

  for (i = 0; i < Depth; i++)
    printf("  ");

  printf("%s", el);

  for (i = 0; attr[i]; i += 2) {
    printf(" %s='%s'", attr[i], attr[i + 1]);
  }

  printf("\n");
  Depth++;
}

void XMLCALL endblock(void *data, const char *el)
{
	Depth--;
}

void XMLCALL textblock(void *data, const char *text, int lenght)
{
  printf("\n%4d: Text - ", Eventcnt++);
  fwrite(txt, txtlen, sizeof(char), stdout);
}

/* vim: set ts=4 sw=4 tw=0: */
