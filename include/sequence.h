#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "itemset.h"

typedef struct {
    Itemset **itemsets;
    size_t size;
    size_t capacity;
    size_t total_items;
    size_t support;
} Sequence;

Sequence* sequence_create();
Sequence* sequence_create_atomic(size_t item);
Sequence* sequence_deepcopy(Sequence *sequence);
Sequence* sequence_remove_item_at_index(Sequence *sequence, size_t index);
void sequence_free(Sequence *sequence);
size_t sequence_get_hash(Sequence *sequence);
//potencijalni alternativni nacin za kreiranje l2 sekvenci
//Sequence* sequence_i_extension_atomic(Sequence *s1, Sequence *s2);
//Sequence* sequence_s_extension_atomic(Sequence *s1, Sequence *s2);
Sequence* sequence_i_extension(Sequence *sequence, size_t item);
Sequence* sequence_s_extension(Sequence *sequence, size_t item);
Sequence* sequence_join_i_extension(Sequence *s1, Sequence *s2);
Sequence* sequence_join_s_extension(Sequence *s1, Sequence *s2);
bool sequence_mutate_i_add(Sequence *sequence, size_t item);
bool sequence_mutate_s_add(Sequence *sequence, size_t item);
bool sequence_can_join(Sequence *s1, Sequence *s2);
Sequence* sequence_join(Sequence *s1, Sequence *s2);
bool sequence_is_subsequence(Sequence *supersequence, Sequence *subsequence);
size_t* sequence_get_all_items(Sequence *sequence, size_t* size);
bool sequence_equals(Sequence *sequence1, Sequence *sequence2);
char* sequence_toString(Sequence *sequence);
char* sequence_toSPMF(Sequence *sequence);
//for debug
void print_sequence(Sequence *s);
void print_sequence_to_file(Sequence *s, FILE *fajl);
#endif