/*
 * SPPoint.c
 *
 *  Created on: 21 במאי 2016
 *      Author: Oleg
 */

#include <stdio.h>
#include "SPPoint.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>

/**
 * Allocates a new point in the memory.
 * Given data array, dimension dim and an index.
 * The new point will be P = (p_0,p_2,...,p_{dim-1})
 * such that the following holds
 *
 * - The ith coordinate of the P will be p_i
 * - p_i = data[i]
 * - The index of P = index
 *
 * @return
 * NULL in case allocation failure ocurred OR data is NULL OR dim <=0 OR index <0
 * Otherwise, the new point is returned
 */

SPPoint spPointCreate(double* data, int dim, int index) {
	if (data == NULL || dim <= 0 || index < 0)
		return NULL;
	SPPoint point = (SPPoint) malloc(sizeof(*point));
	if (point == NULL)
		return NULL;
	(*point).data = (double *) malloc(dim * sizeof(double));
	if (((*point).data) == NULL)
		return NULL;
	(*point).index = index;
	(*point).dimension = dim;
	int i;
	for (i = 0; i < dim; i++)
		(*point).data[i] = data[i];
	return point;
}

/**
 * Allocates a copy of the given point.
 *
 * Given the point source, the functions returns a
 * new pint P = (P_1,...,P_{dim-1}) such that:
 * - P_i = source_i (The ith coordinate of source and P are the same)
 * - dim(P) = dim(source) (P and source have the same dimension)
 * - index(P) = index(source) (P and source have the same index)
 *
 * @param source - The source point
 * @assert (source != NUlL)
 * @return
 * NULL in case memory allocation occurs
 * Others a copy of source is returned.
 */
SPPoint spPointCopy(SPPoint source) {
	assert(source!=NULL);
	SPPoint copyPoint = (SPPoint) malloc(sizeof(*copyPoint));
	if (copyPoint == NULL)
		return NULL;
	(*copyPoint).data = (double *) malloc(
			((*source).dimension) * sizeof(double));
	if ((*copyPoint).data == NULL)
		return NULL;
	(*copyPoint).index = (*source).index;
	(*copyPoint).dimension = (*source).dimension;
	int i;
	for (i = 0; i < ((*source).dimension); i++)
		(*copyPoint).data[i] = (*source).data[i];
	return copyPoint;
}

/**
 * Free all memory allocation associated with point,
 * if point is NULL nothing happens.
 */
void spPointDestroy(SPPoint point) {
	if (point != NULL) {
		free((*point).data);
		free(point);
	}
}

/**
 * A getter for the dimension of the point
 *
 * @param point - The source point
 * @assert point != NULL
 * @return
 * The dimension of the point
 */
int spPointGetDimension(SPPoint point) {
	assert(point!=NULL);
	return (*point).dimension;
}

/**
 * A getter for the index of the point
 *
 * @param point - The source point
 * @assert point != NULL
 * @return
 * The index of the point
 */
int spPointGetIndex(SPPoint point) {
	assert(point!=NULL);
	return (*point).index;

}

/**
 * A getter for specific coordinate value
 *
 * @param point - The source point
 * @param axis  - The coordinate of the point which
 * 				  its value will be retreived
 * @assert point!=NULL && axis < dim(point)
 * @return
 * The value of the given coordinate (p_axis will be returned)
 */
double spPointGetAxisCoor(SPPoint point, int axis) {
	assert(point!=NULL && axis<(*point).dimension);
	assert(axis >= 0);
	return (*point).data[axis];
}

/**
 * Calculates the L2-squared distance between p and q.
 * The L2-squared distance is defined as:
 * (p_1 - q_1)^2 + (p_2 - q_1)^2 + ... + (p_dim - q_dim)^2
 *
 * @param p - The first point
 * @param q - The second point
 * @assert p!=NULL AND q!=NULL AND dim(p) == dim(q)
 * @return
 * The L2-Squared distance between p and q
 */

double spPointL2SquaredDistance(SPPoint p, SPPoint q) {
	assert(p!=NULL && q!=NULL&& (p->dimension)==(q->dimension));
	double distance = 0;
	int i;
	for (i = 0; i < (*p).dimension; i++)
		distance += (double) ((*p).data[i] - (*q).data[i])
				* (double) ((*p).data[i] - (*q).data[i]);
	return distance;

}

