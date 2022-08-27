from pynq import Overlay
from pynq import MMIO

base = Overlay("./design_1.bit")
dma = base.axi_dma_0 # AXI DMA を操作するハンドラ

from pynq import allocate # 利用可能な領域を確保するメソッドを利用可能にする
import numpy as np
import time

SIZE=128
LOOP=2474
t0 = time.time()

odd_buffer = allocate(shape=(SIZE*SIZE,), dtype=np.uint32)
even_buffer = allocate(shape=(SIZE*SIZE,), dtype=np.uint32)

init = np.loadtxt('init.txt').ravel()
for i in range(SIZE * SIZE):
    odd_buffer[i] = init[i]

for i in range(LOOP // 2):
    dma.sendchannel.transfer(odd_buffer)
    dma.recvchannel.transfer(even_buffer)
    dma.sendchannel.wait()
    dma.recvchannel.wait()
    
    dma.sendchannel.transfer(even_buffer)
    dma.recvchannel.transfer(odd_buffer)
    dma.sendchannel.wait()
    dma.recvchannel.wait()

print(' '.join(str(_) for _ in odd_buffer))
passed = True
for i in range(SIZE * SIZE):
    if odd_buffer[i] != 0:
        passed = False
print("Generations:", LOOP)
if passed:
    print("All cells died.")
t1 = time.time()
print("Elapsed time:", t1-t0)
