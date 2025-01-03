CC = gcc
CFLAGS = -Wall -Iutils/param -Iutils/file -Iutils/key_manager -Ilibs -I/usr/include/openssl
LDFLAGS = -lssl -lcrypto -lmysqlclient

SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = libs
UTILS_PARAM_DIR = utils/param
UTILS_FILE_DIR = utils/file
UTILS_KEY_MANAGER_DIR = utils/key_manager
UTILS_DATABASE_DIR = utils/database

SRCS = $(SRC_DIR)/sectrans.c $(SRC_DIR)/server.c $(UTILS_PARAM_DIR)/param.c $(UTILS_FILE_DIR)/file.c $(UTILS_KEY_MANAGER_DIR)/key_manager.c $(UTILS_DATABASE_DIR)/database.c
CLIENT_OBJS = $(OBJ_DIR)/sectrans.o $(OBJ_DIR)/param.o $(OBJ_DIR)/file.o $(OBJ_DIR)/key_manager.o $(OBJ_DIR)/database.o
SERVER_OBJS = $(OBJ_DIR)/server.o $(OBJ_DIR)/param.o $(OBJ_DIR)/file.o $(OBJ_DIR)/key_manager.o $(OBJ_DIR)/database.o

CLIENT_TARGET = sectrans
SERVER_TARGET = server

# Default target to build both client and server
all: $(CLIENT_TARGET) $(SERVER_TARGET)

# Rule for building the client target
$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CLIENT_OBJS) -o $(CLIENT_TARGET) -L$(LIB_DIR) -lclient -lserver -Wl,-rpath=$(LIB_DIR) $(LDFLAGS)

# Rule for building the server target
$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(SERVER_OBJS) -o $(SERVER_TARGET) -L$(LIB_DIR) -lclient -lserver -Wl,-rpath=$(LIB_DIR) $(LDFLAGS)

# Rule to compile .c files to .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/param.o: $(UTILS_PARAM_DIR)/param.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/file.o: $(UTILS_FILE_DIR)/file.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/key_manager.o: $(UTILS_KEY_MANAGER_DIR)/key_manager.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/database.o: $(UTILS_DATABASE_DIR)/database.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build artifacts
clean:
	rm -rf $(OBJ_DIR) $(CLIENT_TARGET) $(SERVER_TARGET)

.PHONY: all clean

# clean all data files
clean_data:
	rm -rf client_files/*
	rm -rf server_files/*
	rm -rf client_key.bin
	rm -rf signatures/*
	rm -rf encrypted_file.bin
	rm -rf user_token.bin