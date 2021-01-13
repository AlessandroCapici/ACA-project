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


	int N;	// Total no. of data_points in the file
	point *data_points;	// array of data points
	point *centroids;	// array of centroids of clusters
	int num_iterations = 0;
	
	printf("%d", &data_points);
	readDataset("../datasets/dataset_5.txt", &N, &data_points);
//	printf("%d", N);
	
	int i;
	for(i = 0; i < N; i++) {
		printf("ciao	1\n");
		printf("%d \n", i);
		
		int x, y, z;
		printf("ciao	2\n");
	
		printf("%d", data_points[i].coordinates[0]);
		x = data_points[i].coordinates[0];
		y = data_points[i].coordinates[1];
		z = data_points[i].coordinates[2];
		
		printf("ciao	3\n");
	
		printf("Point %d: x= %f y=%f z=%f", N, x, y, z);
		printf("ciao	4\n");
	
	}
	
	free(data_points);
	return 0;

}

// return the number of points in the file
void readDataset(const char *filename, int *N_points, point *data_points) {

	FILE *fptr;
	fptr = fopen(filename, "r");

	if(fptr == NULL) {
		printf("There is a problem with the file read\n");
		exit(1);
	}

	int readChars;
	//remember, the first line is the number of points in the file	
	readChars = fscanf(fptr, "%d \n", N_points);
	
	
	printf("The chars read are: %d \n", readChars);	
	printf("No. of data points in the dataset: %d \n", *N_points);
	
	// Each data point is has so many coordinates as DIMENSION
	data_points = malloc((*N_points) * sizeof(point));
	
//	while(fgets(buffer,,sizeof(buffer )),fptr)!=NULL){
//	}

	int i;
	
	for (i = 0; i < (*N_points); i++) {
		int x, y, z;
		
		fscanf(fptr, "%d %d %d", &x, &y, &z);
		// Questa scan dovrebbe memorizzare dei float, non dei double
		//fscanf(fptr, "%0.2f %0.2f %0.2f", &x, &y, &z);
		//printf("%f %f %f \n", x, y, z);
		data_points[i].coordinates[0] = x;
		data_points[i].coordinates[1] = y;
		data_points[i].coordinates[2] = z;
		printf("%d %d %d \n", x, y, z);
	}
	
	
	fclose(fptr);
}

