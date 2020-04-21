
#include "includes.h"

#define MAX_FILE_NAME_LEN 256

int initBlueClient(const char *remoteAddr, int channel, int *sock)
{
    struct sockaddr_rc laddr, raddr;
    struct hci_dev_info di;

    if (hci_devinfo(0, &di) < 0)
    {
        perror("HCI device info failed");
        exit(1);
    }

    printf("Local device %s\n", batostr(&di.bdaddr));

    laddr.rc_family = AF_BLUETOOTH;
    laddr.rc_bdaddr = di.bdaddr;
    laddr.rc_channel = 0;

    raddr.rc_family = AF_BLUETOOTH;
    str2ba(remoteAddr, &raddr.rc_bdaddr);
    raddr.rc_channel = channel;

    if ((*sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0)
    {
        perror("socket client");
    }

    if (bind(*sock, (struct sockaddr *)&laddr, sizeof(laddr)) < 0)
    {
        perror("bind socket client");
        exit(1);
    }

    if (connect(*sock, (struct sockaddr *)&raddr, sizeof(raddr)) < 0)
    {
        perror("connect to remote addr");
        exit(1);
    }
    /*
    printf("sending data to %d \n", *sock);
    if(send(*sock,"allo", 6, 0) < 0) {
        perror("sending to server");
        exit(1);
    }
    */

    return 0;
}

int listAudioFiles(int sock)
{
    char buffer[MAX_FILE_NAME_LEN];
    char **files = NULL;
    char message[] = "LIST";
    ssize_t len = strlen(message) + 1;
    ssize_t totalWritten = 0;
    ssize_t written = 0;
    ssize_t ret = 0;
    size_t numberOfFiles, fileCounter = 0;
    while (totalWritten < len)
    {
        written = write(sock, message, len - totalWritten);
        if (written < 0)
        {
            perror("writing to client socket");
            exit(1);
        }
        totalWritten += written;
        printf("sending LIST to client\n");
    }
    ret = read(sock, &numberOfFiles, sizeof(size_t));
    printf("have number of file : %d\n", numberOfFiles);
    if (ret < 0)
    {
        perror("Error reading number of files");
        exit(1);
    }
    files = (char **)malloc(numberOfFiles * sizeof(char *));
    for (int i = 0; i < numberOfFiles; i++)
    {
        files[i] = (char *)malloc(MAX_FILE_NAME_LEN * sizeof(char));
    }
    while (fileCounter < numberOfFiles)
    {
        ret = read(sock, buffer, MAX_FILE_NAME_LEN);
        memcpy(files[fileCounter++], buffer, MAX_FILE_NAME_LEN);
    }
    printf("Audio files availaible : \n");
    for (int i = 0; i  < numberOfFiles; i++)
    {
        printf("%s\n", files[i]);
        free(files[i]);
    }
    printf("_________________________\n");
    free(files);
    return 0;
}
