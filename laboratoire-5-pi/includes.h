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
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>

#include <opus/opus.h>



#define PACKETS_SIZE 108

typedef struct Ret{
    int requestType;
    int filter;
    int success;
    char audioFile[512];
}ret_t;

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

uchar *map_file_encode(const char *fn);
uchar* map_file_decode(const char* fn, size_t* size);
int decode(const char* audioFile);
int encode(const char *audioIn, const char *audioOut);

int initBlueServer(int *sock, int *client, struct sockaddr_rc *addr);
int serveClient(int *client, ret_t *ret);
int listAudioFiles(const char* dir, int client);
int playAudioFile(int client, ret_t *ret);
int selectFilter(ret_t *ret);
int waitForConnection(int *sock, int *client, struct sockaddr_rc *addr);