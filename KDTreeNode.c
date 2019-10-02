/*
 * KDTreeNode.c
 *
 *  Created on: 25 баев 2016
 *      Author: Tal
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "KDArray.h"
#include "KDTreeNode.h"
#include "SPPoint.h"
#include "SPConfig.h"
#include "SPBPriorityQueue.h"
#include "SPLogger.h"

struct kd_tree_node_t {
	int dim; //splitting dimension
	double val; // median value of the splitting dimension
	struct kd_tree_node_t* left;
	struct kd_tree_node_t* right;
	SPPoint data;
};

KDTreeNode* InitKDTree(SPKDArray kdArray, KDTreeSplitMethod splitMethod,
		int dimensions, int incrementalCurrentDimension) {
	if (kdArray == NULL || dimensions <= 0|| incrementalCurrentDimension < 0 ||
	getMat(kdArray) == NULL || getArrayOfPoints(kdArray) == NULL) {
		spLoggerPrintError("InitKDTree - Invalid arguments", __FILE__, __func__,
				__LINE__);
		return NULL;
	}
	if (splitMethod != MAX_SPREAD && splitMethod != RANDOM
			&& splitMethod != INCREMENTAL) {
		spLoggerPrintError("InitKDTree - Invalid splitMethod argument",
				__FILE__, __func__, __LINE__);
		return NULL;
	}
	KDTreeNode* node = (KDTreeNode*) malloc(sizeof(*node));
	if (node == NULL) {
		spLoggerPrintError("Allocation Failure", __FILE__, __func__, __LINE__);
		return NULL;
	}
	int size = getSize(kdArray);
	int splitCoor = 0;
	node->dim = -1;
	node->val = -1;
	node->left = NULL;
	node->right = NULL;
	node->data = NULL;
	if (size == 1) {
		node->data = spPointCopy(getArrayOfPoints(kdArray)[0]);
		destroyArrayOfPoints(getArrayOfPoints(kdArray), size);
		destroyMat(getMat(kdArray), dimensions);
		free(kdArray);
		if (node->data == NULL) {
			spLoggerPrintError("Error while copying a point to node->data",
					__FILE__, __func__, __LINE__);
			destroy(node);
			return NULL;
		}
		return node;
	}
	splitCoor = findDimension(kdArray, splitMethod, dimensions,
			incrementalCurrentDimension);
	node->dim = splitCoor;
	node->val = getMedianValue(kdArray, size, splitCoor);
	SPKDArray * doubleArray = Split(kdArray, splitCoor);
	if (doubleArray == NULL) {
		spLoggerPrintError("SPKDArrays returned from split equals to NULL",
				__FILE__, __func__, __LINE__);
		destroy(node);
		return NULL;
	}

	// double Array

	node->left = InitKDTree(doubleArray[0], splitMethod, dimensions, splitCoor);
	node->right = InitKDTree(doubleArray[1], splitMethod, dimensions,
			splitCoor);
	node->data = NULL;
	free(doubleArray);
	return node;
}
int findDimension(SPKDArray kdArray, KDTreeSplitMethod splitMethod,
		int dimensions, int incrementalCurrentDimension) {
	if (splitMethod == MAX_SPREAD) {
		int** mat = getMat(kdArray);
		double maxSpread = -1;
		int coorMaxSpread = 0;
		SPPoint* arrayOfPoints = getArrayOfPoints(kdArray);
		for (int i = 0; i < dimensions; i++) {
			// mat is sorted
			int minPointIndex = mat[i][0];
			int maxPointIndex = mat[i][getSize(kdArray) - 1];
			double diff = spPointGetAxisCoor(arrayOfPoints[maxPointIndex], i)
					- spPointGetAxisCoor(arrayOfPoints[minPointIndex], i);
			if (maxSpread < diff) {
				maxSpread = diff;
				coorMaxSpread = i;
			}
		}
		return coorMaxSpread;
	} else if (splitMethod == RANDOM) {
		// random number between 0 to dimensions-1
		srand(time(NULL));
		return rand() % dimensions;
	} else { // splitMethod = INCREMENTAL
		return (incrementalCurrentDimension + 1) % dimensions;
	}
}

double getMedianValue(SPKDArray kdArray, int size, int splitCoor) {
	int middle = (int) ceil((double) size / 2);
	int** mat = getMat(kdArray);
	int pointIndex = mat[splitCoor][middle];
	return spPointGetAxisCoor(getArrayOfPoints(kdArray)[pointIndex], splitCoor);
}

void kNearestNeighbors(KDTreeNode* curr, SPBPQueue *bpq, SPPoint p) {
	int lastState = 0; //0-left 1-right
	SPListElement element = NULL;
	//if NULL do nothing
	if (curr == NULL || p == NULL) {
		return;
	}
	//if leaf Add the current point to the BPQ
	if (isLeaf(curr)) {
		element = spListElementCreate(spPointGetIndex(curr->data),
				spPointL2SquaredDistance(curr->data, p));
		if (spBPQueueEnqueue(*bpq, element) != SP_BPQUEUE_SUCCESS) {
			//print message
		}
		spListElementDestroy(element);
		return;
	}

	//Recursively search the half of the tree that contains the test point
	if (spPointGetAxisCoor(p, getDim(curr)) <= getVal(curr)) {
		lastState = 0;
		kNearestNeighbors(getLeftChild(curr), bpq, p);
	} else {
		lastState = 1;
		kNearestNeighbors(getRightChild(curr), bpq, p);
	}
	//If the candidate hypersphere crosses this splitting plane, look on the
	//other side of the plane by examining the other subtree

	if (!spBPQueueIsFull(*bpq)
			|| pow(getVal(curr) - spPointGetAxisCoor(p, getDim(curr)), 2)
					< spBPQueueMaxValue(*bpq)) {
		if (lastState == 0) {
			kNearestNeighbors(getRightChild(curr), bpq, p);
		} else {
			kNearestNeighbors(getLeftChild(curr), bpq, p);
		}
	}
}

bool isLeaf(KDTreeNode* node) {
	if (node->left == NULL && node->right == NULL) {
		return true;
	}
	return false;
}

int getDim(KDTreeNode* node) {
	return node->dim;
}

double getVal(KDTreeNode* node) {
	return node->val;
}

KDTreeNode* getLeftChild(KDTreeNode* node) {
	return node->left;
}

KDTreeNode* getRightChild(KDTreeNode* node) {
	return node->right;
}
SPPoint getPoint(KDTreeNode* node) {
	return node->data;
}

void destroy(KDTreeNode* node) {
	if (node == NULL)
		return;
	destroy(node->left);
	destroy(node->right);
	spPointDestroy(node->data);
	free(node);
}
