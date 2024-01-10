#ifndef HASHMAP_C
#define HASHMAP_C

typedef struct bucket_node_t
{
    struct
    {
        STRING20 key;
        void*    value;
    } pair;

    struct bucket_node_t* next;
} BucketNode;

typedef struct bucket_t
{
    BucketNode* head;
    BucketNode* tail;

    int capacity;
} Bucket;

typedef struct hashmap_t
{
    unsigned int     bucket_size;
    struct bucket_t* buckets;
} Hashmap;

void hash_initHashMap(Hashmap* hash);
bool hash_keyExists(Hashmap* hash, STRING20 key);
void hash_insert(Hashmap* hash, STRING20 key, void* value);
void hash_fetch(Hashmap* hash, STRING20, void* out);
void hash_delete(Hashmap* hash, STRING20);
void hash_print(Hashmap* hash);

// TODO: add type conversion to STRING20
// eg: SVECTOR to STRING20, etc
// in order to use something else as a key

#endif
