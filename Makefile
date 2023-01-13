all: op1 op2 op3

op1: op1.c
	mpiicc -O0 op1.c -o op1 -lmpi -std=c99

op2: op2.c
	mpiicc -O0 op2.c -o op2 -lmpi -std=c99

op3: op3.c
	mpiicc -O0 op3.c -o op3 -lmpi -std=c99