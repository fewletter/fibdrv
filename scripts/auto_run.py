import subprocess
import numpy as np
import matplotlib.pyplot as plt

runs = 50

def outlier_filter(datas, threshold = 2):
    datas = np.array(datas)
    z = np.abs((datas - datas.mean()) / datas.std())
    return datas[z < threshold]

def data_processing(data_set, n):
    catgories = data_set[0].shape[0]
    samples = data_set[0].shape[1]
    final = np.zeros((catgories, samples))

    for c in range(catgories):        
        for s in range(samples):
            final[c][s] =                                                    \
                outlier_filter([data_set[i][c][s] for i in range(n)]).mean()
    return final


if __name__ == "__main__":
    Ys = []
    for i in range(runs):
        comp_proc = subprocess.run('sudo ./test_time', shell = True)
        output = np.loadtxt('data.txt', dtype = 'float').T
        Ys.append(np.delete(output, 0, 0))
    X = output[0]
    Y = data_processing(Ys, runs)

    fig, ax = plt.subplots(1, 1, sharey = True)
    plt.title('50 runs of each fibonacci-iterative', fontsize = 16)
    plt.xlabel(r'$n_{th}$ fibonacci', fontsize = 16)
    plt.ylabel('time (ns)', fontsize = 16)

    plt.plot(X, Y[0], marker = '+', markersize = 7, label = 'user')
    plt.plot(X, Y[1], marker = '*', markersize = 3, label = 'kernel')
    plt.plot(X, Y[2], marker = '^', markersize = 3, label = 'kernel to user')
    plt.legend(loc = 'upper left')
    plt.grid()
    fig.savefig('after_process.png')

