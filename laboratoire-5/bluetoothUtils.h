#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>

# include "includes.h"

int initBlueServer(int *sock, int *client);
int initBlueClient(const char *remoteAddr, int channel, int *sock);