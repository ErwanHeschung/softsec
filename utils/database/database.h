#ifndef _DATABASE_H_
#define _DATABASE_H_
int init_mysql_database();

char *list_user_files(unsigned char *token);

int check_file_owner(char *filename, unsigned char *token);

unsigned char *login_user(char *username, char *password);
unsigned char *signup_user(char *username, char *password);

char *add_file(char *filename, unsigned char *token);
char *get_file(char *filename, unsigned char *token);

int store_token(unsigned char *token, unsigned char *username);
unsigned char *get_token(unsigned char *username);
int check_token(unsigned char *token, unsigned char *username);

void clean_tokens();
#endif