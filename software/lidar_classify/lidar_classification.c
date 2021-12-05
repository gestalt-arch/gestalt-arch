#include "lidar_classification.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_ELEMENT 420
#define LIDAR_WIDTH .9
#define POLE_1_WIDTH 1.2
#define POLE_2_WIDTH 1.7

//uint16_t NUM_ELEMENT; //Make const later,

/*
	REMOVE. For testing purposes.
*/
void parse(char *file, float* distance, float* theta, uint8_t* num_objects) {
	FILE* data = fopen(file, "r");
	char temp_word[4096];
	char* token;

	*num_objects = atoll(fgets(temp_word, 4096, data));
	
	fgets(temp_word, 4096, data);
	token = strtok(temp_word, " ");
	for (int i = 0; i < NUM_ELEMENT; i++) {
		theta[i] = strtof(token, NULL);
		token = strtok(NULL, " ");
	}

	fgets(temp_word, 4096, data);
	token = strtok(temp_word, " ");
	for (int i = 0; i < NUM_ELEMENT; i++) {
		distance[i] = strtof(token, NULL);
		token = strtok(NULL, " ");
	}
}

/*
	REMOVE. For testing purposes.
*/
void print(float* distance, float* theta, uint8_t num_objects) {
	printf("Data of the file.\n");
	printf("%u\n", num_objects);

	for (int i = 0; i < NUM_ELEMENT; i++) {
		printf("%f ", distance[i]);
	}

	printf("\n");

	for (int i = 0; i < NUM_ELEMENT; i++) {
		printf("%f ", theta[i]);
	}
}
 
float get_width(float theta1, float theta2, float distance1, float distance2) {
	return sqrtf(distance1 * distance1 + distance2 * distance2 - 2 * distance1 * distance2 * cosf((theta2 - theta1)*0.0175));
}

#define delta .25
#define delta_ .2
classification_t classify(float* distance, float* theta, uint8_t num_objects) {
	int found = 0;
	int in_object = 0;
	uint16_t indices[num_objects][2];

	for (uint16_t i = 1; i < NUM_ELEMENT; i++) {
		if (fabs(distance[i - 1] - distance[i]) > delta) {
			if (!in_object) {
				in_object = 1;
				indices[found][0] = i;
			}
			else {
				in_object = 0;
				indices[found][1] = i - 1;
				found++;
			}
		}
	}

	printf("%u", found);

	if (found != num_objects) {
		return LC_FAIL;
	}

	float width;
	float theta1;
	float theta2;
	float distance1;
	float distance2;

	for (uint8_t i = 0; i < num_objects; i++) {
		theta1 = theta[indices[i][0]];
		theta2 = theta[indices[i][1]];
		distance1 = distance[indices[i][0]];
		distance2 = distance[indices[i][1]];

		printf("FOUND OBJECT\n");
		printf("theta1: %f\n", theta1);
		printf("theta2: %f\n", theta2);
		printf("distance1: %f\n", distance1);
		printf("distance2: %f\n", distance2);
		width = get_width(theta1, theta2, distance1, distance2);
		printf("width: %f\n", width);

		if (fabs(width - LIDAR_WIDTH) <= delta_) {
			printf("OBJECT IS TYPE LIDAR\n");
		}
		else if (fabs(width - POLE_1_WIDTH) <= delta_) {
			printf("OBJECT IS TYPE POLE1\n");
		}
		else if (fabs(width - POLE_2_WIDTH) <= delta_) {
			printf("OBJECT IS TYPE POLE2\n");
		}
		else {
			printf("INVALID WIDTH!");
			return LC_FAIL;
		}
	}

	return LC_SUCCESS;
}



int main(int argc, char **argv) {
	uint16_t MAX_SIZE = ceil(3000 / 7);

    float distance[MAX_SIZE];
	float theta[MAX_SIZE];
	uint8_t num_objects;

	parse(argv[1], distance, theta, &num_objects); // Remove later.
	//print(distance, theta, num_objects); // Remove later.

	if (classify(distance, theta, num_objects) == LC_SUCCESS) {
		return 0;
	}
	printf("FAILURE");
	return 0;
}