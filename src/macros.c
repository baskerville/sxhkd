#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "macros.h"

#define MAX_CHUNKS 32
#define CHUNK_MAX_LEN 255

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif

#define safe_strlen(x) (((x)==NULL)?0:strlen((x)))

// Text chunks records {{{
// States of the FSM for transforming a string using a set of macros.
enum PARSE_STATE {
    NORMAL,         // before {{macro}} 
    MACRO,          // inside {{macro}}
    POST_MACRO,     // after {{macro}}
};

// Text chunks for text transform.
typedef struct{
    int len;
    char *with;
} txt_chunk;
// }}}

// Hash Table for Macros {{{
// MurMur3 hash function
// Adapted from:
// https://github.com/stephane-martin/cycapture/blob/master/cycapture/murmur.c
// Credit: Stephane Martin
static FORCE_INLINE uint32_t hash(const void *data, size_t nbytes) {
    if (data == NULL || nbytes == 0)
        return 0;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    const int nblocks = nbytes / 4;
    const uint32_t *blocks = (const uint32_t *) (data);
    const uint8_t *tail = (const uint8_t *) data + nblocks * 4;

    uint32_t h = 0;

    int i;
    uint32_t k;
    for (i = 0; i < nblocks; i++) {
        k = blocks[i];

        k *= c1;
        k = (k << 15) | (k >> (32 - 15));
        k *= c2;

        h ^= k;
        h = (h << 13) | (h >> (32 - 13));
        h = (h * 5) + 0xe6546b64;
    }

    k = 0;
    switch (nbytes & 3) {
        case 3:
            k ^= tail[2] << 16;
        case 2:
            k ^= tail[1] << 8;
        case 1:
            k ^= tail[0];
            k *= c1;
            k = (k << 15) | (k >> (32 - 15));
            k *= c2;
            h ^= k;
    };

    h ^= nbytes;

    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

static FORCE_INLINE uint32_t hash_to_index(const ht_t *ht, uint32_t h) {
    return h % ht->size;
}

// Trim a string left and right and return its new length
static FORCE_INLINE size_t str_trim(char *src){
    size_t i = 0;
    size_t ts = 0;
    size_t j = safe_strlen(src) - 1;
    if(src == NULL || j <= 0)
        return 0;
    while(j > 0 && isspace(src[j])) j--;
    while(i <= j && isspace(src[i])) i++;
    ts = j - i + 1;
    memcpy(src, src+i, ts);
    src[ts] = '\0';
    return ts;
}

ht_t *ht_create(const size_t size) {
    ht_t *ht = malloc(sizeof(ht_t));
    if(ht == NULL)
        return ht;
    ht->size = size;
    ht->entries = calloc(size, sizeof(macro_t));
    if(ht->entries == NULL){
        free(ht);
        return NULL;
    }
    return ht;
}

void ht_set(const ht_t *ht, const void *key, const size_t k_len, const void *value, const size_t v_len) {
    uint32_t h = 0;
    size_t slot = 0;
    size_t i = 0;

    if(ht == NULL || key == NULL || k_len == 0)
        return;

    h = hash(key, k_len);

    for (i = 0; i < ht->size; i++){
        slot = hash_to_index(ht, (h+1));
        if(ht->entries[slot].key == NULL){
            ht->entries[slot].key = malloc(k_len);
            ht->entries[slot].value = malloc(v_len);
            memcpy(ht->entries[slot].key, key, k_len);
            memcpy(ht->entries[slot].value, value, v_len);
            ht->entries[slot].k_len = k_len;
            ht->entries[slot].v_len = v_len;
            break;
        } else if(k_len == ht->entries[slot].k_len){
            if(memcmp(ht->entries[slot].key, key, k_len) == 0){
                free(ht->entries[slot].value);
                ht->entries[slot].value = malloc(v_len);
                memcpy(ht->entries[slot].value, value, v_len);
                ht->entries[slot].k_len = k_len;
                ht->entries[slot].v_len = v_len;
                break;
            }
        }
    }
}

void *ht_get(const ht_t *ht, const void *key, const size_t k_len) {
    uint32_t h = 0;
    size_t slot = 0;

    if(ht == NULL || key == NULL || k_len == 0)
        return NULL;

    h = hash(key, k_len);

    for (size_t i = 0; i < ht->size; i++){
        slot = hash_to_index(ht, (h+1));
        if(ht->entries[slot].key == NULL){
            return NULL;
        } else if(k_len == ht->entries[slot].k_len && memcmp(ht->entries[slot].key, key, k_len) == 0){
            return ht->entries[slot].value;
        }
    }
    return NULL;
}

void ht_del(const ht_t *ht, const void *key, const size_t k_len) {
    uint32_t h = 0;
    size_t slot = 0;

    if(ht == NULL || key == NULL || k_len == 0)
        return;

    h = hash(key, k_len);

    for (size_t i = 0; i < ht->size; i++){
        slot = hash_to_index(ht, (h+1));
        if(ht->entries[slot].key == NULL){
            break;
        } else if(k_len == ht->entries[slot].k_len && memcmp(ht->entries[slot].key, key, k_len) == 0){
            free(ht->entries[slot].key);
            free(ht->entries[slot].value);
            ht->entries[slot].key = NULL;
            ht->entries[slot].value = NULL;
            ht->entries[slot].k_len = 0;
            ht->entries[slot].v_len = 0;
            break;
        }
    }
}

void ht_destroy(ht_t *ht) {

    if(ht == NULL)
        return;

    for(size_t i=0; i < ht->size; i++){
        if(ht->entries[i].key != NULL){
            free(ht->entries[i].key);
            free(ht->entries[i].value);
            ht->entries[i].key = NULL;
            ht->entries[i].value = NULL;
        }
    }

    free(ht->entries);
    free(ht);
}

// }}}

// {{{ Macros Processor
char *ht_process_macros(const ht_t *ht, const char * restrict src){
    enum PARSE_STATE state = NORMAL;
    size_t i = 0; 
    size_t last_normal = 0;
    size_t txt_len = 0;
    size_t def_len = 0;
    size_t result_len = 0;
    size_t with_len = 0;
    size_t trimmed_len = 0;
    size_t ch_count = 0;
    size_t src_len = safe_strlen(src);
    txt_chunk chunks[MAX_CHUNKS];
    char def[CHUNK_MAX_LEN] = {0};
    char *result = NULL;

    if(src_len == 0){
        // Return an empty string anyways.
        result = malloc(1);
        result[0] = '\0';
        return result;
    }

    if(ht == NULL){
        // No ht, return a copy of src.
        result = malloc(src_len + 1);
        if(result != NULL){
            memcpy(result, src, src_len);
            result[src_len] = '\0';
        }
        return result;
    }

    while(i < src_len){
        switch (state) {
            case NORMAL:
                if(i >= 1 && src[i] == '{' && src[i-1] == '{')
                    state = MACRO;
                break;
            case MACRO:
                if(src[i] == '}'){
                    state = POST_MACRO;
                } else if (src[i] != '{'){
                    def[def_len++] = src[i];
                }
                break;
            case POST_MACRO:
                if(src[i] == '}'){
                    def[def_len] = '\0';
                    txt_len = i - (def_len + 3) - last_normal;
                    if(txt_len > 0){
                        chunks[ch_count].len = txt_len;
                        chunks[ch_count].with = (char*)src+last_normal;
                        ch_count++;
                        result_len += txt_len;
                    }
                    // check whether def is a defined macro.
                    trimmed_len = str_trim(def);
                    char *with = ht_get(ht, def, trimmed_len + 1);
                    // Save the macro expansion chunk, if any.
                    if(with != NULL){
                        with_len = safe_strlen(with);
                        chunks[ch_count].len = with_len;
                        chunks[ch_count].with = with;
                        ch_count++;
                        last_normal = i+1;
                        result_len += with_len;
                    }
                } else {
                    def[0] = '\0';
                }
                def_len = 0;
                state = NORMAL;
                break;
        }
        i++;
    }

    // Append a last chunk if last_normal did not reach the end
    txt_len = src_len - last_normal;
    if(txt_len > 0){
        chunks[ch_count].len = txt_len;
        chunks[ch_count].with = (char*)src+last_normal;
        ch_count++;
        result_len += txt_len;
    }

    result = malloc(result_len + 1);

    if(result != NULL){
        result[0] = '\0';
        txt_chunk chunk ;
        for(size_t k = 0; k < ch_count; k++){
            chunk = chunks[k];
            strncat(result, chunk.with, chunk.len);
        }
        result[result_len] = '\0';
    }

    return result;
}
//}}}

// {{{ Insert processed macro in the macros Hash Table.
void ht_set_processed(const ht_t *ht, const char *key, const char *value) {
    char *processed = ht_process_macros(ht, value);
    ht_set(ht, key, safe_strlen(key)+1, processed, safe_strlen(processed)+1);
    free(processed);
}
// }}}
