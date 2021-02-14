#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <omp.h>

#define DIMENSIONS 3
#define MAX_ITERATIONS 800
#define THRESHOLD 1e-4
#define N_CENTROIDS 30

#define N_THR 12

#define DATASET_FILE "datasets/dataset_1000000_4.txt"
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
point *read_file3D(int *N_points);
void writeCentroids3D(int K,centroid *c);
void printPoints3D(point* p, int N);

//distance functions
double findEuclideanDistance3D(point point, centroid centroid);
double findEuclideanDistance(point point, centroid centroid);

//k-mean algorithm functions
void replaceCentroid(centroid *c);
centroid *initializeCentroids(int k, centroid *centroids, int N_points, point *points);
centroid *kMeansParallel(int k, centroid *centroids, int N_points, point *points,int *num_iterations,int num_thread);
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
	printf("Parallel Execution\n");
	double start_time = omp_get_wtime();
	centroids = kMeansParallel(N_CENTROIDS, centroids, N_points, points,&num_iterations,N_THR);
	double end = omp_get_wtime();
	free(centroids);
	free(points);
	printf("Num iterazioni: %d tempo:%f",num_iterations,end-start_time);
	return 0;
}

point *read_file3D(int *N_points) {

	FILE *f;

	printf("Trying to open ");
	printf("%s",DATASET_FILE);
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
	float div = (float) (*c).count_points;
	if(div != 0) {
		for(i = 0; i < DIMENSIONS; i++){
			(*c).coordinates[i] = (*c).sum_coordinates[i] / div;			
		}
	}


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

	*num_iterations = 0;
	bool isChanged = true; // this variable is shared
	
	int i,j;
	double min_distance, current_distance;
	int p_sum_coordinates_matrix[N_CENTROIDS][DIMENSIONS] = {}; // 'p' means 'private' for each thread
	int p_sum_points[N_CENTROIDS] = {}; 
			

	while(*num_iterations < MAX_ITERATIONS && isChanged) {
		int e, s;
		
		for(e = 0; e < K; e++){
			centroids[e].count_points = 0; 
				for(s = 0; s < DIMENSIONS; s++){
					centroids[e].sum_coordinates[s] = 0;
				}
		}
		
		#pragma omp parallel shared(isChanged) private(i,j, min_distance, current_distance, p_sum_coordinates_matrix, p_sum_points)
		{

			#pragma omp for 
			for(i = 0; i < N_points; i++) {
				min_distance = __DBL_MAX__; // min_distance is assigned the largest possible double value
				//here we tie the point with the centroid
				for(j = 0; j < K; j++) {
					current_distance = sqrt(pow((double) (data_points[i].coordinates[0] - centroids[j].coordinates[0]), 2)+pow((double) (data_points[i].coordinates[1] - centroids[j].coordinates[1]), 2)+pow((double) (data_points[i].coordinates[2] - centroids[j].coordinates[2]), 2));
					if(current_distance < min_distance) {
						min_distance = current_distance;
						//assign the ID of the cluster as the number of the centroid
						data_points[i].ID_cluster = j;
						isChanged = true;
					}
				}

				//we update the number of point for the centroid
				p_sum_points[data_points[i].ID_cluster]++;
				
				//here we start the sum
				for(j = 0; j < DIMENSIONS; j++) {
					p_sum_coordinates_matrix[data_points[i].ID_cluster][j] += data_points[i].coordinates[j];
					
				}

			}

			#pragma omp critical
			for(i = 0; i < N_CENTROIDS; i++){
				centroids[i].count_points += p_sum_points[i];
				for(j = 0; j < DIMENSIONS; j++){
					centroids[i].sum_coordinates[j] += p_sum_coordinates_matrix[i][j];
				}	
			}

		}

		//recenter centroid based on its points
		#pragma omp parallel for
		for(e = 0; e < K; e++) {
			replaceCentroid(&centroids[e]);
		}

		(*num_iterations)++;
	}

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

centroid *kMeansParallel(int k, centroid *centroids, int N_points, point *points,int *num_iterations,int num_thread) {

	//initialize centroids
	centroids = initializeCentroids(k, centroids, N_points, points);

	omp_set_num_threads(num_thread);
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
