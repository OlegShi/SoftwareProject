/*
 * KDArray.h
 *
 *  Created on: 21 баев„ 2016
 *      Author: Oleg
 */

#ifndef KDARRAY_H_
#define KDARRAY_H_

#include "SPPoint.h"

/** type used to define a point in array of points**/
typedef struct augmented_point AUGPoint;

/** type used to define KD-Array **/
typedef struct kd_array* SPKDArray;

/**
 *
 * Initializes the kd-array with the data given by arr.

 * @return
 * 	NULL - If arr=NULL or allocations failed or size<=0
 * 	or not all the dimensions of the points are equal.
 * 	A new KDArray in case of success.
 */
SPKDArray Init(SPPoint* arr, int size);

/**
 *
 * Returns two kd-arrays (kdLeft, kdRight) such that the first
 * n/2 points with respect to the coordinate coor are in kdLeft ,
 * and the rest of the points are in kdRight.
 * @return
 * NULL - If kdArr=NULL or allocations failed or coor<0
 * SPKDArray array of size two in case of success.
 */
SPKDArray * Split(SPKDArray kdArr, int coor);

/** getters **/
SPPoint* getArrayOfPoints(SPKDArray kdArray);
int getSize(SPKDArray kdArray);
int** getMat(SPKDArray kdArray);

/** free resources functions **/
void destroyArrayOfPoints(SPPoint* arr, int index);
void destroyMat(int** mat, int index);

#endif /* KDARRAY_H_ */
