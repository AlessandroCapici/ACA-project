#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <omp.h>

#define DIMENSIONS 3
#define MAX_ITERATIONS 800
#define THRESHOLD 1e-4
#define N_CENTROIDS 3

#define N_THR 4

#define DATASET_FILE "../datasets/dataset_50000_4.txt"
#define OUTPUT_FILE "../result/centroid.txt"

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
point *read_file3D(int *N_points);
void writeCentroids3D(int K,centroid *c);
void printPoints3D(point* p, int N);

//distance functions
double findEuclideanDistance3D(point point, centroid centroid);
double findEuclideanDistance(point point, centroid centroid);

//k-mean algorithm functions
int processClusterSerial(int N_points, int K, point *data_points, centroid *centroids, int *num_iterations);
void replaceCentroid(centroid *c);
centroid *kMeanSerial(int k, centroid *centroids, int N_points, point *points, int *num_iterations);
centroid *initializeCentroids(int k, centroid *centroids, int N_points, point *points);
centroid *kMeansParallel(int k, centroid *centroids, int N_points, point *points,int *num_iterations);
int processClusterParallel(int N_points, int K, point *data_points, centroid *centroids, int *num_iterations);

int main(int argc, char const *argv[]) {

	point *points;	// array of data points
	centroid *centroids;	// D array of centroids of clusters

	int N_points; // number of points from dataset file
	int num_iterations = 0; // number of iterations needed to end the kmeans algo

	//read point from dataset
	points = read_file3D(&N_points);

	if(points != NULL) {
		printf("File read\nThere are %d points\n\n", N_points);

	} else {
		printf("Error while reading file\n");
		return 1;
	}

	//printPoints3D(points, 5);

	//k-mean algo

	double start_time = omp_get_wtime();
	centroids = kMeansParallel(N_CENTROIDS, centroids, N_points, points,&num_iterations);
	double end = omp_get_wtime();

	printf("Done algo. Num of iterations: %d\n", num_iterations);
	printf("Time needed for serial algorithm: %f\n\n", end - start_time);

	//write result
	writeCentroids3D(N_CENTROIDS, centroids);

	if(num_iterations == MAX_ITERATIONS) {
		printf("It has been reached the max number of iterations possible: %d\n",num_iterations);

	} else {
		printf("Number of iterations: %d\n",num_iterations);

	}

	free(points);
	free(centroids);

	return 0;
}

point *read_file3D(int *N_points) {

	FILE *f;

	printf("Trying to open ");
	printf(DATASET_FILE);
	printf("\n");

	if (!(f=fopen(DATASET_FILE,"r"))) {
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
	for(i = 0; i < DIMENSIONS; i++){
		(*c).coordinates[i] = (*c).sum_coordinates[i] / (float) (*c).count_points;
	//	printf("%f ", (*c).coordinates[i]);
	}

	//printf("\n\n");

}

double findEuclideanDistance(point point, centroid centroid) {

	double sum = 0;
	int i;

	for(i = 0; i < DIMENSIONS; i++) {
		sum += pow((double) (point.coordinates[i] - centroid.coordinates[i]), 2);
	}

	return sqrt(sum);
}


int processClusterParallel(int N_points, int K, point *data_points, centroid *centroids, int *num_iterations) {

	//i centroidi sono gia inizializzati altrove

	*num_iterations = 0;
	bool isChanged = true;

	int *sum_of_points = calloc(N_CENTROIDS, sizeof(int)); // normal array that hold the number of points bind to each centroid
	if(sum_of_points == NULL) {
		printf("Error while allocating space in processClusterParallel function\n");
		return 1;
	}

	int length_matrix = N_CENTROIDS*DIMENSIONS;
	int *sum_of_coordinates_matrix = calloc(length_matrix, sizeof(int)); // matrix that for each centroids hold the sum of the coordinates(separated)
	if(sum_of_points == NULL) {
		printf("Error while allocating space in processClusterParallel function\n");
		return 1;
	}
	static int local_sum_of_points[N_CENTROIDS]={};
	static int local_sum_of_coordinates_matrix [N_CENTROIDS][DIMENSIONS]={};


	while(*num_iterations < MAX_ITERATIONS && isChanged) {

		#pragma omp parallel shared(sum_of_coordinates_matrix, sum_of_points, isChanged)
		{

			double min_distance, current_distance;
			int i, j;

			#pragma omp for reduction(+: local_sum_of_points,local_sum_of_coordinates_matrix)

			for(i = 0; i < N_points; i++) {
				min_distance = __DBL_MAX__; // min_distance is assigned the largest possible double value

				//here we tie the point with the centroid
				for(j = 0; j < K; j++) {
					current_distance = findEuclideanDistance(data_points[i], centroids[j]);

					if(current_distance < min_distance) {
						min_distance = current_distance;
						//assign the ID of the cluster as the number of the centroid
						data_points[i].ID_cluster = j;
						isChanged = true;
					}
				}

				//we update the number of point for the centroid
				local_sum_of_points[data_points[i].ID_cluster]++;

				//here we start the sum
				for(j = 0; j < DIMENSIONS; j++) {
				//	centroids[data_points[i].ID_cluster].sum_coordinates[j] += data_points[i].coordinates[j];
				//	sum_of_coordinates_matrix[(data_points[i].ID_cluster * DIMENSIONS) + j] += data_points[i].coordinates[j];
					local_sum_of_coordinates_matrix[data_points[i].ID_cluster][j]+=data_points[i].coordinates[j];
				}

			}

		}

		int i, j;
		//recenter centroid based on its points
		for(j = 0; j < K; j++) {
			centroids[j].count_points = local_sum_of_points[j];

			for(i = 0; i < DIMENSIONS; i++){
				//centroids[j].sum_coordinates[i] = sum_of_coordinates_matrix[(j * DIMENSIONS) + i];
					centroids[j].sum_coordinates[i] = local_sum_of_coordinates_matrix[j][i];
			}
			replaceCentroid(&centroids[j]);
		}

		(*num_iterations)++;
	}

	free(sum_of_coordinates_matrix);
	free(sum_of_points);

	return 0;

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

centroid *kMeansParallel(int k, centroid *centroids, int N_points, point *points,int *num_iterations) {

	//initialize centroids
	centroids = initializeCentroids(k, centroids, N_points, points);

	omp_set_num_threads(N_THR);
	processClusterParallel(N_points,k,points,centroids,num_iterations);
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
