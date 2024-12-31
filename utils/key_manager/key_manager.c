#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define AES_KEY_LENGTH 32 // 256-bit AES key
#define AES_IV_LENGTH 16  // 128-bit IV

unsigned char *get_or_generate_keys()
{
  static unsigned char aes_key[AES_KEY_LENGTH];
  FILE *key_file = fopen("client_key.bin", "rb");

  if (key_file)
  {
    fread(aes_key, 1, AES_KEY_LENGTH, key_file);
    fclose(key_file);
  }
  else
  {
    if (!RAND_bytes(aes_key, AES_KEY_LENGTH))
    {
      fprintf(stderr, "Error generating AES key.\n");
      exit(EXIT_FAILURE);
    }
    key_file = fopen("client_key.bin", "wb");
    if (!key_file)
    {
      fprintf(stderr, "Error opening key file for writing.\n");
      exit(EXIT_FAILURE);
    }
    fwrite(aes_key, 1, AES_KEY_LENGTH, key_file);
    fclose(key_file);
  }

  return aes_key;
}

void encrypt_file(const char *filename, unsigned char *aes_key)
{
  FILE *input = fopen(filename, "rb");
  FILE *output = fopen("encrypted_file.bin", "wb");
  unsigned char iv[AES_IV_LENGTH];
  unsigned char buffer[1024];
  unsigned char ciphertext[1024 + AES_BLOCK_SIZE];
  int len, ciphertext_len;

  if (!input || !output)
  {
    fprintf(stderr, "Error opening file.\n");
    exit(EXIT_FAILURE);
  }

  if (!RAND_bytes(iv, AES_IV_LENGTH))
  {
    fprintf(stderr, "Error generating IV.\n");
    exit(EXIT_FAILURE);
  }

  fwrite(iv, 1, AES_IV_LENGTH, output);

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv);

  while ((len = fread(buffer, 1, sizeof(buffer), input)) > 0)
  {
    EVP_EncryptUpdate(ctx, ciphertext, &ciphertext_len, buffer, len);
    fwrite(ciphertext, 1, ciphertext_len, output);
  }

  EVP_EncryptFinal_ex(ctx, ciphertext, &ciphertext_len);
  fwrite(ciphertext, 1, ciphertext_len, output);

  EVP_CIPHER_CTX_free(ctx);
  fclose(input);
  fclose(output);
}

void decrypt_file(const char *filename, unsigned char *aes_key)
{
  FILE *input = fopen(filename, "rb");
  FILE *output = fopen("client_files/decrypted_file.txt", "wb");
  unsigned char iv[AES_IV_LENGTH];
  unsigned char buffer[1024];
  unsigned char plaintext[1024];
  int len, plaintext_len;

  if (!input || !output)
  {
    fprintf(stderr, "Error opening file.\n");
    exit(EXIT_FAILURE);
  }

  fread(iv, 1, AES_IV_LENGTH, input);

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv);

  while ((len = fread(buffer, 1, sizeof(buffer), input)) > 0)
  {
    EVP_DecryptUpdate(ctx, plaintext, &plaintext_len, buffer, len);
    fwrite(plaintext, 1, plaintext_len, output);
  }

  EVP_DecryptFinal_ex(ctx, plaintext, &plaintext_len);
  fwrite(plaintext, 1, plaintext_len, output);

  EVP_CIPHER_CTX_free(ctx);
  fclose(input);
  fclose(output);
}

void generate_signature(const char *filename, const char *sig_filename)
{
  const char *sig_dir = "signatures";
  struct stat st = {0};
  if (stat(sig_dir, &st) == -1)
  {
    mkdir(sig_dir, 0700);
  }

  char sig_filepath[256];
  snprintf(sig_filepath, sizeof(sig_filepath), "%s/%s.md5", sig_dir, sig_filename);

  FILE *input = fopen(filename, "rb");
  FILE *sig_file = fopen(sig_filepath, "wb");
  unsigned char buffer[1024];
  unsigned char md5_digest[EVP_MAX_MD_SIZE];
  unsigned int md5_digest_len;
  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

  if (!input || !sig_file)
  {
    fprintf(stderr, "Error opening file.\n");
    exit(EXIT_FAILURE);
  }

  EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
  int len;
  while ((len = fread(buffer, 1, sizeof(buffer), input)) > 0)
  {
    EVP_DigestUpdate(mdctx, buffer, len);
  }
  EVP_DigestFinal_ex(mdctx, md5_digest, &md5_digest_len);

  fwrite(md5_digest, 1, md5_digest_len, sig_file);

  EVP_MD_CTX_free(mdctx);
  fclose(input);
  fclose(sig_file);
}

int verify_signature(const char *filename, const char *sig_filename)
{
  char sig_filepath[256];
  snprintf(sig_filepath, sizeof(sig_filepath), "signatures/%s.md5", sig_filename);

  FILE *input = fopen(filename, "rb");
  FILE *sig_file = fopen(sig_filepath, "rb");
  unsigned char buffer[1024];
  unsigned char md5_digest[EVP_MAX_MD_SIZE];
  unsigned char expected_md5_digest[EVP_MAX_MD_SIZE];
  unsigned int md5_digest_len;
  unsigned int expected_md5_digest_len = EVP_MD_size(EVP_md5());
  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

  if (!input || !sig_file)
  {
    fprintf(stderr, "Error opening file: %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fread(expected_md5_digest, 1, expected_md5_digest_len, sig_file);

  EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
  int len;
  while ((len = fread(buffer, 1, sizeof(buffer), input)) > 0)
  {
    EVP_DigestUpdate(mdctx, buffer, len);
  }
  EVP_DigestFinal_ex(mdctx, md5_digest, &md5_digest_len);

  EVP_MD_CTX_free(mdctx);
  fclose(input);
  fclose(sig_file);

  return memcmp(md5_digest, expected_md5_digest, expected_md5_digest_len) == 0;
}
