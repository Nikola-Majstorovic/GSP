#ifndef ITEMSET_H
#define ITEMSET_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    size_t *items;
    size_t size;
    size_t capacity;
    bool is_sorted;
} Itemset;

Itemset* itemset_create();
Itemset* itemset_join(Itemset* i1, Itemset *i2);
Itemset* itemset_deepcopy(Itemset *itemset);
Itemset* itemset_deepcopy_without_item_at(Itemset *itemset, size_t index);
size_t itemset_get_hash(Itemset *itemset);
void itemset_free(Itemset* itemset);
bool itemset_add(Itemset* itemset, size_t item);
void itemset_sort(Itemset* itemset);
bool itemset_contains(Itemset* itemset, size_t item);
bool itemset_equals(Itemset* s1, Itemset* s2);
bool itemset_is_subset(Itemset* subset, Itemset* superset);
size_t itemset_difference_count(Itemset* s1, Itemset* s2);

#endif