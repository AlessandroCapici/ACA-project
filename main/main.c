#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define DIMENSIONS 3

typedef struct {
	int ID_point;
	int ID_cluster;
	float coordinates[DIMENSIONS];
}point;

void readDataset(const char *filename, int *N_points, point *data_points);


int main(int argc, char **argv) {

	//	omp_set_num_threads(6);

	/* Correct structure of command line arguments
	 - ./a.out
	 - dataset filename,
	 - number of clusters,
	 - number of threads,
	 - filename to write the clustered data points,
	 - filename to write the coordinated of the centroid
	 */

/*
	if (argc < 6) {
		printf("Less no. of command line arguments\n\n");
		return 0;
	} else if (argc > 6) {
		printf("Too many command line arguments\n\n");
		return 0;
	}

	// Correct no. of command line arguments

	const char *dataset_filename = argv[1];
	const int K = atoi(argv[2]);
	const int num_threads = atoi(argv[3]);
	const char *data_points_output_filename = argv[4];
*/

	int N;	// Total no. of data_points in the file
	point *data_points;	// array of data points
	point *centroids;	// array of centroids of clusters
	int num_iterations = 0;
	
	readDataset("../dataset/dataset_5.txt", &N, data_points);
	
	int i;
	
	for(i = 0; i < N; i++) {
		int x, y, z;
		x = data_points[i].coordinates[0];
		y = data_points[i].coordinates[1];
		z = data_points[i].coordinates[2];

		printf("Point %d: x= %f y=%f z=%f", N, x, y, z);
	}
	
	return 0;

}

// return the number of points in the file
void readDataset(const char *filename, int *N_points, point *data_points) {
	FILE *fptr = fopen(filename, "r");

	//remember, the first line is the number of points in the file	
	//fscanf(fptr, "%d", N_points);
	//printf("%d", *N_points);
	*N_points = 5;
	printf("No. of data points in the dataset: %d \n", *N_points);

	// Each data point is has so many coordinates as DIMENSION
	data_points = malloc((*N_points) * sizeof(point));
	
//	while(fgets(buffer,,sizeof(buffer )),fptr)!=NULL){
//	}

	int i;
	printf("%d", *N_points);
	for (i = 0; i < (*N_points); i++) {
		printf("pippo");
		int x, y, z;
		fscanf(fptr, "%d %d %d", &x, &y, &z);
		printf("%d %d %d", &x, &y, &z);
	//	*(*data_points + i) = temp;
		data_points[i].coordinates[0] = x;
		data_points[i].coordinates[1] = y;
		data_points[i].coordinates[2] = z;
	}
	
	
	fclose(fptr);
}

