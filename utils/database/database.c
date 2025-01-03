#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <mysql/mysql.h>
#include <openssl/sha.h>
#include "../utils/key_manager/key_manager.h"

#define SERVER_URL "127.0.0.1"
#define SERVER_USER "user"
#define SERVER_PASSWORD "YZT-tqk4zwd*awz9muk"
#define SERVER_DATABASE "sectrans"

int init_mysql_database()
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    const char *server = SERVER_URL;
    const char *user = SERVER_USER;
    const char *password = SERVER_PASSWORD;
    const char *database = SERVER_DATABASE;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, password, database, 3306, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }

    if (mysql_query(conn, "CREATE TABLE IF NOT EXISTS user (id INT PRIMARY KEY AUTO_INCREMENT, name VARCHAR(255), password VARCHAR(255))"))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }

    if (mysql_query(conn, "CREATE TABLE IF NOT EXISTS files (id INT PRIMARY KEY AUTO_INCREMENT, filename VARCHAR(255), hashname VARCHAR(255) NOT NULL DEFAULT '', owner_id INT, FOREIGN KEY (owner_id) REFERENCES user(id))"))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }

    if (mysql_query(conn, "CREATE TABLE IF NOT EXISTS tokens (id INT PRIMARY KEY AUTO_INCREMENT, token VARCHAR(255), user_id INT, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, FOREIGN KEY (user_id) REFERENCES user(id))"))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }

    mysql_close(conn);

    return EXIT_SUCCESS;
}

unsigned char *generate_token(size_t length)
{
    if (length == 0)
    {
        return NULL;
    }

    unsigned char *token = (unsigned char *)malloc(length + 1);
    if (!token)
    {
        perror("Failed to allocate memory for token");
        return NULL;
    }

    srand((unsigned int)time(NULL));

    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t charset_size = sizeof(charset) - 1;

    for (size_t i = 0; i < length; i++)
    {
        token[i] = charset[rand() % charset_size];
    }

    token[length] = '\0';

    return token;
}

int store_token(unsigned char *token, unsigned char *username)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    const char *server = SERVER_URL;
    const char *user = SERVER_USER;
    const char *pass = SERVER_PASSWORD;
    const char *database = SERVER_DATABASE;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, pass, database, 3306, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return 0;
    }
    char escaped_token[2 * 32 + 1];
    unsigned long escaped_length = mysql_real_escape_string(conn, escaped_token, (char *)token, 32);

    char query[1024];
    snprintf(query, 1024, "INSERT INTO tokens (token, user_id, created_at) VALUES ('%s', (SELECT id FROM user WHERE name = '%s'), NOW())", escaped_token, username);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return 0;
    }

    mysql_close(conn);

    return 1;
}
unsigned char *get_token(unsigned char *username)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    const char *server = SERVER_URL;
    const char *user = SERVER_USER;
    const char *pass = SERVER_PASSWORD;
    const char *database = SERVER_DATABASE;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, pass, database, 3306, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    char query[1024];
    snprintf(query, 1024, "SELECT token FROM tokens WHERE user_id = (SELECT id FROM user WHERE name = '%s')", username);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    res = mysql_store_result(conn);
    if (res)
    {
        while ((row = mysql_fetch_row(res)))
        {
            unsigned char *token = (unsigned char *)strdup(row[0]);
            mysql_close(conn);
            return token;
        }
        mysql_free_result(res);
    }

    mysql_close(conn);

    return NULL;
}
int check_token(unsigned char *token, unsigned char *username)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    const char *server = SERVER_URL;
    const char *user = SERVER_USER;
    const char *pass = SERVER_PASSWORD;
    const char *database = SERVER_DATABASE;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, pass, database, 3306, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return 0;
    }

    char query[1024];
    snprintf(query, 1024, "SELECT token FROM tokens WHERE token = '%s' AND user_id = (SELECT id FROM user WHERE name = '%s')", token, username);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res)
    {
        while ((row = mysql_fetch_row(res)))
        {
            printf("row: %s\n", row[0]);
            mysql_free_result(res);
            mysql_close(conn);
            return 1;
        }
        mysql_free_result(res);
    }
    else
    {
        printf("No result returned\n");
    }

    mysql_close(conn);

    return 0;
}
char *list_user_files(unsigned char *token)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    const char *server = SERVER_URL;
    const char *user = SERVER_USER;
    const char *password = SERVER_PASSWORD;
    const char *database = SERVER_DATABASE;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, password, database, 3306, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    char *username = NULL;
    char *token_str = (char *)token;
    char query[1024];
    snprintf(query, 1024, "SELECT name FROM user WHERE id = (SELECT user_id FROM tokens WHERE token = '%s')", token_str);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    res = mysql_store_result(conn);
    if (res)
    {
        while ((row = mysql_fetch_row(res)))
        {
            username = strdup(row[0]);
        }
        mysql_free_result(res);
    }

    if (username == NULL)
    {
        return NULL;
    }

    snprintf(query, 1024, "SELECT filename FROM files WHERE owner_id = (SELECT id FROM user WHERE name = '%s')", username);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    res = mysql_store_result(conn);

    char *files = NULL;
    if (res)
    {
        while ((row = mysql_fetch_row(res)))
        {
            if (files == NULL)
            {
                files = strdup(row[0]);
            }
            else
            {
                char *temp = strdup(row[0]);
                files = realloc(files, strlen(files) + strlen(temp) + 2);
                strcat(files, "\n");
                strcat(files, temp);
                free(temp);
            }
        }
        mysql_free_result(res);
    }

    mysql_close(conn);

    return files;
}

int check_file_owner(char *filename, unsigned char *token)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    const char *server = SERVER_URL;
    const char *user = SERVER_USER;
    const char *password = SERVER_PASSWORD;
    const char *database = SERVER_DATABASE;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, password, database, 3306, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return 0;
    }

    char *username = NULL;
    char *token_str = (char *)token;
    char query[1024];
    snprintf(query, 1024, "SELECT name FROM user WHERE id = (SELECT user_id FROM tokens WHERE token = '%s')", token_str);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res)
    {
        while ((row = mysql_fetch_row(res)))
        {
            username = strdup(row[0]);
        }
        mysql_free_result(res);
    }

    if (username == NULL)
    {
        return 0;
    }

    snprintf(query, 1024, "SELECT hashname FROM files WHERE owner_id = (SELECT id FROM user WHERE name = '%s') AND hashname = '%s'", username, filename);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);

    int is_owner = 0;
    if (res)
    {
        while ((row = mysql_fetch_row(res)))
        {
            is_owner = 1;
        }
        mysql_free_result(res);
    }

    mysql_close(conn);

    return is_owner;
}

unsigned char *login_user(char *username, char *password)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    const char *server = SERVER_URL;
    const char *user = SERVER_USER;
    const char *pass = SERVER_PASSWORD;
    const char *database = SERVER_DATABASE;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, pass, database, 3306, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    char query[1024];
    snprintf(query, 1024, "SELECT name,password FROM user WHERE name = '%s'", username);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    res = mysql_store_result(conn);
    if (res)
    {
        while ((row = mysql_fetch_row(res)))
        {
            if (verify_password(password, row[1]) == 0)
            {
                mysql_close(conn);
                return NULL;
            }
            if (strcmp(row[0], username) == 0)
            {
                unsigned char *token = generate_token(32);
                store_token(token, (unsigned char *)username);
                mysql_close(conn);
                return token;
            }
        }
        mysql_free_result(res);
    }

    mysql_close(conn);

    return NULL;
}
unsigned char *signup_user(char *username, char *password)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    const char *server = SERVER_URL;
    const char *user = SERVER_USER;
    const char *pass = SERVER_PASSWORD;
    const char *database = SERVER_DATABASE;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, pass, database, 3306, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    char query[1024];
    snprintf(query, 1024, "SELECT name FROM user WHERE name = '%s'", username);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    res = mysql_store_result(conn);

    if (res)
    {
        while ((row = mysql_fetch_row(res)))
        {
            if (strcmp(row[0], username) == 0)
            {
                mysql_close(conn);
                return NULL;
            }
        }
        mysql_free_result(res);
    }

    char *enc_pass = enc_password(password);
    if (enc_pass == NULL)
    {
        return NULL;
    }
    snprintf(query, 1024, "INSERT INTO user (name, password) VALUES ('%s', '%s')", username, enc_pass);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    unsigned char *token = generate_token(32);
    store_token(token, (unsigned char *)username);
    mysql_close(conn);

    return token;
}

char *generate_hashname(const char *filename)
{
    if (filename == NULL)
    {
        return NULL;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    unsigned long long timestamp = (unsigned long long)ts.tv_sec * 1000000000ULL + ts.tv_nsec;

    srand((unsigned int)ts.tv_nsec);
    int random_value = rand();

    char unique_input[512];
    snprintf(unique_input, sizeof(unique_input), "%s:%llu:%d", filename, timestamp, random_value);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)unique_input, strlen(unique_input), hash);

    char *hashname = (char *)malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    if (!hashname)
    {
        perror("Failed to allocate memory for hashname");
        return NULL;
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(hashname + (i * 2), "%02x", hash[i]);
    }
    hashname[SHA256_DIGEST_LENGTH * 2] = '\0';

    return hashname;
}

char *add_file(char *filename, unsigned char *token)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    const char *server = SERVER_URL;
    const char *user = SERVER_USER;
    const char *pass = SERVER_PASSWORD;
    const char *database = SERVER_DATABASE;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, pass, database, 3306, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    char *username = NULL;
    char *token_str = (char *)token;
    char query[1024];
    snprintf(query, 1024, "SELECT name FROM user WHERE id = (SELECT user_id FROM tokens WHERE token = '%s')", token_str);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    res = mysql_store_result(conn);
    if (res)
    {
        while ((row = mysql_fetch_row(res)))
        {
            username = strdup(row[0]);
        }
        mysql_free_result(res);
    }

    if (username == NULL)
    {
        return NULL;
    }

    char *hashname = generate_hashname(filename);
    printf("Hashname: %s\n", hashname);

    snprintf(query, 1024, "INSERT INTO files (filename, hashname, owner_id) VALUES ('%s', '%s', (SELECT id FROM user WHERE name = '%s'))", filename, hashname, username);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    mysql_close(conn);

    return hashname;
}

char *get_file(char *filename, unsigned char *token)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    const char *server = SERVER_URL;
    const char *user = SERVER_USER;
    const char *pass = SERVER_PASSWORD;
    const char *database = SERVER_DATABASE;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, pass, database, 3306, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    char *username = NULL;
    char *token_str = (char *)token;
    char query[1024];
    snprintf(query, 1024, "SELECT name FROM user WHERE id = (SELECT user_id FROM tokens WHERE token = '%s')", token_str);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    res = mysql_store_result(conn);
    if (res)
    {
        while ((row = mysql_fetch_row(res)))
        {
            username = strdup(row[0]);
        }
        mysql_free_result(res);
    }

    if (username == NULL)
    {
        return NULL;
    }

    snprintf(query, 1024, "SELECT hashname FROM files WHERE owner_id = (SELECT id FROM user WHERE name = '%s') AND filename = '%s'", username, filename);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    res = mysql_store_result(conn);

    char *server_filename = NULL;
    if (res)
    {
        while ((row = mysql_fetch_row(res)))
        {
            server_filename = strdup(row[0]);
        }
        mysql_free_result(res);
    }

    mysql_close(conn);

    return server_filename;
}

void clean_tokens()
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    const char *server = SERVER_URL;
    const char *user = SERVER_USER;
    const char *pass = SERVER_PASSWORD;
    const char *database = SERVER_DATABASE;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, pass, database, 3306, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return;
    }

    char query[1024];
    snprintf(query, 1024, "DELETE FROM tokens WHERE created_at < NOW() - INTERVAL 10 MINUTE");

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return;
    }

    mysql_close(conn);
}