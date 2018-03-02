## CS4222 Homework 2 Report

Lu Yu (A0130221H),Zhan Yuli (A0144315N), Group 17



### 1. The Maximum Achievable Frequency for the Data Collection

The maximum achievable frequency is about 185-192Hz. 

The result is measured by using `rtimer` with an interval set to `RTIMER_ARCH_SECOND*0.001` to constantly call `get_mpu_reading()`. This theoretically gives us a maximum of 1000 timer interrupts per second. In reality, 185-192 samples are collected per second due to IO overhead.

We choose `rtimer` over `etimer` in the given code because `etimer` uses `CLOCK_SECOND` for handling system time. By definition:

```
CLOCK_SECOND; // The number of ticks per second. 
```

which is an OS dependent constant. Our preliminary test shows that `CLOCK_SECOND` is set to 128 on CC2650 which renders `etimer` incapable of calling a function at any frequency >128Hz.



### 2. Data Charts

 ![dataplot_fixed](C:\Users\senyu\workspace\CS4222-WirelessNetworking\hw2\dataplot_fixed.PNG)

![dataplot_m](C:\Users\senyu\workspace\CS4222-WirelessNetworking\hw2\dataplot_m.PNG)



### 3. Total Number of Steps for the Two Walks

| Filename                | Duration | Step Count |
| ----------------------- | -------- | ---------- |
| 17_accel_data_fixed.csv | 1m 30s   | 164        |
| 17_accel_data_m.csv     | 1m 30s   | 167        |



### 4. Description of the Algorithm implemented

We designed an algorithm that uses the mean crossing rate of the accelerometer data to give an estimate of the step count.

By analysing the movement of human walking, we can see that in one step, the walking person would experience a major upward and a major downward movement to get back to his/her original position. So by looking at the accelerometer data on the y-axis, we would expect the curve to cross its mean value twice in one step. The same goes for x and z axis as well.

To remove the noise(small body movements, jitters), we used the Savitzky–Golay filter to smoothen the curve. Then we calculate the mean value of the data and shift all the data points by that mean value so that the curve would be along the 0-axis. Then we calculate the zero crossing rate and divide the value by 2 to get the estimate of the step count.

By using the algorithm on multiple data, we discovered that a window size of 50-70 for the filter would yield optimal accuracy.

 

### 5. Accuracy of the Algorithm
| Filename                | Actual steps | count_step |
| ----------------------- | ------------ | ---------- |
| 17_accel_data_fixed.csv | 164          | 172        |
| 17_accel_data_m.csv     | 167          | 169        |
