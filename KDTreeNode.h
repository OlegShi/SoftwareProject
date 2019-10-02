/*
 * KDTreeNode.h
 *
 *  Created on: 25 баев 2016
 *      Author: Tal
 */

#ifndef KDTREENODE_H_
#define KDTREENODE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "KDArray.h"
#include "SPPoint.h"
#include "SPConfig.h"
#include "SPBPriorityQueue.h"

/** type used to define KDTreeNode **/
typedef struct kd_tree_node_t KDTreeNode;

/**
 *
 * Initializes the kdTreeNode with the data given by kdArray.

 * @return
 * 	NULL - If kdArray==NULL or allocations failed or dimensions <= 0 or incrementalCurrentDimension < 0
 * 	or the split method is not one of the values in KDTreeSplitMethod  enum.
 * 	A new KDTreeNode in case of success.
 */
KDTreeNode* InitKDTree(SPKDArray kdArray, KDTreeSplitMethod splitMethod,
		int dimensions, int incrementalCurrentDimension);

/** return the dimension we need to work with according to splitMethod parameter **/
int findDimension(SPKDArray kdArray, KDTreeSplitMethod splitMethod,
		int dimensions, int incrementalCurrentDimension);

/** get the median value according to the given split coordinate. **/
double getMedianValue(SPKDArray kdArray, int size, int splitCoor);

/** check if node is a leaf **/
bool isLeaf(KDTreeNode* node);

/** getters (KDTreeNode information) **/
int getDim(KDTreeNode* node);
double getVal(KDTreeNode* node);
KDTreeNode* getLeftChild(KDTreeNode* node);
KDTreeNode* getRightChild(KDTreeNode* node);
SPPoint getPoint(KDTreeNode* node);
void destroy(KDTreeNode* node);

/** update bpq to include the k similar points to SPPoint p **/
void kNearestNeighbors(KDTreeNode* curr, SPBPQueue *bpq, SPPoint p);

#endif /* KDTREENODE_H_ */
