#ifndef HASHSET_H
#define HASHSET_H

#include "sequence.h"

typedef struct HashSetNode{
    Sequence *sequence;
    struct HashSetNode *next;
} HashSetNode;

typedef struct {
    HashSetNode **buckets;
    size_t capacity;
    size_t size;
} HashSet;


HashSet* hash_set_create(size_t capacity);
void hash_set_free(HashSet *hash_set);
bool hash_set_add(HashSet *hash_set, Sequence *sequence);
//Sequence* hash_set_find(HashSet *hash_set, Sequence *sequence);
bool hash_set_contains(HashSet *hash_set, Sequence *sequence);
//sequence* hash_set_remove(HashSet *hash_set, Sequence *sequence);
Sequence** hash_set_extract_all(HashSet *hash_set, size_t *size);
Sequence** hash_set_get_iterator(HashSet *hash_set, size_t *iterator_size);

#endif