#include <stdio.h>
#include <stdlib.h>
//#include <omp.h>

#define DIMENSIONS 3

struct point {
	int ID_point;
	int ID_cluster;
	float coordinates[DIMENSIONS];
};
struct point *read_file(FILE *f,int *N_points);

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
	struct point *points;	// array of data points
	struct point *centroids;	// array of centroids of clusters
	int N_points;
	int num_iterations = 0;
  int i;

  if (!(fp=fopen("../datasets/dataset_5.txt","r"))) {
    printf("Error\n");
    return 1;
  }

  points=read_file(fp,&N_points);
	printf("%d\n",N_points);
  for ( i = 0; i < 5; i++) {
    printf("%f %f %f\n",points[i].coordinates[0],points[i].coordinates[1],points[i].coordinates[2] );
  }
	free(points);
  return 0;
}

struct point *read_file(FILE *f,int *N_points)
{
    struct point *p;
    int conv,i=0;
    char buf[1000];
		//remember, the first line is the number of points in the file
    fgets(buf, sizeof(buf), f);
    sscanf(buf, "%d",N_points);

		// Each data point is has so many coordinates as DIMENSION
    if (!(p = malloc((*N_points) * sizeof(*p)))) {
        return NULL;
    }

    while (fgets(buf, sizeof(buf), f)) {
        conv = sscanf(buf,"%f %f %f", &p[i].coordinates[0],
                &p[i].coordinates[1],&p[i].coordinates[2]);
        i++;
    }
    return p;
}
