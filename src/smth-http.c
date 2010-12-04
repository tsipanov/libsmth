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

////////////////////////////////////////////////////////////////////////////////
// char *mkdtemp(char *template);
// inizializza 10 handlers per connettersi al sito (o il numero che vuoi)
// con 6 urls da uno e 6 dall'altro...
// Una volta che l'handler ha finito, gli cambi l'url...
// una directory per ogni fetcher...
// per sicurezza, usiamo sempre e solo tmpfile....
// aggiungi trackno
// private data <- inserire con il file+timestamp+stream nella struct sopra...
// tmpfile();
//1. scarica il Manifest
//2. scopri quanto dura un frammento
//3. fai buffer a sufficienza
//4. scarica continuamente audio e video
//5. apri un folder temporaneo
static bool reinithandle(Fetcher *f)
{   char fname[36];
/*	snprintf(fname, 36, "/home/Sanfi/Scrivania/test%d.all", i);*/
/*	FILE* test = fopen(fname, "w"); //e se si usasse mkstemp? tmpfile(); //*/
/*	curl_easy_setopt(eh, CURLOPT_URL, "http://localhost:631"); //sempre lo stesso handle...*/
/*	curl_easy_setopt(eh, CURLOPT_WRITEDATA, suca);*/
	return true;
}

#if 0
error_t SMTH_fetchmanifest();
error_t SMTH_fetchfragment();

error_t compilemanifesturl
error_t compilechunkurl

/* check for average download speed */
curl_easy_getinfo(curl_handle, CURLINFO_SPEED_DOWNLOAD, &val); //bytes/secondo double
#endif

////////////////////////////////////////////////////////////////////////////////

/** The number of opened handles. */
static count_t handles = 0;

error_t SMTH_initfetcher(Fetcher *f)
{
	count_t i;

	if (!handles) /* do it only once. */
	{   if (curl_global_init(CURL_GLOBAL_ALL)) return FECTHER_FAILED_INIT;
		handles++;
	}

	f->handle = curl_multi_init();
	if (!f->handle) return FECTHER_NO_MEMORY;

	/* limit the total amount of connections this multi handle uses */
	curl_multi_setopt(f->handle, CURLMOPT_MAXCONNECTS, FETCHER_MAX_TRANSFERS);

	for (i = 0; i < FETCHER_MAX_TRANSFERS; i++)
	{
		CURL *eh = curl_easy_init();
		if (!eh) return FECTHER_NO_MEMORY;

		if (!reinithandle(NULL)) return FETCHER_HANDLE_NOT_INITIALISED;

		/* Use the default write function */
		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, NULL);
		/* Some servers don't like requests without a user-agent field... */
		curl_easy_setopt(eh, CURLOPT_USERAGENT, FETCHER_USERAGENT);
		/* No headers written, only body. */
		curl_easy_setopt(eh, CURLOPT_HEADER, 0L);
		/* No verbose messages. */
		curl_easy_setopt(eh, CURLOPT_VERBOSE, 0L);
		/* with old versions of libcurl: no progress meter */
		curl_easy_setopt(eh, CURLOPT_NOPROGRESS, 1L);

		if (curl_multi_add_handle(f->handle, eh))
		{   curl_easy_cleanup(eh);
			return FECTHER_HANDLE_NOT_ADDED;
		}
	}

	return FETCHER_SUCCESS;
}

error_t SMTH_disposefetcher(Fetcher *f)
{	if (curl_multi_cleanup(f->handle)) return FETCHER_HANDLE_NOT_CLEANED;
	handles--;
	if (!handles) curl_global_cleanup();
}

/* vim: set ts=4 sw=4 tw=0: */
