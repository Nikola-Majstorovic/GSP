#ifndef SEQUENCESDATABASE_H
#define SEQUENCESDATABASE_H

#include "sequence.h"

typedef struct {
    Sequence **sequences;
    size_t size;
    size_t capacity;
} SequencesDatabase;

SequencesDatabase* database_load(const char *filename);
void database_free(SequencesDatabase *db);
size_t database_calculate_support(SequencesDatabase *db, Sequence *sequence);

#endif