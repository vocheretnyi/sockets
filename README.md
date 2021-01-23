# Lab 1 (Operating Systems)

- Language - C
- Platform - Linux (Ubuntu 20.04)

# How to run
1. Build the client and server `*.c` files, by running comman: **```gcc my_file.c -o my_file```**
2. Run server, with passing 2 parameters - socket type (`UNIX` or `INET`) and blocking type - `0 | 1`.
3. Run client by passing 1 parameter - socket type (`UNIX` or `INET`).

# Results

|Method|Time for 1GB data (in sec)|
|------|-------------|~~~~
|INET Non-blocking|<center>27.436</center>|
|INET blocking|<center>39.103</center>|
|UNIX non-blocking|<center>6.498</center>|
|UNIX blocking|<center>7.432</center>|

