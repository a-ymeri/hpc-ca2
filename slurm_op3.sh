#!/bin/bash -l

### COMP328 (lab03): example batch script
###           usage: "sbatch run.sh"
###         purpose: to run serial and then parallel OpenMP & MPI on requested number of cores
###    restrictions: this script can only handle single nodes
### (c) mkbane, University of Liverpool (2020)

# Specific course queue and max wallclock time
#SBATCH -p course -t 2

# Defaults on Barkla (but set to be safe)
## Specify the current working directory as the location for executables/files
#SBATCH -D ./
## Export the current environment to the compute node
#SBATCH --export=ALL

# load modules
## intel compiler
module load compilers/intel/2019u5
## intel mpi wrapper and run time
module load mpi/intel-mpi/2019u5/bin

# SLURM terms
## nodes            relates to number of nodes
## ntasks-per-node  relates to MPI processes per node
## cpus-per-task    relates to OpenMP threads (per MPI process)

# determine number of cores requested (NB this is single node implementation)
## further options available via examples: /opt/apps/Slurm_Examples/sbatch*sh
echo "Node list                    : $SLURM_JOB_NODELIST"
echo "Number of nodes allocated    : $SLURM_JOB_NUM_NODES or $SLURM_NNODES"
echo "Number of threads or processes          : $SLURM_NTASKS"
echo "Number of processes per node : $SLURM_TASKS_PER_NODE"
echo "Requested tasks per node     : $SLURM_NTASKS_PER_NODE"
echo "Requested CPUs per task      : $SLURM_CPUS_PER_TASK"
echo "Scheduling priority          : $SLURM_PRIO_PROCESS"

# no check for expected inputs (note that more than 1 node is allowed for MPI)


# parallel using MPI
SRC=op3.c
EXE=${SRC%%.c}.exe
echo compiling $SRC to $EXE

# # compile and run, with 1 MPI process up to 40 MPI processes
# mpiicc -O0 $SRC -o $EXE -std=c99 && \
#       (
#         for i in {1..40}
#         do
#           echo "Running ${EXE} with $i MPI processes"
#           mpirun -np $i ./${EXE} a.dat b.dat c.dat
#         done
#       ) \
#       || echo $SRC did not built to $EXE

#compile and run with 1, 2, 4, 8, 16, 32 MPI processes
mpiicc -O0 $SRC -o $EXE -std=c99 && \
      (
        for i in 1 2 4 8 16 32
        do
          echo "Running ${EXE} with $i MPI processes"
          for j in {1..3}
          do
            mpirun -np $i ./${EXE} $1 $2 $3
          done
        done
      ) \
      || echo $SRC did not built to $EXE