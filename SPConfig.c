/*
 * SPConfig.c
 *
 *  Created on: 11 ·‡Â‚ 2016
 *      Author: Tal
 */
#include "SPConfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#define SP_FILE_OPEN_MODE "r"

#define SP_PCA_DIMENSION_DEFAULT_VALUE 20
#define SP_PCA_FILENAME_DEFAULT_VALUE "pca.yml"
#define SP_NUM_OF_FEATURES_DEFAULT_VALUE 100
#define SP_NUM_OF_SIMILAR_IMAGES_DEFAULT_VALUE 1
#define SP_KNN_DEFAULT_VALUE 1
#define SP_LOGGER_LEVEL_DEFAULT_VALUE 3
#define SP_LOGGER_FILENAME_DEFAULT_VALUE "stdout"
#define MAX_LENGTH 1025
#define PCA_DIMENSION_MIN_RANGE 10
#define PCA_DIMENSION_MAX_RANGE 28
#define LOGGER_LEVEL_MIN_RANGE 1
#define LOGGER_LEVEL_MAX_RANGE 4

#define FILE_PRINT "File: "
#define LINE_PRINT "Line: "
#define MESSAGE_CONFIGURATION_PRINT "Message: Invalid configuration line\n"
#define MESSAGE_CONSTRAINT_PRINT "Message: Invalid value - constraint not met\n"
#define MESSAGE_NOT_SET_PRINT_PART1 "Parameter "
#define MESSAGE_NOT_SET_PRINT_PART2 " is not set\n"
#define ALLOCATION_PRINT "Allocation failure error"
#define INVALID_COMMAND_LINE "Invalid command line : use -c "
#define FILE_CANT_OPEN_PART1 "The configuration file "
#define FILE_CANT_OPEN_PART2 " couldnít be open\n"
#define DEFAULT_FILE_CANT_OPEN "The default configuration file spcbir.config couldnít be open\n"

#define DEFAULT_FILE_NAME "spcbir.config"

bool isEmptyLine(char line[]); // check if the line is empty
bool isAComment(char line[]); // check if the line is a comment line according to the definition in the project
bool containComment(char line[]); // checl if the line contains a comment
bool containOneEqualSign(char line[]);
bool stringContainWhiteSpaces(char str[], char* part, char stopChar); // check if there are white spaces in the middle of a string
bool isANumber(char* str); // check if str is actually a number
/** Get config information (getters) **/
int spConfigGetLoggerLevel(const SPConfig config, SP_CONFIG_MSG* msg);
int spConfigGetNumOfSimilarImages(const SPConfig config, SP_CONFIG_MSG* msg);
char* spConfigGetLoggerFilename(const SPConfig config, SP_CONFIG_MSG* msg);

/** type used to define SPConfig **/
struct sp_config_t {
	char* spImagesDirectory;
	char* spImagesPrefix;
	char* spImagesSuffix;
	int spNumOfImages;
	int spPCADimension;
	char* spPCAFilename;
	int spNumOfFeatures;
	bool spExtractionMode;
	int spNumOfSimilarImages;
	KDTreeSplitMethod spKDTreeSplitMethod;
	int spKNN;
	bool spMinimalGUI;
	int spLoggerLevel;
	char* spLoggerFilename;
};

SPConfig config = NULL;

SPConfig spConfigCreate(const char* filename, SP_CONFIG_MSG* msg) {
	char line[MAX_LENGTH]; // Current line in the configuration file
	char* partA; // The string until the equal sign '=' in case of suspected system variable line.
	char* partB; // The string until the end of the line or end of the file in case of suspected system variable line.
	bool resultA; // True if partA string contains white spaces, otherwise false
	bool resultB; // True if partB string contains white spaces, otherwise false
	int checkNum = 0; // Variable to store the integer in case of a string value that should be integer
	int i = 0; // Index to go over the line and find the place that partB should start (after the '=')
	char c; // Check if the current char c equals to '='
	int k = 0; // Index for the line number

	// Check if there is any variable that didn't set according to the configuration file
	bool isSpImagesDirectorySet = false, isSpImagesPrefixSet = false,
			isSpImagesSuffixSet = false, isSpNumOfImagesSet = false,
			isSpPCADimensionSet = false, isSpPCAFilenameSet = false,
			isSpNumOfFeaturesSet = false, isSpExtractionModeSet = false,
			isSpNumOfSimilarImagesSet = false, isSpKDTreeSplitMethodSet = false,
			isSpKNNSet = false, isSpMinimalGUISet = false, isSpLoggerLevelSet =
					false, isSpLoggerFilenameSet = false;
	assert(msg != NULL);
	// Allocations
	config = (SPConfig) malloc(sizeof(*config));
	config->spImagesDirectory = (char*) malloc(sizeof(char) * MAX_LENGTH);
	config->spImagesPrefix = (char*) malloc(sizeof(char) * MAX_LENGTH);
	config->spImagesSuffix = (char*) malloc(sizeof(char) * MAX_LENGTH);
	config->spPCAFilename = (char*) malloc(sizeof(char) * MAX_LENGTH);
	config->spLoggerFilename = (char*) malloc(sizeof(char) * MAX_LENGTH);
	partA = (char*) malloc(sizeof(char) * MAX_LENGTH);
	partB = (char*) malloc(sizeof(char) * MAX_LENGTH);
	if (config->spImagesDirectory == NULL || config->spImagesPrefix == NULL
			|| config->spImagesSuffix == NULL || config->spPCAFilename == NULL
			|| config->spLoggerFilename == NULL || partA == NULL
			|| partB == NULL || config == NULL) {
		//Allocation failure
		printf("%s", ALLOCATION_PRINT); // we don't have a logger yet, so print to stdout.
		*msg = SP_CONFIG_ALLOC_FAIL;
		spConfigDestroy(config);
		free(partA);
		free(partB);
		return NULL;
	}

	if (filename == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		spConfigDestroy(config);
		return NULL;
	}
	FILE* configurationFile = fopen(filename, SP_FILE_OPEN_MODE);
	if (configurationFile == NULL) {
		//Open failed
		*msg = SP_CONFIG_CANNOT_OPEN_FILE;
		spConfigDestroy(config);
		return NULL;
	}

	// Read line by line in the configuration file and check whether it is a empty line, comment line,
	// system variable line, or neither (invalid configuration file).
	while (fgets(line, MAX_LENGTH, configurationFile) != NULL) {
		k++;
		i = 0; // Reset index every new line
		if (isEmptyLine(line)) {
			// This is an empty line, go to the next line
			continue;
		} else if (isAComment(line)) {
			// // This is an comment line, go to the next line
			continue;
		} else if (containComment(line)) {
			// if a line is not a comment line but contains a comment, then this is an error.
			printf("%s%s\n", FILE_PRINT, filename);
			printf("%s%d\n", LINE_PRINT, k);
			printf("%s", MESSAGE_CONFIGURATION_PRINT);
			*msg = SP_CONFIG_INVALID_CONFIGURATION_FILE;
			fclose(configurationFile);
			spConfigDestroy(config);
			free(partA);
			free(partB);
			return NULL;
		} else {
			// The last remaining option is a system variable line, so a valid input should be
			// something like string=string (doesn't include white spaces).
			if (containOneEqualSign(line)) {
				resultA = stringContainWhiteSpaces(line, partA, '=');
				while ((c = line[i]) != '=')
					i++;
				// partB should be the string after the '=' (equal sign) until the end of the line.
				resultB = stringContainWhiteSpaces(line + i + 1, partB, '\n');
				// Check if partA or partB is an empty string or if partA string contains white spaces
				// in the middle.
				if (!resultA || isEmptyLine(partA) || isEmptyLine(partB)) {
					printf("%s%s\n", FILE_PRINT, filename);
					printf("%s%d\n", LINE_PRINT, k);
					printf("%s", MESSAGE_CONFIGURATION_PRINT);
					*msg = SP_CONFIG_INVALID_CONFIGURATION_FILE;
					fclose(configurationFile);
					spConfigDestroy(config);
					free(partA);
					free(partB);
					return NULL;
				}
				// similar check for partB
				if (!resultB) {
					resultB = stringContainWhiteSpaces(line + i + 1, partB,
							'\0');
					// The reason for doing this is because this line could be the last line in the file
					// so we should set the stop char to '\0' and not '\n'
					if (!resultB) {
						printf("%s%s\n", FILE_PRINT, filename);
						printf("%s%d\n", LINE_PRINT, k);
						printf("%s", MESSAGE_CONFIGURATION_PRINT);
						*msg = SP_CONFIG_INVALID_CONFIGURATION_FILE;
						fclose(configurationFile);
						spConfigDestroy(config);
						free(partA);
						free(partB);
						return NULL;
					}
				}
				// now for sure we have stringA=stringB situation, we should check if stringA is equal to
				// one of the system variable, and if so, we should check that stringB is equal to
				// appropriate value.

				if (strcmp(partA, "spImagesDirectory") == 0) {
					isSpImagesDirectorySet = true;
					strcpy(config->spImagesDirectory, partB);
				} else if (strcmp(partA, "spImagesPrefix") == 0) {
					isSpImagesPrefixSet = true;
					strcpy(config->spImagesPrefix, partB);
				} else if (strcmp(partA, "spImagesSuffix") == 0) {
					if (strcmp(partB, ".jpg") == 0 || strcmp(partB, ".png") == 0
							|| strcmp(partB, ".bmp") == 0
							|| strcmp(partB, ".gif") == 0) {
						isSpImagesSuffixSet = true;
						strcpy(config->spImagesSuffix, partB);
					} else {
						printf("%s%s\n", FILE_PRINT, filename);
						printf("%s%d\n", LINE_PRINT, k);
						printf("%s", MESSAGE_CONSTRAINT_PRINT);
						*msg = SP_CONFIG_INVALID_STRING;
						fclose(configurationFile);
						spConfigDestroy(config);
						free(partA);
						free(partB);
						return NULL;
					}
				} else if (strcmp(partA, "spNumOfImages") == 0) {
					// check if partB is a positive number
					checkNum = atoi(partB);
					if (!isANumber(partB) || checkNum == 0) {
						printf("%s%s\n", FILE_PRINT, filename);
						printf("%s%d\n", LINE_PRINT, k);
						printf("%s", MESSAGE_CONSTRAINT_PRINT);
						*msg = SP_CONFIG_INVALID_INTEGER;
						fclose(configurationFile);
						spConfigDestroy(config);
						free(partA);
						free(partB);
						return NULL;
					} else {
						isSpNumOfImagesSet = true;
						config->spNumOfImages = checkNum;
					}
				} else if (strcmp(partA, "spPCADimension") == 0) {
					// check if partB is in the range [10,28]
					checkNum = atoi(partB);
					if (isANumber(
							partB) && checkNum >= PCA_DIMENSION_MIN_RANGE && checkNum <= PCA_DIMENSION_MAX_RANGE) {
						isSpPCADimensionSet = true;
						config->spNumOfImages = checkNum;
					} else {
						printf("%s%s\n", FILE_PRINT, filename);
						printf("%s%d\n", LINE_PRINT, k);
						printf("%s", MESSAGE_CONSTRAINT_PRINT);
						*msg = SP_CONFIG_INVALID_INTEGER;
						fclose(configurationFile);
						spConfigDestroy(config);
						free(partA);
						free(partB);
						return NULL;
					}
				} else if (strcmp(partA, "spPCAFilename") == 0) {
					isSpPCAFilenameSet = true;
					strcpy(config->spPCAFilename, partB);
				} else if (strcmp(partA, "spNumOfFeatures") == 0) {
					// check if partB is a positive number
					checkNum = atoi(partB);
					if (!isANumber(partB) || checkNum == 0) {
						printf("%s%s\n", FILE_PRINT, filename);
						printf("%s%d\n", LINE_PRINT, k);
						printf("%s", MESSAGE_CONSTRAINT_PRINT);
						*msg = SP_CONFIG_INVALID_INTEGER;
						fclose(configurationFile);
						spConfigDestroy(config);
						free(partA);
						free(partB);
						return NULL;
					} else {
						isSpNumOfFeaturesSet = true;
						config->spNumOfFeatures = checkNum;
					}
				} else if (strcmp(partA, "spExtractionMode") == 0) {
					if (strcmp(partB, "true") == 0) {
						isSpExtractionModeSet = true;
						config->spExtractionMode = true;
					} else if (strcmp(partB, "false") == 0) {
						isSpExtractionModeSet = true;
						config->spExtractionMode = false;
					} else {
						printf("%s%s\n", FILE_PRINT, filename);
						printf("%s%d\n", LINE_PRINT, k);
						printf("%s", MESSAGE_CONSTRAINT_PRINT);
						*msg = SP_CONFIG_INVALID_BOOLEAN;
						fclose(configurationFile);
						spConfigDestroy(config);
						free(partA);
						free(partB);
						return NULL;
					}
				} else if (strcmp(partA, "spNumOfSimilarImages") == 0) {
					// check if partB is a positive number
					checkNum = atoi(partB);
					if (!isANumber(partB) || checkNum == 0) {
						printf("%s%s\n", FILE_PRINT, filename);
						printf("%s%d\n", LINE_PRINT, k);
						printf("%s", MESSAGE_CONSTRAINT_PRINT);
						*msg = SP_CONFIG_INVALID_INTEGER;
						fclose(configurationFile);
						spConfigDestroy(config);
						free(partA);
						free(partB);
						return NULL;
					} else {
						isSpNumOfSimilarImagesSet = true;
						config->spNumOfSimilarImages = checkNum;
					}
				} else if (strcmp(partA, "spKDTreeSplitMethod") == 0) {
					if (strcmp(partB, "RANDOM") == 0) {
						isSpKDTreeSplitMethodSet = true;
						config->spKDTreeSplitMethod = RANDOM;
					} else if (strcmp(partB, "MAX_SPREAD") == 0) {
						isSpKDTreeSplitMethodSet = true;
						config->spKDTreeSplitMethod = MAX_SPREAD;
					} else if (strcmp(partB, "INCREMENTAL") == 0) {
						isSpKDTreeSplitMethodSet = true;
						config->spKDTreeSplitMethod = INCREMENTAL;
					} else {
						printf("%s%s\n", FILE_PRINT, filename);
						printf("%s%d\n", LINE_PRINT, k);
						printf("%s", MESSAGE_CONSTRAINT_PRINT);
						*msg = SP_CONFIG_INVALID_ENUM_KDTREE;
						fclose(configurationFile);
						spConfigDestroy(config);
						free(partA);
						free(partB);
						return NULL;
					}

				} else if (strcmp(partA, "spKNN") == 0) {
					// check if partB is a positive number
					checkNum = atoi(partB);
					if (!isANumber(partB) || checkNum == 0) {
						printf("%s%s\n", FILE_PRINT, filename);
						printf("%s%d\n", LINE_PRINT, k);
						printf("%s", MESSAGE_CONSTRAINT_PRINT);
						*msg = SP_CONFIG_INVALID_INTEGER;
						fclose(configurationFile);
						spConfigDestroy(config);
						free(partA);
						free(partB);
						return NULL;
					} else {
						isSpKNNSet = true;
						config->spKNN = checkNum;
					}
				} else if (strcmp(partA, "spMinimalGUI") == 0) {
					if (strcmp(partB, "true") == 0) {
						isSpMinimalGUISet = true;
						config->spMinimalGUI = true;
					} else if (strcmp(partB, "false") == 0) {
						isSpMinimalGUISet = true;
						config->spMinimalGUI = false;
					} else {
						printf("%s%s\n", FILE_PRINT, filename);
						printf("%s%d\n", LINE_PRINT, k);
						printf("%s", MESSAGE_CONSTRAINT_PRINT);
						*msg = SP_CONFIG_INVALID_BOOLEAN;
						fclose(configurationFile);
						spConfigDestroy(config);
						free(partA);
						free(partB);
						return NULL;
					}
				} else if (strcmp(partA, "spLoggerLevel") == 0) {
					// check if partB is a positive number
					checkNum = atoi(partB);
					if (isANumber(
							partB) && checkNum >= LOGGER_LEVEL_MIN_RANGE && checkNum <= LOGGER_LEVEL_MAX_RANGE) {
						isSpLoggerLevelSet = true;
						config->spLoggerLevel = checkNum;
					} else {
						printf("%s%s\n", FILE_PRINT, filename);
						printf("%s%d\n", LINE_PRINT, k);
						printf("%s", MESSAGE_CONSTRAINT_PRINT);
						*msg = SP_CONFIG_INVALID_INTEGER;
						fclose(configurationFile);
						spConfigDestroy(config);
						free(partA);
						free(partB);
						return NULL;
					}
				} else if (strcmp(partA, "spLoggerFilename") == 0) {
					isSpLoggerFilenameSet = true;
					strcpy(config->spLoggerFilename, partB);
				} else {
					// In this case the current line is invalid, neither a comment/empty line nor
					// system parameter configuration.
					printf("%s%s\n", FILE_PRINT, filename);
					printf("%s%d\n", LINE_PRINT, k);
					printf("%s", MESSAGE_CONFIGURATION_PRINT);
					*msg = SP_CONFIG_INVALID_CONFIGURATION_FILE;
					fclose(configurationFile);
					spConfigDestroy(config);
					free(partA);
					free(partB);
					return NULL;
				}
			}
		}
	}
	// Check if all the system variables is set
	if (!isSpImagesDirectorySet) {
		printf("%s%s\n", FILE_PRINT, filename);
		printf("%s%d\n", LINE_PRINT, k);
		printf("%s%s%s", MESSAGE_NOT_SET_PRINT_PART1, "spImagesDirectory",
				MESSAGE_NOT_SET_PRINT_PART2);
		*msg = SP_CONFIG_MISSING_DIR;
		fclose(configurationFile);
		spConfigDestroy(config);
		free(partA);
		free(partB);
		return NULL;
	}
	if (!isSpImagesPrefixSet) {
		printf("%s%s\n", FILE_PRINT, filename);
		printf("%s%d\n", LINE_PRINT, k);
		printf("%s%s%s", MESSAGE_NOT_SET_PRINT_PART1, "spImagesPrefix",
				MESSAGE_NOT_SET_PRINT_PART2);
		*msg = SP_CONFIG_MISSING_PREFIX;
		fclose(configurationFile);
		spConfigDestroy(config);
		free(partA);
		free(partB);
		return NULL;
	}
	if (!isSpImagesSuffixSet) {
		printf("%s%s\n", FILE_PRINT, filename);
		printf("%s%d\n", LINE_PRINT, k);
		printf("%s%s%s", MESSAGE_NOT_SET_PRINT_PART1, "spImagesSuffix",
				MESSAGE_NOT_SET_PRINT_PART2);
		*msg = SP_CONFIG_MISSING_SUFFIX;
		fclose(configurationFile);
		spConfigDestroy(config);
		free(partA);
		free(partB);
		return NULL;
	}
	if (!isSpNumOfImagesSet) {
		printf("%s%s\n", FILE_PRINT, filename);
		printf("%s%d\n", LINE_PRINT, k);
		printf("%s%s%s", MESSAGE_NOT_SET_PRINT_PART1, "spNumOfImages",
				MESSAGE_NOT_SET_PRINT_PART2);
		*msg = SP_CONFIG_MISSING_NUM_IMAGES;
		fclose(configurationFile);
		spConfigDestroy(config);
		free(partA);
		free(partB);
		return NULL;
	}
	if (!isSpPCADimensionSet) {
		config->spPCADimension = SP_PCA_DIMENSION_DEFAULT_VALUE;
	}
	if (!isSpPCAFilenameSet) {
		config->spPCAFilename = SP_PCA_FILENAME_DEFAULT_VALUE;
	}
	if (!isSpNumOfFeaturesSet) {
		config->spNumOfFeatures = SP_NUM_OF_FEATURES_DEFAULT_VALUE;
	}

	if (!isSpExtractionModeSet) {
		config->spExtractionMode = true;
	}
	if (!isSpNumOfSimilarImagesSet) {
		config->spNumOfSimilarImages = SP_NUM_OF_SIMILAR_IMAGES_DEFAULT_VALUE;

	}
	if (!isSpKDTreeSplitMethodSet) {
		config->spKDTreeSplitMethod = MAX_SPREAD;

	}
	if (!isSpKNNSet) {
		config->spKNN = SP_KNN_DEFAULT_VALUE;

	}
	if (!isSpMinimalGUISet) {
		config->spMinimalGUI = false;

	}
	if (!isSpLoggerLevelSet) {
		config->spLoggerLevel = SP_LOGGER_LEVEL_DEFAULT_VALUE;
	}
	if (!isSpLoggerFilenameSet) {
		config->spLoggerFilename = SP_LOGGER_FILENAME_DEFAULT_VALUE;

	}
	free(partA);
	free(partB);
	*msg = SP_CONFIG_SUCCESS;
	return config;
}

bool isEmptyLine(char line[]) {
	int i = 0;
	char c;
	while ((c = line[i]) != '\0') {
		if (!isspace(c))
			return false;
		i++;
	}
	return true;
}

bool isAComment(char line[]) {
	int i = 0;
	char c;
	while ((c = line[i]) != '\0') {
		if (c == '#')
			return true;
		else if (!isspace(c))
			// if c is not # and c is not a white space than this line is not a comment line because
			// in a comment line, the char # should be the first char in the line or first char
			// after white spaces
			return false;
		i++;
	}
	return false;
}

bool containComment(char line[]) {
	int i = 0;
	char c;
	while ((c = line[i]) != '\0') {
		if (c == '#')
			return true;
		i++;
	}
	return false;
}

bool containOneEqualSign(char line[]) {
	int i = 0;
	char c;
	bool seenAlready = false;
	while ((c = line[i]) != '\0') {
		if (c == '=') {
			if (!seenAlready) {
				seenAlready = true;
			} else {
				// There's more than 1 equal sign, so this line is invalid.
				return false;
			}
		}
		i++;
	}
	return seenAlready;
}

bool stringContainWhiteSpaces(char str[], char* part, char stopChar) {
	int i = 0; // index for line
	int j = 0; // index for part
	char c;
	bool stringHasStarted = false;
	bool endOfStringHasReached = false;
	while ((c = str[i]) != stopChar) {
		if (isspace(c)) {
			if (stringHasStarted) {
				endOfStringHasReached = true;
			}
		} else {
			if (!stringHasStarted) {
				stringHasStarted = true;
			} else {
				if (endOfStringHasReached) {
					// at least 2 separated string before the equal sign is invalid.
					return false;
				}
			}
			part[j] = c; // We want to get the actual string if it's valid (no white spaces in the middle)
			j++;
		}
		i++;
	}
	part[j] = '\0';
	return true;
}

bool isANumber(char* str) {
	int i = 0;
	char c;
	while ((c = str[i]) != '\0') {
		if (c < '0' || c > '9') {
			// c is not a number
			if (c == '.') {
				// c is not a integer
				return false;
			} else if (c == '+') {
				if (i != 0)
					return false;
			}
		}
		i++;
	}
	return true;
}

/*
 * Returns true if spExtractionMode = true, false otherwise.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if spExtractionMode = true, false otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
bool spConfigIsExtractionMode(const SPConfig config, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	if (config == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return false;
	}
	*msg = SP_CONFIG_SUCCESS;
	return config->spExtractionMode;
}

/*
 * Returns true if spMinimalGUI = true, false otherwise.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if spExtractionMode = true, false otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
bool spConfigMinimalGui(const SPConfig config, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	if (config == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return false;
	}
	*msg = SP_CONFIG_SUCCESS;
	return config->spMinimalGUI;
}

/*
 * Returns the number of images set in the configuration file, i.e the value
 * of spNumOfImages.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return positive integer in success, negative integer otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
int spConfigGetNumOfImages(const SPConfig config, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	if (config == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return -1;
	}
	*msg = SP_CONFIG_SUCCESS;
	return config->spNumOfImages;
}
/*
 * Returns the number of features to be extracted. i.e the value
 * of spNumOfFeatures.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return positive integer in success, negative integer otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
int spConfigGetNumOfFeatures(const SPConfig config, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	if (config == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return -1;
	}
	*msg = SP_CONFIG_SUCCESS;
	return config->spNumOfFeatures;
}
/**
 * Returns the dimension of the PCA. i.e the value of spPCADimension.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return positive integer in success, negative integer otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
int spConfigGetPCADim(const SPConfig config, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	if (config == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return -1;
	}
	*msg = SP_CONFIG_SUCCESS;
	return config->spPCADimension;
}
/**
 * Given an index 'index' the function stores in imagePath the full path of the
 * ith image.
 *
 * For example:
 * Given that the value of:
 *  spImagesDirectory = "./images/"
 *  spImagesPrefix = "img"
 *  spImagesSuffix = ".png"
 *  spNumOfImages = 17
 *  index = 10
 *
 * The functions stores "./images/img10.png" to the address given by imagePath.
 * Thus the address given by imagePath must contain enough space to
 * store the resulting string.
 *
 * @param imagePath - an address to store the result in, it must contain enough space.
 * @param config - the configuration structure
 * @param index - the index of the image.
 *
 * @return
 * - SP_CONFIG_INVALID_ARGUMENT - if imagePath == NULL or config == NULL
 * - SP_CONFIG_INDEX_OUT_OF_RANGE - if index >= spNumOfImages
 * - SP_CONFIG_SUCCESS - in case of success
 */
SP_CONFIG_MSG spConfigGetImagePath(char* imagePath, const SPConfig config,
		int index) {
	if (imagePath == NULL || config == NULL) {
		return SP_CONFIG_INVALID_ARGUMENT;
	}
	if (index >= config->spNumOfImages) {
		return SP_CONFIG_INDEX_OUT_OF_RANGE;
	}
	sprintf(imagePath, "%s%s%d%s", config->spImagesDirectory,
			config->spImagesPrefix, index, config->spImagesSuffix);
	return SP_CONFIG_SUCCESS;
}

/**
 * The function stores in pcaPath the full path of the pca file.
 * For example given the values of:
 *  spImagesDirectory = "./images/"
 *  spPcaFilename = "pca.yml"
 *
 * The functions stores "./images/pca.yml" to the address given by pcaPath.
 * Thus the address given by pcaPath must contain enough space to
 * store the resulting string.
 *
 * @param imagePath - an address to store the result in, it must contain enough space.
 * @param config - the configuration structure
 * @return
 *  - SP_CONFIG_INVALID_ARGUMENT - if imagePath == NULL or config == NULL
 *  - SP_CONFIG_SUCCESS - in case of success
 */
SP_CONFIG_MSG spConfigGetPCAPath(char* pcaPath, const SPConfig config) {
	if (pcaPath == NULL || config == NULL) {
		return SP_CONFIG_INVALID_ARGUMENT;
	}
	sprintf(pcaPath, "%s%s", config->spImagesDirectory, config->spPCAFilename);
	return SP_CONFIG_SUCCESS;
}
/**
 * Frees all memory resources associate with config.
 * If config == NULL nothig is done.
 */
void spConfigDestroy(SPConfig config) {
	if (config == NULL)
		return;
	free(config->spImagesDirectory);
	free(config->spImagesPrefix);
	free(config->spImagesSuffix);
	free(config->spLoggerFilename);
	free(config->spPCAFilename);
	free(config);
	config = NULL;
}

int spConfigGetNumOfSimilarImages(const SPConfig config, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	if (config == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return -1;
	}
	*msg = SP_CONFIG_SUCCESS;
	return config->spNumOfSimilarImages;
}

int getSpKNN(const SPConfig config, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	if (config == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return -1;
	}
	*msg = SP_CONFIG_SUCCESS;
	return config->spKNN;
}

int spConfigGetLoggerLevel(const SPConfig config, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	if (config == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return -1;
	}
	*msg = SP_CONFIG_SUCCESS;
	return config->spLoggerLevel;
}

char* spConfigGetLoggerFilename(const SPConfig config, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	if (config == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return NULL;
	}
	*msg = SP_CONFIG_SUCCESS;
	return config->spLoggerFilename;
}

SP_CONFIG_MSG spConfigGetImageFeatsPath(char* imagePath, const SPConfig config,
		int index, char* extensionFeats) {
	if (imagePath == NULL || config == NULL || extensionFeats == NULL) {
		return SP_CONFIG_INVALID_ARGUMENT;
	}
	if (index >= config->spNumOfImages) {
		return SP_CONFIG_INDEX_OUT_OF_RANGE;
	}
	sprintf(imagePath, "%s%s%d%s", config->spImagesDirectory,
			config->spImagesPrefix, index, extensionFeats);
	return SP_CONFIG_SUCCESS;
}

KDTreeSplitMethod spConfigGetSplitMethod(const SPConfig config) {
	return config->spKDTreeSplitMethod;
}
