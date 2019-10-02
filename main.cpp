#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "SPImageProc.h"
extern "C" {
#include "SPPoint.h"
#include "SPLogger.h"
#include "SPConfig.h"
#include "main_aux.h"
#include "KDArray.h"
#include "KDTreeNode.h"
#include "SPBPriorityQueue.h"

}
#define MAX_LENGTH 1025
using namespace sp;

int main(int argc, char** argv) {
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	char extensionFeats[] = ".feats";
	int actualNumberOfImages = 0;
	SPConfig config = NULL;
	if (argc == 1) { // default file
		config = spConfigCreate("spcbir.config", &msg);
		if (config == NULL) {
			if (msg == SP_CONFIG_CANNOT_OPEN_FILE)
				printf("%s",
						"The default configuration file spcbir.config couldn’t be open");
			exit(0);
		}
	} else if (argc == 3 && strcmp(argv[1], "-c") == 0) {
		config = spConfigCreate(argv[2], &msg);
		if (config == NULL) {
			if (msg == SP_CONFIG_CANNOT_OPEN_FILE) {
				printf("%s %s %s", "The configuration file", argv[2],
						"couldn’t be open");
			}
			exit(0);
		}
	} else {
		printf("%s", "Invalid command line : use -c <config_filename>");
		exit(0);
	}
	SP_LOGGER_LEVEL level = (SP_LOGGER_LEVEL) spConfigGetLoggerLevel(config,
			&msg); // get loger level from config
	char* loggerFileName = spConfigGetLoggerFilename(config, &msg);
	if (strcmp(loggerFileName, "stdout") == 0)
		loggerFileName = NULL; // if loggerFileName = NULL than we print to stdout
	if (spLoggerCreate(loggerFileName, level) != SP_LOGGER_SUCCESS) {
		printf("%s", "Error : Can't initalize logger");
		spConfigDestroy(config);
		exit(0);
	}
	int numOfImages = spConfigGetNumOfImages(config, &msg);
	int totalNumberOfFeatures = 0, numOfFeats = 0;
	int dimension = spConfigGetPCADim(config, &msg);
	int spNumOfSimilarImages = spConfigGetNumOfSimilarImages(config, &msg);
	if (spNumOfSimilarImages <= 0) {
		spLoggerPrintError("Error : number of similar images <= 0", __FILE__,
				__func__, __LINE__);
		spConfigDestroy(config);
		spLoggerDestroy();
		exit(0);
	}
	char* imagePath = (char*) malloc(sizeof(char) * MAX_LENGTH); // for example: "./images/img10.png"
	char* imageFeatsExtensionPath = (char*) malloc(sizeof(char) * MAX_LENGTH); // for example: "./images/img10.feats"
	SPPoint* arr = NULL; // contain total number of points from all images
	if (imagePath == NULL || imageFeatsExtensionPath == NULL) {
		spLoggerPrintError("Allocation Failure", __FILE__, __func__, __LINE__);
		freeResources(imagePath, imageFeatsExtensionPath, NULL, NULL, NULL);
		spConfigDestroy(config);
		spLoggerDestroy();
		exit(0);
	}
	ImageProc imagePro(config);
	if (spConfigIsExtractionMode(config, &msg)) { // we should be in extractionMode to write feats files
		int j = 0;
		for (int i = 0; i < numOfImages; i++) {
			msg = spConfigGetImagePath(imagePath, config, i);
			if (msg != SP_CONFIG_SUCCESS) {
				if (msg == SP_CONFIG_INVALID_ARGUMENT)
					spLoggerPrintError("Error : imagepath = NULL", __FILE__,
							__func__, __LINE__);
				else
					// msg == SP_CONFIG_INDEX_OUT_OF_RANGE
					spLoggerPrintError(
							"Error : The given index is bigger than the number of images",
							__FILE__, __func__, __LINE__);
				freeResources(imagePath, imageFeatsExtensionPath, NULL, NULL,
				NULL);
				spConfigDestroy(config);
				spLoggerDestroy();
				exit(0);
			}
			msg = spConfigGetImageFeatsPath(imageFeatsExtensionPath, config, i,
					extensionFeats);
			if (msg != SP_CONFIG_SUCCESS) {
				if (msg == SP_CONFIG_INVALID_ARGUMENT)
					spLoggerPrintError(
							"Error : The path of the feats file = NULL",
							__FILE__, __func__, __LINE__);
				else
					// msg == SP_CONFIG_INDEX_OUT_OF_RANGE
					spLoggerPrintError(
							"Error : The given index is bigger than the number of images",
							__FILE__, __func__, __LINE__);
				freeResources(imagePath, imageFeatsExtensionPath, NULL, NULL,
				NULL);
				spConfigDestroy(config);
				spLoggerDestroy();
				exit(0);
			}
			SPPoint* points = imagePro.getImageFeatures(imagePath, i,
					&numOfFeats);
			if (points != NULL) {
				actualNumberOfImages++;
				totalNumberOfFeatures += numOfFeats;
				createFeatsFileForImage(points, i, numOfFeats,
						imageFeatsExtensionPath);
				arr = (SPPoint*) realloc(arr,
						totalNumberOfFeatures * sizeof(*arr));

				j = totalNumberOfFeatures - numOfFeats;
				for (int k = 0; k < numOfFeats; k++) {
					arr[j] = spPointCopy(points[k]);
					j++;
				}
				free(points);
			}
		}
		// if we don't have enough images then quit
		if (actualNumberOfImages < spNumOfSimilarImages) {
			spLoggerPrintError(
					"actual number of images is smaller than the number of similar images we were asked to present",
					__FILE__, __func__, __LINE__);
			freeResources(imagePath, imageFeatsExtensionPath, NULL, NULL, NULL);
			spConfigDestroy(config);
			spLoggerDestroy();
			exit(0);
		}

	} else {
		arr = ExtractFeaturesFromFiles(numOfImages, imageFeatsExtensionPath,
				extensionFeats, &totalNumberOfFeatures, &msg, config,
				&actualNumberOfImages);

		// if we don't have enough images then quit
		if (actualNumberOfImages < spNumOfSimilarImages) {
			spLoggerPrintError(
					"actual number of images is smaller than the number of similar images we were asked to present",
					__FILE__, __func__, __LINE__);
			freeResources(imagePath, imageFeatsExtensionPath, NULL, NULL, NULL);
			spConfigDestroy(config);
			spLoggerDestroy();
			exit(0);
		}
	}
	KDTreeNode* kdTreeNode = InitKDTree(Init(arr, totalNumberOfFeatures),
			spConfigGetSplitMethod(config), dimension, dimension);
	for (int i = 0; i < totalNumberOfFeatures; i++)
		spPointDestroy(arr[i]);
	free(arr);
	if (kdTreeNode == NULL) {
		spLoggerPrintError("kdTree node = NULL", __FILE__, __func__, __LINE__);
		freeResources(imagePath, imageFeatsExtensionPath, NULL, NULL, NULL);
		spConfigDestroy(config);
		spLoggerDestroy();
		exit(0);
	}
	SPBPQueue bpq = NULL;
	SPPoint *featuresOfQuery = NULL;
	char* candidatePath = (char*) malloc(sizeof(char) * MAX_LENGTH);
	Hits * arrayOfHits = NULL; // contains a index for every image and the number of hits for that image.
	int * indexesOfBestCandidates = (int *) malloc(
			sizeof(int) * spNumOfSimilarImages); // decreasing order (i.e the best is first And so on)
	if (indexesOfBestCandidates == NULL || candidatePath == NULL) {
		spLoggerPrintError("Allocation Failure", __FILE__, __func__, __LINE__);
		freeResources(imagePath, imageFeatsExtensionPath, candidatePath,
				indexesOfBestCandidates, NULL);
		destroy(kdTreeNode);
		spConfigDestroy(config);
		spLoggerDestroy();
		spBPQueueDestroy(bpq);
		exit(0);
	}
	int spKNN = getSpKNN(config, &msg);
	bpq = spBPQueueCreate(spKNN);
	if (bpq == NULL) {
		spLoggerPrintError("Error while creating bpq", __FILE__, __func__,
		__LINE__);
		freeResources(imagePath, imageFeatsExtensionPath, candidatePath,
				indexesOfBestCandidates, NULL);
		destroy(kdTreeNode);
		spConfigDestroy(config);
		spLoggerDestroy();
		exit(0);
	}
	// Query part
	char* query = (char*) malloc(MAX_LENGTH * sizeof(char));
	printf("%s", "Please enter an image path:\n");
	fflush(NULL);
	scanf("%s", query);
	fflush(NULL);
	char terminateString[] = "<>";
	while (strcmp(query, terminateString) != 0) {
		featuresOfQuery = imagePro.getImageFeatures(query, numOfImages,
				&numOfFeats);
		if (featuresOfQuery == NULL) {
			spLoggerPrintWarning("Invalid query", __FILE__, __func__, __LINE__);
			printf("%s", "Please enter an image path:\n");
			fflush(NULL);
			scanf("%s", query);
			fflush(NULL);
			continue;
		}
		arrayOfHits = (Hits *) malloc(numOfImages * sizeof(*arrayOfHits));
		if (arrayOfHits == NULL) {
			spLoggerPrintError("Allocation Failure", __FILE__, __func__,
			__LINE__);
			freeResources(imagePath, imageFeatsExtensionPath, candidatePath,
					indexesOfBestCandidates, query);
			destroy(kdTreeNode);
			spConfigDestroy(config);
			spLoggerDestroy();
			spBPQueueDestroy(bpq);
			exit(0);
		}
		initializeArray(arrayOfHits, numOfImages); //fill with zeros
		for (int i = 0; i < numOfFeats; i++) {
			kNearestNeighbors(kdTreeNode, &bpq, featuresOfQuery[i]); // update bpq to contain k nearest neighbors
			updateArrayOfHits(arrayOfHits, bpq);
			spBPQueueClear(bpq);
			spBPQueueSetSize(bpq, spKNN);
		}
		calculateTheBestIndexes(indexesOfBestCandidates, arrayOfHits,
				spNumOfSimilarImages, numOfImages);
		free(arrayOfHits);

		if (spConfigMinimalGui(config, &msg)) {
			//check msg and act accordingly
			for (int i = 0; i < spNumOfSimilarImages; i++) {
				msg = spConfigGetImagePath(candidatePath, config,
						indexesOfBestCandidates[i]);
				//check msg and act accordingly
				imagePro.showImage(candidatePath);
			}
		} else {
			printf("%s %s %s", "Best candidates for -", query, "- are:\n");
			fflush(NULL);
			for (int i = 0; i < spNumOfSimilarImages; i++) {
				msg = spConfigGetImagePath(candidatePath, config,
						indexesOfBestCandidates[i]);
				printf("%s%s", candidatePath, "\n");
				fflush(NULL);
			}
		}
		free(featuresOfQuery);
		printf("%s", "Please enter an image path:\n");
		fflush(NULL);
		scanf("%s", query);
		fflush(NULL);
	}
	printf("%s", "Exiting...\n");
	fflush(NULL);
	// free all resources
	freeResources(imagePath, imageFeatsExtensionPath, candidatePath,
			indexesOfBestCandidates, query);
	destroy(kdTreeNode);
	spConfigDestroy(config);
	spLoggerDestroy();
	spBPQueueDestroy(bpq);
	return 0;
}
