#include <stdio.h>  /* Standard input/output definitions */
#include <string.h> /* String function definitions */
#include <unistd.h> /* UNIX standard function definitions */
#include <fcntl.h>  /* File control definitions */
#include <errno.h>  /* Error number definitions */
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <opus/opus.h>
#include <pthread.h>
#include <getopt.h>
#include <alsa/asoundlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>


#define PACKETS_SIZE 160
#define MAX_BUFFER_PACKETS 64
#define MAX_PACKETS_ADVANCE 48
#define TRESHOLD 24
#define DECODED_AUDIO_NAME "decodedAudio.wav"

#define CHUNK 80
#define BIT_RATE 128000

typedef unsigned char uchar;

// Information sur le header WAV:
//https://web.archive.org/web/20140327141505/https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
//http://www.topherlee.com/software/pcm-tut-wavformat.html

typedef struct wavfile_header_s
{
    char ChunkID[4];   /*  4   */
    int32_t ChunkSize; /*  4   */
    char Format[4];    /*  4   */

    char Subchunk1ID[4];   /*  4   */
    int32_t Subchunk1Size; /*  4   */
    int16_t AudioFormat;   /*  2   */
    int16_t NumChannels;   /*  2   */
    int32_t SampleRate;    /*  4   */
    int32_t ByteRate;      /*  4   */
    int16_t BlockAlign;    /*  2   */
    int16_t BitsPerSample; /*  2   */

    char Subchunk2ID[4];
    int32_t Subchunk2Size;
} wavfile_header_t;

typedef struct Sync
{
    uchar buff[MAX_BUFFER_PACKETS][PACKETS_SIZE];
    int writer;
    int reader;
    int sock;
    int keepGoing;
    int delta;
    pthread_mutex_t *lock;
    pthread_cond_t *condTooFew;
}sync_t;

typedef struct Decoder
{
    wavfile_header_t *header;
    OpusDecoder *decoder;
    opus_int32 chunk_size;
    int frame_duration_ms;
    int frame_size;
    opus_int16 *wavData;
    opus_int16 *save;
    size_t nReady;
} decoder_t;

typedef struct AudioFile{
    const char *fileName;
    FILE *fp;
    snd_pcm_t *handle;
}audio_t;


uchar *map_file_encode(const char *fn);
uchar* map_file_decode(const char* fn, size_t* size);
int decode(const char* audioFile, const char *audioOut);
int encode(const char *audioIn, const char *audioOut);
int prepareDecoding(sync_t *buffer, decoder_t *decoder, audio_t *audioFile);
int decodeAndPlaySignal(uchar *signal, decoder_t *decoder, audio_t *audioFile, int generateDecoded);

int initBlueClient(const char *remoteAddr, int channel, int *sock);
int listAudioFiles(int sock);

int initAlsa(audio_t *audio, decoder_t *decoder);
int closePcm(audio_t *audio);