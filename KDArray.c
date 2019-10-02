/*
 * KDArray.c
 *
 *  Created on: 21 баев„ 2016
 *      Author: Oleg
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "KDArray.h"
#include "SPPoint.h"
#include "SPLogger.h"

#define DOUBLE_ARRAY_SIZE 2

//this data type will be used for sorting purposes
struct augmented_point {
	SPPoint point;
	int index; // indicates the index of the point in array of points
	int rowNumInMat; // indicates the coordinate which the points will be sorted by
};
//represents our kd-array
struct kd_array {
	SPPoint* arrayOfPoints;
	int size;
	int** mat;
};

bool equalDimensionForAllPoints(SPPoint* arr, int size);
void destroyMat(int** mat, int index);

int cmp(const void * a, const void * b);
void fillXArray(int * x, SPKDArray kdArr, int coor, int middle);
void destroyAuxArrays(int * x, SPKDArray *doubleKdArray, int * map1, int * map2);
void destroyAugPoints(AUGPoint * augPointsArray, int index);

SPKDArray Init(SPPoint* arr, int size) {
	AUGPoint * augPointsArray = NULL;
	SPKDArray kdArray = NULL;
	int dimension = 0;
	if (arr == NULL || size <= 0) {
		return NULL;
	}
	//make sure all the Dimensions are the same
	if (!equalDimensionForAllPoints(arr, size)) {
		spLoggerPrintError("different dimensions to points", __FILE__, __func__,
		__LINE__);
		return NULL;
	}
	if (arr[0] != NULL)
		dimension = spPointGetDimension(arr[0]);
	else {
		spLoggerPrintError("no points in arr, arr[0] = NULL", __FILE__,
				__func__, __LINE__);
		return NULL;
	}

	// Allocations
	kdArray = (SPKDArray) malloc(sizeof(*kdArray));
	if (kdArray == NULL) {
		spLoggerPrintError("Allocation Failure", __FILE__, __func__, __LINE__);
		return NULL;
	}
	augPointsArray = (AUGPoint*) malloc(size * sizeof(*augPointsArray));
	if (augPointsArray == NULL) {
		spLoggerPrintError("Allocation Failure", __FILE__, __func__, __LINE__);
		free(kdArray);
		return NULL;
	}
	//alloc memory for points
	kdArray->arrayOfPoints = (SPPoint*) malloc(
			size * sizeof(*kdArray->arrayOfPoints));
	if (kdArray->arrayOfPoints == NULL) {
		spLoggerPrintError("Allocation Failure", __FILE__, __func__, __LINE__);
		free(kdArray);
		free(augPointsArray);
		return NULL;
	}

	kdArray->size = size;
	//alloc memory for our Marix
	kdArray->mat = (int **) malloc(dimension * sizeof(int*));
	if (kdArray->mat == NULL) {
		spLoggerPrintError("Allocation Failure", __FILE__, __func__, __LINE__);
		destroyArrayOfPoints(kdArray->arrayOfPoints, kdArray->size);
		free(kdArray);
		free(augPointsArray);
		return NULL;
	}
	for (int i = 0; i < dimension; i++) {
		kdArray->mat[i] = (int *) malloc(size * sizeof(int));
		if (kdArray->mat[i] == NULL) {
			spLoggerPrintError("Allocation Failure", __FILE__, __func__,
			__LINE__);
			destroyMat(kdArray->mat, i);
			free(kdArray->arrayOfPoints);
			free(kdArray);
			free(augPointsArray);
			return NULL;
		}
	}
	//copy the given points to our kd_array struct and AUGPoint struct
	for (int i = 0; i < size; i++) {
		kdArray->arrayOfPoints[i] = spPointCopy(arr[i]);
		if (kdArray->arrayOfPoints[i] == NULL) {
			spLoggerPrintError("Allocation Failure", __FILE__, __func__,
			__LINE__);
			destroyMat(kdArray->mat, dimension);
			destroyArrayOfPoints(kdArray->arrayOfPoints, i);
			destroyAugPoints(augPointsArray, i);
			free(kdArray);
			return NULL;
		}
		augPointsArray[i].point = spPointCopy(arr[i]);
		if (augPointsArray[i].point == NULL) {
			spLoggerPrintError("Allocation Failure", __FILE__, __func__,
			__LINE__);
			destroyMat(kdArray->mat, dimension);
			destroyArrayOfPoints(kdArray->arrayOfPoints, i + 1);
			destroyAugPoints(augPointsArray, i);
			free(kdArray);
			return NULL;
		}
		augPointsArray[i].index = i;
	}

	//filling our Mat
	for (int i = 0; i < dimension; i++) {
		//changing the coordinate that we gonna sort by
		for (int j = 0; j < size; j++) {
			augPointsArray[j].rowNumInMat = i;
		}
		//sort via rowNumInMat coordinate
		qsort(augPointsArray, size, sizeof(struct augmented_point), cmp);

		//filling the indexes
		for (int j = 0; j < size; j++) {
			kdArray->mat[i][j] = augPointsArray[j].index;
		}
	}
	destroyAugPoints(augPointsArray, size);
	return kdArray;
}

SPKDArray * Split(SPKDArray kdArr, int coor) {
	if (kdArr == NULL
			|| coor
					< 0|| getMat(kdArr) == NULL || getArrayOfPoints(kdArr) == NULL) {
		spLoggerPrintError("Split method - INVALID ARGUMENTS", __FILE__,
				__func__, __LINE__);
		return NULL;
	}
	SPKDArray *doubleKdArray = NULL;
	SPKDArray kdLeft = NULL;
	SPKDArray kdRight = NULL;
	int l = 0;	// counter for LeftKDArray
	int r = 0;	// counter for rightKDArray
	int* map1 = NULL;
	int* map2 = NULL;
	int dimension = 0;
	int middle = (int) ceil((double) kdArr->size / 2); // index of the middle
	int *x = (int *) malloc(kdArr->size * sizeof(int));
	if (x == NULL) {
		spLoggerPrintError("Split method - Allocation Failure", __FILE__,
				__func__, __LINE__);
		return NULL;
	}
	doubleKdArray = (SPKDArray *) malloc(DOUBLE_ARRAY_SIZE * sizeof(SPKDArray));
	if (doubleKdArray == NULL) {
		spLoggerPrintError("Split method - Allocation Failure", __FILE__,
				__func__, __LINE__);
		destroyAuxArrays(x, NULL, NULL, NULL);
		return NULL;
	}
	map1 = (int *) malloc(kdArr->size * sizeof(int));
	if (map1 == NULL) {
		spLoggerPrintError("Split method - Allocation Failure", __FILE__,
				__func__, __LINE__);
		destroyAuxArrays(x, doubleKdArray, NULL, NULL);
		return NULL;
	}
	map2 = (int *) malloc(kdArr->size * sizeof(int));
	if (map1 == NULL) {
		spLoggerPrintError("Split method - Allocation Failure", __FILE__,
				__func__, __LINE__);
		destroyAuxArrays(x, doubleKdArray, map1, NULL);
		return NULL;
	}
	//initialize map1 & map2 with -1
	for (int i = 0; i < kdArr->size; i++) {
		map1[i] = -1;
		map2[i] = -1;
	}

	//alloc left kd-Array
	kdLeft = (SPKDArray) malloc(sizeof(*kdLeft));
	if (kdLeft == NULL) {
		spLoggerPrintError("Split method - Allocation Failure", __FILE__,
				__func__, __LINE__);
		destroyAuxArrays(x, doubleKdArray, map1, map2);
		return NULL;
	}

	kdLeft->arrayOfPoints = (SPPoint*) malloc(middle * sizeof(SPPoint));
	if (kdLeft->arrayOfPoints == NULL) {
		spLoggerPrintError("Split method - Allocation Failure", __FILE__,
				__func__, __LINE__);
		destroyAuxArrays(x, doubleKdArray, map1, map2);
		free(kdLeft);
		return NULL;
	}
	kdLeft->size = middle;
	//alloc the Mats of kdLeft and kdright
	if (kdArr->arrayOfPoints[0] != NULL)
		dimension = spPointGetDimension(kdArr->arrayOfPoints[0]);
	else {
		spLoggerPrintError("No points in kdArr.", __FILE__, __func__, __LINE__);
		destroyAuxArrays(x, doubleKdArray, map1, map2);
		free(kdLeft);
		return NULL;
	}
	kdLeft->mat = (int **) malloc(dimension * sizeof(int*));
	if (kdLeft->mat == NULL) {
		spLoggerPrintError("Allocation Failure", __FILE__, __func__, __LINE__);
		destroyAuxArrays(x, doubleKdArray, map1, map2);
		destroyArrayOfPoints(kdLeft->arrayOfPoints, kdLeft->size);
		free(kdLeft);
		return NULL;
	}
	for (int i = 0; i < dimension; i++) {
		kdLeft->mat[i] = (int *) malloc(middle * sizeof(int));
		if (kdLeft->mat[i] == NULL) {
			spLoggerPrintError("Allocation Failure", __FILE__, __func__,
			__LINE__);
			destroyAuxArrays(x, doubleKdArray, map1, map2);
			destroyArrayOfPoints(kdLeft->arrayOfPoints, kdLeft->size);
			destroyMat(kdLeft->mat, i);
			free(kdLeft);
			return NULL;
		}
	}

	kdRight = (SPKDArray) malloc(sizeof(*kdRight));
	if (kdRight == NULL) {
		spLoggerPrintError("Allocation Failure", __FILE__, __func__, __LINE__);
		destroyAuxArrays(x, doubleKdArray, map1, map2);
		destroyArrayOfPoints(kdLeft->arrayOfPoints, kdLeft->size);
		destroyMat(kdLeft->mat, kdLeft->size);
		free(kdLeft);
		return NULL;
	}
	kdRight->size = kdArr->size - middle;
	kdRight->arrayOfPoints = (SPPoint*) malloc(kdRight->size * sizeof(SPPoint));
	if (kdRight->arrayOfPoints == NULL) {
		spLoggerPrintError("Allocation Failure", __FILE__, __func__, __LINE__);
		destroyAuxArrays(x, doubleKdArray, map1, map2);
		destroyArrayOfPoints(kdLeft->arrayOfPoints, kdLeft->size);
		destroyMat(kdLeft->mat, kdLeft->size);
		free(kdLeft);
		free(kdRight);
		return NULL;
	}
	kdRight->mat = (int **) malloc(dimension * sizeof(int*));
	if (kdRight->mat == NULL) {
		spLoggerPrintError("Allocation Failure", __FILE__, __func__, __LINE__);
		destroyAuxArrays(x, doubleKdArray, map1, map2);
		destroyArrayOfPoints(kdLeft->arrayOfPoints, kdLeft->size);
		destroyArrayOfPoints(kdRight->arrayOfPoints, kdRight->size);
		destroyMat(kdLeft->mat, dimension);
		free(kdLeft);
		free(kdRight);
		return NULL;
	}
	for (int i = 0; i < dimension; i++) {
		kdRight->mat[i] = (int *) malloc(kdRight->size * sizeof(int));
		if (kdRight->mat[i] == NULL) {
			spLoggerPrintError("Allocation Failure", __FILE__, __func__,
			__LINE__);
			destroyAuxArrays(x, doubleKdArray, map1, map2);
			destroyArrayOfPoints(kdLeft->arrayOfPoints, kdLeft->size);
			destroyArrayOfPoints(kdRight->arrayOfPoints, kdRight->size);
			destroyMat(kdLeft->mat, dimension);
			free(kdLeft);
			destroyMat(kdLeft->mat, i);
			free(kdRight);
			return NULL;
		}
	}

	// fill x array according to coordinate coor
	fillXArray(x, kdArr, coor, middle);

	for (int i = 0; i < kdArr->size; i++) {
		if (x[i] == 0) {
			kdLeft->arrayOfPoints[l] = spPointCopy(kdArr->arrayOfPoints[i]);
			if (kdLeft->arrayOfPoints[l] == NULL) {
				spLoggerPrintError("point = NULL", __FILE__, __func__,
				__LINE__);
				destroyAuxArrays(x, doubleKdArray, map1, map2);
				destroyArrayOfPoints(kdLeft->arrayOfPoints, l);
				free(kdLeft);
				destroyArrayOfPoints(kdRight->arrayOfPoints, r);
				free(kdRight);
				return NULL;
			}
			map1[i] = l; // map1[k] = -1, default for k where x[k] != 0
			l++;
		}
		//x[i]==1
		else {
			kdRight->arrayOfPoints[r] = spPointCopy(kdArr->arrayOfPoints[i]);
			if (kdRight->arrayOfPoints[r] == NULL) {
				spLoggerPrintError("point = NULL", __FILE__, __func__,
				__LINE__);
				destroyAuxArrays(x, doubleKdArray, map1, map2);
				destroyArrayOfPoints(kdLeft->arrayOfPoints, l);
				free(kdLeft);
				destroyArrayOfPoints(kdRight->arrayOfPoints, r);
				free(kdRight);
				return NULL;
			}
			map2[i] = r;
			r++;
		}
	}
	//filling the Mats of kdLeft and kdRight
	for (int i = 0; i < dimension; i++) {
		l = r = 0;
		for (int j = 0; j < kdArr->size; j++) {
			if (x[kdArr->mat[i][j]] == 0) {
				kdLeft->mat[i][l] = map1[kdArr->mat[i][j]];
				l++;
			}
			//x[kdArr->mat[i][j]]==1
			else {
				kdRight->mat[i][r] = map2[kdArr->mat[i][j]];
				r++;
			}
		}
	}
	doubleKdArray[0] = kdLeft;
	doubleKdArray[1] = kdRight;
	destroyAuxArrays(x, NULL, map1, map2);
	destroyArrayOfPoints(kdArr->arrayOfPoints, kdArr->size);
	destroyMat(kdArr->mat, dimension);
	free(kdArr);
	return doubleKdArray;

}

/** filling x array according to the coordinate coor sorting **/
void fillXArray(int * x, SPKDArray kdArr, int coor, int middle) {
	for (int i = 0; i < middle; i++) {
		x[kdArr->mat[coor][i]] = 0;
	}
	for (int i = middle; i < kdArr->size; i++) {
		x[kdArr->mat[coor][i]] = 1;
	}
}

/** check if all points have equal dimension **/
bool equalDimensionForAllPoints(SPPoint* arr, int size) {
	if (arr == NULL)
		printf("%s", "arr = null");
	for (int i = 0; i < size - 1; i++) {
		if (arr[i] != NULL && arr[i + 1] != NULL)
			if (spPointGetDimension(arr[i])
					!= spPointGetDimension(arr[i + 1])) {
				return false;
			}
	}
	return true;
}

/** free mat until the index we reached. **/
void destroyMat(int** mat, int index) {
	for (int i = 0; i < index; i++) {
		free(mat[i]);
	}
	free(mat);
}

/** free resources in array of SPPoints **/
void destroyArrayOfPoints(SPPoint* arr, int index) {
	for (int i = 0; i < index; i++) {
		spPointDestroy(arr[i]);
	}
	free(arr);
}

/** free resources in augPoints array **/
void destroyAugPoints(AUGPoint * augPointsArray, int index) {
	for (int i = 0; i < index; i++) {
		spPointDestroy(augPointsArray[i].point);
	}
	free(augPointsArray);
}

/** free resources in the helper arrays we used **/
void destroyAuxArrays(int * x, SPKDArray *doubleKdArray, int * map1, int * map2) {
	if (x != NULL)
		free(x);
	if (doubleKdArray != NULL)
		free(doubleKdArray);
	if (map1 != NULL)
		free(map1);
	if (map2 != NULL)
		free(map2);
}

/** compare function for SPPoint, allows us to sort the points **/
int cmp(const void * a, const void * b) {
	const struct augmented_point *elem1 = (struct augmented_point *) a;
	const struct augmented_point *elem2 = (struct augmented_point *) b;
	if (spPointGetAxisCoor(elem1->point, elem1->rowNumInMat)
			> spPointGetAxisCoor(elem2->point, elem2->rowNumInMat))
		return 1;
	else if (spPointGetAxisCoor(elem1->point, elem1->rowNumInMat)
			< spPointGetAxisCoor(elem2->point, elem2->rowNumInMat))
		return -1;
	else
		return 0;
}

/** getters **/

SPPoint* getArrayOfPoints(SPKDArray kdArray) {
	if (kdArray == NULL)
		return NULL;
	return kdArray->arrayOfPoints;
}
int getSize(SPKDArray kdArray) {
	if (kdArray == NULL)
		return -1;
	return kdArray->size;
}
int** getMat(SPKDArray kdArray) {
	if (kdArray == NULL)
		return NULL;
	return kdArray->mat;
}

