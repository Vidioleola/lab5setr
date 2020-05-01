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

int initBlueServer(int *sock, struct sockaddr_rc *addr)
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
  return 0;
}

int serveClient(int *client, ret_t *ret)
{
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
    if (i < 0)
    {
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
    if (i < 0)
    {
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
    if (i < 0)
    {
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

int playAudioFile(int client, ret_t *client_ret, int flag)
{

  /* this snipet is not working correctly : there is part of audio missing at decoding
  int ret;

  uchar *input_ptr = map_file_encode(client_ret->audioFile); // Pointeur de début de fichier
  if (input_ptr == MAP_FAILED)
  {
    printf("fail to map file");
    return -1;
  }
  uchar *reading_end = input_ptr; // Pointeur actif

  // On récupère le header wav
  wavfile_header_t *header = (wavfile_header_t *)input_ptr;
  // Donnés utile pour l'encodage
  int32_t sample_rate = header->SampleRate;
  int16_t n_channels = header->NumChannels;
  int32_t audio_size = header->Subchunk2Size;
  int32_t wav_header_size = sizeof(wavfile_header_t);
  int32_t total_file_size = wav_header_size + audio_size;

  // Initialisation de l'encodeur Opus
  OpusEncoder *encoder = NULL;
  int size = 0;
  size = opus_encoder_get_size(n_channels);
  encoder = (OpusEncoder *)malloc(size);
  if (!encoder)
  {
    printf("fail to create encoder");
    return -1;
  }
  // Ici on détermine le sample rate, le nombre de channel et un option pour la méthode d'encodage.
  // OPUS_APPLICATION_RESTRICTED_LOWDELAY nous donne la plus faible latence, mais vous pouvez essayer d'autres options.
  ret = opus_encoder_init(encoder, sample_rate, n_channels, OPUS_APPLICATION_RESTRICTED_LOWDELAY);
  if (ret != 0)
  {
    printf("fail to init %d", ret);
    return ret;
  }

  // Bitrate du signal encodé : Vous devez trouver le meilleur compromis qualité/transmission.
  ret = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BIT_RATE)); //you MUST change the chunk_size in decode depending of value.
  //ex : with 28000 chunk is 18, with 64000 it's 40.
  if (ret != 0)
  {
    printf("fail to set bitrate");
    return ret;
  }

  // Complexité de l'encodage, permet un compromis qualité/latence
  ret = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(5));
  if (ret != 0)
  {
    printf("fail to set complexity");
    return ret;
  }

  // Variation du bitrate.
  ret = opus_encoder_ctl(encoder, OPUS_SET_VBR(0));
  if (ret != 0)
  {
    printf("fail to set vbr");
    return ret;
  }
  // type de signal à encoder (dans notre cas il s'agit de la musique)
  ret = opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));
  if (ret != 0)
  {
    printf("fail to set signal");
    return ret;
  }
  int k = 0;
  int frame_duration_ms = 5; // Une durée plus grande que 10 ms va introduire énormément de bruit dans un signal musical.
  int frame_size = sample_rate / 1000 * frame_duration_ms;
  int stride, iter;
  if (BIT_RATE == 128000)
  {
    stride = 80;
    iter = 2;
  }
  else if (BIT_RATE == 64000)
  {
    stride = 40;
    iter = 4;
  }
  else
  {
    perror("Bit rate can only be 64000 or 128000");
    exit(1);
  }
  int max_data_size = stride * iter;
  uchar *senderBuffer = malloc(sizeof(uchar) * max_data_size);

  //sending header
  size_t totalWritten = 0;
  int written = 0;
  while (totalWritten < sizeof(wavfile_header_t))
  {
    written = write(client, header, sizeof(wavfile_header_t) - totalWritten);
    if (written < 0)
    {
      perror("writing header to client socket");
      exit(1);
    }
    totalWritten += written;
    printf("sending %d data (header) to client\n", written);
  }

  reading_end += wav_header_size; // On skip le header pour débuter l'encodage

  while (1)
  {
    for (int i = 0; i < iter; i++)
    {
      ret = opus_encode(encoder, (opus_int16 *)reading_end, frame_size, (senderBuffer + i * stride), stride);
      if (ret <= 0)
      {
        printf("fail to encode");
        break;
      }
      //printf("ret is : %d\n", ret);

      // Incrémente le pointeur
      reading_end += frame_size;
      if ((uchar *)reading_end >= input_ptr + total_file_size)
      {
        goto end;
      }
    }
    totalWritten = 0;
    written = 0;
    while (totalWritten < PACKETS_SIZE)
    {
      written = write(client, senderBuffer, PACKETS_SIZE - totalWritten);
      if (written < 0)
      {
        perror("writing to client socket");
        exit(1);
      }
      totalWritten += written;
      //printf("sending %d data to client\n", written);
    }
    k++;
  }
end:
  printf("EOF audio reached at iteration %d \n", k);
  //sending stop...
  write(client, "STOP", 5);
  close(client);
  free(senderBuffer);
  client_ret->success = 1;
  return 0;
  */

  int k = 0;
  FILE *fp;
  char buffer[PACKETS_SIZE];
  if (flag == 0)
  {
    fp = fopen(client_ret->audioFile, "r");
  }
  else
  {
    fp = fopen(client_ret->audioFileFilter, "r");
  }
  if (fp == NULL)
  {
    printf("Error opening audio file %s \n", client_ret->audioFile);
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
  return 0;
}

int encodeAndFilter(ret_t *client_ret)
{
  strcpy(client_ret->audioFileFilter, client_ret->audioFile);
  if (client_ret->filter == 1)
    strcat(client_ret->audioFileFilter, "_filtered_lp");
  else if (client_ret->filter == 2)
    strcat(client_ret->audioFileFilter, "_filtered_hp");
  encode(client_ret->audioFile, client_ret->audioFileFilter, client_ret->filter);
  memset(client_ret->audioFile, '\0', 512);
  memset(client_ret->audioFileFilter, '\0', 512);

  return 0;
}