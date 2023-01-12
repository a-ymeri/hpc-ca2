import sys
import random
def main():
    #read sys args
    b = int(sys.argv[1])
    m = int(sys.argv[2])
    n = int(sys.argv[3])

    #write to a file b m n, and then second line being b * m * n floats
    f = open(sys.argv[4], "w")

    f.write(str(b) + " " + str(m) + " " + str(n) + " \n")

    for i in range(b * m * n):
        #random float between 0 and 1000, with at most 6 decimal places
        f.write(str(round(random.random() * 1000, 6)) + " ")
    
    f.close()


    


main() 