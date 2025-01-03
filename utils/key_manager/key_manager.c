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
#define SALT_SIZE 16
#define HASH_SIZE 32 // SHA256 produces 32-byte hashes

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

void decrypt_file(const char *filename, unsigned char *aes_key, const char *output_filename)
{
  FILE *input = fopen(filename, "rb");
  char *path = "client_files/";
  char *output_filename_with_path = malloc(strlen(path) + strlen(output_filename) + 1);
  strcpy(output_filename_with_path, path);
  strcat(output_filename_with_path, output_filename);
  FILE *output = fopen(output_filename_with_path, "wb");
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

unsigned char *get_db_user_token(char *username)
{
  FILE *token_file = fopen("user_token.bin", "rb");

  if (token_file)
  {
    char line[256];
    while (fgets(line, sizeof(line), token_file))
    {
      char *user = strtok(line, ":");
      char *token = strtok(NULL, ":");
      if (token != NULL)
      {
        size_t len = strlen(token);
        if (len > 0 && token[len - 1] == '\n')
        {
          token[len - 1] = '\0';
        }
      }
      if (strcmp(user, username) == 0)
      {
        fclose(token_file);
        return (unsigned char *)strdup(token);
      }
    }
    fclose(token_file);
  }

  return NULL;
}

int write_token_to_file(unsigned char *token, char *username)
{
  FILE *token_file = fopen("user_token.bin", "w");

  if (token_file)
  {
    fprintf(token_file, "%s:%s\n", username, token);
    fclose(token_file);
  }
  else
  {
    fprintf(stderr, "Error opening token file for writing.\n");
    exit(EXIT_FAILURE);
  }

  return 1;
}

char *get_db_username()
{
  FILE *token_file = fopen("user_token.bin", "rb");

  if (token_file)
  {
    char line[256];
    while (fgets(line, sizeof(line), token_file))
    {
      char *user = strtok(line, ":");
      char *token = strtok(NULL, ":");
      return strdup(user);
    }
    fclose(token_file);
  }

  return NULL;
}

int generate_salt(unsigned char *salt, size_t size)
{
  if (!salt || size == 0)
    return 0;
  return RAND_bytes(salt, size);
}

char *hash_password(const char *password, unsigned char *salt)
{
  if (!password || !salt)
    return NULL;

  unsigned char hash[HASH_SIZE];
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (!ctx)
  {
    perror("Failed to create hash context");
    return NULL;
  }

  if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1)
  {
    perror("Failed to initialize hash function");
    EVP_MD_CTX_free(ctx);
    return NULL;
  }

  if (EVP_DigestUpdate(ctx, salt, SALT_SIZE) != 1)
  {
    perror("Failed to hash salt");
    EVP_MD_CTX_free(ctx);
    return NULL;
  }

  if (EVP_DigestUpdate(ctx, password, strlen(password)) != 1)
  {
    perror("Failed to hash password");
    EVP_MD_CTX_free(ctx);
    return NULL;
  }

  if (EVP_DigestFinal_ex(ctx, hash, NULL) != 1)
  {
    perror("Failed to finalize hash");
    EVP_MD_CTX_free(ctx);
    return NULL;
  }

  EVP_MD_CTX_free(ctx);

  char *result = (char *)malloc((SALT_SIZE + HASH_SIZE) * 2 + 1);
  if (!result)
  {
    perror("Failed to allocate memory for hash result");
    return NULL;
  }

  for (int i = 0; i < SALT_SIZE; i++)
  {
    sprintf(result + i * 2, "%02x", salt[i]);
  }

  for (int i = 0; i < HASH_SIZE; i++)
  {
    sprintf(result + (SALT_SIZE + i) * 2, "%02x", hash[i]);
  }

  result[(SALT_SIZE + HASH_SIZE) * 2] = '\0';
  return result;
}

int verify_password(const char *password, const char *stored_hash)
{
  if (!password || !stored_hash)
    return 0;

  unsigned char salt[SALT_SIZE];
  for (int i = 0; i < SALT_SIZE; i++)
  {
    sscanf(stored_hash + i * 2, "%2hhx", &salt[i]);
  }

  char *calculated_hash = hash_password(password, salt);
  if (!calculated_hash)
    return 0;

  int result = (strcmp(calculated_hash, stored_hash) == 0);
  free(calculated_hash);

  return result;
}

char *enc_password(char *password)
{
  unsigned char salt[SALT_SIZE];
  if (!generate_salt(salt, SALT_SIZE))
  {
    perror("Failed to generate salt");
    return NULL;
  }

  char *hashed_password = hash_password(password, salt);
  if (!hashed_password)
  {
    printf("Failed to hash password\n");
    return NULL;
  }

  return hashed_password;
}