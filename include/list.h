#ifndef LIST_H
#define LIST_H

#include "sequence.h"

typedef struct SequencesListNode {
    struct SequencesListNode *next;
    Sequence *sequence;
} SequencesListNode;

typedef struct {
    SequencesListNode *head;
    SequencesListNode *tail;
    size_t size;
} SequencesList;

SequencesList* list_create();
void list_append(SequencesList *list, Sequence *sequence);
Sequence* list_remove_first(SequencesList *list);
Sequence* list_remove_last(SequencesList *list);
void list_free(SequencesList *list);
size_t list_size(SequencesList *list);
//For debug
void list_sort(SequencesList* list);

#endif