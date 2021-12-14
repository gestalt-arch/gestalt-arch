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

#define LIDAR_RADIUS .5
#define POLE_1_RADIUS .75
#define POLE_2_RADIUS 1

#define DEG_TO_RAD 0.0174533

#define POLE_1_X 10.43108
#define POLE_1_Y -1.644364

#define POLE_2_X -8.56922
#define POLE_2_Y -10.64436

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
	return sqrtf(distance1 * distance1 + distance2 * distance2 - 2 * distance1 * distance2 * cosf((theta2 - theta1) * DEG_TO_RAD));
}

void get_loc(struct coordinate* cord, float pole1_theta, float pole1_distance, float pole2_theta, float pole2_distance) {
	float a1 = 1 / tanf(pole1_theta * DEG_TO_RAD);
	float b1 = -1;
	float c1 = -(POLE_1_Y) + (1 / tanf(pole1_theta * DEG_TO_RAD)) * (POLE_1_X);

	float a2 = 1 / tanf(pole2_theta * DEG_TO_RAD);
	float b2 = -1;
	float c2 = -(POLE_2_Y) + (1 / tanf(pole2_theta * DEG_TO_RAD)) * (POLE_2_X);

	float x = (b2 * c1 - b1 * c2) / (a1 * b2 - a2 * b1);
	float y = (a1 * c2 - a2 * c1) / (a1 * b2 - a2 * b1);

	cord->x = x;
	cord->y = y;
}

#define delta .35
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

	printf("Found: %u Objects\n", found);

	if (found <= num_objects) {
		float width;
		float theta1;
		float theta2;
		float distance1;
		float distance2;

		float pole1_theta;
		float pole1_distance;

		float pole2_theta;
		float pole2_distance;

		for (uint8_t i = 0; i < found; i++) {
			theta1 = theta[indices[i][0]];
			theta2 = theta[indices[i][1]];
			distance1 = distance[indices[i][0]];
			distance2 = distance[indices[i][1]];

			//size_t index = ceil((indices[i][0] + indices[i][1])/2);

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
				pole1_theta = fabs(theta1 + theta2) / 2;
				pole1_distance = fabs(distance1 + distance2) / 2;
				//pole1_theta = theta[index];
				//pole1_distance = distance[index];
			}
			else if (fabs(width - POLE_2_WIDTH) <= delta_) {
				printf("OBJECT IS TYPE POLE2\n");
				pole2_theta = fabs(theta1 + theta2) / 2;
				pole2_distance = fabs(distance1 + distance2) / 2;
				//pole2_theta = theta[index];
				//pole2_distance = distance[index];
			}
			else {
				printf("INVALID WIDTH!");
				return LC_FAIL;
			}
		}

		struct coordinate cord;

		get_loc(&cord, pole1_theta, pole1_distance, pole2_theta, pole2_distance);

		printf("pole1theta: %f\n", pole1_theta);
		printf("pole2theta: %f\n", pole2_theta);
		printf("pole1distance: %f\n", pole1_distance);
		printf("pole2distance: %f\n", pole2_distance);
		printf("BOT POSITION = (%f, %f)\n", cord.x, cord.y);
	}

	if (found != num_objects) {
		return LC_FAIL;
	}

	return LC_SUCCESS;
}
/*
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
*/