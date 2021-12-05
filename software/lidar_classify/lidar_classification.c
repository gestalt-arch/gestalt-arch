#include "lidar_classification.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint16_t NUM_ELEMENT;

/*
	REMOVE. For testing purposes.
*/
void parse(char *file, float* distance, float* theta) {
	FILE* data = fopen(file, "r");
	char temp_word[4096];
	char* token;

	NUM_ELEMENT = atoll(fgets(temp_word, 4096, data));
	
	fgets(temp_word, 4096, data);
	token = strtok(temp_word, " ");
	for (int i = 0; i < NUM_ELEMENT; i++) {
		distance[i] = strtof(token, NULL);
		token = strtok(NULL, " ");
	}

	fgets(temp_word, 4096, data);
	token = strtok(temp_word, " ");
	for (int i = 0; i < NUM_ELEMENT; i++) {
		theta[i] = strtof(token, NULL);
		token = strtok(NULL, " ");
	}
}

/*
	REMOVE. For testing purposes.
*/
void print(float* distance, float* theta) {
	printf("Data of the file.\n");
	printf("%u\n", NUM_ELEMENT);

	for (int i = 0; i < NUM_ELEMENT; i++) {
		printf("%f ", distance[i]);
	}

	printf("\n");

	for (int i = 0; i < NUM_ELEMENT; i++) {
		printf("%f ", theta[i]);
	}
}
 
classification_t classify() {
	return LC_SUCCESS;
}

int main(int argc, char **argv) {
	uint16_t MAX_SIZE = ceil(3000 / 7);

    float distance[MAX_SIZE];
	float theta[MAX_SIZE];

	parse(argv[1], distance, theta); // Remove later.
	print(distance, theta); // Remove later.

	classify(distance, theta);

	return 0;
}