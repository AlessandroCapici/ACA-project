#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
//#include <omp.h>

#define DIMENSIONS 3
#define MAX_ITERATIONS 200
#define THRESHOLD 1e-4

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

point *read_file3D(FILE *f,int *N_points);
double findEuclideanDistance3D(point point, centroid centroid);
double findEuclideanDistance(point point, centroid centroid);
void replaceCentroid(centroid *c);
centroid *kMeanSerial3D(int N_k,point *p);

int main(int argc, char const *argv[]) {
	//	omp_set_num_threads(6);

	/* Correct structure of command line arguments
	 - ./a.out
	 - dataset filename,
	 - number of clusters,
	 - number of threads,
	 - filename to write the clustered data points,
	 - filename to write the coordinated of the centroid
	 */

 	FILE *fp;
	point *points;	// array of data points
	centroid *centroids;	// D array of centroids of clusters

	int N_points;
	int num_iterations = 0;
	int i;

	if (!(fp=fopen("../datasets/dataset_5.txt","r"))) {
		printf("Error\n");
		return 1;
	}

	points = read_file3D(fp,&N_points);
	centroids=kMeanSerial3D(4,points);
	printf("%d\n",N_points);


	for ( i = 0; i < 3; i++) {
		printf("%f %f %f %d\n", centroids[i].coordinates[0],centroids[i].coordinates[1],centroids[i].coordinates[2],centroids[i].ID_cluster);

	}



	free(points);
	 return 0;
}

point *read_file3D(FILE *f,int *N_points)
{
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
        conv = sscanf(buf,"%f %f %f",
				&p[i].coordinates[0],
                &p[i].coordinates[1],
				&p[i].coordinates[2]);
		p[i].ID_point = i;
        i++;
    }
    return p;
}

void replaceCentroid(centroid *c) {

	int i;
	for(i = 0; i < DIMENSIONS; i++){
		(*c).coordinates[i] = (*c).sum_coordinates[i] / (float) (*c).sum_coordinates[i];

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
	bool isChanged = false;

	while(iteration_count < MAX_ITERATIONS && isChanged) {

		double min_distance, current_distance;
		isChanged = false;

		int i;
		for(i = 0; i < N_points; i++) {
			min_distance = __DBL_MAX__; // min_distance is assigned the largest possible double value

			int j;
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
			for(j = 0; j < K; j++) {
				replaceCentroid(&centroids[j]);
			}

		}

		iteration_count++;

	}

	return iteration_count;

}

centroid *kMeanSerial3D(int N_k,point *p) {
	//inizializzazione centroidi (ricordarsi di usare la c alloc)
	centroid *c;
	int i;
	// Each data point is has so many coordinates as DIMENSION
	if (!(c = calloc(N_k,sizeof(*c)))) {
			return NULL;
	}
	for (i = 0; i < N_k ; i++) {
		c[i].coordinates[0]=p[i].coordinates[0];
		c[i].coordinates[1]=p[i].coordinates[1];
		c[i].coordinates[2]=p[i].coordinates[2];
	}
	return c;

}
