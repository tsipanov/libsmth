/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-dump.c: verbosely dump Manifests and Fragments
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

/**
 * \internal
 * \file   smth-dump.c
 * \brief  Manifest and Fragment dump.
 * \author Stefano Sanfilippo
 * \date   12th July 2010
 */

#include <string.h>
#include <sys/stat.h>
#include <smth-dump.h>


static const char shortbar[] = "=======================";
static const char  longbar[] = "==========================================\
==================";

/**
 * \brief Dumps the manifest content as an ASCII tree to specified file.
 *
 * \param m      The manifest to be dumped
 * \param output The file that will receive the output.
 */
void SMTH_dumpmanifest(Manifest *m, FILE *output)
{	count_t i;

	fprintf(output, "\n%sMANIFEST SUMMARY%s\n\n", shortbar, shortbar);
	fprintf(output, "Manifest\n");
	fprintf(output, " +-duration: %llu ticks\n", m->duration);
	fprintf(output, " +-tick scale: %llu/s\n", m->tick);
	fprintf(output, " +-the stream is live: %s\n", m->islive? "yes":"no");
	fprintf(output, " +-no. of fragments cached: %d\n", m->lookahead);
	fprintf(output, " +-DVR window length: %.3Lfs\n",
		m->dvrwindow / (long double) m->tick);

	if (m->armor->content)
	{	fprintf(output, " +-armor\n");
		fprintf(output, " |  +-id: ");
		fwrite(m->armorID, sizeof(byte_t), sizeof(uuid_t), stdout);
		fprintf(output, "\n |  `-payload: ");
		fwrite(m->armor->content, sizeof(byte_t), m->armor->length, output);
		fprintf(output, "\n");
	}

	fprintf(output, " `-streams\n");
	if (m->streams)
	{	for (i = 0; m->streams[i]; i++)
		{   Stream *stream = m->streams[i];
			char rootcorner = m->streams[i+1]? '+': '`';
			char rootbar = m->streams[i+1]? '|': ' ';
			fprintf(output, "    %c-%s\n", rootcorner, stream->name);
			fprintf(output, "    %c  +-type: %d\n", rootbar, stream->type);
			if (stream->type == AUDIO)
			{	fprintf(output, "    %c  +-subtype: %s\n", rootbar,
					stream->subtype);
			}
			fprintf(output, "    %c  +-url: <%s>\n", rootbar, stream->url);
			fprintf(output, "    %c  +-parent stream: %s\n",
				rootbar, stream->parent);
			fprintf(output, "    %c  +-tick scale: %llu\n",
				rootbar, stream->tick);
			fprintf(output, "    %c  +-max size: %dx%dpx\n", rootbar,
				stream->maxsize.width, stream->maxsize.height);
			fprintf(output, "    %c  +-best size: %dx%dpx\n", rootbar,
				stream->bestsize.width, stream->bestsize.height);
			fprintf(output, "    %c  +-is embedded: %s\n", rootbar,
				stream->isembedded? "yes":"no");
			fprintf(output, "    %c  +-no. of chunks: %d (0 = auto)\n", rootbar,
				stream->chunksno);
			fprintf(output, "    %c  +-no. of tracks: %d (0 = auto)\n", rootbar,
				stream->tracksno);

			count_t j;
			if (stream->tracks)
			{	fprintf(output, "    %c  +-tracks\n", rootbar);
				char waitingchunks = stream->chunks? '|': ' ';
				for (j = 0; stream->tracks[j]; j++)
				{	Track *track = stream->tracks[j];
					char trackbar = stream->tracks[j+1]? '|': ' ';
					char trackcorner = stream->tracks[j+1]? '+': '`';
					fprintf(output, "    %c  %c  %c-track %d\n", rootbar,
						waitingchunks, trackcorner, track->index);
					fprintf(output, "    %c  %c  %c  +-bitrate: %4.2fkB/s\n",
						rootbar, waitingchunks, trackbar, track->bitrate/1000.);
					fprintf(output, "    %c  %c  %c  +-sample rate: %dB/s\n",
						rootbar, waitingchunks, trackbar, track->samplerate);
					if (stream->type == VIDEO)
					{	fprintf(output, "    %c  %c  %c  +-screen size: %dx%dpx\n", 
							rootbar, waitingchunks, trackbar,
							track->maxsize.width, track->maxsize.height);
					}
					if (stream->type == AUDIO)
					{	fprintf(output, "    %c  %c  %c  +-packet size: %dB\n",
							rootbar, waitingchunks, trackbar, track->packetsize);
						fprintf(output, "    %c  %c  %c  +-audio tag: %d\n",
							rootbar, waitingchunks, trackbar, track->audiotag);
						fprintf(output, "    %c  %c  %c  +-size of sample: %dB\n",
							rootbar, waitingchunks, trackbar,
							track->bitspersample);
					}
					fprintf(output, "    %c  %c  %c  +-fourCC type: %s\n",
						rootbar, waitingchunks, trackbar, track->fourcc);
					fprintf(output, "    %c  %c  %c  +-header: %s (hex)\n",
						rootbar, waitingchunks, trackbar, track->header);
					fprintf(output, "    %c  %c  %c  +-no. of channels: %d\n",
						rootbar, waitingchunks, trackbar, track->channelsno);
					fprintf(output, "    %c  %c  %c  +-NAL unit length: %dB\n",
						rootbar, waitingchunks, trackbar, track->nalunitlength);

					if (*track->attributes) /* if the content is not NULL */
					{   count_t m;
						fprintf(output, "    %c  %c  %c  `-attributes\n",
							rootbar, waitingchunks, trackbar);

						for(m = 0; track->attributes[m]; m+=2)
						{   char attrcorner = track->attributes[m+2]? '+': '`';
							fprintf(output, "    %c  %c  %c     %c-%s = %s\n",
								rootbar, waitingchunks, trackbar, attrcorner,
								track->attributes[i], track->attributes[i+1]);
						}
					}
				}
			}
			if (stream->chunks)
			{	fprintf(output, "    %c  `-chunks\n", rootbar);
				for (j = 0; stream->chunks[j]; j++)
				{	Chunk *chunk = stream->chunks[j];
					char chunkbar = stream->chunks[j+1]? '|': ' ';
					char chunkcorner = stream->chunks[j+1]? '+': '`';
					fprintf(output, "    %c     %c-chunk no.%d\n", rootbar,
						chunkcorner, chunk->index);
					fprintf(output, "    %c     %c  +-duration: %.3Lfs\n",
						rootbar, chunkbar, chunk->duration / (long double)m->tick);
					fprintf(output, "    %c     %c  `-timestamp: %.3Lfs\n",
						rootbar, chunkbar, chunk->time / (long double)m->tick);
				}
			}
		}
	}
	fprintf(output, "\n%s\n\n", longbar);
}

/**
 * \brief Dumps the fragment metadata as an ASCII tree to specified file.
 *
 * \param vc     The fragment to be dumped
 * \param output The file that will receive the output.
 */
void SMTH_dumpfragment(Fragment *vc, FILE *output)
{
	int i;

	fprintf(output, "\n%sDATA SUMMARY%s\n\n", shortbar, shortbar);
	fprintf(output, "Fragment no.%d [%d bytes of data]\n", vc->index, vc->size);
	fprintf(output, " +-timestap: %llu\n", vc->timestamp);
	fprintf(output, " +-duration: %llu\n", vc->duration);
	fprintf(output, " +-first settings: 0x%08lx\n", vc->settings);

	if (vc->armor.type != NONE)
	{	fprintf(output, " +-armor\n");
		fprintf(output, " | +-type: %d\n", vc->armor.type);
		fprintf(output, " | +-id: ");
		fwrite(vc->armor.id, sizeof(byte_t), sizeof(uuid_t), stdout);
		fprintf(output, "\n");
	}

	fprintf(output, " | +-size of init vectors: %x\n", vc->armor.vectorsize);
	fprintf(output, " | `-number of init vectors: %d\n", vc->armor.vectorno);
	fprintf(output, " +-fragment defaults\n");
	fprintf(output, " | +-offset: 0x%x\n", vc->defaults.dataoffset);
	fprintf(output, " | +-index: %d\n", vc->defaults.index);
	fprintf(output, " | +-duration: %d ticks\n", vc->defaults.duration);
	fprintf(output, " | +-size: %d\n", vc->defaults.size);
	fprintf(output, " | `-settings: 0x%08lx\n", vc->defaults.settings);
	fprintf(output, " `-samples (%d in total)\n", vc->sampleno);

	for ( i = 0; i < vc->sampleno; i++)
	{
		char corner = (i == vc->sampleno - 1)? '`': '+';
		char bar = (i == vc->sampleno - 1)? ' ': '|';
		fprintf(output, "   %c-sample #%d\n", corner, i + 1);
		fprintf(output, "   %c +-duration: %d ticks\n",
			bar, vc->samples[i].duration);
		fprintf(output, "   %c +-size: %d bytes\n",
			bar, vc->samples[i].size);
		fprintf(output, "   %c +-settings: 0x%08lx\n",
			bar, vc->samples[i].settings);
		fprintf(output, "   %c `-offset: 0x%x\n",
			bar, vc->samples[i].timeoffset);
	}

	fprintf(output, "\n%s\n\n", longbar);
}

/**
 * \brief Dumps the fragment payload, one file per fragment in the directory
 *        named after the given filename as \c "${filename}.d".
 *        For instance, \c /home/foo/bar.ism will be dumped to
 *        \c /home/foo/bar.ism.d/*.vc1
 *
 * \param vc        The manifest to be dumped
 * \param directory The directory name to which output the fragments. If it
 *                  does not exist, it will be created.
 */
void SMTH_dumppayload(Fragment* vc, char* ifile)
{
	char ofile[strlen(ifile)+16];
	int i, offset = 0;
	//FIXME SECURE
	for( i = 0; i < vc->sampleno; i++)
	{
		sprintf(ofile, "%s.d", ifile);
		mkdir(ofile, 0755);
		sprintf(ofile, "%s.d/%04d.vc1", ifile, i);
		int size = vc->samples[i].size;
		FILE *output = fopen(ofile, "wb");
		fwrite(&(vc->data[offset]), sizeof (byte_t), size, output);
		offset += size;
		fclose(output);
	}
}

/* vim: set ts=4 sw=4 tw=0: */
