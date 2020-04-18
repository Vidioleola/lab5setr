#include "bluetoothUtils.h"

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

int sendData(int sock, const char *data, size_t len)
{
    ssize_t ret;
    size_t sended = 0;
    while (sended < len)
    {
        ret = send(sock, data, len - sended, 0);
        if (ret < 0)
        {
            perror("sending data to server failed");
            exit(1);
        }
        sended += ret;
    }
    return 0;
}
