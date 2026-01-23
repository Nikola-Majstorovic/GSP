#include "gsp.h"
#include <stdio.h>
#include <stdlib.h>
#include "sequencesdatabase.h"
#include "sequence.h"
#include "itemset.h"
#include "list.h"
#include "hashset.h"
#include <time.h>

static void gsp_generate_frequent_l1(SequencesDatabase *db, size_t minsupp, SequencesList *frequent_sequences, size_t **frequent_items, size_t *frequent_items_size);
static void gsp_generate_frequent_l2(SequencesDatabase *db, size_t *frequent_items, size_t frequent_items_size, size_t minsupp, HashSet *current_lvl);
static void gsp_generate_candidates(HashSet *current_lvl, HashSet *next_lvl);
static void gsp_check_support(SequencesDatabase *db, size_t minsupp, HashSet *next_lvl, HashSet *current_lvl);
static void gsp_transfer_level(SequencesList *destination_list, HashSet *source_list);
static int compare_size_ts(const void *a, const void *b);
static size_t* get_all_unique_items(SequencesDatabase *db, size_t *unique_size);
static bool passes_prunning(Sequence *sequence, HashSet *hash_set);

static int compare_size_ts(const void *a, const void *b) {
    return (*(size_t*)a - *(size_t*)b);
}

static size_t* get_all_unique_items(SequencesDatabase *db, size_t *unique_size) {
    if(!db || db->size == 0 || !unique_size) {
        return NULL;
    }
    size_t all_items_capacity = 16;
    size_t all_items_size = 0;
    size_t *all_items = (size_t*)malloc(all_items_capacity * sizeof(size_t));
    if(!all_items) {
        return NULL;
    }
    for(size_t i = 0; i < db->size; i++) {
        Sequence *sequence = db->sequences[i];
        size_t sequence_unique_items_size = 0;
        size_t *sequence_unique_items = sequence_get_all_items(sequence, &sequence_unique_items_size);
        if(!sequence_unique_items) {
            free(all_items);
            return NULL;
        }
        if(all_items_size + sequence_unique_items_size >= all_items_capacity) {
            size_t required_capacity = all_items_size+sequence_unique_items_size;
            all_items_capacity *= (required_capacity > all_items_capacity * 2) ? required_capacity : all_items_capacity*2;
            size_t *tmp = realloc(all_items, all_items_capacity * sizeof(size_t));
            if(!tmp) {
                free(all_items);
                return NULL;
            }
            all_items = tmp;
        }
        memcpy(all_items + all_items_size, sequence_unique_items, sequence_unique_items_size * sizeof(size_t));
        all_items_size += sequence_unique_items_size;
        free(sequence_unique_items);
    }

    qsort(all_items, all_items_size, sizeof(size_t), compare_size_ts);

    size_t unique_index = 1;
    for(size_t i = 1; i < all_items_size; i++) {
        if(all_items[i] != all_items[unique_index-1]) {
            all_items[unique_index++] = all_items[i];
        }
    } 
    size_t *unique_items = realloc(all_items, (unique_index) * sizeof(size_t));
    if(!unique_items) {
        free(all_items);
        return NULL;
    }
    *unique_size = unique_index;

    return unique_items;
}

static void gsp_generate_frequent_l1(SequencesDatabase *db, size_t minsupp, SequencesList *frequent_sequences, size_t **frequent_items, size_t *frequent_items_size) {
    if(!db) {
        return;
    }
    size_t unique_items_size = 0;
    size_t *unique_items = get_all_unique_items(db, &unique_items_size);

    *frequent_items_size = 0;
    *frequent_items = malloc(unique_items_size * sizeof(size_t));
    if(!(*frequent_items)) {
        free(unique_items);
        return;
    }
    for(size_t i = 0; i < unique_items_size; i++) {
        Sequence *sequence = sequence_create_atomic(unique_items[i]);
        sequence->support = database_calculate_support(db, sequence);;
        if(sequence->support < minsupp) {
            sequence_free(sequence);
            continue;
        } else {
            list_append(frequent_sequences, sequence);
            (*frequent_items)[(*frequent_items_size)++] = unique_items[i];
        }
    }
    free(unique_items);
    if(*frequent_items_size == 0) {
        free(*frequent_items);
        *frequent_items = NULL;
        *frequent_items_size = 0;
        return;
    }

    size_t* tmp = realloc(*frequent_items, (*frequent_items_size)*sizeof(size_t));
    if(!tmp) {
        free(*frequent_items);
        *frequent_items = NULL;
        *frequent_items_size = 0;
        return;
    } else {
        *frequent_items = tmp;
    }
    return;    
}

//refaktorisati ovaj deo !!! (vratiti originalno da se naknadno filtrira)
/*static void gsp_generate_frequent_l2(SequencesDatabase *db, size_t *frequent_items, size_t frequent_items_size, size_t minsupp, HashSet *current_lvl) {
    if(!db || !frequent_items || !current_lvl) {
        return;
    } 
    for(size_t i = 0; i < frequent_items_size; i++) {
        for(size_t j = i; j < frequent_items_size; j++) {
            Sequence *sequence = sequence_create_atomic(frequent_items[i]);
            sequence_mutate_s_add(sequence, frequent_items[j]);
            sequence->support = database_calculate_support(db, sequence);
            if(sequence->support < minsupp) {
                sequence_free(sequence);
            } else {
                hash_set_add(current_lvl, sequence);
            }
            if(i != j) {
                sequence = sequence_create_atomic(frequent_items[j]);
                sequence_mutate_s_add(sequence, frequent_items[i]);
                sequence->support = database_calculate_support(db, sequence);
                if(sequence->support < minsupp) {
                    sequence_free(sequence);
                } else {
                    hash_set_add(current_lvl, sequence);
                }
                sequence = sequence_create_atomic(frequent_items[i]);
                sequence_mutate_i_add(sequence, frequent_items[j]);
                sequence->support = database_calculate_support(db, sequence);
                if(sequence->support < minsupp) {
                    sequence_free(sequence);
                } else {
                    hash_set_add(current_lvl, sequence);
                }
            }
        }
    }

}*/

static void gsp_generate_frequent_l2(SequencesDatabase *db, size_t *frequent_items, size_t frequent_items_size, size_t minsupp, HashSet *current_lvl) {
    if(!db || !frequent_items || !current_lvl) {
        return;
    } 

    for(size_t i = 0; i < frequent_items_size; i++) {
        for(size_t j = i; j < frequent_items_size; j++) {
            Sequence *sequence = sequence_create_atomic(frequent_items[i]);
            sequence_mutate_s_add(sequence, frequent_items[j]);
            sequence->support = database_calculate_support(db, sequence);
            if(sequence->support < minsupp) {
                sequence_free(sequence);
            } else {
                hash_set_add(current_lvl, sequence);
            }
            if(i != j) {
                sequence = sequence_create_atomic(frequent_items[j]);
                sequence_mutate_s_add(sequence, frequent_items[i]);
                sequence->support = database_calculate_support(db, sequence);
                if(sequence->support < minsupp) {
                    sequence_free(sequence);
                } else {
                    hash_set_add(current_lvl, sequence);
                }
                sequence = sequence_create_atomic(frequent_items[i]);
                sequence_mutate_i_add(sequence, frequent_items[j]);
                sequence->support = database_calculate_support(db, sequence);
                if(sequence->support < minsupp) {
                    sequence_free(sequence);
                } else {
                    hash_set_add(current_lvl, sequence);
                }
            }
        }
    }

}


static bool passes_prunning(Sequence *sequence, HashSet *hash_set) {
    for(size_t i = 0; i < sequence->total_items; i++) {
        Sequence *new_sequence = sequence_remove_item_at_index(sequence, i);
        if(!hash_set_contains(hash_set, new_sequence)) {
            sequence_free(new_sequence);
            return false;
        }
        sequence_free(new_sequence);
    }
    return true;
}

static void gsp_generate_candidates(HashSet *current_lvl, HashSet *next_lvl) {
    if(!current_lvl || !next_lvl) {
        return;
    }
    size_t iterator_size = 0;
    Sequence** iterator = hash_set_get_iterator(current_lvl, &iterator_size);
    if(!iterator) {
        return;
    }
    for(size_t i = 0; i < iterator_size;i++) {
        Sequence *outer_sequence = iterator[i];
        for(size_t j = 0; j < iterator_size; j++) {
            Sequence *inner_sequence = iterator[j];
            if(i != j) {
                Sequence *new_sequence = sequence_join(outer_sequence, inner_sequence);
                if(new_sequence != NULL && passes_prunning(new_sequence, current_lvl)) {
                    hash_set_add(next_lvl, new_sequence);
                }
                else {
                    sequence_free(new_sequence);
                }
            }
        }
    }
    free(iterator);
}

static void gsp_transfer_level(SequencesList *destination_list, HashSet *source) {
    if(!destination_list || !source) {
        return;
    }
    size_t source_size = 0;
    Sequence** sequences = hash_set_extract_all(source, &source_size);
    for(size_t i = 0; i < source_size; i++) {
        list_append(destination_list, sequences[i]);
    }
    free(sequences);
}

static void gsp_check_support(SequencesDatabase *db, size_t minsupp, HashSet *next_lvl, HashSet *current_lvl) {
    if(!db || !next_lvl || !current_lvl) {
        return;
    }
    size_t sequences_size = 0;
    Sequence** sequences = hash_set_extract_all(next_lvl, &sequences_size);
    for(size_t i = 0; i < sequences_size; i++) {
        Sequence *sequence = sequences[i];
        sequence->support = database_calculate_support(db, sequence);
        if(sequence->support < minsupp) {
            sequence_free(sequence);
        } else {
            hash_set_add(current_lvl, sequence);
        }
    }
    free(sequences);
}



SequencesList* gsp(SequencesDatabase *db, size_t minsupp, StatusCallback status_cb) {
    if(!db) {
        return NULL;
    }
    if(status_cb) {status_cb("Start!");}
    SequencesList *frequent_sequences = list_create();
    HashSet *current_lvl = hash_set_create(0);
    HashSet *next_lvl = hash_set_create(0);

    size_t frequent_items_size = 0;
    size_t *frequent_items = NULL;
    if(status_cb) {status_cb("Finding frequent items...");}
    gsp_generate_frequent_l1(db, minsupp, frequent_sequences, &frequent_items, &frequent_items_size);
    if(frequent_sequences->size == 0 || frequent_items_size  == 0) {
        if(status_cb) {status_cb("No frequent items!");}
        free(frequent_items);
        hash_set_free(next_lvl);
        hash_set_free(current_lvl); 
        list_free(frequent_sequences);
        return NULL;
    }
    if(status_cb) {status_cb("All frequent L1 sequences found...");}

    if(status_cb) {status_cb("Genereting L2 sequences...");}
    gsp_generate_frequent_l2(db, frequent_items, frequent_items_size, minsupp, current_lvl);
    if(status_cb) {status_cb("All frequent L2 sequences found...");}

    size_t lvl_count = 3;
    while(current_lvl->size > 0) {
        char msg_buffer[128];
        snprintf(msg_buffer, sizeof(msg_buffer), "Generating L%zu candidates and pruning...", lvl_count);
        if(status_cb) {status_cb(msg_buffer);}
        gsp_generate_candidates(current_lvl, next_lvl);
        gsp_transfer_level(frequent_sequences, current_lvl);
        if(status_cb) {status_cb("Calculating support...");}
        gsp_check_support(db, minsupp, next_lvl, current_lvl);
        char msg_buffer2[128];
        snprintf(msg_buffer2, sizeof(msg_buffer2), "All frequent L%zu candidates are found...", lvl_count);
        if(status_cb) {status_cb(msg_buffer2);}
        lvl_count++;
    }
    if(status_cb) {status_cb("Finished!");}
    free(frequent_items);
    hash_set_free(current_lvl);
    hash_set_free(next_lvl);
    return frequent_sequences;
}
