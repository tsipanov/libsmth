/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-dump.h: verbosely dump Manifests and Fragments (header)
 *
 * 12th July 2010
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

#ifndef __SMTH_DUMP_H__
#define __SMTH_DUMP_H__

/**
 * \file   smth-dump.h
 * \brief  Manifest and Fragment dump (header)
 * \author Stefano Sanfilippo
 * \date   12th July 2010
 */

#include <stdio.h>
#include <smth-manifest-parser.h>
#include <smth-fragment-parser.h>

void SMTH_dumpmanifest(Manifest  *m, FILE *output);
void SMTH_dumpfragment(Fragment *vc, FILE *output);
void SMTH_dumppayload(Fragment* vc, char* directory);

#endif /* __SMTH_DUMP_H__ */

/* vim: set ts=4 sw=4 tw=0: */
