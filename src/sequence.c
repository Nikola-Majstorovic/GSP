#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sequence.h"

static bool sequence_add_itemset(Sequence *sequence, Itemset *itemset);
static Sequence* sequence_deepcopy_without_last(Sequence *sequence);

static int compare_size_ts(const void *a, const void *b) {
    return (*(size_t*)a - *(size_t*)b);
}

Sequence* sequence_create() {
    Sequence *sequence = malloc(sizeof(Sequence));
    sequence->size = 0;
    sequence->capacity = 10;
    sequence->total_items = 0;
    sequence->support = 0;
    sequence->itemsets = (Itemset**)malloc(sequence->capacity * sizeof(Itemset*));
    return sequence;
}

Sequence* sequence_create_atomic(size_t item) {
    Sequence *sequence = sequence_create();
    if(!sequence) {
        return NULL;
    }
    Itemset *itemset = itemset_create();
    if(!itemset) {
        sequence_free(sequence);
        return NULL;
    }
    if(!itemset_add(itemset, item)) {
        itemset_free(itemset);
        sequence_free(sequence);
        return NULL;
    }
    if(!sequence_add_itemset(sequence, itemset)) {
        itemset_free(itemset);
        sequence_free(sequence);
        return NULL;
    }
    return sequence;
}

Sequence* sequence_deepcopy(Sequence *sequence) {
    Sequence *new_sequence = sequence_create();
    for(size_t i = 0; i < sequence->size; i++) {
        sequence_add_itemset(new_sequence, itemset_deepcopy(sequence->itemsets[i]));
    }
    new_sequence->support = sequence->support;
    return new_sequence;    
}

//izmeniti, ruzna implementacija
Sequence* sequence_remove_item_at_index(Sequence *sequence, size_t index) {
    Sequence *new_sequence = sequence_create();
    //size_t size = 0;
    size_t i = 0;
    while(i < sequence->size)
    {
        if(new_sequence->total_items + sequence->itemsets[i]->size > index) {
            Itemset *new_itemset = itemset_deepcopy_without_item_at(sequence->itemsets[i], index-new_sequence->total_items);
            if(new_itemset) {
                sequence_add_itemset(new_sequence, new_itemset);
            }
            i++;
            break;
        } else {
            sequence_add_itemset(new_sequence, itemset_deepcopy(sequence->itemsets[i]));
            i++;
        }
    }
    while(i < sequence->size) {
        sequence_add_itemset(new_sequence, itemset_deepcopy(sequence->itemsets[i])); 
        i++;
    }
    if(new_sequence->size == 0) {
        sequence_free(new_sequence);
        return NULL;
    }
    return new_sequence;
}

void sequence_free(Sequence *sequence) {
    if(sequence) {
        for(size_t i = 0; i < sequence->size; i++) {
            itemset_free(sequence->itemsets[i]);
        }
        free(sequence->itemsets);
        free(sequence);
    }
}

size_t sequence_get_hash(Sequence *sequence) {
    size_t hash = 5381;

    for(size_t i = 0; i < sequence->size; i++) {
        size_t itemset_hash = itemset_get_hash(sequence->itemsets[i]);
        hash = ((hash << 5) + hash) + itemset_hash;

        hash = (hash * 31) + 47; 
    }
    
    return hash;
}

size_t* sequence_get_all_items(Sequence *sequence, size_t *size) {
    if(!sequence) {
        return NULL;
    }
    //prsize_tf("Allocating all_items...\n");
    size_t *all_items = malloc(sizeof(size_t)*sequence->total_items);
    if(!all_items) {
        return NULL;
    }
    //printf("all_items allocated\n");

    size_t index = 0;
    //print_sequence(sequence);
    for(size_t i = 0; i < sequence->size; i++) {
        Itemset *itemset = sequence->itemsets[i];
        for(size_t j = 0; j < itemset->size; j++) {
            all_items[index++] = itemset->items[j];
            //printf("%d ", all_items[index-1]);
        }
    }
    //printf("Sorting\n");
    qsort(all_items, sequence->total_items, sizeof(size_t), compare_size_ts);
    //printf("Sorted\n");
    size_t unique_index = 0;
    for(size_t i = 1; i < sequence->total_items; i++) {
        //printf("%d ", all_items[i]);
        if(all_items[i] != all_items[unique_index]) {
            unique_index++;
            all_items[unique_index] = all_items[i];
        }
    }
    //printf("Reallocing unique_items...\n");
    size_t *unique_items = realloc(all_items, (unique_index+1) * sizeof(size_t));
    if(!unique_items) {
        free(all_items);
        return NULL; 
    }
    //printf("unique_items reallocated\n");
    *size = unique_index+1;
    return unique_items;
}

bool sequence_equals(Sequence *sequence1, Sequence *sequence2) {
    if(!sequence1 || !sequence2) {
        return false;
    }
    if(sequence1->size != sequence2->size || sequence1->total_items != sequence2->total_items) {
        return false;
    }
    size_t n = sequence1->size;
    for(size_t i = 0; i < n; i++) {
        if(!itemset_equals(sequence1->itemsets[i], sequence2->itemsets[i])) {
            return false;
        }
    }
    return true;
}

Sequence* sequence_i_extension(Sequence *sequence, size_t item) {
    if(!sequence || sequence->size < 1) {
        return NULL;
    }

    Sequence *new_sequence = sequence_deepcopy(sequence);
    if(!new_sequence) {
        return NULL;
    }
    if(!itemset_add(new_sequence->itemsets[new_sequence->size-1], item)) {
        sequence_free(new_sequence);
        return NULL;
    }
    itemset_sort(new_sequence->itemsets[new_sequence->size-1]);
    new_sequence->total_items++;
    return new_sequence;
}

Sequence* sequence_s_extension(Sequence *sequence, size_t item) {
    if(!sequence) {
        return NULL;
    }
    Sequence *new_sequence = sequence_deepcopy(sequence);
    if(!new_sequence) {
        return NULL;
    }

    /*if(new_sequence->size >= new_sequence->capacity) {
        new_sequence->capacity *= 2;
        new_sequence->itemsets = (Itemset**)realloc(new_sequence->itemsets, new_sequence->capacity * sizeof(Itemset*));
        if(!new_sequence->itemsets) {
            printf("Realloc failed!\n");
            sequence_free(new_sequence);
            return NULL;
        }
    }*/
    Itemset *tmp = itemset_create();
    if(!tmp) {
        sequence_free(new_sequence);
        return NULL;
    }
    if(!itemset_add(tmp, item)) {
        itemset_free(tmp);
        sequence_free(new_sequence);
        return NULL;
    }
    //new_sequence->itemsets[new_sequence->size++] = tmp;
    //new_sequence->total_items++;
    if(!sequence_add_itemset(new_sequence, tmp)) {
        itemset_free(tmp);
        sequence_free(new_sequence);
        return NULL;
    }
    return new_sequence;
}

bool sequence_mutate_i_add(Sequence *sequence, size_t item) {
    if(!sequence || sequence->size < 1) {
        return false;
    }

    if(!itemset_add(sequence->itemsets[sequence->size-1], item)) {
        return false;
    }
    itemset_sort(sequence->itemsets[sequence->size-1]);
    sequence->total_items++;
    return true;
}

bool sequence_mutate_s_add(Sequence *sequence, size_t item) {
    if(!sequence) {
        return false;
    }

    Itemset *tmp = itemset_create();
    if(!tmp) {
        return false;
    }
    if(!itemset_add(tmp, item)) {
        itemset_free(tmp);
        return false;
    }
    if(!sequence_add_itemset(sequence, tmp)) {
        itemset_free(tmp);
        sequence_free(sequence);
        return false;
    }
    return true;
}

Sequence* sequence_join_i_extension(Sequence *s1, Sequence *s2) {
    if(!s1 || !s2) {
        return NULL;
    }
    Sequence *new_sequence = sequence_deepcopy_without_last(s1);
    if(!new_sequence) {
        return NULL;
    }
    Itemset *new_itemset = itemset_deepcopy(s2->itemsets[s2->size-1]);
    if(!new_itemset) {
        sequence_free(new_sequence);
        return NULL;
    }
    if(!sequence_add_itemset(new_sequence, new_itemset)) {
        itemset_free(new_itemset);
        sequence_free(new_sequence);
        return NULL;
    }
    return new_sequence;
}

Sequence* sequence_join_s_extension(Sequence *s1, Sequence *s2) {
    if(!s1 || !s2) {
        return NULL;
    }
    Sequence *new_sequence = sequence_deepcopy(s1);
    if(!new_sequence) {
        return NULL;
    }
    Itemset *new_itemset = itemset_deepcopy(s2->itemsets[s2->size-1]);
    if(!new_itemset) {
        sequence_free(new_sequence);
        return NULL;
    }
    if(!sequence_add_itemset(new_sequence, new_itemset)) {
        itemset_free(new_itemset);
        sequence_free(new_sequence);
        return NULL;
    }
    return new_sequence;
}

bool sequence_is_subsequence(Sequence *supersequence, Sequence *subsequence) {
    if(subsequence->size == 0) {return true;}
    if(supersequence->size < subsequence->size) {return false;}

    size_t i = 0; size_t j = 0;
    while(i < supersequence->size && j < subsequence->size) {
        if(itemset_is_subset(subsequence->itemsets[j], supersequence->itemsets[i])) {j++;}
        i++;
    }
    return j == subsequence->size;
}

// s1 - preffix, s2 - suffix
Sequence* sequence_join(Sequence *s1, Sequence *s2) {
    if(!sequence_can_join(s1, s2)) {
        return NULL;
    }
    if(s2->itemsets[s2->size-1]->size == 1) {
        return sequence_join_s_extension(s1, s2);
    } else {
        return sequence_join_i_extension(s1, s2);
    }
}

bool sequence_can_join(Sequence *s1, Sequence *s2) {
    size_t suffix_body_start = 0;
    size_t suffix_body_end = s2->size - 1;
    size_t preffix_body_start = 1;
    size_t preffix_body_end = s1->size;
    if(s1->itemsets[0]->size > 1) {
        if(!(s1->itemsets[0]->size == s2->itemsets[0]->size + 1) 
        || (!itemset_is_subset(s2->itemsets[0], s1->itemsets[0]))) {
            return false;
        }
        suffix_body_start++;
    }
    if(s2->itemsets[s2->size-1]->size > 1) {
        if(!(s2->itemsets[s2->size-1]->size == s1->itemsets[s1->size-1]->size + 1) 
        || (!itemset_is_subset(s1->itemsets[s1->size-1], s2->itemsets[s2->size-1]))) {
            return false;
        }
        preffix_body_end--;
    }
    if(suffix_body_end - suffix_body_start != preffix_body_end - preffix_body_start) {
        return false;
    }
    size_t n = suffix_body_end - suffix_body_start;
    for(size_t i = 0; i < n; i++) {
        if(!itemset_equals(s1->itemsets[preffix_body_start+i], s2->itemsets[suffix_body_start+i])) {
            return false;
        }
    }
    return true;
}

void print_sequence(Sequence *s) {
    size_t n = s->size;
    printf("<");
    for(size_t i = 0; i < n; i++) {
        size_t m = s->itemsets[i]->size;
        printf("(");
        for(size_t j = 0; j < m; j++) {
            printf("%ld,", s->itemsets[i]->items[j]);
        }
        printf(")");
    }
    printf(">\n");
}

static bool sequence_add_itemset(Sequence *sequence, Itemset *itemset) {
    if(sequence->size >= sequence->capacity) {
        size_t new_capacity = sequence->capacity * 2;
        Itemset **tmp = (Itemset**)realloc(sequence->itemsets, new_capacity * sizeof(Itemset*)); 
        if(!tmp) {
            return false;
        }
        sequence->itemsets = tmp;
        sequence->capacity = new_capacity;
    }
    sequence->itemsets[sequence->size++] = itemset;
    sequence->total_items += itemset->size;  
    return true;
}

static Sequence* sequence_deepcopy_without_last(Sequence *sequence) {
    Sequence *new_sequence = sequence_create();
    for(size_t i = 0; i < sequence->size-1; i++) {
        sequence_add_itemset(new_sequence, itemset_deepcopy(sequence->itemsets[i]));
    }
    new_sequence->support = sequence->support;
    return new_sequence;    
}


void print_sequence_to_file(Sequence *s, FILE *fajl) {
    size_t n = s->size;
    fprintf(fajl, "<");
    for(size_t i = 0; i < n; i++) {
        size_t m = s->itemsets[i]->size;
        fprintf(fajl, "(");
        for(size_t j = 0; j < m; j++) {
            fprintf(fajl, "%ld,", s->itemsets[i]->items[j]);
        }
        fprintf(fajl, ")");
    }
    fprintf(fajl, ">\n");
}

char* sequence_toString(Sequence *sequence) {
    if(sequence == NULL) {
        return NULL;
    }
    size_t capacity = 256;
    char *result = malloc(capacity);
    if(!result) {
        return NULL;
    }
    strcpy(result, "<");
    for(size_t i = 0; i < sequence->size; i++) {
        Itemset *current_itemset = sequence->itemsets[i];
        strcat(result, "(");
        for(size_t j = 0; j < current_itemset->size; j++) {
            char num_buff[32];
            sprintf(num_buff, "%zu", current_itemset->items[j]);
            if(strlen(result) + strlen(num_buff) + 10 > capacity) {
                capacity *= 2;
                result = realloc(result, capacity);
                if(!result) {return NULL;}
            }
            strcat(result, num_buff);
            if(j < current_itemset->size - 1) {
                strcat(result, ",");
            }
        }
        strcat(result, ")");
    }
    strcat(result, ">");
    return result;
}

char* sequence_toSPMF(Sequence *sequence) {
    if(sequence == NULL) {
        return NULL;
    }
    size_t capacity = 256;
    char *result = malloc(capacity);
    if(!result) {
        return NULL;
    }
    result[0] = '\0';
    for(size_t i = 0; i < sequence->size; i++) {
        Itemset *current_itemset = sequence->itemsets[i];
        for(size_t j = 0; j < current_itemset->size; j++) {
            char num_buff[32];
            sprintf(num_buff, "%zu", current_itemset->items[j]);
            if(strlen(result) + strlen(num_buff) + 10 > capacity) {
                capacity *= 2;
                result = realloc(result, capacity);
                if(!result) {return NULL;}
            }
            strcat(result, num_buff);
            strcat(result, " ");
        }
        strcat(result, "-1 ");
    }
    strcat(result, "-2 ");
    char supp_buff[32];
    sprintf(supp_buff, "%zu", sequence->support);
    strcat(result, "support: ");
    strcat(result, supp_buff);
    return result;
}