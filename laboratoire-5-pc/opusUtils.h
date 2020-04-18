#include "includes.h"

#define PACKETS_SIZE 108
#define MAX_BUFFER_PACKETS 64
#define MAX_PACKETS_ADVANCE 48
#define TRESHOLD 24
#define DECODED_AUDIO_NAME "decodedAudio.wav"

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

typedef struct Buffer
{
    char buff[MAX_BUFFER_PACKETS][PACKETS_SIZE];
    int writer;
    int reader;
    int sock;
    int keepGoing;
    int ready;
    int delta;
    pthread_mutex_t *lock;
    pthread_cond_t *condTooFew;
    pthread_cond_t *condToomuch;
}buffer_t;

typedef struct Decoder
{
    wavfile_header_t *header;
    OpusDecoder *decoder;
    opus_int32 chunk_size;
    int frame_duration_ms;
    int frame_size;
    opus_int16 *wavData;
} decoder_t;

typedef struct AudioFile{
    size_t offset;
    const char *fileName;
    FILE *fp;
}audio_t;

uchar *map_file_encode(const char *fn);
uchar* map_file_decode(const char* fn, size_t* size);
int decode(const char* audioFile);
int encode(const char *audioIn, const char *audioOut);
int prepareDecoding(buffer_t *buffer, decoder_t *decoder, audio_t *audioFile);
int decodeSignal(uchar *signal, decoder_t *decoder, audio_t *audioFile);