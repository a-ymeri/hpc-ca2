import sys
def main():
    #generate unsorted list of floating point numbers, with size 1000000, and save it to a file called unsorted.txt
    #then sort the list and save it to a file called sorted.txt

    #read size from command arguments
    import random
    size = int(sys.argv[1])
    unsorted = []
    for i in range(size):
        #add a floating point number with at most 4 decimal places
        unsorted.append(random.uniform(0, 10000))
    f = open("unsorted.txt", "w")
    f.write(str(size) + "\n")
    for i in unsorted:
        f.write(str(i) + " ")
    f.close()

    # #then sort the list and save it to a file called sorted.txt
    # unsorted.sort()
    # f = open("sorted.txt", "w")
    # f.write(str(1000000) + "\n")
    # for i in unsorted:
    #     f.write(str(i) + " ")
    # f.close()


if __name__ == "__main__":
    main()