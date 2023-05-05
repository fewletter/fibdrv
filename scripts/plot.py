import numpy as np
import matplotlib.pyplot as plt

def loadfile(filename):
    data = np.loadtxt(filename)
    time = []
    for i in range(data.shape[0]):
        time.append(data[i][2])
    return time

if __name__ == '__main__':
    time1 = loadfile('data.txt')
    time2 = loadfile('data1.txt')
    time3 = loadfile('data2.txt')
    X = range(101)
    
    fig = plt.figure()
    plt.xlabel("nth fabonacci")
    plt.ylabel("time(ns)")
    plt.title("fabonacci run time")
    plt.plot(X, time1, marker = '+', markersize = 8, label = 'fast doubling without if')
    plt.plot(X, time2, marker = '^', markersize = 8, label = 'fast doubling with clz')
    plt.legend()
    plt.grid()
    fig.savefig('data.png')
    