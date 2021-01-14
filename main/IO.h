#define DIMENSIONS 3


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
