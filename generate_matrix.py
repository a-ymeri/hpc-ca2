import sys
import numpy as np


def op1(input_vec, m, n, filter_vec, k, output_vec, b):

    #input_vec is an array of size b*m*n
    input = np.reshape(input_vec, (b, m, n))
    filter = np.reshape(filter_vec, (k, k))
    output = np.reshape(output_vec, (b, m, n))


    b_lower = (k - 1) // 2
    b_upper = k // 2
    row_start = b_lower
    row_end = m - b_upper
    col_start = b_lower
    col_end = n - b_upper


    for ba in range(b):
        for i in range(m):
            for j in range(n):
                if i < row_start or i >= row_end or j < col_start or j >= col_end:
                    output[ba][i][j] = input[ba][i][j]

    for ba in range(b):
        for i in range(row_start, row_end):
            for j in range(col_start, col_end):
                output[ba][i][j] = 0
                for x in range(k):
                    for y in range(k):
                        output[ba][i][j] += input[ba][i - b_lower + x][j - b_lower + y] * filter[x][y]

def solve():
    #read command line arguments
    input_file = sys.argv[1]
    #second argument is the number of rows in the first matrix
    kernel_file = sys.argv[2]

    #read the input file,
    with open(input_file, 'r') as f:
        #first line is b m n. B is the number of matrices to be convoluted, m is the number of rows in the first matrix, n is the number of columns in the first matrix
        b, m, n = map(int, f.readline().split())
        print(b, m, n)
        #read the matrices. The matrices are stored in one line, separated by spaces
        matrices = list(map(int, f.readline().split()))
        print(matrices)
        #split line2 into b matrices




        #read the kernel
     
    with open(kernel_file, 'r') as f:
        #first line is k l. k is the number of rows in the kernel, l is the number of columns in the kernel
        k, l = map(int, f.readline().split())
        #read the kernel
        kernel = []
        for i in range(k):
            kernel.append(list(map(int, f.readline().split()))) 
    
    #print the matrices
    print('Matrices:')
    for i in range(b):
        print(matrices[i*m*n:(i+1)*m*n])

    #merge the matrices into one array
    input_vec = np.array(matrices)

    res = op1(input_vec, m, n, kernel, k, input_vec, b)

    print('Result:')
    print(res)


        

    # #print the kernel
    # print('Kernel:')
    # for i in range(k):
    #     print(kernel[i])

    
solve();


# def generate():
            