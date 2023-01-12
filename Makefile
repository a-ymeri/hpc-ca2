all: op1 op2 op3

op1: op1.c
	module load compilers/intel/2019u5
	module load mpi/intel-mpi/2019u5/bin
	mpiicc -O0 op1.c -o op1

op2: op2.c
	module load compilers/intel/2019u5
	module load mpi/intel-mpi/2019u5/bin
	mpiicc -O0 op2.c -o op2

op3: op3.c
	module load compilers/intel/2019u5
	module load mpi/intel-mpi/2019u5/bin
	mpiicc -O0 op3.c -o op3