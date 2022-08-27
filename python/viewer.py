import matplotlib.pyplot as plt
import numpy as np
import time
import csv

size = 128
loop = 0
t0 = time.time()

with open('cells.csv') as f:
    reader = csv.reader(f, delimiter=' ')
    for row in reader:
        col = [int(_) for _ in row]
        cells = np.array(col, dtype=bool).reshape(size, size)

        plt.imshow(cells)
        plt.pause(0.01)
        if loop == 0 or loop == 2474:
            print("Generations:", loop)
            plt.pause(5)
        elif loop % 100 == 0:
            print("Generations:", loop)
        plt.cla()
        loop += 1

t1 = time.time()
print("Elapsed time:", t1-t0)
