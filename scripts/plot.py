import numpy as np
import matplotlib.pyplot as plt

data = np.loadtxt('data.txt')

if __name__ == '__main__':
    fig = plt.figure()
    plt.title("python scripts plot")
    plt.xlabel("nth fabonacci")
    plt.ylabel("time(ns)")
    plt.plot(data,label = 'kernel iterative')
    plt.legend()
    fig.savefig('data.png')