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

 

### 5. Accuracy of the Algorithm
| Filename                | Actual steps | count_step |
| ----------------------- | ------------ | ---------- |
| 17_accel_data_fixed.csv | 164          | 172        |
| 17_accel_data_m.csv     | 167          | 169        |
