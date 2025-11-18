#include "kvstore.h"

static char* str_copy(const char* s)
{
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* copy = malloc(len);
    if (copy) 
    {
        strcpy_s(copy, len, s);
    }
    return copy;
}

kv_store_t* kv_store_create(void)
{
    kv_store_t* store = malloc(sizeof(kv_store_t));
    if (!store) return NULL;

    store->pairs = NULL;
    store->count = 0;
    store->capacity = 0;

    store->pairs = malloc(INITIAL_CAPACITY * sizeof(kv_pair_t));
    if (!store->pairs) 
    {
        free(store);
        return NULL;
    }

    for (size_t i = 0; i < INITIAL_CAPACITY; ++i) 
    {
        store->pairs[i].key = NULL;
        store->pairs[i].value = NULL;
    }

    store->count = 0;
    store->capacity = INITIAL_CAPACITY;
    return store;
}

void kv_store_destroy(kv_store_t* store)
{
    if (!store) return;

    if (store->pairs) 
    {
        for (size_t i = 0; i < store->count; ++i) 
        {
            if (store->pairs[i].key) {
                free(store->pairs[i].key);
                store->pairs[i].key = NULL;
            }
            if (store->pairs[i].value) {
                free(store->pairs[i].value);
                store->pairs[i].value = NULL;
            }
        }
        free(store->pairs);
        store->pairs = NULL;
    }

    store->count = 0;
    store->capacity = 0;
    free(store);
}

static int find_index(kv_store_t* store, const char* key)
{
    if (!store || !key) return -1;
    for (size_t i = 0; i < store->count; ++i) 
    {
        if (store->pairs[i].key && strcmp(store->pairs[i].key, key) == 0) 
        {
            return (int)i;
        }
    }
    return -1;
}

static int resize(kv_store_t* store)
{
    if (!store) return 0;
    size_t new_capacity = store->capacity * 2;
    kv_pair_t* new_pairs = realloc(store->pairs, new_capacity * sizeof(kv_pair_t));
    if (!new_pairs) return 0;

    for (size_t i = store->capacity; i < new_capacity; ++i) 
    {
        new_pairs[i].key = NULL;
        new_pairs[i].value = NULL;
    }

    store->pairs = new_pairs;
    store->capacity = new_capacity;
    return 1;
}

int kv_store_set(kv_store_t* store, const char* key, const char* value)
{
    if (!store || !key || !value) return 0;

    int idx = find_index(store, key);
    if (idx >= 0) {
        if (store->pairs[idx].value) free(store->pairs[idx].value);
        store->pairs[idx].value = str_copy(value);
        return 1;
    }

    if (store->count >= store->capacity) 
    {
        if (!resize(store)) return 0;
    }

    store->pairs[store->count].key = str_copy(key);
    store->pairs[store->count].value = str_copy(value);

    if (!store->pairs[store->count].key || !store->pairs[store->count].value) 
    {
        if (store->pairs[store->count].key) 
        {
            free(store->pairs[store->count].key);
            store->pairs[store->count].key = NULL;
        }
        if (store->pairs[store->count].value) 
        {
            free(store->pairs[store->count].value);
            store->pairs[store->count].value = NULL;
        }
        return 0;
    }

    store->count++;
    return 1;
}

char* kv_store_get(kv_store_t* store, const char* key)
{
    if (!store || !key) return NULL;
    int idx = find_index(store, key);
    if (idx < 0 || !store->pairs[idx].value) return NULL;
    return str_copy(store->pairs[idx].value);
}

int kv_store_exists(kv_store_t* store, const char* key)
{
    return find_index(store, key) >= 0;
}

int kv_store_delete(kv_store_t* store, const char* key)
{
    if (!store || !key) return 0;
    int idx = find_index(store, key);
    if (idx < 0) return 0;

    if (store->pairs[idx].key) 
    {
        free(store->pairs[idx].key);
        store->pairs[idx].key = NULL;
    }
    if (store->pairs[idx].value) 
    {
        free(store->pairs[idx].value);
        store->pairs[idx].value = NULL;
    }

    for (size_t i = (size_t)idx; i < store->count - 1; ++i) 
    {
        store->pairs[i] = store->pairs[i + 1];
    }

    store->pairs[store->count - 1].key = NULL;
    store->pairs[store->count - 1].value = NULL;

    store->count--;
    return 1;
}

int kv_store_save_to_file(kv_store_t* store, const char* filename)
{
    if (!store || !filename) return 0;

    FILE* fp = fopen(filename, "w");
    if (!fp) return 0;

    for (size_t i = 0; i < store->count; ++i) 
    {
        if (store->pairs[i].key && store->pairs[i].value) 
        {
            if (strchr(store->pairs[i].key, '=') == NULL) 
            {
                fprintf(fp, "%s=%s\n", store->pairs[i].key, store->pairs[i].value);
            }
        }
    }

    fclose(fp);
    return 1;
}

int kv_store_load_from_file(kv_store_t* store, const char* filename)
{
    if (!store || !filename) return 0;

    FILE* fp = fopen(filename, "r");
    if (!fp) return 0;

    kv_store_destroy(store);

    store->pairs = malloc(INITIAL_CAPACITY * sizeof(kv_pair_t));
    if (!store->pairs) 
    {
        store->count = 0;
        store->capacity = 0;
        fclose(fp);
        return 0;
    }

    for (size_t i = 0; i < INITIAL_CAPACITY; ++i) 
    {
        store->pairs[i].key = NULL;
        store->pairs[i].value = NULL;
    }

    store->count = 0;
    store->capacity = INITIAL_CAPACITY;

    char line[2048];
    while (fgets(line, sizeof(line), fp)) 
    {
        size_t len = strlen(line);
        if (len == 0) continue;
        if (line[len - 1] == '\n') 
        {
            line[len - 1] = '\0';
        }

        char* eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char* key = line;
        char* value = eq + 1;

        kv_store_set(store, key, value);
    }

    fclose(fp);
    return 1;
}
int kv_store_keys(kv_store_t* store, char* buffer, size_t buffer_size)
{
    if (!store || !buffer) return 0;
    buffer[0] = '\0';

    for (size_t i = 0; i < store->count; ++i) 
    {
        if (i > 0) 
        {
            if (strlen(buffer) + 1 >= buffer_size) break;
            strcat_s(buffer, buffer_size, " ");
        }
        if (strlen(buffer) + strlen(store->pairs[i].key) >= buffer_size) break;
        strcat_s(buffer, buffer_size, store->pairs[i].key);
    }

    return 1;
}

void kv_store_flushdb(kv_store_t* store)
{
    if (!store) return;
    kv_store_destroy(store);
    store->pairs = malloc(INITIAL_CAPACITY * sizeof(kv_pair_t));
    if (!store->pairs) 
    {
        store->count = 0;
        store->capacity = 0;
        return;
    }
    for (size_t i = 0; i < INITIAL_CAPACITY; ++i) 
    {
        store->pairs[i].key = NULL;
        store->pairs[i].value = NULL;
    }
    store->count = 0;
    store->capacity = INITIAL_CAPACITY;
}

int kv_store_info(kv_store_t* store, char* buffer, size_t buffer_size, time_t start_time)
{
    if (!store || !buffer) return 0;
    time_t now = time(NULL);
    long uptime = (long)(now - start_time);

    snprintf(buffer, buffer_size,
        "keys=%zu\r\n"
        "connected_clients=1\r\n"
        "uptime=%ld seconds\r\n",
        store->count, uptime
    );
    return 1;
}
int kv_store_incr(kv_store_t* store, const char* key, long* result)
{
    if (!store || !key || !result) return 0;
    char* current = kv_store_get(store, key);
    long val = 0;
    if (current) 
    {
        char* end;
        val = strtol(current, &end, 10);
        if (*end != '\0') {
            free(current);
            return 0;
        }
        free(current);
    }
    val++;
    char new_value[64];
    sprintf(new_value, "%ld", val);
    if (!kv_store_set(store, key, new_value)) return 0;
    *result = val;
    return 1;
}

int kv_store_decr(kv_store_t* store, const char* key, long* result)
{
    if (!store || !key || !result) return 0;
    char* current = kv_store_get(store, key);
    long val = 0;
    if (current)
    {
        char* end;
        val = strtol(current, &end, 10);
        if (*end != '\0') 
        {
            free(current);
            return 0;
        }
        free(current);
    }
    val--;
    char new_value[64];
    sprintf(new_value, "%ld", val);
    if (!kv_store_set(store, key, new_value)) return 0;
    *result = val;
    return 1;
}

int kv_store_append(kv_store_t* store, const char* key, const char* value)
{
    if (!store || !key || !value) return 0;
    char* current = kv_store_get(store, key);
    if (!current) 
    {
        return kv_store_set(store, key, value);
    }
    size_t new_len = strlen(current) + strlen(value) + 1;
    char* new_value = malloc(new_len);
    if (!new_value) 
    {
        free(current);
        return 0;
    }
    strcpy_s(new_value, new_len, current);
    strcat_s(new_value, new_len, value);
    free(current);
    int ok = kv_store_set(store, key, new_value);
    free(new_value);
    return ok;
}

int kv_store_rename(kv_store_t* store, const char* old_key, const char* new_key)
{
    if (!store || !old_key || !new_key) return 0;
    if (strcmp(old_key, new_key) == 0) return 1;
    if (!kv_store_exists(store, old_key)) return 0;
    if (kv_store_exists(store, new_key)) {
        kv_store_delete(store, new_key);
    }
    char* value = kv_store_get(store, old_key);
    if (!value) return 0;
    int ok = kv_store_set(store, new_key, value);
    free(value);
    if (ok) 
    {
        kv_store_delete(store, old_key);
    }
    return ok;
}

char* kv_store_getset(kv_store_t* store, const char* key, const char* new_value)
{
    if (!store || !key || !new_value) return NULL;
    char* old_value = kv_store_get(store, key);
    if (!kv_store_set(store, key, new_value))
    {
        free(old_value);
        return NULL;
    }
    return old_value;
}

const char* kv_store_type(kv_store_t* store, const char* key)
{
    if (!store || !key) return NULL;
    return kv_store_exists(store, key) ? "string" : NULL;
}