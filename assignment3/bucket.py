# Title: Assignment 3
# Author: Erik Blom
# Date: 11-6-2020


import numpy as np
from mpi4py import MPI

# setup MPI
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()
name = MPI.Get_processor_name()
print ("Hello world from process %s running on host %s out of %s processes" %
       (rank, name, size))

# array size
N = 64

# declare array unsorted and final sorted array
sort_num = np.zeros(N,dtype="int") 
raw_num = np.zeros(N, dtype="int")

if rank == 0:
    # initlize array
    raw_num = np.random.randint(low=0,high=N,size=N)
    print ("Unsorted array ", raw_num)

# broadcast the contents of unsorted from 0 to other processes
comm.Bcast(raw_num,root=0)

# determine local min and max for each process
counter = 0
local_min = rank * (N/size)
local_max = (rank+1) * (N/size)
for i in range(N):
    if raw_num[i] >= local_min and raw_num[i] < local_max:
        counter = counter + 1 
print(f"For rank {rank}, max is {local_max}, min is {local_min}, and there are {counter} elements in raw_num that falls within max and min")

# each process creates it's own bucket
local_bucket = np.zeros(counter, dtype="int")
counter=0
for i in range(N):
    if raw_num[i] >= local_min and raw_num[i] < local_max:
        local_bucket[counter] = raw_num[i]
        counter = counter+1

# insertion sort on each bucket
for i in range(counter):
    j = i+1
    while j < counter:
        if local_bucket[i] > local_bucket[j]:
            tmp = local_bucket[i]
            local_bucket[i] = local_bucket[j]
            local_bucket[j] = tmp 
        j += 1

# set up root process
if rank == 0:
    proc_count = np.zeros(size,dtype="int")
else:
    proc_count = None

# populate proc_count
arr = np.array([counter])
comm.Gather(arr,proc_count,root=0)

if rank == 0:
    sort_num = np.zeros(N,dtype="int")
else:
    sort_num = None

# receive final result
comm.Gatherv(sendbuf=local_bucket,recvbuf=(sort_num,proc_count),root=0)

# print sorted array
if rank == 0:
    print ("Sorted array ", sort_num)

