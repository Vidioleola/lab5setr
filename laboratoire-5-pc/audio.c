#include "includes.h"

int main(int argc, char **argv)
{
    static char *device = "default"; /* playback device */
    snd_pcm_t *handle;
    snd_pcm_sframes_t frames;
    snd_output_t *output = NULL;
    unsigned int tmp;
    int err, rate, buff_size;

    FILE *fp = fopen("decodedAudio.wav", "r");
    wavfile_header_t header;

    ssize_t r, tr = 0;
    while ((r = fread(&header, sizeof(char), sizeof(wavfile_header_t) - tr, fp)) > 0)
    {
        tr += r;
    }
    if (r < 0)
    {
        perror("Error receiving header");
        exit(1);
    }
    printf("Wav header read\n");

    // Donnés utile pour le décodage
    int32_t sample_rate = header.SampleRate;
    printf("sample rate is %d\n", sample_rate);
    int16_t n_channels = header.NumChannels;
    printf("channels are %d\n", n_channels);
    int frame_duration_ms = 5; // Une durée plus grande que 10 ms va introduire énormément de bruit dans un signal musical.
    int frame_size = sample_rate / 1000 * frame_duration_ms;
    opus_int16 *buffer = (opus_int16 *)malloc(frame_size * n_channels * sizeof(opus_int16));

    if ((err = snd_pcm_open(&(handle), device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    if ((err = snd_pcm_set_params(handle,
                                  SND_PCM_FORMAT_S16_LE,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  n_channels,
                                  sample_rate,
                                  1,
                                  1000 * frame_duration_ms)) < 0)
    {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    while ((r = fread(buffer, sizeof(opus_int16), 2 * frame_size, fp)) > 0)
    {
        tr = r;
        while (tr < 2 * frame_size)
        {
            r = fread(buffer, sizeof(opus_int16), 2 * frame_size - tr, fp);
            tr += r;
        }
        frames = snd_pcm_writei(handle, buffer, frame_size);
        if (frames < 0)
            frames = snd_pcm_recover(handle, frames, 0);
        if (frames < 0)
        {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
            exit(1);
        }
        if (frames > 0 && frames < (long)frame_size)
            printf("Short write (expected %li, wrote %li)\n", (long)frame_size, frames);
    }
    free(buffer);
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    return 0;
}