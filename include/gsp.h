#ifndef GSP_H
#define GSP_H

#include <stdio.h>
#include <stdlib.h>
#include "sequencesdatabase.h"
#include "sequence.h"
#include "itemset.h"
#include "list.h"
#include "hashset.h"

typedef void (*StatusCallback)(const char* message);

SequencesList* gsp(SequencesDatabase *db, size_t minsupp, StatusCallback status_cb);


#endif