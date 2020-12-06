#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

/* 10 colors plus white are defined.  Many more are possible. */
#define WHITE    "15 15 15 "
#define RED      "15 00 00 "
#define ORANGE   "15 05 00 "
#define YELLOW   "15 10 00 "
#define LTGREEN  "00 13 00 "
#define GREEN    "05 10 00 "
#define LTBLUE   "00 05 10 "
#define BLUE     "00 00 10 "
#define DARKTEAL "00 05 05 "
#define BROWN    "03 03 00 "
#define BLACK    "00 00 00 "
#define DIM 8

void CopyNewToOld(float **new, float **old);
void CalculateNew(float **new, float **old,float *aboveGhost,float *belowGhost,int rank,int section_size);
void PrintGrid(float **new);
void convertGridtoColors(float **heat_grid);
void sendRecvGhostPoints(float **new,float *ghostBelow,float *ghostAbove,int rank,MPI_Status status,int section_size);

int main(int argc, char *argv[]){
	// allocate memory for arrays
	float *ghostAbove = (float *)malloc(DIM*sizeof(float *));
	float *ghostBelow = (float *)malloc(DIM*sizeof(float *));
	float **old = (float **)malloc(DIM * sizeof(float *)); 
	float **new = (float **)malloc(DIM * sizeof(float *)); 
  for (int i=0; i<DIM; i++){ 
  	old[i] = (float *)malloc(DIM * sizeof(float)); 
		new[i] = (float *)malloc(DIM * sizeof(float)); 
	}
	
	int rank,size;
	MPI_Status status;
	int iterations = 10000;

	// initialize grid
	for (int y = 0; y < DIM; y++){
		for (int x = 0; x < DIM; x++){
			old[y][x] = 20*4*y;
		}
	}
	// place fireplace
	int fire_width = (DIM/10)*4;
	int fire_start = (DIM/10)*3;
	for(int x = 0; x < fire_width; x++){
		old[0][fire_start+x] = 300;
	}
	memcpy(new, old, sizeof(float)*DIM*DIM);
	
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	printf("%d",rank);

	// calculate the section size of the grid each process will get (the strip size)	
	int section_size = DIM/size;
	for (int i = 0; i < iterations; i++) {
		sendRecvGhostPoints(new,ghostAbove,ghostBelow,rank,status,section_size);
		// put barrier here
		MPI_Barrier(MPI_COMM_WORLD);
		CalculateNew(new,old,ghostAbove,ghostBelow,rank,section_size);
		CopyNewToOld(new,old);
	}

	// send everything back to process 0
	if (rank != 0){
		MPI_Send(new[rank*section_size],section_size*DIM,MPI_FLOAT,0,0,MPI_COMM_WORLD);
	}
	else {
		float * tempRows = (float *)malloc(DIM*section_size*sizeof(float *));
		for(int i = 1; i < size; i++){
			MPI_Recv(tempRows,section_size*DIM,MPI_FLOAT,i,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			memcpy(tempRows,new[i*section_size],sizeof(float)*DIM*section_size); 
		}
		convertGridtoColors(new);
	}
	MPI_Finalize();
	return 0;
}

void sendRecvGhostPoints(float **new, float *ghostAbove,float *ghostBelow,int rank,MPI_Status status,int section_size){
	if (rank == 0){
		// get the temps from row below
		MPI_Recv(ghostBelow,DIM,MPI_FLOAT,1,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
		// send temps to row below
		MPI_Send(new[rank+section_size-1],DIM,MPI_FLOAT,1,0,MPI_COMM_WORLD);
	}
	else if (rank < DIM-1){
		// get temps from above/below
		MPI_Recv(ghostBelow,DIM,MPI_FLOAT,rank+1,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
		MPI_Recv(ghostAbove,DIM,MPI_FLOAT,rank-1,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
		// send the top most row and the bottom most row of section to process above and below
		MPI_Send(new[(rank*section_size)+section_size-1],DIM,MPI_FLOAT,rank+1,0,MPI_COMM_WORLD);
		MPI_Send(new[rank*section_size],DIM,MPI_FLOAT,rank-1,0,MPI_COMM_WORLD);
	} 
	else {
		// get temps from row above
		MPI_Recv(ghostAbove,DIM,MPI_FLOAT,rank-1,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
		// send temps
		MPI_Send(new[rank*section_size],DIM,MPI_FLOAT,rank-1,0,MPI_COMM_WORLD);
	}	
}

void CalculateNew(float **new,float **old,float *aboveGhost,float *belowGhost,int rank,int section_size){
	// loop over the section (row by row) skipping top and bottom wall
	// loop over each row skipping the walls
	for (int i = 0; i < section_size; i++){
		for (int x = 1; x < DIM-1;x++){
			int y = (rank*section_size)+i;
			if(y != 0 && y != DIM-1){
				// take into account the ghost point above and below for the top and bottom of the section
				// if you are calculating the top of the section
				if(i = 0){
					new[y][x] = 0.25*(old[y][x-1]+aboveGhost[x]+old[y+1][x]+old[y][x+1]);
				}
				// if you are calculating the bottom of the section
				else if(i = section_size-1){
					new[y][x] = 0.25*(old[y][x-1]+old[y-1][x]+belowGhost[x]+old[y][x+1]);
				}
				else{
					new[y][x] = 0.25*(old[y][x-1]+old[y-1][x]+old[y+1][x]+old[rank][x+1]);
				}	
			}
		}
 	}
}

void CopyNewToOld(float **new, float **old){
	memcpy(old,new,sizeof(float)*DIM*DIM);
}

void PrintGrid(float **new){

	for (int i = 0; i < DIM; i++){
		for (int j = 0; j < DIM; j++){
			float x = new[i][j];
			if(j == 0){
				printf("\n");
			}
			printf(" %.1f",x);
		}
 	}
	printf("\n\n");
}

void convertGridtoColors(float **heat_grid){
	char * heat_colors[DIM][DIM];

	int n = DIM;	
	for (int i = 0; i < n; i++){
		for (int j = 0; j < n; j++){
			float temp = heat_grid[i][j];
			if(temp > 250){
				heat_colors[i][j] = RED;
			}
			else if(temp > 180){
				heat_colors[i][j] = ORANGE;
			}
			else if(temp > 120){
				heat_colors[i][j] = YELLOW;
			}
			else if(temp > 80){
				heat_colors[i][j] = LTGREEN;
			}
			else if(temp > 60){
				heat_colors[i][j] = GREEN;
			}
			else if(temp > 50){
				heat_colors[i][j] = LTBLUE;
			}
			else if(temp > 40){
				heat_colors[i][j] = BLUE;
			}
			else if(temp > 30){
				heat_colors[i][j] = DARKTEAL;
			}
			else if(temp > 20){
				heat_colors[i][j] = BROWN;
			}
			else {
				heat_colors[i][j] = BLACK;
			}
		}
	}
  
	 FILE * fp;

   int numcolors = 10;
   int color;

   /* Colors are list in order of intensity */
   char * colors[10] = { RED, ORANGE, YELLOW, LTGREEN, GREEN, 
                         LTBLUE, BLUE, DARKTEAL, BROWN, BLACK };

   int linelen = DIM;
   int numlines = DIM;
   int i, j;

   /* The pnm filename is hard-coded.  */

   fp = fopen("c.pnm", "w");

   /* Print the P3 format header */
   fprintf(fp, "P3\n%d %d\n15\n", linelen, numlines);

   /* Print 300 lines of colors. ASCII makes this easy.           */
   /* Each %s (color string) is a single pixel in the final image */
   for (i=0; i<numlines; i++) {
      for (j=0; j<linelen; j++)  
         fprintf(fp, "%s ", heat_colors[i][j]);
      fprintf(fp, "\n");
   }

   fclose(fp);

   /* Convert the pnm file to a format that is more easily viewed
      in a web browser. */ 
   /*   system("convert c.pnm c.png"); */
   system("convert c.pnm c.gif");   /* png not supported on comp */
}  
