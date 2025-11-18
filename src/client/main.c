#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>
#include "../core/kvstore.h"
static time_t program_start_time = 0;
int parse_args(char* line, char* args[], int max_args);

int main(void)
{
	setlocale(LC_ALL, "Russian_Russia.1251");
	program_start_time = time(NULL);
	system("chcp 1251 > nul");

	kv_store_t* db = kv_store_create();
	if (!db)
	{
		printf("Error: Failed to create database.\n");
		return 1;
	}
	if (kv_store_load_from_file(db, "kvlite.db"))
	{
		printf("[Downloaded data from kvlite.db]\n");
	}
	printf("KVlite v1.0.0 - In-memory Key-Value Store (C)\n");
	printf("------------------------------------------------------------\n");

	char line[1024];
	while (1) {
		printf("KVlite> ");
		if (!fgets(line, sizeof(line), stdin)) break;

		size_t len = strlen(line);
		if (len > 0 && line[len - 1] == '\n')
		{
			line[len - 1] = '\0';
		}
		if (len == 0) continue;

		char* cmd_end = strchr(line, ' ');
		if (!cmd_end)
		{
			cmd_end = line + strlen(line);
		}
		char cmd_copy[64];
		strncpy_s(cmd_copy, sizeof(cmd_copy), line, cmd_end - line);
		cmd_copy[cmd_end - line] = '\0';

		char* args = cmd_end;
		while (*args == ' ') args++;

		if (strcmp(cmd_copy, "exit") == 0)
		{
			break;
		}
		else if (strcmp(cmd_copy, "SET") == 0)
		{
			char* key_end = strchr(args, ' ');
			if (!key_end)
			{
				printf("Error: SET requires key and value.\n");
				continue;
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
				printf("Error: SET requires key and value.\n");
				continue;
			}

			char safe_value[1024];
			strncpy_s(safe_value, sizeof(safe_value), value, sizeof(safe_value) - 1);
			safe_value[sizeof(safe_value) - 1] = '\0';

			if (kv_store_set(db, key, safe_value))
			{
				printf("OK\n");
			}
			else
			{
				printf("Error: Failed to set key-value pair.\n");
			}
		}
		else if (strcmp(cmd_copy, "GET") == 0)
		{
			if (strlen(args) == 0)
			{
				printf("Error: GET requires key.\n");
				continue;
			}
			char* value = kv_store_get(db, args);
			if (value)
			{
				printf("%s\n", value);
				free(value);
			}
			else
			{
				printf("Key not found.\n");
			}
		}
		else if (strcmp(cmd_copy, "DELETE") == 0)
		{
			if (strlen(args) == 0)
			{
				printf("Error: DELETE requires a key.\n");
				continue;
			}
			if (kv_store_delete(db, args))
			{
				printf("OK\n");
			}
			else {
				printf("Key not found.\n");
			}
		}
		else if (strcmp(cmd_copy, "EXISTS") == 0)
		{
			if (strlen(args) == 0)
			{
				printf("Error: EXISTS requires a key.\n");
				continue;
			}
			if (kv_store_exists(db, args))
			{
				printf("true\n");
			}
			else {
				printf("false\n");
			}
		}
		else if (strcmp(cmd_copy, "SAVE") == 0)
		{
			if (strlen(args) == 0)
			{
				printf("Error: SAVE requires a filename.\n");
				continue;
			}
			if (kv_store_save_to_file(db, args))
			{
				printf("OK\n");
			}
			else
			{
				printf("Error: Failed to save file.\n");
			}
		}
		else if (strcmp(cmd_copy, "LOAD") == 0)
		{
			if (strlen(args) == 0)
			{
				printf("Error: LOAD requires filename.\n");
				continue;
			}
			if (kv_store_load_from_file(db, args))
			{
				printf("OK\n");
			}
			else
			{
				printf("Error: Failed to load file.\n");
			}
		}
		else if (strcmp(cmd_copy, "KEYS") == 0)
		{
			char keys_buffer[4096] = "";
			if (kv_store_keys(db, keys_buffer, sizeof(keys_buffer)))
			{
				if (strlen(keys_buffer) == 0)
				{
					printf("(empty)\n");
				}
				else {
					printf("%s\n", keys_buffer);
				}
			}
			else {
				printf("Error: Failed to get keys\n");
			}
		}
		else if (strcmp(cmd_copy, "FLUSHDB") == 0)
		{
			kv_store_flushdb(db);
			printf("OK\n");
		}
		else if (strcmp(cmd_copy, "INFO") == 0)
		{
			char info_buffer[1024] = "";
			if (kv_store_info(db, info_buffer, sizeof(info_buffer), program_start_time))
			{
				printf("%s\n", info_buffer);
			}
			else {
				printf("Error: Failed to get information\n");
			}
		}
		else if (strcmp(cmd_copy, "INCR") == 0)
		{
			if (strlen(args) == 0)
			{
				printf("Error: INCR requires key.\n");
				continue;
			}
			long result;
			if (kv_store_incr(db, args, &result))
			{
				printf("%ld\n", result);
			}
			else
			{
				printf("Error: value is not an integer.\n");
			}
		}
		else if (strcmp(cmd_copy, "DECR") == 0)
		{
			if (strlen(args) == 0)
			{
				printf("Error: DECR requires a key.\n");
				continue;
			}
			long result;
			if (kv_store_decr(db, args, &result))
			{
				printf("%ld\n", result);
			}
			else
			{
				printf("Error: value is not an integer.\n");
			}
		}
		else if (strcmp(cmd_copy, "APPEND") == 0)
		{
			char* key_end = strchr(args, ' ');
			if (!key_end)
			{
				printf("Error: APPEND requires key and value.\n");
				continue;
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
				printf("Error: APPEND requires key and value.\n");
				continue;
			}

			char safe_value[1024];
			strncpy_s(safe_value, sizeof(safe_value), value, sizeof(safe_value) - 1);
			safe_value[sizeof(safe_value) - 1] = '\0';

			if (kv_store_append(db, key, safe_value))
			{
				printf("OK\n");
			}
			else
			{
				printf("Error: Failed to add.\n");
			}
		}
		else if (strcmp(cmd_copy, "RENAME") == 0)
		{
			char* old_key_end = strchr(args, ' ');
			if (!old_key_end)
			{
				printf("Error: RENAME requires old and new key.\n");
				continue;
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
				printf("Error: RENAME requires old and new key.\n");
				continue;
			}

			if (kv_store_rename(db, old_key, new_key)) 
			{
				printf("OK\n");
			}
			else 
			{
				printf("Error: Failed to rename.\n");
			}
		}
		else if (strcmp(cmd_copy, "GETSET") == 0) 
		{
			char* key_end = strchr(args, ' ');
			if (!key_end) 
			{
				printf("Error: GETSET requires key and value.\n");
				continue;
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
				printf("Error: GETSET requires key and value.\n");
				continue;
			}

			char safe_value[1024];
			strncpy_s(safe_value, sizeof(safe_value), value, sizeof(safe_value) - 1);
			safe_value[sizeof(safe_value) - 1] = '\0';

			char* old_value = kv_store_getset(db, key, safe_value);
			if (old_value) 
			{
				printf("%s\n", old_value);
				free(old_value);
			}
			else 
			{
				printf("Error: GETSET failed.\n");
			}
		}
		else if (strcmp(cmd_copy, "TYPE") == 0) 
		{
			if (strlen(args) == 0) 
			{
				printf("Error: TYPE requires a key.\n");
				continue;
			}
			const char* type = kv_store_type(db, args);
			if (type) 
			{
				printf("%s\n", type);
			}
			else 
			{
				printf("none\n");
			}
		}
		else if (strcmp(cmd_copy, "ECHO") == 0) 
		{
			if (strlen(args) == 0) 
			{
				printf("Error: ECHO requires a message.\n");
				continue;
			}
			printf("%s\n", args);
		}
		else if (strcmp(cmd_copy, "PING") == 0) 
		{
			printf("PONG\n");
		}
		else 
		{
			printf("Unknown command: %s\n", cmd_copy);
		}

	}

	kv_store_destroy(db);
	printf("Bye!\n");
	return 0;
}
int parse_args(char* line, char* args[], int max_args) 
{
	if (!line || !args) return 0;
	int count = 0;
	char* p = line;

	while (*p && count < max_args) 
	{
		while (*p == ' ') p++;
		if (*p == '\0') break;

		char quote = '\0';
		if (*p == '"' || *p == '\'') 
		{
			quote = *p;
			p++;
		}

		args[count] = p;
		count++;

		while (*p && (quote ? *p != quote : *p != ' ')) 
		{
			p++;
		}

		if (quote && *p == quote) 
		{
			*p = '\0';
			p++;
		}
		else if (*p == ' ') 
		{
			*p = '\0';
			p++;
		}
	}
	return count;
}