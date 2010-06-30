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
 * \file   smth-manifest-parser.h
 * \brief  XML manifest parser (public header)
 * \author Stefano Sanfilippo
 * \date   30th June 2010
 */

#ifndef __SMTH_MANIFEST_PARSER_H__
#define __SMTH_MANIFEST_PARSER_H__

typedef struct
{
	/** The duration of the content, measured in ticks, as indicated by the
	 *  value of the Manifest::tick field.
	 */
	tick_t duration;
	/** The time scale of the Manifest::duration attribute, specified as the
	 *  number of increments per second. The default value is 10000000.
	 */
	tick_t tick;
	/** Whether the presentation is a live streaming or an on-demand. */
	bool islive;
	/** Size of the server buffer, in number of fragments.
	 *  This field is set to 0 for on-demand content.
	 */
	count_t lookahead;
	/** The length of the DVR window, measured in ticks. If this field is
	 *  omitted for a live presentation or set to 0, the DVR window is
	 *  effectively infinite.
	 *  This field MUST be omitted for on-demand presentations.
	 */
	lenght_t dvrwindow;
	/** A UUID that uniquely identifies the Content Protection System to
	 *  which this ProtectionElement pertains.
	 *
	 *  For instance: {9A04F079-9840-4286-AB92E65BE0885F95}
	 */
	uuid_t armorID;
	/** Opaque data that the Content Protection System identified in the
	 *  Manifest::armorid field can use to enable playback for authorized
	 *  users, encoded using Base64.
	 */
	base64data *armor;
} Manifest;

#endif /* __SMTH_MANIFEST_PARSER_H__ */

/* vim: set ts=4 sw=4 tw=0: */
