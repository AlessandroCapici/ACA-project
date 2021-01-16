#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
//#include <omp.h>

#define DIMENSIONS 3
#define MAX_ITERATIONS 800
#define THRESHOLD 1e-4
#define N_CENTROIDS 3
#define DATASET_FILE "../datasets/dataset_5.txt"
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
void kMeanSerial(int k, centroid *centroids, int N_points, point *points, int *num_iterations);

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
	
	/*
	//k-mean algo
	kMeanSerial(N_CENTROIDS, centroids, N_points, points, &num_iterations);
	printf("Done algo. Num of iterations: %d\n", num_iterations);
	
	//write result
	writeCentroids3D(N_CENTROIDS,centroids);
	if(num_iterations == MAX_ITERATIONS) {
		printf("It has been reached the max number of iterations possible: %d\n",num_iterations);
		
	} else {
		printf("Number of iterations: %d\n",num_iterations);
		
	}
	*/
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
        printf(buf);
        printf("\n");
        
		conv = sscanf(buf,"%d %d %d", &p[i].coordinates[0], &p[i].coordinates[1], &p[i].coordinates[2]);
		
		p[i].ID_point = i;
        
        printf("Printed charaters: %d\n", conv);
        printf("ID= %d x= %d y= %d z= %d\n\n",
			p[i].ID_point,
			p[i].coordinates[0],
			p[i].coordinates[1],
			p[i].coordinates[2]);
        
		i++;
    }
    
    //printPoints3D(p, *N_points);
    
    return p;
}

void printPoints3D(point* p, int N) { 
	
	int i;
	for(i = 0; i < N; i++) {
		printf("x= %d y= %d z= %d\n",
			p[i].coordinates[0],
			p[i].coordinates[1],
			p[i].coordinates[2]);
	}
}

void replaceCentroid(centroid *c) {

	int i;
	for(i = 0; i < DIMENSIONS; i++){
		(*c).coordinates[i] = (*c).sum_coordinates[i] / (float) (*c).count_points;
	}

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

	int iteration_count = 0;
	bool isChanged=true;
	while(iteration_count < MAX_ITERATIONS && isChanged) {
		isChanged = false;
		double min_distance, current_distance;
		isChanged = false;
		int j;
		int i;
		for(i = 0; i < N_points; i++) {
			min_distance = __DBL_MAX__; // min_distance is assigned the largest possible double value

			//here we tie the point with the centroid
			for(j = 0; j < K; j++) {
				current_distance = findEuclideanDistance3D(data_points[i], centroids[j]);

				if(current_distance < min_distance) {
					min_distance = current_distance;
					//assign the ID of the cluster as the number of the centroid
					data_points[i].ID_cluster = j;
					isChanged = true;
				}
			}
			//we update the number of point for the centroid
			centroids[data_points[i].ID_cluster].count_points++;

			//here we start sum
			for(j = 0; j < DIMENSIONS; j++){
				centroids[data_points[i].ID_cluster].sum_coordinates[j] += data_points[i].coordinates[j];
			}

			//recenter centroid based on its points


		}
		for(j = 0; j < K; j++) {
			replaceCentroid(&centroids[j]);
		}
		iteration_count++;

	}

	return iteration_count;

}

void kMeanSerial(int k, centroid *centroids, int N_points, point *points, int *num_iterations) {
	
	int i, j;
	
	if (!(centroids = calloc(k,sizeof(*centroids)))) {
			printf("Error calloc\n");
			exit(1);
	}
	
	//initialize centroids. First k points will be the first k centroids
	for (i = 0; i < k; i++) {
		//for each centroid
		
		for(j = 0; j < DIMENSIONS; j++) {
			//set coordinates
			centroids[i].coordinates[j] = points[i].coordinates[j];
		}
		
	}
	
	//starting the process of clustering
	processClusterSerial(N_points, k, points, centroids, num_iterations);


}

void writeCentroids3D(int K,centroid *c) {
	FILE *fptr = fopen(OUTPUT_FILE, "w");
	fprintf(fptr, "X Y Z\n");
	int i = 0;
	
	for (i = 0; i < K; i++) {
			fprintf(fptr, "%f %f %f", c[i].coordinates[0],c[i].coordinates[1],c[i].coordinates[2]);
			fprintf(fptr, "\n");
	}
	
	fclose(fptr);
}


