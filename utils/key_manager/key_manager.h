#ifndef _KEY_MANAGER_H_
#define _KEY_MANAGER_H_

unsigned char *get_or_generate_keys();
void encrypt_file(const char *filename, unsigned char *aes_key);
void generate_signature(const char *filename, const char *sig_filename);
int verify_signature(const char *filename, const char *sig_filename);
void decrypt_file(const char *filename, unsigned char *aes_key, const char *output_filename);
unsigned char *get_db_user_token(char *username);
int write_token_to_file(unsigned char *token, char *username);
char *get_db_username();
char *enc_password(char *password);
int verify_password(const char *password, const char *stored_hash);
#endif
