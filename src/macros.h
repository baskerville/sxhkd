#ifndef MACROS_H
#define MACROS_H

// Macros Hash Table
typedef struct {
	void *key;
	void *value;
	size_t k_len;
	size_t v_len;
} macro_t;

// A Hash Table is a dynamically allocated array of macros.
typedef struct{
	macro_t *entries;
	size_t size;
} ht_t;

ht_t *ht_create(const size_t size);
void ht_set(const ht_t *ht, const void *key, const size_t k_len, const void *value, const size_t v_len);
void *ht_get(const ht_t *ht, const void *key, const size_t k_len);
void ht_del(const ht_t *ht, const void *key, const size_t k_len);
void ht_destroy(ht_t *ht);
char *ht_process_macros(const ht_t *ht, const char *src);
void ht_set_processed(const ht_t *ht, const char *key, const char *value);

#endif /* ifndef MACROS_H */
