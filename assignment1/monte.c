#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>


int main(int argc, char *argv[])
{
	int i,count,size,p,rank,samples;
	double pi,x,y,start_time,end_time,elapsed_time;
	MPI_Status status;

	// get number of samples to run from cmd line
	samples = atoi(argv[1]);

	// get mpi info	
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);

	// get the starting time if the main process
	if (rank == 0){
		start_time = MPI_Wtime();
	}	
	
	// divide up samples into local samples
	int local_samples = samples/size;
	int local_count = 0;
	for (i = 0;i < local_samples;i++){
		x = (double) rand() / RAND_MAX;
		y = (double) rand() / RAND_MAX;
		if (x*x + y*y <= 1){
			local_count++;	
		}
	}

	// get results from other processes and add to the total count
	int local_result = 0;
	if (rank==0){
		count=local_count;
		for (p=1; p < size;p++){
			MPI_Recv(&local_result,1,MPI_DOUBLE,p,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			count+=local_result;	
		}
	}
	// send local count back to master
	else {
		MPI_Send(&local_count,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD);
	}
	
	if(rank==0){
		// calculate estimate for pi
		pi = 4.0 * (double)count/(double)samples;
		// calculate the elapsed time
		end_time = MPI_Wtime();
		elapsed_time = end_time - start_time;	
		printf("count: %d, samples: %d,estimate of pi: %7.5f, elapsed time: %7.5f secs\n", count,samples,pi,elapsed_time);
	}	

	MPI_Finalize();
	return 0; 
}
