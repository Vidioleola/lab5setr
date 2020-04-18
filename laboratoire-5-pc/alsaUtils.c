/*
 *  This extra small demo sends a random samples to your speakers.
 */
#include "includes.h"

int initAlsa(music_t *music, decoder_t *decoder)
{
	static char *device = "default"; /* playback device */
	snd_output_t *output = NULL;
	int err;

	if ((err = snd_pcm_open(&(music->handle), device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = snd_pcm_set_params(music->handle,
								  SND_PCM_FORMAT_S16_LE,
								  SND_PCM_ACCESS_RW_INTERLEAVED,
								  decoder->header->NumChannels,
								  decoder->header->SampleRate,
								  1,
								  1000*decoder->frame_duration_ms)) < 0)
	{
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	return 0;
}

int playMusic(music_t *music, decoder_t *decoder)
{
	music->frames = snd_pcm_writei(music->handle, decoder->wavData, decoder->frame_size);
	if (music->frames < 0)
		music->frames = snd_pcm_recover(music->handle, music->frames, 0);
	if (music->frames < 0)
	{
		printf("snd_pcm_writei failed: %s\n", snd_strerror(music->frames));
		exit(1);
	}
	if (music->frames > 0 && music->frames < (long)decoder->frame_size)
		printf("Short write (expected %li, wrote %li)\n", (long)decoder->frame_size, music->frames);
	return 0;
}

int closePcm(music_t *music)
{
	snd_pcm_close(music->handle);
	return 0;
}