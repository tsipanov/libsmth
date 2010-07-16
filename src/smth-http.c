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

#include <smth-http-defs.h>

typedef struct
{
} Fetcher;


// char *mkdtemp(char *template);
// inizializza 10 handlers per connettersi al sito (o il numero che vuoi)
// con 6 urls da uno e 6 dall'altro...
// Una volta che l'handler ha finito, gli cambi l'url...

#define MAX_TRANSFERS 10 /* number of simultaneous transfers */

//una directory per ogni fetcher...

static bool globalinit = false;

error_t SMTH_initfetcher(CURLM **cm) //aggiungi trackno
{
	count_t i;

	if (!globalinit) /* do it only once. */
	{   curl_global_init(CURL_GLOBAL_ALL); //rendere statico
		globalinit = true;
	}

	CURLM *multi = curl_multi_init();

	/* we can optionally limit the total amount of connections this multi handle uses */
	curl_multi_setopt(multi, CURLMOPT_MAXCONNECTS, (long) MAX_TRANSFERS);

	for (i = 0; i < MAX_TRANSFERS; i++)
	{
		CURL *eh = curl_easy_init(); //check

		//int mkstemp(char *template) //CURLE_OK
		char fname[36];
		snprintf(fname, 36, "/home/Sanfi/Scrivania/test%d.all", i);
		FILE* suca = fopen(fname, "w");

		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, NULL);
		curl_easy_setopt(eh, CURLOPT_WRITEDATA, suca);
		curl_easy_setopt(eh, CURLOPT_HEADER, 0L);
		curl_easy_setopt(eh, CURLOPT_URL, "http://localhost:631"); //sempre lo stesso handle...
		curl_easy_setopt(eh, CURLOPT_PRIVATE, suca); //private data <- inserire con il file+timestamp+stream nella struct sopra...
		curl_easy_setopt(eh, CURLOPT_VERBOSE, 0L); //controllare errori...
		/* some servers don't like requests that are made without
		 * a user-agent field, so we provide one */
		curl_easy_setopt(eh, CURLOPT_USERAGENT, "foo/bar");
		/* with old versions of libcurl: no progress meter */
		curl_easy_setopt(eh, CURLOPT_NOPROGRESS, 1L);

		curl_multi_add_handle(multi, eh);
	}

	*cm = multi;

	return FETCHER_SUCCESS;
}

void SMTH_disposefetcher(CURLM *cm)
{	curl_multi_cleanup(cm);
	curl_global_cleanup();
}

#if 0
error_t SMTH_fetchmanifest();
error_t SMTH_fetchfragment();

error_t compilemanifesturl
error_t compilechunkurl

/* check for average download speed */
curl_easy_getinfo(curl_handle, CURLINFO_SPEED_DOWNLOAD, &val); //bytes/secondo double
#endif


















/* vim: set ts=4 sw=4 tw=0: */
