/*
 * main_aux.h
 *
 *  Created on: 24 баев 2016
 *      Author: Tal
 */

#ifndef MAIN_AUX_H_
#define MAIN_AUX_H_

#include <stdbool.h>
#include "SPPoint.h"
#include "SPConfig.h"
#include "SPBPriorityQueue.h"

typedef struct Hits {
	int index;
	int hitsValue;
} Hits;

/*
 * stores each of these features to a file which will be located
 * in the directory given by spImagesDirectory.
 *
 * @param points - the array of features
 * @param index - the index of the image
 * @param numOfFeats - the actual features extracted
 * @param fileName - the name of the file(spImagesPrefix+index)
 *
 */
void createFeatsFileForImage(SPPoint* points, int index, int numOfFeats,
		char* fileName);

/** extract the array of points from the feats files **/
SPPoint* ExtractFeaturesFromFiles(int numOfImages,
		char* imageFeatsExtensionPath, char* extensionFeats,
		int* totalNumberOfFeatures, SP_CONFIG_MSG* msg, SPConfig config,
		int* actualNumberOfImages);

/** initiate array of hits (for the images) with zeros. **/
void initializeArray(Hits * arrayOfHits, int size);

/** update array of hits according to the bpq **/
void updateArrayOfHits(Hits * arrayOfHits, SPBPQueue bpq);

/** calculate the most similar images indexes **/
void calculateTheBestIndexes(int *IndexesOfBestCandidates, Hits * arrayOfHits,
		int spNumOfSimilarImages, int numOfImages);
void freeResources(char* imagePath, char* imageFeatsExtensionPath,
		char* candidatePath, int* IndexesOfBestCandidates, char* query);

int cmpHitsFunc(const void* a, const void* b);

#endif /* MAIN_AUX_H_ */
