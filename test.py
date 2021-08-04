import numpy as np


def f(x):
    return x


def integrate(a, b, f, n = 1000):
    x = np.random.uniform(a,b,n)
    return np.mean(f(x))


I = integrate(0, 1, f)
print(I)

