#ifndef _FILE_H_
#define _FILE_H_

int send_file(char *filename, int port);

int receive_file(char *filename, char *dest_output);

#endif