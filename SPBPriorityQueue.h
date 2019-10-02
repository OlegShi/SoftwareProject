#ifndef SPBPRIORITYQUEUE_H_
#define SPBPRIORITYQUEUE_H_
#include "SPListElement.h"
#include <stdbool.h>
/**
 * SP Bounded Priority Queue summary
 *
 * Implements a queue container type.
 * The queue contains a list and capacity which bounds the numbers of elements in the list
 * please refer to SPList.h for usage.
 * The queue decide whether to add a new element based on the value and the index of a new element,
 * remembering the fact that the queue has an upper bound of the number of elements - capacity 
 *
 * The following functions are available:
 *
 *   spBPQueueCreate            - Creates a new empty queue.
 *   spBPQueueCopy              - Creates a copy of a given queue.
 *   spBPQueueDestroy           - Destroys a queue.
 *   spBPQueueClear             - Removes all elements from the queue
 *   spBPQueueSize              - Returns the number of elements in the queue.
 *   spBPQueueGetMaxSize        - Returns the maximum capacity of the queue
 *   spBPQueueEnqueue           - Inserts an element to the queue
 *   spBPQueueDequeue           - Removes the element with the lowest value
 *   spBPQueuePeek              - Returns the element with the lowest value
 *   spBPQueuePeekLast          - Returns the element with the highest value
 *   spBPQueueMinValue          - Returns the minimum value in the queue
 *   spBPQueueMaxValue          - Returns the maximum value in the queue
 *   spBPQueueIsEmpty           - Returns true if the queue is empty, false if not
 *   spBPQueueIsFull	        - Return true if the queue is full, false if not
 *
 */

/** type used to define Bounded priority queue **/
typedef struct sp_bp_queue_t* SPBPQueue;

/** type for error reporting **/
typedef enum sp_bp_queue_msg_t {
	SP_BPQUEUE_OUT_OF_MEMORY,
	SP_BPQUEUE_FULL,
	SP_BPQUEUE_EMPTY,
	SP_BPQUEUE_INVALID_ARGUMENT,
	SP_BPQUEUE_SUCCESS
} SP_BPQUEUE_MSG;

/**
 *
 * creates an empty queue with a given maximum capacity
 *
 * This function creates a new empty queue.
 * @return
 * 	NULL - If allocations failed or if maxSize<=0.
 * 	A new queue in case of success.
 */
SPBPQueue spBPQueueCreate(int maxSize);

/**
 * creates a copy of a given queue
 *
 *The new copy will contain the same list as the list in the source queue and the
 *same capacity as the capacity of the source queue
 *
 *
 * @param source The target queue to copy
 * @return
 * NULL if a NULL was sent or a memory allocation failed.
 * A queue contain the same list and capacity as the source queue
 */
SPBPQueue spBPQueueCopy(SPBPQueue source);

/**
 * Destroys a queue.
 * All memory allocation associated with the queue will be freed,
 * include the list and the elements of the list
 *
 * @param source the target queue which will be freed.
 * 			   if source is NULL, then nothing is done
 */
void spBPQueueDestroy(SPBPQueue source);

/**
 * Removes all elements from target queue.
 * @param source Target queue to remove all element from
 * @return
 */
void spBPQueueClear(SPBPQueue source);

/**
 * Returns the number of elements in the queue.
 *
 * @param source The target source which size is requested.
 * @return
 * -1 if a NULL pointer was sent.
 * Otherwise the number of elements in the queue.
 */
int spBPQueueSize(SPBPQueue source);

/**
 * Returns the maximum capacity of the queue
 *
 * @param source The target source which capacity is requested.
 * @return
 * -1 if a NULL pointer was sent.
 * Otherwise the capacity of the queue.
 */
int spBPQueueGetMaxSize(SPBPQueue source);

/**
 *
 *Inserts a NEW COPY (must be allocated) element to the queue
 *
 * Adds a new element to the queue in the right place according
 * to the value and the index, if the queue is full and the value of
 * the element is the biggest (according to value, index) we will not
 * add the new element to the queue.
 * if the new element as the same value and same index as the "biggest"
 * (according to value+index) element then we won't add the new element
 * to the queue
 *
 * @param source The queue source for which to add an element
 * @param element The element to insert. A copy of the element will be
 * inserted
 * @return
 * SP_BPQUEUE_INVALID_ARGUMENT if a NULL was sent as source or element
 * SP_BPQUEUE_OUT_OF_MEMORY if an allocation failed
 * SP_BPQUEUE_SUCCESS the element has been inserted successfully or
 * the element is too big (according to value + index)
 * SP_BPQUEUE_EMPTY we shoudn't get the message never
 *
 */
SP_BPQUEUE_MSG spBPQueueEnqueue(SPBPQueue source, SPListElement element);

/**
 * removes the element with the lowest value
 *
 * @param source The queue source for which to remove the smallest
 * element
 * @return
 * SP_BPQUEUE_INVALID_ARGUMENT if a NULL was sent as source
 * SP_BPQUEUE_EMPTY if the queue is empty so we can't remove any
 * element because there is not an element in the queue
 * SP_BPQUEUE_SUCCESS the element has been removed successfully
 *
 */
SP_BPQUEUE_MSG spBPQueueDequeue(SPBPQueue source);

/**
 * returns a NEW COPY of the element with the lowest value
 *
 * @param source The queue source for which to take the smallest
 * element
 * @return
 * SP_BPQUEUE_INVALID_ARGUMENT if a NULL was sent as source
 * SP_BPQUEUE_EMPTY if the queue is empty so we can't take any
 * element because there is not an element in the queue
 * SP_BPQUEUE_SUCCESS the element has been taken successfully
 *
 */
SPListElement spBPQueuePeek(SPBPQueue source);

/**
 * returns a NEW COPY of the element with the highest value
 *
 * @param source The queue source for which to take the biggest
 * element
 * @return
 * SP_BPQUEUE_INVALID_ARGUMENT if a NULL was sent as source
 * SP_BPQUEUE_EMPTY if the queue is empty so we can't take any
 * element because there is not an element in the queue
 * SP_BPQUEUE_SUCCESS the element has been taken successfully
 */
SPListElement spBPQueuePeekLast(SPBPQueue source);

/**
 * returns the minimum value in the queue
 *
 * @param source The queue source for which to take the minimum value
 * of an element
 * @return
 * -1 if a NULL was sent as source or if the queue is empty
 * value of the smallest element in the queue
 */
double spBPQueueMinValue(SPBPQueue source);

/**
 * returns the maximum value in the queue
 *
 * @param source The queue source for which to take the maximum value
 * of an element
 * @return
 * -1 if a NULL was sent as source or if the queue is empty
 * value of the biggest element in the queue
 */
double spBPQueueMaxValue(SPBPQueue source);

/**
 * returns true if the queue is empty, false if not
 *
 * @param source - The source queue
 * @assert (source != NUlL)
 * @return
 * 1 if queue is empty
 * 0 if queue is not empty
 */

bool spBPQueueIsEmpty(SPBPQueue source);

/**
 * returns true if the queue is full, false if not
 *
 * @param source - The source queue
 * @assert (source != NUlL)
 * @return
 * 1 if queue is full
 * 0 if queue is not full
 */
bool spBPQueueIsFull(SPBPQueue source);

/**
 * Dequeue the element with the min value in the queue returns the index of this element
 *
 * @param source The queue source for which to take index
 * @return
 * -1 if a NULL was sent as source or if the queue is empty
 * or fail to Dequeue from the queue
 * otherwise the index of the min value in the queue
 */
int spBPQueueIndexOfMinValue(SPBPQueue source);

/** set new size for the bpq **/
void spBPQueueSetSize(SPBPQueue source, int size);

#endif
