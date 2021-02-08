#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <omp.h>

#define DIMENSIONS 3
#define MAX_ITERATIONS 800
#define THRESHOLD 1e-4
#define N_CENTROIDS 3
#define DATASET_FILE "../datasets/dataset_1000000_4.txt"
#define OUTPUT_FILE "../result/centroid.txt"
#define OUTPUT_FILE_TIME "../result/time.txt"

//structure definitions
typedef struct {
	int ID_point;
	int ID_cluster;
	float coordinates[DIMENSIONS];
}point;

typedef struct {
	int ID_cluster;
	int count_points;
	float coordinates[DIMENSIONS];
	int sum_coordinates[DIMENSIONS];
}centroid;

//prototip defininitions:
//IO functions
point *read_file3D(int *N_points,char path[50]);
void writeCentroids3D(int K,centroid *c);
void printPoints3D(point* p, int N);
void writeTime(float time[9], int num_iteration[9], int number_of_point[9]);

//distance functions
double findEuclideanDistance3D(point point, centroid centroid);
double findEuclideanDistance(point point, centroid centroid);

//k-mean algorithm functions
int processClusterSerial(int N_points, int K, point *data_points, centroid *centroids, int *num_iterations);
void replaceCentroid(centroid *c);
centroid *kMeanSerial(int k, centroid *centroids, int N_points, point *points, int *num_iterations);
centroid *initializeCentroids(int k, centroid *centroids, int N_points, point *points);

int main(int argc, char const *argv[]) {

	point *points;	// array of data points
	centroid *centroids;	// D array of centroids of clusters

	int N_points; // number of points from dataset file
	int num_iterations = 0; // number of iterations needed to end the kmeans algo

	points = read_file3D(&N_points,DATASET_FILE);

	if(points != NULL) {
		printf("File read\nThere are %d points\n\n", N_points);

	} else {
		printf("Error while reading file\n");
		return 1;
	}

	//printPoints3D(points, 5);

	//k-mean algo
	printf("Serial Execution\n");
	double start_time = omp_get_wtime();
	centroids = kMeanSerial(N_CENTROIDS, centroids, N_points, points,&num_iterations);
	double end = omp_get_wtime();
	printf("%f\n",end-start_time);
	//write result
	writeCentroids3D(N_CENTROIDS, centroids);
	free(points);
	free(centroids);

	return 0;
}

point *read_file3D(int *N_points,char path[50]) {

	FILE *f;

	if (!(f=fopen(path,"r"))) {
		printf("Error open points file\n");
		return NULL;
	}

	point *p;
	int conv, i=0;
	char buf[1000];
		//remember, the first line is the number of points in the file
	fgets(buf, sizeof(buf), f);
	sscanf(buf, "%d",N_points);

		// Each data point is has so many coordinates as DIMENSION
	if (!(p = malloc((*N_points) * sizeof(*p)))) {
		return NULL;
	}

	while (fgets(buf, sizeof(buf), f)) {

		conv = sscanf(buf, "%f %f %f",
			&p[i].coordinates[0],
			&p[i].coordinates[1],
			&p[i].coordinates[2]);

		p[i].ID_point = i;

		i++;
	}

	return p;
}

void printPoints3D(point* p, int N) {

	int i;
	for(i = 0; i < N; i++) {
		printf("ID= %d x= %f y= %f z= %f\n",
			p[i].ID_point,
			p[i].coordinates[0],
			p[i].coordinates[1],
			p[i].coordinates[2]);
	}
}

void replaceCentroid(centroid *c) {

	int i;
	if ((*c).count_points != 0) {
		for(i = 0; i < DIMENSIONS; i++){
			(*c).coordinates[i] = (*c).sum_coordinates[i] / (float) (*c).count_points;
		}

	}

	//printf("\n\n");

}

double findEuclideanDistance3D(point point, centroid centroid) {
	// Function to find the Euclidean distance between two points in 3 dimensional space

	return sqrt(
		pow(((double) (point.coordinates[0] - centroid.coordinates[0])), 2)
		+ pow(((double) (point.coordinates[1] - centroid.coordinates[1])), 2)
		+ pow(((double) (point.coordinates[2] - centroid.coordinates[2])), 2));
}

double findEuclideanDistance(point point, centroid centroid) {

	double sum = 0;
	int i;

	for(i = 0; i < DIMENSIONS; i++) {
		sum += pow((double) (point.coordinates[i] - centroid.coordinates[i]), 2);
	}

	return sqrt(sum);
}

int processClusterSerial(int N_points, int K, point *data_points, centroid *centroids, int *num_iterations) {

	//i centroidi sono gia inizializzati altrove

	*num_iterations = 0;
	bool isChanged=true;

	while(*num_iterations < MAX_ITERATIONS && isChanged) {

		double min_distance, current_distance;
		isChanged = false;
		int i, j;

		for (i = 0; i < K; i++) {
			centroids[i].count_points=0;
			for (j = 0; j < DIMENSIONS; j++) {
				centroids[i].sum_coordinates[j]=0;
			}
		}

		for(i = 0; i < N_points; i++) {
			min_distance = __DBL_MAX__; // min_distance is assigned the largest possible double value

			//here we tie the point with the centroid
			for(j = 0; j < K; j++) {
				current_distance = pow(((double) (data_points[i].coordinates[0] - centroids[j].coordinates[0])), 2)
				+ pow(((double) (data_points[i].coordinates[1] - centroids[j].coordinates[1])), 2)
				+ pow(((double) (data_points[i].coordinates[2] - centroids[j].coordinates[2])), 2);

				if(current_distance < min_distance) {
					min_distance = current_distance;
					//assign the ID of the cluster as the number of the centroid
					data_points[i].ID_cluster = j;
					isChanged = true;
				}
			}
			//we update the number of point for the centroid
			centroids[data_points[i].ID_cluster].count_points++;

			//here we start the sum
			for(j = 0; j < DIMENSIONS; j++){
				centroids[data_points[i].ID_cluster].sum_coordinates[j] += data_points[i].coordinates[j];
			}

		}

		//recenter centroid based on its points
		for(j = 0; j < K; j++) {
			replaceCentroid(&centroids[j]);
		}

		(*num_iterations)++;

	}

	return 0;

}

centroid *kMeanSerial(int k, centroid *centroids, int N_points, point *points, int *num_iterations) {

	centroids = initializeCentroids(k, centroids, N_points, points);

	//starting the process of clustering
	processClusterSerial(N_points, k, points, centroids, num_iterations);

	return centroids;
}

centroid *initializeCentroids(int k, centroid *centroids, int N_points, point *points) {

	int i, j;
	centroids = calloc(k, sizeof(*centroids));
	if (centroids == NULL) {
		printf("Error calloc\n");
		exit(1);
	}

	if(N_points < k) {
		exit(1);
	}

	//initialize centroids. First k points will be the first k centroids
	for (i = 0; i < k; i++) {
		//for each centroid

		for(j = 0; j < DIMENSIONS; j++) {
			//set coordinates
			centroids[i].coordinates[j] = points[i].coordinates[j];
			//printf("%f ", centroids[i].coordinates[j]);
		}
		//printf("\n");

	}

	return centroids;
}



void writeCentroids3D(int K,centroid *c) {
	FILE *fptr = fopen(OUTPUT_FILE, "w");

	printf("Trying to write on ");
	printf(OUTPUT_FILE);
	printf("\n");

	if(fptr == NULL) {
		printf("Error while opening output file\n");
	}

	fprintf(fptr, "X Y Z\n");

	int i = 0;

	for (i = 0; i < K; i++) {
		fprintf(fptr, "%f %f %f\n", c[i].coordinates[0], c[i].coordinates[1], c[i].coordinates[2]);

	}

	fclose(fptr);
}

void writeTime(float time[9], int num_iteration[9], int number_of_point[9]) {
	FILE *fptr = fopen(OUTPUT_FILE_TIME, "w");

	if(fptr == NULL) {
		printf("Error while opening output file\n");
	}
	for (int i = 0; i < 9; ++i)
	{
		fprintf(fptr, "total point:%d , time:%f , number of iterations:%d\n",number_of_point[i],time[i],num_iteration[i]);
	}

	fclose(fptr);
}
