#include "sequencesdatabase.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    Sequence *current_sequence;
    bool next_is_s_extension;
} ParserState;

static bool database_add_sequence(SequencesDatabase *db, Sequence *sequence);
static bool process_token(const char *token, ParserState *state);
static bool handle_item(size_t item, ParserState *state);
static bool handle_end_of_itemset(ParserState *state);
static bool handle_end_of_sequence(ParserState *state);
static bool is_valid_token(const char *token);
static bool is_empty_line(const char *line);
static Sequence* parse_line(char *line);

SequencesDatabase* database_load(const char *filename) {
    FILE *file = fopen(filename, "r");
    if(!file) {
        printf("Can't open file\n");
        return NULL;
    }
    SequencesDatabase *db = malloc(sizeof(SequencesDatabase));
    if(!db) {
        printf("Can't malloc db\n");
        fclose(file);
        return NULL;
    }
    db->capacity = 16;
    db->size = 0;
    db->sequences = malloc(db->capacity * sizeof(Sequence*));
    if(!db->sequences) {
        printf("Can't malloc sequence\n");
        free(db);
        fclose(file);
        return NULL;
    }
    char line[4096];

    while(fgets(line, sizeof(line), file)) {
        if(is_empty_line(line)) {
            continue;
        }
        Sequence *sequence = parse_line(line);
        if(sequence) {
            if(!database_add_sequence(db, sequence)) {
                sequence_free(sequence);
            }
        }
    }
    fclose(file);
    return db;  
}

size_t database_calculate_support(SequencesDatabase *db, Sequence *sequence) {
    if(db == NULL || sequence == NULL) {
        return 0;
    } 
    size_t support = 0;
    for(size_t i = 0; i < db->size; i++) {
        Sequence *db_sequence = db->sequences[i];
        if(sequence_is_subsequence(db_sequence, sequence)) {
            support++;
        }
    } 
    /*printf("Support for ");
    print_sequence(sequence);
    printf(" is %d\n", support);*/
    return support;
}

void database_free(SequencesDatabase *db) {
    for(size_t i = 0; i < db->size; i++) {
        sequence_free(db->sequences[i]);
    }
    free(db->sequences);
    free(db);
}

static bool database_add_sequence(SequencesDatabase *db, Sequence *sequence) {
    if(db->size >= db->capacity) {
        size_t new_capacity = db->capacity * 2;
        Sequence **tmp = (Sequence **)realloc(db->sequences, new_capacity * sizeof(Sequence*));
        if(!tmp) {
            return false;
        }
        db->sequences = tmp;
        db->capacity = new_capacity;
    }
    db->sequences[db->size++] = sequence;
    return true;
}

static Sequence* parse_line(char *line) {
    ParserState state = {0};
    state.current_sequence = NULL;
    state.next_is_s_extension = true;
    
    char *token = strtok(line, " \t\n");
    while(token != NULL) {
        if(!process_token(token, &state)) {
            if(state.current_sequence) {
                sequence_free(state.current_sequence);
            }
            return NULL;
        }
        token  = strtok(NULL, " \t\n");
    }
    return state.current_sequence;
}

static bool process_token(const char *token, ParserState *state) {
    if(strcmp(token, "-2") == 0) {
        return handle_end_of_sequence(state);
    }
    if(strcmp(token, "-1") == 0) {
        return handle_end_of_itemset(state);
    }
    if(!is_valid_token(token)) {
        return false;
    }
    size_t value = atoi(token);
    if(value <= 0) {
        return false;
    }
    return handle_item(value, state);

}

static bool handle_item(size_t item, ParserState *state) {
    Sequence *s = state->current_sequence;
    bool success = false;

    if(s == NULL) {
        s = sequence_create_atomic(item);
        if(s) {
            state->current_sequence = s;
            success = true;
        }
    } else if (state->next_is_s_extension) {
        success = sequence_mutate_s_add(state->current_sequence, item);
    } else {
        success = sequence_mutate_i_add(state->current_sequence, item);
    }
    if(success) {
        state->next_is_s_extension = false;
    }
    return success;
}



static bool handle_end_of_itemset(ParserState *state) {
    state->next_is_s_extension = true;
    return true;
}

static bool handle_end_of_sequence(ParserState *state) {
    state->next_is_s_extension = true;
    return true;
}

static bool is_empty_line(const char *line) {
    if (!line) return true;
    
    for (const char *p = line; *p; p++) {
        if (!isspace(*p)) {
            return false;
        }
    }
    return true;
}

static bool is_valid_token(const char *token) {
    if (!token || *token == '\0') return false;

    for (const char *p = token; *p; p++) {
        if (!isdigit(*p)) {
            return false;
        }
    }
    if (atoi(token) <= 0) return false;
    
    return true;
}
