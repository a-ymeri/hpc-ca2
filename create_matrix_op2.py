import sys
import random
def main():
    #read sys args
    m = int(sys.argv[1])
    n = int(sys.argv[2])

    #write to a file b m n, and then second line being b * m * n floats
    f = open(sys.argv[3], "w")

    f.write(str(m) + " " + str(n) + " \n")

    for i in range(m * n):
        #random float between 0 and 1000
        f.write(str(random.random() * 1000) + " ")
    
    f.close()


    


main() 