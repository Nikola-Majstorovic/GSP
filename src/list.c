#include "list.h"

static SequencesListNode* createNode(Sequence *sequence);

SequencesList* list_create() {
    SequencesList *list = malloc(sizeof(SequencesList));
    if(!list) {
        return NULL;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

/*void list_append(SequencesList *list, SequencesListNode *node) {
    if(!list || !node) {
        return;
    }
    node->next = NULL;
    if(!list->head) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
}*/

void list_append(SequencesList *list, Sequence *sequence) {
    SequencesListNode *node = createNode(sequence);
    if(!list || !node) {
        return;
    }
    if(!list->head) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
}

Sequence* list_remove_first(SequencesList *list) {
    if(!list || !list->head) {
        return NULL;
    }
    SequencesListNode *node = list->head;
    Sequence *sequence = node->sequence;
    if(list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = list->head->next;
    }
    free(node);
    list->size--;
    return sequence;
}

Sequence* list_remove_last(SequencesList *list) {
    if(!list || !list->tail) {
        return NULL;
    }
    SequencesListNode *node = list->tail;
    Sequence *sequence = node->sequence;
    if(list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        SequencesListNode *current = list->head;
        while(current->next != list->tail) {
            current = current->next;
        }
        list->tail = current;
        current->next = NULL;
    }
    free(node);
    list->size--;
    return sequence;
}

void list_free(SequencesList *list) {
    if(!list) {
        return;
    }
    SequencesListNode *current = list->head;
    while(current) {
        SequencesListNode *next = current->next;
        sequence_free(current->sequence);
        free(current);
        current = next;
    }
    free(list);
}
size_t list_size(SequencesList *list) {
    return list->size;
}

static SequencesListNode* createNode(Sequence *sequence) {
    SequencesListNode *node = malloc(sizeof(SequencesListNode));
    if(!node) {
        return NULL;
    }
    node->next = NULL;
    node->sequence = sequence;
    return node;
}

/// FOR DEBUG!!! DETE LATER !!!

static int compare_sequences(const void *a, const void *b) {
    // a i b su pokazivači na elemente niza Sequence*
    // dakle, tipovi su: Sequence** a i Sequence** b
    Sequence* seq1 = *(Sequence**)a;
    Sequence* seq2 = *(Sequence**)b;
    if(seq1->total_items < seq2->total_items) {
        return -1;
    } else if (seq1->total_items > seq2->total_items) {
        return 1;
    } else {
        return seq2->support - seq1->support;
    }
    // Poredjenje po support polju
    if (seq1->support < seq2->support) return -1;
    if (seq1->support > seq2->support) return 1;
    return 0;
    
    // ILI jednostavnije:
    // return (seq1->support - seq2->support);
    // ali ovo može da ima problem sa overflow-om za velike brojeve
}

void list_sort(SequencesList* list) {
    Sequence** array = (Sequence**)malloc(list->size * sizeof(Sequence*));
    size_t index = 0;
    while(list->head) {
        array[index++] = list_remove_first(list);
    }
    qsort(array, index, sizeof(Sequence*), compare_sequences);
    printf("sorted\n");
    for(size_t i = 0; i < index; i++) {
        print_sequence(array[i]);
        list_append(list, array[i]);
        array[i] = NULL;
    }
    printf("finished\n");
    free(array);
}