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
 * \date   12th June 2010 ~ 7-11th Dicember 2010
 */

#include <string.h>
#include <stdlib.h>

#include <curl/curl.h>

#include <smth-http-defs.h>

/**
 * \brief Fetch all the fragments referred by a \c Manifest::Stream field.
 *
 * The \c Manifest may be obtained via \c SMTH_fetchmanifest or directly parsed
 * from local media.
 *
 * \param url     The url from which retrieve the files.
 * \param stream  The \c Stream from which to fetch fragments.
 * \param dirpath Pointer to the buffer to be filled with the location of a
 *                temporary directory where files where downloaded.
 *
 * \warning       The returned string buffer MUST be freed.
 * \return        Pointer to path variable, or NULL
 */
char* SMTH_fetch(const char *url, Stream *stream, bitrate_t maxbitrate)
{
	Fetcher f;
	int queue, running_no = -1;
	error_t error;
	CURLMsg *msg;

	if (!stream) return NULL;

	if (stream->isembedded) return unembed(stream);

	if (!url) return NULL;

	f.maxbitrate = maxbitrate;
	f.stream = stream;
	f.urlmodel = malloc(snprintf(NULL, 0, "%s/%s", url, stream->url));
	sprintf(f.urlmodel, "%s/%s", url, stream->url);

	error = initfetcher(&f);
	if (error) return NULL; //error;

	while (running_no)
	{
		/* Submit all transfers... */
		while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(f.handle, &running_no));

		if (running_no)
		{   error = resetfetcher(&f);
			if (error) goto end;
		}

		while ((msg = curl_multi_info_read(f.handle, &queue)))
		{
			if (msg->msg == CURLMSG_DONE)
			{
				FILE *file;
				double time = 0.;

				curl_easy_getinfo(msg->easy_handle, CURLINFO_SPEED_DOWNLOAD, &time);
				f.downloadtime = (bitrate_t)(sizeof (byte_t) * time);
				curl_easy_getinfo(msg->easy_handle, CURLOPT_PRIVATE, &file);
/*				fclose(file); XXX*/

				curl_multi_remove_handle(f.handle, msg->easy_handle);
				curl_easy_cleanup(msg->easy_handle);

				reinithandle(&f);
			}
			else
			{   error = FETCHER_TRANFER_FAILED;
				goto end;
			}
		}
	}

	error = FETCHER_SUCCESS; /* assigned only if everything went fine. */

end:
	disposefetcher(&f);
	if (error) return NULL;

	return f.cachedir;
}

/**
 * \brief Fetches the manifest from a given url
 *
 * \param url    The url from which retrieve a manifest
 * \param params Any param necessary to invoke the url.
 * \return       A pointer to the manifest stream, or NULL
 */
FILE* SMTH_fetchmanifest(const char *url, const char *params)
{
	CURL *handle;
	CURLcode error;
	char filename[] = FETCHER_MANIFEST_TEMPLATE;
	char manifesturl[FETCHER_MAX_URL_LENGTH];

	snprintf(manifesturl, FETCHER_MAX_FILENAME_LENGTH,  "%s/Manifest%c%s",
		url, (params? '?': 0), params);

	/* Open a temporary file */
	FILE *output = tmpfile();

	/* Build downloader */
	if (!(handle = curl_easy_init())) return NULL;

	/* Set the url from which to retrieve the chunk */
	curl_easy_setopt(handle, CURLOPT_URL, manifesturl);
	/* Write to the provided file handler */
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, output);
	/* Use the default write function */
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, NULL);
	/* Some servers don't like requests without a user-agent field... */
	curl_easy_setopt(handle, CURLOPT_USERAGENT, FETCHER_USERAGENT);
	/* No headers written, only body. */
	curl_easy_setopt(handle, CURLOPT_HEADER, 0L);
	/* No verbose messages. */
	curl_easy_setopt(handle, CURLOPT_VERBOSE, 0L);
	/* with old versions of libcurl: no progress meter */
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);

	if (curl_easy_perform(handle))
	{
		curl_easy_cleanup(handle);
		close(output);
		return NULL;
	}

	curl_easy_cleanup(handle);

	/* Rewind the output stream */
	rewind(output);
	return output;
}

/*------------------------- HIC SUNT LEONES (CODICIS) ------------------------*/

/** The number of opened handles. */
static count_t handles = 0;

/**
 * \brief Properly initialises a \c Fetcher before use.
 *
 * \param f Pointer to the fetcher structure to be initialised.
 * \param m Pointer to the \c Stream from which to compile the Fetcher.
 * \return  FETCHER_SUCCESS or an appropriate error code.
 */
static error_t initfetcher(Fetcher *f)
{
	count_t i;

	f->chunk_no = 0; /* essential */

	if (!handles && curl_global_init(CURL_GLOBAL_ALL))
		return FECTHER_FAILED_INIT; /* do it only once. */

	f->handle = curl_multi_init();
	if (!f->handle) return FECTHER_NO_MEMORY;

	/* limit the total amount of connections this multi handle uses */
	curl_multi_setopt(f->handle, CURLMOPT_MAXCONNECTS, FETCHER_MAX_TRANSFERS);

	/* Create a new temp dir */
	char* template = strdup(FETCHER_DIRECTOTY_TEMPLATE);
	f->cachedir = mkdtemp(template);

	/* Build the fetcher */
	for (i = 0; i < FETCHER_MAX_TRANSFERS; ++i)
	{	error_t error = reinithandle(f);
		if (error) return error;
	}

	++handles;

	f->downloadtime = 0;

	return FETCHER_SUCCESS;
}

/**
 * \brief Extracts and writes to cache embedded data.
 *
 * \param s The Stream from which to extract data.
 * \return  Pointer to the file handle, or NULL.
 */
static FILE *unembed(Stream *s)
{
	int i;
	for (i = 0; s->chunks[i]; ++i)
	{
		//TODO
	}

	return NULL;
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

	//free(f->cachedir);
	free(f->urlmodel);

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
	{	timeout.tv_sec = sleep_time / 1000;
		timeout.tv_usec = (sleep_time % 1000) * 1000;

		if (0 > select(max_fd+1, &read, &write, &except, &timeout))
			return FETCHER_NO_MULTIPLEX;
	}

	return FETCHER_SUCCESS;
}

/**
 * \brief Set an appropriate url and output file for each transfer.
 *
 * \param f The fetcher from which to retrieve urls and metadata.
 * \return  FETCHER_SUCCESS or an appropriate error code. 
 */
static error_t reinithandle(Fetcher *f)
{
	CURL *handle;
	FILE *output;
	char filename[FETCHER_MAX_FILENAME_LENGTH]; /* FIXME more efficient! */
	char urlbuffer[FETCHER_MAX_URL_LENGTH];
	char *chunkurl;

	/* The chunk to be parsed right now */
	f->nextchunk = f->stream->chunks[f->chunk_no];
	/* Ops! chunks are over! Bye bye. */
	if (!f->nextchunk) return FETCHER_SUCCESS;
	/* Increase the index to dereference next chunk */
	f->chunk_no++; /* FIXME turn to pointer operation */
/*	Chunk *next = f->stream->chunks++; // will leak */
/*	if (!next) return FETCHER_SUCCESS;*/
/*	f->nextchunk = *next;*/

	chunkurl = compileurl(f, urlbuffer);

	/* Build and open cache file */
	snprintf(filename, FETCHER_MAX_FILENAME_LENGTH,  "%s/%lu",
		f->cachedir, f->nextchunk->time);
	output = fopen(filename, "wx"); /* FIXME with GLIBC */
	if (!output) return FETCHER_NO_FILE;

	/* Build downloader */
	if (!(handle = curl_easy_init())) return FECTHER_NO_MEMORY;
	/* Set the url from which to retrieve the chunk */
	curl_easy_setopt(handle, CURLOPT_URL, chunkurl);
	/* Write to the provided file handler */
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, output);
	/* Use the default write function */
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, NULL);
	/* Store the file descriptor to close it later */
	curl_easy_setopt(handle, CURLOPT_PRIVATE, output);
	/* Some servers don't like requests without a user-agent field... */
	curl_easy_setopt(handle, CURLOPT_USERAGENT, FETCHER_USERAGENT);
	/* No headers written, only body. */
	curl_easy_setopt(handle, CURLOPT_HEADER, 0L);
	/* No verbose messages. */
	curl_easy_setopt(handle, CURLOPT_VERBOSE, 0L);
	/* with old versions of libcurl: no progress meter */
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);

	if (curl_multi_add_handle(f->handle, handle))
	{   curl_easy_cleanup(handle);
		return FECTHER_HANDLE_NOT_ADDED;
	}


	return FETCHER_SUCCESS;
}

/**
 * \brief Compile a valid url to send a \c Chunk request
 *
 * \param buffer The char buffer to be filled with the new url.
 * \return       A pointer to the filled buffer.
 */
static char *compileurl(Fetcher *f, char *buffer)
{
	char temp[FETCHER_MAX_URL_LENGTH]; /* FIXME find something less painful */

	replace(temp, FETCHER_MAX_URL_LENGTH, f->urlmodel,
		FETCHER_START_TIME_PLACEHOLDER, "%lu", f->nextchunk->time);
	replace(buffer, FETCHER_MAX_URL_LENGTH, temp,
		FETCHER_BITRATE_PLACEHOLDER, "%u", getbitrate(f));

	return buffer;
}

/**
 * \brief Estimates the suitable bitrate, according to the network download speed.
 *
 * \param f The fetcher from which to get data
 * \return  A suitable bitrate.
 */
static bitrate_t getbitrate(Fetcher *f)
{
	int i;
	bitrate_t rightone = 0;
	Track **tracks = f->stream->tracks;

	for (i = 0; tracks[i]; ++i)
	{
		bitrate_t br = tracks[i]->bitrate;

		if ((br > rightone) && (!f->maxbitrate || (br <= f->maxbitrate))
		&& (!f->downloadtime || !f->nextchunk->duration ||
			(br <= (FETCHER_MAX_OVERHEAD_RATIO * f->nextchunk->duration /
				f->stream->tick * f->downloadtime)))
		)
		{   rightone = br;
		}
	}

	if (!rightone) /* If can't find any, pick the smallest */
	{
		rightone = tracks[0]; // assuming each one has at least tracks...
		for (i = 0; tracks[i]; ++i)
		{
			bitrate_t br = tracks[i]->bitrate;
			if (br < rightone) rightone = br;
		}

		f->maxbitrate = rightone; /* so that this won't happen again */
	}

/*	fprintf(stderr, "%dkbps (%.2lf%% overhead ratio)\n", rightone / 1000,*/
/*		100. * f->nextchunk->duration / f->stream->tick * f->downloadtime /rightone);*/

	return rightone;
}

/**
 * \brief Replace a string with another
 *
 * \warning For the sake of speed, this will copy only the first
 *          \c FETCHER_REPLACE_FORMAT_LENGTH characters.
 * \param  buffer  the buffer to fill
 * \param  source  the string to process
 * \param  search  the token to find
 * \param  the printf token for the replacing tag (e.g. "%lu" or "%d")
 * \param  replace the token to replace
 * \return pointer to the string buffer
 */
static
char *replace(char *buffer, size_t size, const char *source, char *search,
	const char *format, void *replace)
{
	char *position;
	char specs[FETCHER_REPLACE_FORMAT_LENGTH+1];
	/* if no match, just say hello */
	if (!(position = strstr(source, search))) return strcpy(buffer, source);
	/* compile format string */
	snprintf(specs, FETCHER_REPLACE_FORMAT_LENGTH, "%s%%s", format);
	/* replace */
	strncpy(buffer, source, position-source);  
	buffer[position-source] = 0;
	snprintf(buffer+(position-source), size, specs,
		replace, position+strlen(search));
	return buffer;
}

/* vim: set ts=4 sw=4 tw=0: */
