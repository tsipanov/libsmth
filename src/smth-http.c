/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-http.c : web transfer glue
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
 * \file   smth-http.c
 * \brief  Web transfer glue.
 * \author Stefano Sanfilippo
 * \date   12th June 2010
 */

#include <curl/multi.h>
#include <smth-http-defs.h>

typedef struct
{
} Fetcher;

#if 0
//       char *mkdtemp(char *template);


static void init(CURLM *cm)
{
	CURL *eh = curl_easy_init(); //check

	//int mkstemp(char *template) //CURLE_OK

	curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt(eh, CURLOPT_WRITEDATA, FILE*);
	curl_easy_setopt(eh, CURLOPT_HEADER, 0L);
	curl_easy_setopt(eh, CURLOPT_URL, "http://localhost"); //sempre lo stesso handle...
	curl_easy_setopt(eh, CURLOPT_PRIVATE, FILE*); //private data <- inserire con il timestamp nella struct sopra...
	curl_easy_setopt(eh, CURLOPT_VERBOSE, 0L);
	/* some servers don't like requests that are made without
	 * a user-agent field, so we provide one */
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, useragent);

	curl_multi_add_handle(cm, eh);
}

error_t SMTH_initfetcher(Fetcher *context);
void SMTH_destroyfetcher(Fetcher *context);
error_t SMTH_fetchmanifest();
error_t SMTH_fetchfragment();

error_t compilemanifesturl
error_t compilechunkurl

/* check for average download speed */
curl_easy_getinfo(curl_handle, CURLINFO_SPEED_DOWNLOAD, &val); //bytes/secondo double
/* no progress meter please */
curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

//Una volta che l'handler ha finito, gli cambi l'url...
#endif


















/* vim: set ts=4 sw=4 tw=0: */
