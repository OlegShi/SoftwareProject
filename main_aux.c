/*
 * main_aux.c
 *
 *  Created on: 24 баев 2016
 *      Author: Tal
 */

#include "main_aux.h"

#include "SPConfig.h"
#include "SPPoint.h"
#include "SPListElement.h"
#include "SPLogger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#define MAX_LENGTH 1025

void createFeatsFileForImage(SPPoint* points, int index, int numOfFeats,
		char* fileName) {

	int pointDimension = 0;
	FILE * fp = fopen(fileName, "w");

	// stores the info in the following order:
	// 1.index of image
	// 2.actual number of features
	// 3.data for every dimension separated by space

	// stores the index of the image
	fprintf(fp, "%d\n", index);
	// stores the actual number of features at the beginning
	fprintf(fp, "%d\n", numOfFeats);
	for (int i = 0; i < numOfFeats; i++) {
		pointDimension = spPointGetDimension(points[i]);
		fprintf(fp, "%d %d", pointDimension, spPointGetIndex(points[i]));
		for (int j = 0; j < pointDimension; j++) {
			fprintf(fp, " %.4g", spPointGetAxisCoor(points[i], j)); // save 4 digits after the point
		}
		fprintf(fp, "%s", "\n");
	}
	fclose(fp);
}

SPPoint* ExtractFeaturesFromFiles(int numOfImages,
		char* imageFeatsExtensionPath, char* extensionFeats,
		int* totalNumberOfFeatures, SP_CONFIG_MSG* msg, SPConfig config,
		int* actualNumberOfImages) {
	SPPoint* arr = NULL;
	int dimension = 0;
	int index = 0;
	int numOfFeats = 0;
	int indexArray = 0;
	for (int i = 0; i < numOfImages; i++) {
		*msg = spConfigGetImageFeatsPath(imageFeatsExtensionPath, config, i,
				extensionFeats);
		FILE * fp = fopen(imageFeatsExtensionPath, "r"); // get the "i" feats file
		if (fp == NULL) {
			char message[MAX_LENGTH];
			sprintf(message, "%s %d %s", "File", i, "doesn't exist");
			spLoggerPrintWarning(message, __FILE__, __func__, __LINE__);
			continue;
		}
		fscanf(fp, "%d\n", &index);
		fscanf(fp, "%d\n", &numOfFeats);
		*totalNumberOfFeatures += numOfFeats;
		arr = (SPPoint*) realloc(arr, *totalNumberOfFeatures * sizeof(*arr));
		for (int j = 0; j < numOfFeats; j++) {
			fscanf(fp, "%d\n", &dimension); // get the dimension from the current point
			fscanf(fp, "%d\n", &index); // get the index from the current point
			double * arrValuesPoint = (double*) malloc(
					sizeof(double) * numOfFeats);
			for (int k = 0; k < dimension - 1; k++) {
				fscanf(fp, " %lg", &arrValuesPoint[k]); // get the double array from the current point
			}
			fscanf(fp, "%lg\n", &arrValuesPoint[dimension - 1]);
			arr[indexArray] = spPointCreate(arrValuesPoint, dimension, i);
			indexArray++;
		}
		*actualNumberOfImages = *actualNumberOfImages + 1;
		fclose(fp);
	}
	return arr;
}

void initializeArray(Hits * arrayOfHits, int size) {
	for (int i = 0; i < size; i++) {
		arrayOfHits[i].index = i;
		arrayOfHits[i].hitsValue = 0;
	}
}

void updateArrayOfHits(Hits * arrayOfHits, SPBPQueue bpq) {
	int size = spBPQueueSize(bpq);
	int index = 0;
	for (int i = 0; i < size; i++) {
		index = spBPQueueIndexOfMinValue(bpq);
		arrayOfHits[index].hitsValue += 1;
	}
}

void calculateTheBestIndexes(int *IndexesOfBestCandidates, Hits * arrayOfHits,
		int spNumOfSimilarImages, int numOfImages) {
	qsort(arrayOfHits, numOfImages, sizeof(Hits), cmpHitsFunc);
	for (int i = 0; i < spNumOfSimilarImages; i++) {
		IndexesOfBestCandidates[i] = arrayOfHits[i].index;
	}
}

int cmpHitsFunc(const void* a, const void* b) {
	const Hits* elem1 = (Hits*) a;
	const Hits* elem2 = (Hits*) b;

	if ((elem1->hitsValue) > (elem2->hitsValue))
		return -1;
	else if ((elem1->hitsValue) < (elem2->hitsValue))
		return 1;
	else if ((elem1->index) > (elem2->index))
		return 1;
	else if ((elem1->index) < (elem2->index))
		return -1;
	return 0; //should not get here
}

void freeResources(char* imagePath, char* imageFeatsExtensionPath,
		char* candidatePath, int* indexesOfBestCandidates, char* query) {
	if (imagePath != NULL)
		free(imagePath);
	if (imageFeatsExtensionPath != NULL)
		free(imageFeatsExtensionPath);
	if (candidatePath != NULL)
		free(candidatePath);
	if (indexesOfBestCandidates != NULL)
		free(indexesOfBestCandidates);
	if (query != NULL)
		free(query);
}
