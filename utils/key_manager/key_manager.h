#ifndef _KEY_MANAGER_H_
#define _KEY_MANAGER_H_

unsigned char *get_or_generate_keys();
void encrypt_file(const char *filename, unsigned char *aes_key);
void generate_signature(const char *filename, const char *sig_filename);
int verify_signature(const char *filename, const char *sig_filename);
void decrypt_file(const char *filename, unsigned char *aes_key);

#endif
