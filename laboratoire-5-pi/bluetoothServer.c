/*
  BlueZ example code to build an rfcomm server.
  This code just creates a socket and accepts
  connections from a remote bluetooth device.
  Programmed by Bastian Ballmann
  http://www.geektown.de
  Compile with gcc -lbluetooth <executable> <source>
*/

#include "includes.h"

#define CHANNEL 4
#define QUEUE 10

int initBlueServer(int *sock,struct sockaddr_rc *addr){
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
  return 0;
}

int serveClient(int *client, ret_t *ret){
  char msg[5];
  char buff[256];
  ssize_t i = read(*client, msg, 5);
  if (i < 0)
  {
    perror("Error reading client request");
    close(*client);
    return (1);
  }

  if (strcmp(msg, "LIST") == 0)
  {
    ret->requestType = 0;
  }
  else if (strcmp(msg, "PLAY") == 0)
  {
    ret->requestType = 1;
    i = read(*client, buff, 256);
    if (i < 0){
      perror("Error reading client file request");
      close(*client);
      return 1;
    }
    strcat(ret->audioFile, buff); //append to ./music/
  }
  else if (strcmp(msg, "FILP") == 0)
  {
    ret->requestType = 2;
    ret->filter = 1;
    i = read(*client, buff, 256);
    if (i < 0){
      perror("Error reading client file request");
      close(*client);
      return 1;
    }
    strcat(ret->audioFile, buff); //append to ./music/
  }
  else if (strcmp(msg, "FIHP") == 0)
  {
    ret->requestType = 2;
    ret->filter = 2;
    i = read(*client, buff, 256);
    if (i < 0){
      perror("Error reading client file request");
      close(*client);
      return 1;
    }
    strcat(ret->audioFile, buff); //append to ./music/
  }
  else
  {
    perror("Unknown request from client");
    close(*client);
    return (1);
  }

  return 0;
}

int listAudioFiles(const char *dir, int client){
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

int waitForConnection(int *sock, int *client, struct sockaddr_rc *addr){
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

int playAudioFile(int client, ret_t *ret){

  int k = 0;
  FILE *fp;
  char buffer[PACKETS_SIZE];
  
  fp = fopen(ret->audioFile, "r");
  if(fp == NULL){
    printf("Error opening audio file");
    return 1;
  }
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
  ret->success = 1;
  return 0;
}

int encodeAndFilter(ret_t *ret){
  char *audioOut = ret->audioFile;
  if(ret->filter==1) strcat(audioOut, "_filtered_lp");
  else if(ret->filter==2) strcat(audioOut, "_filtered_hp");
  encode(ret->audioFile, audioOut, ret->filter);
  return 0;
}