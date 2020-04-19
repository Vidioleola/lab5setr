/*
 *  This extra small demo sends a random samples to your speakers.
 */
#include "includes.h"

int initAlsa(audio_t *audio, decoder_t *decoder)
{
	static char *device = "default"; /* playback device */
	snd_pcm_hw_params_t *params;
	snd_output_t *output = NULL;
	int err;

	if ((err = snd_pcm_open(&(audio->handle), device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
		
	if ((err = snd_pcm_set_params(audio->handle,
								  SND_PCM_FORMAT_S16_LE,
								  SND_PCM_ACCESS_RW_INTERLEAVED,
								  decoder->header->NumChannels,
								  decoder->header->SampleRate,
								  1,
								  1000 * decoder->frame_duration_ms)) < 0)
	{
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	
	return 0;
}

int closePcm(audio_t *audio)
{
	snd_pcm_drain(audio->handle);
	snd_pcm_close(audio->handle);
	return 0;
}