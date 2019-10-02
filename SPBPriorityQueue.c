/*

 * SPBPriorityQueue.c
 *
 *  Created on: 31 במאי 2016
 *      Author: Tal
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include "SPBPriorityQueue.h"
#include "SPListElement.h"
#include "SPList.h"

struct sp_bp_queue_t {
	SPList list;
	int maxSize;
};

SPBPQueue spBPQueueCreate(int maxSize) {
	if (maxSize <= 0)
		return NULL;
	SPBPQueue queue = (SPBPQueue) malloc(sizeof(*queue));
	if (queue == NULL)
		return NULL;
	queue->list = spListCreate();
	if (queue->list == NULL) {
		spBPQueueDestroy(queue);
		return NULL;
	}
	queue->maxSize = maxSize;
	return queue;
}

SPBPQueue spBPQueueCopy(SPBPQueue source) {
	if (source == NULL)
		return NULL;
	SPBPQueue copyQueue = (SPBPQueue) malloc(sizeof(*copyQueue));
	if (copyQueue == NULL)
		return NULL;
	copyQueue->list = spListCopy(source->list);
	if (copyQueue->list == NULL) {
		spBPQueueDestroy(copyQueue);
		return NULL;
	}
	copyQueue->maxSize = source->maxSize;
	return copyQueue;
}

void spBPQueueDestroy(SPBPQueue source) {
	if (source != NULL) {
		spListDestroy(source->list);
		free(source);
	}
	return;
}

void spBPQueueClear(SPBPQueue source) {
	if (source != NULL) {
		spListClear(source->list);
	}
	source->maxSize = 0;
	return;
}

int spBPQueueSize(SPBPQueue source) {
	if (source == NULL)
		return -1;
	return spListGetSize(source->list);
}

int spBPQueueGetMaxSize(SPBPQueue source) {
	if (source == NULL)
		return -1;
	return source->maxSize;
}

SP_BPQUEUE_MSG spBPQueueEnqueue(SPBPQueue source, SPListElement element) {
	if (source == NULL || element == NULL)
		return SP_BPQUEUE_INVALID_ARGUMENT;
	SPListElement copyElement = spListElementCopy(element);
	if (copyElement == NULL)
		return SP_BPQUEUE_OUT_OF_MEMORY;

	//Case1 : queue isn't full
	if ((spListGetSize(source->list)) < (source->maxSize)) {
		SP_LIST_FOREACH(SPListElement, e, source->list)
		{

			/* we will add the new element before e only if e is
			 * bigger than the new element
			 */
			if (spListElementCompare(e, copyElement) >= 1) {
				spListInsertBeforeCurrent(source->list, copyElement);
				spListElementDestroy(copyElement);

				return SP_BPQUEUE_SUCCESS;
			}
		}
		/* The value of the new element is the biggest
		 * (according to value+index)
		 */
		spListInsertLast(source->list, copyElement);
		spListElementDestroy(copyElement);
		return SP_BPQUEUE_SUCCESS;
	}

	//Case2 : queue is full

	else {
		/* maybe we don't need to go over all the queue's elements because
		 * the new element is the biggest and the queue is full so we
		 * don't need to insert the new element to the queue
		 */
		SPListElement last = spBPQueuePeekLast(source);
		if (spListElementCompare(copyElement, last) >= 0) {
			spListElementDestroy(copyElement);
			spListElementDestroy(last);
			return SP_BPQUEUE_SUCCESS;
		}
		// therefore the loop must stop somewhere
		else {
			spListElementDestroy(last);
			SP_LIST_FOREACH(SPListElement, e, source->list)
			{
				if (spListElementCompare(e, copyElement) >= 1) {
					spListInsertBeforeCurrent(source->list, copyElement);
					spListGetLast(source->list);
					spListRemoveCurrent(source->list);
					spListElementDestroy(copyElement);

					return SP_BPQUEUE_SUCCESS;
				}
			}
		}
	}
	// shouldn't get here
	return SP_BPQUEUE_EMPTY;
}

SP_BPQUEUE_MSG spBPQueueDequeue(SPBPQueue source) {
	if (source == NULL)
		return SP_BPQUEUE_INVALID_ARGUMENT;
	if ((spListGetSize(source->list)) == 0)
		return SP_BPQUEUE_EMPTY;
	spListGetFirst(source->list);
	spListRemoveCurrent(source->list);

	return SP_BPQUEUE_SUCCESS;
}

SPListElement spBPQueuePeek(SPBPQueue source) {
	if (source == NULL)
		return NULL;
	if ((spListGetSize(source->list)) == 0)
		return NULL;

	SPListElement copy = spListElementCopy(spListGetFirst(source->list));
	return copy;
}

SPListElement spBPQueuePeekLast(SPBPQueue source) {
	if (source == NULL)
		return NULL;
	if ((spListGetSize(source->list)) == 0)
		return NULL;
	spListGetLast(source->list);

	SPListElement copy = spListElementCopy(spListGetLast(source->list));
	return copy;
}

double spBPQueueMinValue(SPBPQueue source) {

	if (source == NULL)
		return -1;
	if ((spListGetSize(source->list)) == 0)
		return -1;

	return spListElementGetValue(spListGetFirst(source->list));

}

double spBPQueueMaxValue(SPBPQueue source) {

	if (source == NULL)
		return -1;
	if ((spListGetSize(source->list)) == 0)
		return -1;

	return spListElementGetValue(spListGetLast(source->list));

}

bool spBPQueueIsEmpty(SPBPQueue source) {
	assert(source != NULL);
	return ((spListGetSize(source->list)) == 0);
}

bool spBPQueueIsFull(SPBPQueue source) {
	assert(source != NULL);
	return (spListGetSize(source->list) == source->maxSize);
}
int spBPQueueIndexOfMinValue(SPBPQueue source) {
	int index;
	if (source == NULL)
		return -1;
	if ((spListGetSize(source->list)) == 0)
		return -1;
	index = spListElementGetIndex(spListGetFirst(source->list));
	if (spBPQueueDequeue(source) != SP_BPQUEUE_SUCCESS) {
		return -1;
	}
	return index;
}

void spBPQueueSetSize(SPBPQueue source, int size) {
	if (source == NULL)
		return;
	source->maxSize = size;
}
