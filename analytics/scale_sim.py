import numpy as np
import matplotlib.pylab as plt
import ruptures as rpt
from scipy import signal as sig

adj_amplitude = 5000
adj_index = 2700

height_empty = 4000
height_sit = -2000 

t = np.linspace(0, 6000, 600, endpoint=True)
signal = height_empty+height_sit*sig.square(2 * np.pi * 0.0007 * t)+adj_amplitude*np.heaviside(t-adj_index,0.5)

 



# change point detection
model = "l1"  # "l2", "rbf"
algo = rpt.Pelt(model=model, min_size=1, jump=5).fit(signal)
my_bkps = algo.predict(pen=10)

# show results
fig = rpt.display(signal, my_bkps, figsize=(10, 6))

plt.show()
