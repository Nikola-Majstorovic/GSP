#include "itemset.h"

Itemset* itemset_create() {
    Itemset *itemset = malloc(sizeof(Itemset));
    itemset->size = 0;
    itemset->capacity = 10;
    itemset->items = malloc(itemset->capacity * sizeof(size_t));
    itemset->is_sorted = false;
    return itemset;
}

Itemset* itemset_join(Itemset* i1, Itemset *i2) {
    if(!i1->is_sorted) {itemset_sort(i1);}
    if(!i2->is_sorted) {itemset_sort(i2);}
    Itemset* new_itemset = itemset_create();
    size_t i = 0; size_t j = 0;
    while(i < i1->size && j < i2->size) {
        if(i1->items[i] <= i2->items[j]) {
            itemset_add(new_itemset, i1->items[i]);
            if(i1->items[i] == i2->items[j]) {j++;}
            i++;
        } else {
            itemset_add(new_itemset, i2->items[j]);
            j++;
        }
    }
    while(i < i1->size) {
        itemset_add(new_itemset, i1->items[i]);
        i++;
    }
    while(j < i2->size) {
        itemset_add(new_itemset, i2->items[j]);
        j++;
    }
    return new_itemset;
}

Itemset* itemset_deepcopy(Itemset* itemset) {
    Itemset* new_itemset = malloc(sizeof(Itemset));
    new_itemset->size = itemset->size;
    new_itemset->capacity = itemset->capacity;
    new_itemset->is_sorted = itemset->is_sorted;
    new_itemset->items = malloc(itemset->capacity * sizeof(size_t));
    memcpy(new_itemset->items, itemset->items, itemset->size * sizeof(size_t));
    return new_itemset;
}

Itemset* itemset_deepcopy_without_item_at(Itemset *itemset, size_t index) {
    if(!itemset->is_sorted) {itemset_sort(itemset);}
    Itemset *new_itemset = itemset_create();
    for(size_t i = 0; i < index; i++) {
        itemset_add(new_itemset, itemset->items[i]);
    }
    for(size_t i = index+1; i < itemset->size; i++) {
        itemset_add(new_itemset, itemset->items[i]);
    }
    if(new_itemset->size == 0) {
        itemset_free(new_itemset);
        return NULL;
    }
    return new_itemset;
}

void itemset_free(Itemset* itemset) {
    if(itemset) {
        free(itemset->items);
        free(itemset);
    }
}

size_t itemset_get_hash(Itemset *itemset) {
    size_t hash = 5381;
    for(size_t i = 0; i < itemset->size; i++) {
        size_t item = itemset->items[i];
        hash = ((hash << 5) + hash) + (size_t)item;
    }
    return hash;
}

bool itemset_add(Itemset* itemset, size_t item) {
    if(itemset->size >= itemset->capacity) {
        size_t new_capacity = itemset->capacity * 2;
        size_t* tmp = realloc(itemset->items, new_capacity * sizeof(size_t));
        if(!tmp) return false;
        itemset->items = tmp;
        itemset->capacity = new_capacity;
    }
    itemset->items[itemset->size++] = item;
    itemset->is_sorted = false;
    return true;
}

static int compare_size_ts(const void *a, const void *b) {
    return (*(size_t*)a - *(size_t*)b);
}

void itemset_sort(Itemset* itemset) {
    if(itemset->size > 1) {
        qsort(itemset->items, itemset->size, sizeof(size_t), compare_size_ts);
    }
    itemset->is_sorted = true;
}

bool itemset_contains(Itemset* itemset, size_t item) {
    if(!itemset->is_sorted) {itemset_sort(itemset);}
    for(size_t i = 0; i < itemset->size; i++) {
        if(itemset->items[i] == item) {return true;}
        if(itemset->items[i] > item) {break;}
    }
    return false;
}
bool itemset_equals(Itemset* s1, Itemset* s2) {
    if(s1->size != s2->size) {return false;}
    if(!s1->is_sorted) {itemset_sort(s1);}
    if(!s2->is_sorted) {itemset_sort(s2);}
    for(size_t i = 0; i < s1->size; i++) {
        if(s1->items[i] != s2->items[i]) {return false;}
    }
    return true;
}

bool itemset_is_subset(Itemset* subset, Itemset* superset) {
    if(!subset->is_sorted) {itemset_sort(subset);}
    if(!superset->is_sorted) {itemset_sort(superset);}
    size_t i = 0; size_t j = 0;
    while(i < subset->size && j < superset->size) {
        if(subset->items[i] == superset->items[j]) {
            i++; j++;
        } else if (subset->items[i] > superset->items[j]) {
            j++;
        } else {
            break;
        }
    }
    return i == subset->size;
}

size_t itemset_difference_count(Itemset* s1, Itemset* s2) {
    if(!s1->is_sorted) {itemset_sort(s1);}
    if(!s2->is_sorted) {itemset_sort(s2);}
    size_t diff_counter = 0;
    size_t i = 0; size_t j = 0;
    while(i < s1->size && j < s2->size) {
        if(s1->items[i] < s2->items[j]) {
            i++;
            diff_counter++;
        } else if (s1->items[i] > s2->items[j]) {
            j++;
            diff_counter++;
        } else {
            i++; j++;
        }
    }

    diff_counter += (s1->size - i) + (s2->size - j);
    return diff_counter;
}
