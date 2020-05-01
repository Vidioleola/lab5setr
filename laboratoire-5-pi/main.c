#include <getopt.h>
#include <pthread.h>

#include "includes.h"

int main(int argc, char **argv){
    int c;
    int long_index = 0;
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
            break;
        default:
            fprintf(stderr, "Unknown arg %s \n", optarg);
            return 1;
            break;
        }
    }

    int client, sock;
    struct sockaddr_rc addr;
    //asking what does the client wants
    initBlueServer(&sock, &addr);
    while (1)
    {
        ret_t ret;
        strcpy(ret.audioFile, "./music/");
        waitForConnection(&sock, &client, &addr);
        serveClient(&client, &ret);
        switch (ret.requestType)
        {
        case 0:
            listAudioFiles(audioFilesDir, client);
            break;
        case 1:
            playAudioFile(client, &ret);
            goto end;
            break;
        case 2:
            encodeAndFilter(&ret);
            write(client, "OK", 3);
            playCompressedAudio(client, &ret);
            goto end;
            break;
        default:
            perror("unknown ret from serveClient");
            exit(1);
            break;
        }
    }
    end:
    close(sock);
    printf("completed \n");
    return 0;
}