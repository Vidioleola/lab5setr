/*
  BlueZ example code to build an rfcomm server.
  This code just creates a socket and accepts
  connections from a remote bluetooth device.
  Programmed by Bastian Ballmann
  http://www.geektown.de
  Compile with gcc -lbluetooth <executable> <source>
*/

#include "bluetoothUtils.h"

#define CHANNEL 4
#define QUEUE 10

int initBlueServer(int *sock, int *client)
{
  unsigned int alen;
  struct sockaddr_rc addr;

  if ((*sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0)
  {
    perror("socket");
    exit(1);
  }

  addr.rc_family = AF_BLUETOOTH;
  bacpy(&addr.rc_bdaddr, BDADDR_ANY);
  addr.rc_channel = htobs(CHANNEL);
  alen = sizeof(addr);

  if (bind(*sock, (struct sockaddr *)&addr, alen) < 0)
  {
    perror("bind");
    exit(1);
  }

  listen(*sock, QUEUE);
  printf("Waiting for connections...\n\n");
  
  if ((*client = accept(*sock, (struct sockaddr *)&addr, &alen)) < 0)
  {
    printf("connection failed \n");
    exit(1);
  }
  /*
  printf("connection sucessful with client %d and server sock : %d \n", *client, *sock);
  //write(*client, "12345678", 9);
  char buffer[6];
  read(*client, buffer, 6);
  printf("data is : %s \n", buffer);
  */
  return 0;
}