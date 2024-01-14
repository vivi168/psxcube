#include "stdafx.h"

#define DEFAULT_BUCKET_SIZE 11
#define SEED 1234
#define KEY_SIZE 20

static int
hash_strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == 0)
			return (0);
	return (*(unsigned char *)s1 - *(unsigned char *)--s2);
}

static void appendToBucket(Bucket* bucket, STRING20 key, void* value);

static inline uint32_t murmur_32_scramble(uint32_t k);
static uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed);

void hash_initHashMap(Hashmap* hash)
{
    hash->bucket_size = DEFAULT_BUCKET_SIZE;
    hash->buckets = malloc3(sizeof(Bucket) * hash->bucket_size);

    for (int i = 0; i < hash->bucket_size; i++) {
        hash->buckets[i].head = NULL;
        hash->buckets[i].tail = NULL;

        hash->buckets[i].capacity = 0;
    }
}

bool hash_keyExists(Hashmap* hash, STRING20 key)
{
    int h = murmur3_32(key, KEY_SIZE, SEED) % hash->bucket_size;

    BucketNode* curr;

    curr = hash->buckets[h].head;
    while (curr != NULL) {
        if (hash_strcmp(key, curr->pair.key) == 0) return true;

        curr = curr->next;
    }

    return false;
}

void hash_insert(Hashmap* hash, STRING20 key, void* value)
{
    int h = murmur3_32(key, KEY_SIZE, SEED) % hash->bucket_size;

    printf("%s %d\n", key, h);

    // check if key exists before appending

    if (hash_keyExists(hash, key)) {
        printf("[WARN] KEY PAIR EXISTS !!! %d\n");
        // TODO replace instead of appending
    }

    appendToBucket(&hash->buckets[h], key, value);

    // check length
    // if bucket is too long, increase buckets size and rehash
}

static void appendToBucket(Bucket* bucket, STRING20 key, void* value)
{
    BucketNode* new_node = malloc3(sizeof(BucketNode));
    IO_memcpy(new_node->pair.key, key, KEY_SIZE);
    new_node->pair.value = value;
    new_node->next = NULL;

    bucket->capacity++;

    if (bucket->head == NULL) {
        bucket->head = new_node;
        bucket->tail = new_node;
        return;
    }

    bucket->tail->next = new_node;
    bucket->tail = new_node;
}

void hash_fetch(Hashmap* hash, STRING20 key, void** out)
{
    int h = murmur3_32(key, KEY_SIZE, SEED) % hash->bucket_size;

    BucketNode* curr;
    curr = hash->buckets[h].head;
    while (curr != NULL) {
        if (hash_strcmp(key, curr->pair.key) == 0) {
            printf("FOUND! %p \n%s => %s\n", curr->pair.value, key, curr->pair.key);
            *out = curr->pair.value;
            return;
        }

        curr = curr->next;
    }

    *out = NULL;
}

void hash_delete(Hashmap* hash, STRING20 key)
{
    int h = murmur3_32(key, KEY_SIZE, SEED) % hash->bucket_size;

    BucketNode* curr;
    BucketNode* prev = NULL;
    curr = hash->buckets[h].head;
    printf("DELETE %s AT %d\n", key, h);
    while (curr != NULL) {
        printf("LOOKING: %s %s\n", key, curr->pair.key);
        if (hash_strcmp(key, curr->pair.key) == 0) {
            break;
        }

        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) {
        printf("Node with key [%s] not found\n", key);
        return;
    }

    if (prev == NULL) {
        hash->buckets[h].head = curr->next;
    } else {
        prev->next = curr->next;
    }
    hash->buckets[h].capacity--;
    free3(curr);
}

void hash_print(Hashmap* hash)
{
    printf("----- PRINT HASH\n");
    for (int i = 0; i < 11; i++) {
        BucketNode* curr = hash->buckets[i].head;

        printf("\t----- BUCKET %d %d\n", i, hash->buckets[i].capacity);

        while (curr != NULL) {
            printf("\t\t { '%s' => %p }\n", curr->pair.key, curr->pair.value);
            curr = curr->next;
        }
    }
}

// https://en.wikipedia.org/wiki/MurmurHash
static inline uint32_t murmur_32_scramble(uint32_t k)
{
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

static uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed)
{
    uint32_t h = seed;
    uint32_t k;
    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) {
        // Here is a source of differing results across endiannesses.
        // A swap here has no effects on hash properties though.
        IO_memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    // A swap is *not* necessary here because the preceding loop already
    // places the low bytes in the low places according to whatever endianness
    // we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur_32_scramble(k);
    /* Finalize. */
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}
