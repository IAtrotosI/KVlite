#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <locale.h>
#include <time.h>
#include <ws2tcpip.h>
#include "../core/kvstore.h"

#pragma comment(lib, "ws2_32.lib")
static time_t server_start_time = 0;
#define PORT 6379
#define BUFFER_SIZE 2048

int handle_command(kv_store_t* db, char* request, char* response, size_t response_size)
{
        if (!request || !response) return 0;

        size_t len = strlen(request);
        while (len > 0 && (request[len - 1] == '\n' || request[len - 1] == '\r')) 
        {
            request[--len] = '\0';
        }
        if (len == 0) 
        {
            strcpy_s(response, response_size, "ERR empty command\n");
            return 1;
        }

        char* cmd_end = strchr(request, ' ');
        if (!cmd_end) 
        {
            cmd_end = request + len;
        }
        char cmd[64];
        size_t cmd_len = cmd_end - request;
        if (cmd_len >= sizeof(cmd)) cmd_len = sizeof(cmd) - 1;
        memcpy(cmd, request, cmd_len);
        cmd[cmd_len] = '\0';

        char* args = cmd_end;
        while (*args == ' ') args++;

        if (strcmp(cmd, "SET") == 0) 
        {
            if (strlen(args) == 0) 
            {
                strcpy_s(response, response_size, "ERR SET requires key and value\n");
                return 1;
            }

            char* key_end = strchr(args, ' ');
            if (!key_end) 
            {
                strcpy_s(response, response_size, "ERR SET requires key and value\n");
                return 1;
            }

            char key[256];
            size_t key_len = key_end - args;
            if (key_len >= sizeof(key)) key_len = sizeof(key) - 1;
            memcpy(key, args, key_len);
            key[key_len] = '\0';

            char* value = key_end + 1;
            while (*value == ' ') value++;

            if (strlen(key) == 0 || strlen(value) == 0) 
            {
                strcpy_s(response, response_size, "ERR SET requires key and value\n");
                return 1;
            }

            char safe_value[1024];
            strncpy_s(safe_value, sizeof(safe_value), value, sizeof(safe_value) - 1);
            safe_value[sizeof(safe_value) - 1] = '\0';

            if (kv_store_set(db, key, safe_value))
            {
                strcpy_s(response, response_size, "OK\n");
            }
            else 
            {
                strcpy_s(response, response_size, "ERR failed to set\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "GET") == 0) 
        {
            if (strlen(args) == 0) 
            {
                strcpy_s(response, response_size, "ERR GET requires key\n");
                return 1;
            }
            char* value = kv_store_get(db, args);
            if (value) 
            {
                strcpy_s(response, response_size, value);
                strcat_s(response, response_size, "\n");
                free(value);
            }
            else 
            {
                strcpy_s(response, response_size, "NOT_FOUND\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "DELETE") == 0) 
        {
            if (strlen(args) == 0) 
            {
                strcpy_s(response, response_size, "ERR DELETE requires key\n");
                return 1;
            }
            if (kv_store_delete(db, args)) 
            {
                strcpy_s(response, response_size, "OK\n");
            }
            else 
            {
                strcpy_s(response, response_size, "NOT_FOUND\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "EXISTS") == 0) 
        {
            if (strlen(args) == 0) 
            {
                strcpy_s(response, response_size, "ERR EXISTS requires key\n");
                return 1;
            }
            if (kv_store_exists(db, args)) 
            {
                strcpy_s(response, response_size, "true\n");
            }
            else 
            {
                strcpy_s(response, response_size, "false\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "SAVE") == 0) 
        {
            if (strlen(args) == 0) 
            {
                strcpy_s(response, response_size, "ERR SAVE requires filename\n");
                return 1;
            }
            if (kv_store_save_to_file(db, args)) 
            {
                strcpy_s(response, response_size, "OK\n");
            }
            else 
            {
                strcpy_s(response, response_size, "ERR save failed\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "LOAD") == 0) 
        {
            if (strlen(args) == 0) 
            {
                strcpy_s(response, response_size, "ERR LOAD requires filename\n");
                return 1;
            }
            if (kv_store_load_from_file(db, args))
            {
                strcpy_s(response, response_size, "OK\n");
            }
            else
            {
                strcpy_s(response, response_size, "ERR load failed\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "KEYS") == 0) 
        {
            char keys_buffer[4096] = "";
            if (kv_store_keys(db, keys_buffer, sizeof(keys_buffer))) 
            {
                if (strlen(keys_buffer) == 0) 
                {
                    strcpy_s(response, response_size, "(empty)\n");
                }
                else 
                {
                    strcpy_s(response, response_size, keys_buffer);
                    strcat_s(response, response_size, "\n");
                }
            }
            else 
            {
                strcpy_s(response, response_size, "ERR keys failed\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "FLUSHDB") == 0) 
        {
            kv_store_flushdb(db);
            strcpy_s(response, response_size, "OK\n");
            return 1;
        }
        else if (strcmp(cmd, "INFO") == 0) 
        {
            char info_buffer[1024] = "";
            time_t now = time(NULL);
            long uptime = (long)(now - server_start_time);

            snprintf(info_buffer, sizeof(info_buffer),
                "keys=%zu\r\n"
                "connected_clients=1\r\n"
                "uptime=%ld seconds",
                db->count, uptime
            );

            strcpy_s(response, response_size, info_buffer);
            strcat_s(response, response_size, "\r\n");
            return 1;
        }
        else if (strcmp(cmd, "INCR") == 0) 
        {
            if (strlen(args) == 0)
            {
                strcpy_s(response, response_size, "ERR INCR requires key\n");
                return 1;
            }
            long result;
            if (kv_store_incr(db, args, &result))
            {
                char res_str[32];
                sprintf(res_str, "%ld\n", result);
                strcpy_s(response, response_size, res_str);
            }
            else 
            {
                strcpy_s(response, response_size, "ERR value is not an integer\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "DECR") == 0)
        {
            if (strlen(args) == 0) 
            {
                strcpy_s(response, response_size, "ERR DECR requires key\n");
                return 1;
            }
            long result;
            if (kv_store_decr(db, args, &result)) 
            {
                char res_str[32];
                sprintf(res_str, "%ld\n", result);
                strcpy_s(response, response_size, res_str);
            }
            else 
            {
                strcpy_s(response, response_size, "ERR value is not an integer\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "APPEND") == 0)
        {
            if (strlen(args) == 0) 
            {
                strcpy_s(response, response_size, "ERR APPEND requires key and value\n");
                return 1;
            }

            char* key_end = strchr(args, ' ');
            if (!key_end)
            {
                strcpy_s(response, response_size, "ERR APPEND requires key and value\n");
                return 1;
            }

            char key[256];
            size_t key_len = key_end - args;
            if (key_len >= sizeof(key)) key_len = sizeof(key) - 1;
            memcpy(key, args, key_len);
            key[key_len] = '\0';

            char* value = key_end + 1;
            while (*value == ' ') value++;

            if (strlen(key) == 0 || strlen(value) == 0)
            {
                strcpy_s(response, response_size, "ERR APPEND requires key and value\n");
                return 1;
            }

            char safe_value[1024];
            strncpy_s(safe_value, sizeof(safe_value), value, sizeof(safe_value) - 1);
            safe_value[sizeof(safe_value) - 1] = '\0';

            if (kv_store_append(db, key, safe_value)) 
            {
                strcpy_s(response, response_size, "OK\n");
            }
            else 
            {
                strcpy_s(response, response_size, "ERR append failed\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "RENAME") == 0) 
        {
            char* old_key_end = strchr(args, ' ');
            if (!old_key_end) 
            {
                strcpy_s(response, response_size, "ERR RENAME requires old_key and new_key\n");
                return 1;
            }

            char old_key[256];
            size_t old_key_len = old_key_end - args;
            if (old_key_len >= sizeof(old_key)) old_key_len = sizeof(old_key) - 1;
            memcpy(old_key, args, old_key_len);
            old_key[old_key_len] = '\0';

            char* new_key = old_key_end + 1;
            while (*new_key == ' ') new_key++;

            if (strlen(old_key) == 0 || strlen(new_key) == 0)
            {
                strcpy_s(response, response_size, "ERR RENAME requires old_key and new_key\n");
                return 1;
            }

            if (kv_store_rename(db, old_key, new_key))
            {
                strcpy_s(response, response_size, "OK\n");
            }
            else
            {
                strcpy_s(response, response_size, "ERR rename failed\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "GETSET") == 0)
        {
            if (strlen(args) == 0)
            {
                strcpy_s(response, response_size, "ERR GETSET requires key and value\n");
                return 1;
            }

            char* key_end = strchr(args, ' ');
            if (!key_end)
            {
                strcpy_s(response, response_size, "ERR GETSET requires key and value\n");
                return 1;
            }

            char key[256];
            size_t key_len = key_end - args;
            if (key_len >= sizeof(key)) key_len = sizeof(key) - 1;
            memcpy(key, args, key_len);
            key[key_len] = '\0';

            char* value = key_end + 1;
            while (*value == ' ') value++;

            if (strlen(key) == 0 || strlen(value) == 0)
            {
                strcpy_s(response, response_size, "ERR GETSET requires key and value\n");
                return 1;
            }

            char safe_value[1024];
            strncpy_s(safe_value, sizeof(safe_value), value, sizeof(safe_value) - 1);
            safe_value[sizeof(safe_value) - 1] = '\0';

            char* old_value = kv_store_getset(db, key, safe_value);
            if (old_value)
            {
                strcpy_s(response, response_size, old_value);
                strcat_s(response, response_size, "\n");
                free(old_value);
            }
            else {
                strcpy_s(response, response_size, "ERR getset failed\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "TYPE") == 0) 
        {
            if (strlen(args) == 0) 
            {
                strcpy_s(response, response_size, "ERR TYPE requires key\n");
                return 1;
            }
            const char* type = kv_store_type(db, args);
            if (type)
            {
                strcpy_s(response, response_size, type);
                strcat_s(response, response_size, "\n");
            }
            else
            {
                strcpy_s(response, response_size, "none\n");
            }
            return 1;
        }
        else if (strcmp(cmd, "ECHO") == 0)
        {
            strcpy_s(response, response_size, args);
            strcat_s(response, response_size, "\n");
            return 1;
        }
        else if (strcmp(cmd, "PING") == 0)
        {
            strcpy_s(response, response_size, "PONG\n");
            return 1;
        }
        else if (strcmp(cmd, "QUIT") == 0)
        {
            strcpy_s(response, response_size, "BYE\n");
            return -1;
        }
        else 
        {
            strcpy_s(response, response_size, "ERR unknown command\n");
            return 1;
        }
    }

int main(void)
{
	setlocale(LC_ALL, "Russian_Russia.1251");
	system("chcp 1251 > nul");
	time_t server_start_time = time(NULL);
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
		printf("Error: WSAStartup failed\n");
		return 1;
	}

	SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == INVALID_SOCKET) 
    {
		printf("Error: Failed to create socket\n");
		WSACleanup();
		return 1;
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) 
    {
		printf("Error: bind failed\n");
		closesocket(server_socket);
		WSACleanup();
		return 1;
	}

	if (listen(server_socket, 1) == SOCKET_ERROR) 
    {
		printf("Error: listen failed\n");
		closesocket(server_socket);
		WSACleanup();
		return 1;
	}

	printf("KVlite the server is running on the port %d\n", PORT);
	printf("Waiting for connections...\n");

	kv_store_t* db = kv_store_create();
	if (!db) 
    {
		printf("Error: Failed to create database\n");
		closesocket(server_socket);
		WSACleanup();
		return 1;
	}

	kv_store_load_from_file(db, "kvlite.db");

	while (1)
    {
		SOCKET client_socket = accept(server_socket, NULL, NULL);
		if (client_socket == INVALID_SOCKET)
        {
			printf("Error: accept failed\n");
			break;
		}

		printf("The client has connected.\n");

		char buffer[BUFFER_SIZE];
		char response[BUFFER_SIZE];

		while (1) 
        {
			int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
			if (bytes <= 0) 
            {
				break;
			}
			buffer[bytes] = '\0';

			int result = handle_command(db, buffer, response, sizeof(response));
			if (result == -1) 
            {
				send(client_socket, response, (int)strlen(response), 0);
				break;
			}
			if (result == 1) 
            {
				send(client_socket, response, (int)strlen(response), 0);
			}
		}

		closesocket(client_socket);
		printf("The client has disconnected.\n");
	}

	kv_store_destroy(db);
	closesocket(server_socket);
	WSACleanup();
	printf("The server is stopped.\n");
	return 0;
}