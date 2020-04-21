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

int initBlueServer(int *sock, int *client, struct sockaddr_rc *addr)
{
  unsigned int alen;

  if ((*sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0)
  {
    perror("socket");
    exit(1);
  }

  addr->rc_family = AF_BLUETOOTH;
  bacpy(&addr->rc_bdaddr, BDADDR_ANY);
  addr->rc_channel = htobs(CHANNEL);
  alen = sizeof(*addr);

  if (bind(*sock, (struct sockaddr *)addr, alen) < 0)
  {
    perror("bind");
    exit(1);
  }

  listen(*sock, QUEUE);
  printf("Waiting for connections...\n\n");

  if ((*client = accept(*sock, (struct sockaddr *)addr, &alen)) < 0)
  {
    printf("connection failed \n");
    exit(1);
  }
  return 0;
}

int serveClient(int *client, int *ret)
{
  char msg[5];
  ssize_t i = read(*client, msg, 5);
  if (i < 0)
  {
    perror("Error reading client request");
    close(*client);
    return (1);
  }

  if (strcmp(msg, "LIST") == 0)
  {
    *ret = 0;
  }
  else if (strcmp(msg, "PLAY") == 0)
  {
    *ret = 1;
  }
  else
  {
    perror("Unknown request from client");
    close(*client);
    return (1);
  }

  return 0;
}
int listAudioFiles(const char *dir, int client)
{
  struct dirent *de; // Pointer for directory entry
  size_t count = 0;

  // opendir() returns a pointer of DIR type.
  DIR *dr = opendir(dir);

  if (dr == NULL) // opendir returns NULL if couldn't open directory
  {
    printf("Could not open directory %s\n", dir);
    exit(1);
  }

  while ((de = readdir(dr)) != NULL)
  {
    if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, ".."))
    {
      count++;
    }
  }
  closedir(dr);
  write(client, &count, sizeof(size_t));
  dr = opendir(dir);
  while ((de = readdir(dr)) != NULL)
  {
    if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, ".."))
    {
      ssize_t totalWritten = 0;
      ssize_t written = 0;
      ssize_t len = 256;
      while (totalWritten < len)
      {
        written = write(client, de->d_name, len - totalWritten);
        if (written < 0)
        {
          perror("writing to client socket");
          closedir(dr);
          return (1);
        }
        totalWritten += written;
        printf("sending dir content %s to client\n", de->d_name);
      }
    }
  }
  closedir(dr);
  close(client);
  return 0;
}

int waitForConnection(int *sock, int *client, struct sockaddr_rc *addr)
{
  unsigned int alen;
  alen = sizeof(*addr);
  
  listen(*sock, QUEUE);
  printf("Waiting for connections...\n\n");

  if ((*client = accept(*sock, (struct sockaddr *)addr, &alen)) < 0)
  {
    printf("connection failed \n");
    exit(1);
  }
  return 0;
}