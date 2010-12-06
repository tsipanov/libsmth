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

/**
 * \brief Fetch all the fragments referred by a \c Manifest struct.
 *
 * The \c Manifest may be obtained via \c SMTH_fetchmanifest or directly parsed
 * from local media.
 *
 * \param m The \c Manifest from which to fetch fragments.
 * \return  FETCHER_SUCCESS on successfull operation, or an appropriate error code.
 */
error_t SMTH_fetch(Manifest *m)
{
	Fetcher f;
	int running_no = -1;
	error_t error;

	error = initfetcher(&f, m);
	if (error) return error;

	while (running_no)
	{
		/* Submit all transfers... */
		while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(f.handle, &running_no));

		if (running_no)
		{   error = resetfetcher(&f);
			if (error) goto end; //aumenta il numero di handlers...
		}

		error = execfetcher(&f);
		if (error) goto end;
	}

	error = FETCHER_SUCCESS; /* assigned only if everything went fine. */

end:
	disposefetcher(&f);
	return error;
}

/*------------------------- HIC SUNT LEONES (CODICIS) ------------------------*/

/** The number of opened handles. */
static count_t handles = 0;

/**
 * \brief Runs a fetcher and wait until all the transfers are over.
 *
 * \param f The \c Fetcher to execute.
 * \return  FETCHER_SUCCESS or an appropriate error code.
 */
static error_t execfetcher(Fetcher *f)
{
	CURLMsg *msg;
	int queue;

	while ((msg = curl_multi_info_read(f->handle, &queue)))
	{
		if (msg->msg == CURLMSG_DONE)
		{
			curl_multi_remove_handle(f->handle, msg->easy_handle);
			curl_easy_cleanup(msg->easy_handle);
		}
		else return FETCHER_TRANFER_FAILED;
	}

	return FETCHER_SUCCESS;
}

/**
 * \brief Properly initialises a \c Fetcher before use.
 *
 * \param f Pointer to the fetcher structure to be initialised.
 * \param m Pointer to the manifest from which to compile the Fetcher.
 * \return  FETCHER_SUCCESS or an appropriate error code.
 */
static error_t initfetcher(Fetcher *f, Manifest *m)
{
	count_t i;

	if (!handles && curl_global_init(CURL_GLOBAL_ALL))
		return FECTHER_FAILED_INIT; /* do it only once. */

	f->handle = curl_multi_init();
	if (!f->handle) return FECTHER_NO_MEMORY;

	/* limit the total amount of connections this multi handle uses */
	curl_multi_setopt(f->handle, CURLMOPT_MAXCONNECTS, FETCHER_MAX_TRANSFERS);

	for (i = 0; i < 27; ++i) //FIXME < FETCHER_MAX_TRANSFERS
	{
		CURL *eh = curl_easy_init();
		if (!eh) return FECTHER_NO_MEMORY;

		/* The file to cache to */ //FIXME
		FILE *output = fopen("test.html", "a"); 
		/* Set the url from which to retrieve the chunk */
		curl_easy_setopt(eh, CURLOPT_URL, "http://localhost:631");

		/* Write to the provided file handler */
		curl_easy_setopt(eh, CURLOPT_WRITEDATA, output);
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

	++handles;
	return FETCHER_SUCCESS;
}

/**
 * \brief Properly disposes of a \c Fetcher.
 *
 * \param f The fetcher to be disposed.
 * \return  FETCHER_SUCCESS or FETCHER_HANDLE_NOT_CLEANED if something bad happened.
 */
static error_t disposefetcher(Fetcher *f)
{
	if (f->handle && curl_multi_cleanup(f->handle))
		return FETCHER_HANDLE_NOT_CLEANED;

	--handles;
	if (!handles) curl_global_cleanup();

	return FETCHER_SUCCESS;
}

/**
 * \brief Resets \c Fetcher internals.
 *
 * \param f The fetcher to be resetted.
 * \return  FETCHER_SUCCESS or an appropriate error code.
 */
static error_t resetfetcher(Fetcher *f)
{
	long sleep_time;
	int max_fd;
	fd_set read, write, except;
	struct timeval timeout;

	FD_ZERO(&read); FD_ZERO(&write); FD_ZERO(&except);

	if (curl_multi_fdset(f->handle, &read, &write, &except, &max_fd))
		return FETCHER_FAILED_FDSET;

	if (curl_multi_timeout(f->handle, &sleep_time))
		return FETCHER_CONNECTION_TIMEOUT;

	if (sleep_time == -1) sleep_time = 100;

	if (max_fd == -1)
	{	sleep(sleep_time / 1000); /* on MS Windows, Sleep(sleep_time); */
	}
	else
	{	timeout.tv_sec = sleep_time/1000;
		timeout.tv_usec = (sleep_time%1000)*1000;

		if (0 > select(max_fd+1, &read, &write, &except, &timeout))
			return FETCHER_NO_MULTIPLEX;
	}

	return FETCHER_SUCCESS;
}

/* vim: set ts=4 sw=4 tw=0: */
