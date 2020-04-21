#include <getopt.h>
#include <pthread.h>

#include "opusUtils.h"
#include "bluetoothUtils.h"

#define PACKETS_SIZE 108

int main(int argc, char **argv)
{
    int c;
    int long_index = 0;
    int debugFlag = 0;
    int filter = 0;
    char *filterName = NULL;
    int sourceName = 0;
    int err = 0;
    const char *audioFileInDebug = "haba.wav";
    const char *audioFilesDir = "./music";
    char *audioFileIn = NULL;
    static struct option long_options[] =
        {
            /* These options set a flag. */
            {"debug", no_argument, 0, 'z'},
            {"source", required_argument, 0, 's'},
            {"filter", required_argument, 0, 'f'},
            {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "zs:f:", long_options, &long_index)) != -1)
    {
        switch (c)
        {
        case 's':
            if (optarg)
            {
                audioFileIn = (char *)malloc((strlen(optarg) + 1) * sizeof(char));
                strcpy(audioFileIn, optarg);
                sourceName = 1;
            }
            else
            {
                printf("missing source name with -s\n");
                return 1;
            }
            break;
        case 'f':
            if (optarg)
            {
                filterName = (char *)malloc((strlen(optarg) + 1) * sizeof(char));
                strcpy(filterName, optarg);
                filter = 1;
            }
            else
            {
                printf("missing filter name with -f\n");
                return 1;
            }
            break;
        case 'z':
            debugFlag = 1;
            break;
        default:
            fprintf(stderr, "Unknown arg %s \n", optarg);
            return 1;
            break;
        }
    }

    if (sourceName == 1)
    {
        printf("specified source file for audio : %s\n", audioFileIn);
    }
    else
    {
        printf("specified source file for audio : %s\n", audioFileInDebug);
    }

    if (filter == 1)
    {
        printf("specified  filter for audio : %s\n", filterName);
    }

    if (debugFlag == 1)
    {
        err = encode(audioFileInDebug, "audioEncode.out");
    }
    else
    {
        err = encode(audioFileIn, "audioEncode.out");
    }

    int k = 0;
    FILE *fp;
    char buffer[PACKETS_SIZE];
    int client, sock, ret;
    struct sockaddr_rc addr;
    //asking what does the client wants
    err = initBlueServer(&sock, &client, &addr);
    while (1)
    {
        err = serveClient(&client, &ret);
        switch (ret)
        {
        case 0:
            listAudioFiles(audioFilesDir, client);
            break;
        case 1:
            goto sendAudio;
            break;
        default:
            perror("unknown ret from serveClient");
            exit(1);
            break;
        }
        waitForConnection(&sock, &client, &addr);
    }
sendAudio:
    fp = fopen("audioEncode.out", "r");
    fread(buffer, sizeof(char), sizeof(wavfile_header_t), fp);
    if (ferror(fp))
    {
        perror("Error reading compressed audio header (server)");
        exit(1);
    }
    if (feof(fp))
    {
        perror("Reached EOF while reading audio header (server)");
        exit(1);
    }
    //sending header
    size_t totalWritten = 0;
    int written = 0;
    while (totalWritten < sizeof(wavfile_header_t))
    {
        written = write(client, buffer, sizeof(wavfile_header_t) - totalWritten);
        if (written < 0)
        {
            perror("writing header to client socket");
            exit(1);
        }
        totalWritten += written;
        printf("sending %d data (header) to client\n", written);
    }
    //sending data
    while (1)
    {
        fread(buffer, sizeof(char), PACKETS_SIZE, fp);
        if (ferror(fp))
        {
            perror("Error reading compressed audio (server)");
            exit(1);
        }
        if (feof(fp))
        {
            break;
        }

        totalWritten = 0;
        written = 0;
        while (totalWritten < PACKETS_SIZE)
        {
            written = write(client, buffer, PACKETS_SIZE - totalWritten);
            if (written < 0)
            {
                perror("writing to client socket");
                exit(1);
            }
            totalWritten += written;
            printf("sending %d data to client\n", written);
        }
        k++;
    }
    printf("EOF audio reached at iteration %d \n", k);
    //sending stop...
    write(client, "STOP", 5);
    close(client);
    close(sock);
    free(audioFileIn);
    free(filterName);
    printf("completed \n");
    return 0;
}