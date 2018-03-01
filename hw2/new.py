
# coding: utf-8

# In[1]:

"""
Convert stdout readings to csv files

For internal use. NOT to be submitted.
"""
import math

# flist = ['1min30sec163steps', '1min30sec164hold', '1min30sec167steps', '1min30sec177hold']

flist = ['accel-interval-noiseless.file', 'accel-interval-noisy.file', 'accel-noiseless', 'accel-noisy', 'accel-vnoisy']

# for f in flist:
# 	with open('out/'+f, 'r') as fin:
# 		with open('csv/'+f+'.csv', 'w') as fout:
# 			lines = fin.readlines()
# 			for line in lines:
# 				if 'G' in line:
# 					fout.write(line.replace('G','').replace(' ',''))



# In[89]:

import matplotlib.pyplot as plt
import numpy as np

for f in flist:
    target = 200
    steps = []
    x_pts = []
    data = np.genfromtxt('archive_csv/'+f+'.csv', delimiter=',', names=['x', 'y', 'z'])

    x_cor = []
    y_cor = []
    z_cor = []
    a_cor = []
    G = 9.8

    for row in data:
        x_cor.append(float(row[0]))
        y_cor.append(float(row[1]))
        z_cor.append(float(row[2]))
        a_cor.append(math.sqrt(math.pow(float(row[0]),2) + math.pow(float(row[1]),2) + math.pow(float(row[2]), 2)))


    t = np.arange(0.0, 11922, 1)

    # plt.plot(t[300:1600], x_cor[300:1600], t[300:1600], y_cor[300:1600],t[300:1600], z_cor[300:1600])

    # plt.xlabel('time (s)')
    # plt.ylabel('voltage (mV)')
    # plt.title('About as simple as it gets, folks')
    # plt.grid(True)
    # plt.savefig("test.png")
    #plt.show()

    def savitzky_golay(y, window_size, order, deriv=0, rate=1):
        r"""Smooth (and optionally differentiate) data with a Savitzky-Golay filter.
        The Savitzky-Golay filter removes high frequency noise from data.
        It has the advantage of preserving the original shape and
        features of the signal better than other types of filtering
        approaches, such as moving averages techniques.
        Parameters
        ----------
        y : array_like, shape (N,)
            the values of the time history of the signal.
        window_size : int
            the length of the window. Must be an odd integer number.
        order : int
            the order of the polynomial used in the filtering.
            Must be less then `window_size` - 1.
        deriv: int
            the order of the derivative to compute (default = 0 means only smoothing)
        Returns
        -------
        ys : ndarray, shape (N)
            the smoothed signal (or it's n-th derivative).
        Notes
        -----
        The Savitzky-Golay is a type of low-pass filter, particularly
        suited for smoothing noisy data. The main idea behind this
        approach is to make for each point a least-square fit with a
        polynomial of high order over a odd-sized window centered at
        the point.
        Examples
        --------
        t = np.linspace(-4, 4, 500)
        y = np.exp( -t**2 ) + np.random.normal(0, 0.05, t.shape)
        ysg = savitzky_golay(y, window_size=31, order=4)
        import matplotlib.pyplot as plt
        plt.plot(t, y, label='Noisy signal')
        plt.plot(t, np.exp(-t**2), 'k', lw=1.5, label='Original signal')
        plt.plot(t, ysg, 'r', label='Filtered signal')
        plt.legend()
        plt.show()
        References
        ----------
        .. [1] A. Savitzky, M. J. E. Golay, Smoothing and Differentiation of
           Data by Simplified Least Squares Procedures. Analytical
           Chemistry, 1964, 36 (8), pp 1627-1639.
        .. [2] Numerical Recipes 3rd Edition: The Art of Scientific Computing
           W.H. Press, S.A. Teukolsky, W.T. Vetterling, B.P. Flannery
           Cambridge University Press ISBN-13: 9780521880688
        """
        import numpy as np
        from math import factorial
        
        try:
            window_size = np.abs(np.int(window_size))
            order = np.abs(np.int(order))
        except ValueError:
            raise ValueError("window_size and order have to be of type int")
        if window_size % 2 != 1 or window_size < 1:
            raise TypeError("window_size size must be a positive odd number")
        if window_size < order + 2:
            raise TypeError("window_size is too small for the polynomials order")
        order_range = range(order+1)
        half_window = (window_size -1) // 2
        # precompute coefficients
        b = np.mat([[k**i for i in order_range] for k in range(-half_window, half_window+1)])
        m = np.linalg.pinv(b).A[deriv] * rate**deriv * factorial(deriv)
        # pad the signal at the extremes with
        # values taken from the signal itself
        firstvals = y[0] - np.abs( y[1:half_window+1][::-1] - y[0] )
        lastvals = y[-1] + np.abs(y[-half_window-1:-1][::-1] - y[-1])
        y = np.concatenate((firstvals, y, lastvals))
        return np.convolve( m[::-1], y, mode='valid')


    print('===========================')
    print('file:'+f)
    print('target:'+str(target))
    # In[90]:
    for i in range(5,102,2):
        x_cor_smooth = savitzky_golay(np.array(x_cor), i, 3)
        y_cor_smooth = savitzky_golay(np.array(y_cor), i, 3)
        z_cor_smooth = savitzky_golay(np.array(z_cor), i, 3)

        a_cor_smooth = savitzky_golay(np.array(a_cor), i, 3)

        # In[91]:

        x_cor_smooth


        # In[92]:

        x_mean = np.mean(x_cor_smooth)
        y_mean = np.mean(y_cor_smooth)
        z_mean = np.mean(z_cor_smooth)
        a_mean = np.mean(a_cor_smooth)

        # In[93]:

        x_cor_smooth_minused = x_cor_smooth - x_mean
        y_cor_smooth_minused = y_cor_smooth - y_mean
        z_cor_smooth_minused = z_cor_smooth - z_mean
        a_cor_smooth_minused = a_cor_smooth - a_mean

        # In[94]:

        x_cor_smooth_minused


        # In[95]:

        x_zero_crossings = np.where(np.diff(np.sign(x_cor_smooth_minused)))[0]
        y_zero_crossings = np.where(np.diff(np.sign(y_cor_smooth_minused)))[0]
        z_zero_crossings = np.where(np.diff(np.sign(z_cor_smooth_minused)))[0]
        a_zero_crossings = np.where(np.diff(np.sign(a_cor_smooth_minused)))[0]

        # In[98]:

        print("i="+str(i)+":",str(a_zero_crossings.size/2-target))
        x_pts.append(i)
        steps.append(a_zero_crossings.size/2-target)

    plt.plot(x_pts, steps)

plt.axhline(y=0.0, color='r', linestyle='-')
plt.show()
    # In[ ]:



