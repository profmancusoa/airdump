# airdump

![airdump_logo](https://github.com/user-attachments/assets/0cfef053-9f48-4b26-ad84-adad20f8e306)

**airdump** is a lightweight yet powerful tool for monitoring WiFi networks. It periodically captures key wireless parameters and dumps them into log files for later analysis and processing.

## Key Features


- Periodic scanning of WiFi signal
- Captures SSID, signal strength, quality, channel, frequency, and more
- Automatic logging at customizable intervals
- Saves logs in CSV format
- Simple and configurable command-line interface

## Dependencies

```bash
linux-vdso.so.1 (0x000074e2f6f06000)
libcap.so.2 => /usr/lib/libcap.so.2 (0x000074e2f6ec6000)
libncursesw.so.6 => /usr/lib/libncursesw.so.6 (0x000074e2f6e57000)
libm.so.6 => /usr/lib/libm.so.6 (0x000074e2f6d5d000)
libnl-cli-3.so.200 => /usr/lib/libnl-cli-3.so.200 (0x000074e2f6d51000)
libnl-genl-3.so.200 => /usr/lib/libnl-genl-3.so.200 (0x000074e2f6d4a000)
libnl-nf-3.so.200 => /usr/lib/libnl-nf-3.so.200 (0x000074e2f6d31000)
libnl-route-3.so.200 => /usr/lib/libnl-route-3.so.200 (0x000074e2f6c9c000)
libnl-3.so.200 => /usr/lib/libnl-3.so.200 (0x000074e2f6c7a000)
libc.so.6 => /usr/lib/libc.so.6 (0x000074e2f6a8c000)
/lib64/ld-linux-x86-64.so.2 => /usr/lib64/ld-linux-x86-64.so.2 (0x000074e2f6f08000)
libgcc_s.so.1 => /usr/lib/libgcc_s.so.1 (0x000074e2f6a5f000)
```

## Build

```bash
$ git clone https://github.com/profmancusoa/airdump.git
$
$ cd airdump
$
$ make or make release // to build it for production
$
$ make debug // with lots of debugging messages
```

## Usage

```bash
Usage: airdump [-hv] [-d ms] [-c count] <-i ifname> <-o dumpfile>
  -h            This help screen
  -v            Print version details
  -d [ms]       Scan period in milliseconds - default 100
  -c [count]    Exact number of measure to dump - default 2^32
  -i <ifname>   Use specified network interface
  -o <dumpfile> File to dump air information

$ airdump -i wlan0 -d 200 -c 3000 -o dump.csv

Capture (-c) 3000 samples, one every (-d) 200 ms and dump it to (-o) dump.csv file

```


## Credits

This project is eavily inspired by the great tool [wavemon](https://github.com/uoaerg/wavemon), however the code structure is quite different

It provides almost the same information as provided by Info screen of wavemon, but does not have a UI as it task is just to dumo info to a file.

