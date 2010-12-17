/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth.h: public API
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

#ifndef __SMTH_H__
#define __SMTH_H__

/**
 * \file   smth.h
 * \brief  libsmth public API
 * \author Stefano Sanfilippo
 * \date   14th December 2010
 */

/* Opaque pointer, just to hide any tentation */
typedef void *SMTHh;

SMTHh SMTH_open(const char *url, const char *params);
void SMTH_close(SMTHh handle);

#endif /* __SMTH_H__ */

/* vim: set ts=4 sw=4 tw=0: */
