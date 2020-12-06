/*
	Erik Blom 
 	Assignment 2
	10-12-2020 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// set dimension of heat map here
#define DIM 1000

void CopyNewToOld(float new[DIM][DIM], float old[DIM][DIM]);
void CalculateNew(float new[DIM][DIM], float old[DIM][DIM]);
void ConvertGridtoColors(float heat_grid[DIM][DIM]);

// inititialize the heatmaps here
float old[DIM][DIM];
float new[DIM][DIM];

int main(int argc,char *argv[]){
	// set amount of iterations here	
	int iterations = 10000;
	
	// initialize heat grid
	for (int y = 0; y < DIM; y++){
		for (int x = 0; x < DIM; x++){
			old[y][x] = 20;
		}
	}
	
	// place fireplace
	int fire_width = (DIM/10)*4;
	int fire_start = (DIM/10)*3;
	for(int x = 0; x < fire_width; x++){
		old[0][fire_start+x] = 300;
	}
	memcpy(new, old, sizeof(float)*DIM*DIM);

	// caculate new temps over x iterations
	for (int i = 0; i < iterations; i++) {
		CalculateNew(new,old);
		CopyNewToOld(new,old);
	}
	// create heat map image from the 2d array
	ConvertGridtoColors(new);	
	return 0;
}

// calculate the new temperatures for the heat grid
void CalculateNew(float new[][DIM], float old[][DIM]){
	// skip over the walls and fireplace
	int n = DIM;
	for (int i = 1; i < n-1; i++){
		for (int j = 1; j < n-1; j++){
			new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+old[i][j-1]+old[i][j+1]);
		}
 	}
}

// copys newly generated heat map to the old one
void CopyNewToOld(float new[][DIM], float old[][DIM]){
	memcpy(old,new,sizeof(float)*DIM*DIM);
}

// converts the heat grid into colors based on temps
// and then into an image
void ConvertGridtoColors(float heat_grid[][DIM]){
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
}  
