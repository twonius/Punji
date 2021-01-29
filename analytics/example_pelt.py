import numpy as np
import matplotlib.pylab as plt
import ruptures as rpt




# creation of data
n, dim = 500, 1
n_bkps, sigma = 3, 1
signal, b = rpt.pw_constant(n, dim, n_bkps, noise_std=sigma)
   
    
print(signal)

# change point detection
model = "l1"  # "l2", "rbf"
algo = rpt.Pelt(model=model, min_size=1, jump=5).fit(signal)
my_bkps = algo.predict(pen=10)

# show results
fig = rpt.display(signal, b, my_bkps, figsize=(10, 6))
print("b:", b)
print("my_bkps: ", my_bkps)
plt.show()

