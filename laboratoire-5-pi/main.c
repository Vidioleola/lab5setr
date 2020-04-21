#include <getopt.h>
#include <pthread.h>

#include "includes.h"

int main(int argc, char **argv)
{
    int c;
    int long_index = 0;
    int debugFlag = 0;
    const char *audioFileInDebug = "haba.wav";
    const char *audioFileEncodedDebug = "./music/audioEncode.out";
    const char *audioFilesDir = "./music";
    static struct option long_options[] =
        {
            /* These options set a flag. */
            {"debug", no_argument, 0, 'z'},
            {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "z", long_options, &long_index)) != -1)
    {
        switch (c)
        {
        case 'z':
            debugFlag = 1;
            break;
        default:
            fprintf(stderr, "Unknown arg %s \n", optarg);
            return 1;
            break;
        }
    }

    int client, sock;
    ret_t ret;
    ret.success =0;
    strcpy(ret.audioFile, "./music/");
    struct sockaddr_rc addr;
    //asking what does the client wants
    initBlueServer(&sock, &addr);
    while (ret.success == 0)
    {
        waitForConnection(&sock, &client, &addr);
        serveClient(&client, &ret);
        switch (ret.requestType)
        {
        case 0:
            listAudioFiles(audioFilesDir, client);
            break;
        case 1:
            playAudioFile(client, &ret); //ret.success is set here
            break;
        case 2:
            selectFilter(&ret);
            break;
        default:
            perror("unknown ret from serveClient");
            exit(1);
            break;
        }
    }
    close(sock);
    //free(filterName);
    printf("completed \n");
    return 0;
}