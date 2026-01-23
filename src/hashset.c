#include "hashset.h"
#include "sequence.h"

static size_t hash_set_get_bucket_index(HashSet *hash_set, size_t hash_value) {
    if(hash_set->capacity == 0) {
        return 0;
    }
    return hash_value % hash_set->capacity;
}

HashSet* hash_set_create(size_t capacity) {
    if (capacity == 0) {
        capacity = 2003;
    }
    HashSet *hash_set = (HashSet*)malloc(sizeof(HashSet));
    if (!hash_set) {
        return NULL;
    } 

    hash_set->capacity = capacity;
    hash_set->size = 0;

    hash_set->buckets = (HashSetNode**)calloc(capacity, sizeof(HashSetNode*));
    if(!hash_set->buckets) {
        free(hash_set);
        return NULL;
    }

    return hash_set;
}

void hash_set_free(HashSet *hash_set) {
    if(!hash_set) {
        return;
    }

    for(size_t i = 0; i < hash_set->capacity; i++) {
        HashSetNode *current = hash_set->buckets[i];
        while(current) {
            HashSetNode *to_free = current;
            current = current->next;
            if(to_free->sequence != NULL) {
                sequence_free(to_free->sequence);
            }
            free(to_free);
        }
    }
    free(hash_set->buckets);
    free(hash_set);
}

bool hash_set_add(HashSet *hash_set, Sequence *sequence) {
    if(hash_set == NULL || sequence == NULL) {
        return false;
    }
    size_t hash_value = sequence_get_hash(sequence);
    size_t index = hash_set_get_bucket_index(hash_set, hash_value);
    HashSetNode *current = hash_set->buckets[index];
    while(current != NULL) {
        if (current->sequence == NULL) {
            current = current->next;
            printf("pristup NULL pokazivacu\n");
            continue;
        }
        if(sequence_equals(current->sequence, sequence)) {
            sequence_free(sequence);
            return false;
        }
        current = current->next;
    }

    HashSetNode *new_node = (HashSetNode*)malloc(sizeof(HashSetNode));
    if(!new_node) {
        return false;
    }
    new_node->sequence = sequence;
    new_node->next = hash_set->buckets[index];
    hash_set->buckets[index] = new_node;
    hash_set->size++;
    return true;
}

bool hash_set_contains(HashSet *hash_set, Sequence *sequence) {
    if(!hash_set || !sequence) {
        return false;
    }

    size_t hash_value = sequence_get_hash(sequence);
    size_t index = hash_set_get_bucket_index(hash_set, hash_value);
    HashSetNode *current = hash_set->buckets[index];
    while(current != NULL) {
        if(sequence_equals(current->sequence, sequence)) {
            return true;
        }
        current = current->next;
    }
    return false;
}

Sequence** hash_set_extract_all(HashSet *hash_set, size_t *size) {
    if(!hash_set || !size) {
        return NULL;
    }
    Sequence** sequence_array = (Sequence**)malloc(hash_set->size*sizeof(Sequence*));
    if(!sequence_array) {
        return NULL;
    }
    size_t current_index = 0; 
    for(size_t i = 0; i < hash_set->capacity; i++) {
        HashSetNode *current_node = hash_set->buckets[i];
        while(current_node != NULL) {
            HashSetNode *to_free = current_node;
            sequence_array[current_index++] = current_node->sequence;
            current_node = current_node->next;
            to_free->next = NULL;
            free(to_free);
        }
        hash_set->buckets[i] = NULL;
    }
    *size = hash_set->size;
    hash_set->size = 0;
    return sequence_array;
}

Sequence** hash_set_get_iterator(HashSet *hash_set, size_t *iterator_size) {
    if(!hash_set || !iterator_size) {
        return NULL;
    }
    Sequence** sequence_array = (Sequence**)malloc(hash_set->size*sizeof(Sequence*));
    if(!sequence_array) {
        return NULL;
    }
    size_t current_index = 0; 
    for(size_t i = 0; i < hash_set->capacity; i++) {
        HashSetNode *current_node = hash_set->buckets[i];
        while(current_node != NULL) {
            sequence_array[current_index++] = current_node->sequence;
            current_node = current_node->next;
        }
    }
    *iterator_size = hash_set->size;
    return sequence_array;    
}