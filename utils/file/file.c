#include "../../libs/client.h"
#include "../../libs/server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_BUFFER_SIZE 1024
#define PATH_BUFFER_SIZE 1035

int send_file(char *filename, int port)
{
  FILE *file = fopen(filename, "rb");
  printf("Sending file %s\n", filename);
  if (file == NULL)
  {
    printf("File not found\n");
    return -1;
  }

  char buffer[1024] = {0};

  while (!feof(file))
  {
    fread(buffer, 1024, 1, file);
    sndmsg(buffer, port);
    memset(buffer, 0, sizeof(buffer));
  }

  snprintf(buffer, 1024, "EOF");
  sndmsg(buffer, port);

  return 0;
}

int receive_file(char *filename, char *dest_dir)
{
  char full_path[1024];
  if (dest_dir[0] == '.')
  {
    snprintf(full_path, 1024, "%s", filename);
  }
  else
  {
    sprintf(full_path, "%s/%s", dest_dir, filename);
  }
  FILE *file = fopen(full_path, "wb");
  printf("Receiving file %s\n", full_path);
  if (file == NULL)
  {
    printf("File not found\n");
    return -1;
  }

  char buffer[1024] = {0};
  while (1)
  {
    getmsg(buffer);
    if (strncmp(buffer, "EOF", 3) == 0)
    {
      break;
    }
    fwrite(buffer, strlen(buffer), 1, file);
  }
  fclose(file);
  return 0;
}

int send_string(char *str, int port)
{
  char buffer[1024] = {0};
  snprintf(buffer, 1024, "%s", str);
  sndmsg(buffer, port);
  return 0;
}

void delete_file_client(char *filename)
{
  char command[1024];
  snprintf(command, 1024, "rm %s", filename);
  system(command);
}
