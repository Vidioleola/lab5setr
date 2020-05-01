#include "includes.h"

void *worker(void *data)
{
    const char *stop = "STOP";
    sync_t *d = (sync_t *)data;
    int i;
    while (1)
    {
        pthread_mutex_lock(d->lock);
        i = read(d->sock, d->buff[d->reader], PACKETS_SIZE);
        if (strcmp(d->buff[d->reader], stop) == 0)
        {
            d->keepGoing = 0;
            printf("Received STOP from server \n");
            pthread_mutex_unlock(d->lock);
            break;
        }

        if (d->reader > d->writer)
        {
            d->delta = d->reader - d->writer;
        }
        else if (d->writer > d->reader)
        {
            d->delta = MAX_BUFFER_PACKETS - d->writer + d->reader;
        }

        while (d->delta > MAX_PACKETS_ADVANCE)
        {
            pthread_cond_wait(d->condTooFew, d->lock);
        }
        d->reader = (d->reader + 1) % MAX_BUFFER_PACKETS;
        pthread_mutex_unlock(d->lock);
    }
    pthread_exit(0);
}

void checkErrors(int err, const char *message)
{
    if (err)
    {
        fprintf(stderr, "Error %d : %s \n", err, message);
    }
}

int main(int argc, char **argv)
{
    int c;
    int long_index = 0;
    int debugFlag, generateDecoded, listFiles = 0;
    int addrFlag, audioFileNameFlag = 0;
    char addrName[64];
    char *addrDebug = "B8:27:EB:D7:BF:66";
    char audioFileName[256];
    int filterType = 0;
    int err = 0;
    static struct option long_options[] =
        {
            /* These options set a flag. */
            {"generateDecoded", no_argument, 0, 'g'},
            {"debug", no_argument, 0, 'z'},
            {"list", no_argument, 0, 'l'},
            {"filter", required_argument, 0, 'f'},
            {"addr", required_argument, 0, 'a'},
            {"source", required_argument, 0, 's'},
            {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "gzla:s:f:", long_options, &long_index)) != -1)
    {
        switch (c)
        {
        case 'a':
            if (optarg)
            {
                strcpy(addrName, optarg);
                addrFlag = 1;
            }
            else
            {
                printf("missing addr name with -a\n");
                return 1;
            }
            break;
        case 's':
            if (optarg)
            {
                strcpy(audioFileName, optarg);
                audioFileNameFlag = 1;
            }
            else
            {
                printf("missing source name with -s\n");
                return 1;
            }
            break;
        case 'f': //1 = low-pass filter, 2 = high-pass filter
            if (optarg)
            {
                filterType = atoi(optarg);
                if (filterType < 1 || filterType > 2)
                {
                    filterType = 0;
                    printf("invalid filter argument specified, use 1 for a lowpass, 2 for a highpass");
                    return 1;
                }
            }
            else
            {
                printf("no filter argument specified");
                return 1;
            }
            break;
        case 'z':
            debugFlag = 1;
            break;
        case 'g':
            generateDecoded = 1;
            break;
        case 'l':
            listFiles = 1;
            break;
        default:
            fprintf(stderr, "Unknown arg %s \n", optarg);
            return 1;
            break;
        }
    }
    char *addr = addrName;
    if (addrFlag == 1)
    {
        printf("specified addr for audio : %s\n", addrName);
    }
    else
    {
        printf("specified addr for audio : %s\n", addrDebug);
    }

    if (debugFlag == 1)
    {
        addr = addrDebug;
    }

    int sock;
    err = initBlueClient(addr, 4, &sock);
    checkErrors(err, "initBlueClient failed");

    if (listFiles == 1)
    {
        listAudioFiles(sock);
        return 0;
    }

    if (audioFileNameFlag != 1)
    {
        printf("You must select an audioFile. Use -l to list\n");
        return 1;
    }

    //ask to filter an audioFile
    if (filterType)
    {
        char fMsg[2][5] = {"FILP", "FIHP"};
        char buff[3];
        write(sock, fMsg[filterType - 1], 5);
        write(sock, audioFileName, 256);
        read(sock, buff, 3);
        if (strcmp(buff, "OK") == 0)
        {
            printf("Filtering completed\n");
        }
        else
        {
            printf("Error while filtering\n");
            close(sock);
            return 1;
        }
    }
    else
    {
        //ask an audioFile without filter
        write(sock, "PLAY", 5);
        write(sock, audioFileName, 256);
    }

    pthread_cond_t condTooFew = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    wavfile_header_t header;
    decoder_t decoder;
    decoder.header = &header;

    audio_t audio;
    audio.fileName = DECODED_AUDIO_NAME;
    audio.fp = NULL;

    sync_t data;
    data.condTooFew = &condTooFew;
    data.lock = &lock;
    data.reader = 0;
    data.writer = 0;
    data.sock = sock;
    data.keepGoing = 1;
    data.delta = 0;

    prepareDecoding(&data, &decoder, &audio);
    initAlsa(&audio, &decoder);

    //this thread reads as fast as possible
    pthread_t threadBuffer;
    int ret = pthread_create(&threadBuffer, NULL, worker, (void *)&data);

    //main loop to decode and play audio
    while (data.keepGoing == 1)
    {
        while (data.reader == data.writer && data.keepGoing == 1)
        {
            usleep(5); //important to keep this if doing optimizations!
        }
        pthread_mutex_lock(data.lock);
        if (data.reader > data.writer)
        {
            data.delta = data.reader - data.writer;
        }
        else if (data.writer > data.reader)
        {
            data.delta = MAX_BUFFER_PACKETS - data.writer + data.reader;
        }
        if (data.delta < TRESHOLD)
        {
            pthread_cond_signal(data.condTooFew);
        }
        decodeAndPlaySignal(data.buff[data.writer], &decoder, &audio, generateDecoded);

        data.writer = (data.writer + 1) % MAX_BUFFER_PACKETS;
        pthread_mutex_unlock(data.lock);
    }
    pthread_join(threadBuffer, NULL);
    printf("completed \n");
    closePcm(&audio);
    free(decoder.save);
    pthread_mutex_destroy(data.lock);
    pthread_cond_destroy(data.condTooFew);
    return 0;
}