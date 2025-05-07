# airdump

![airdump_logo](https://github.com/user-attachments/assets/0cfef053-9f48-4b26-ad84-adad20f8e306)

**airdump** is a lightweight yet powerful tool for monitoring WiFi networks. It periodically captures key wireless parameters and dumps them into log files for later analysis and processing.

## Key Features


- Periodic scanning of WiFi signal
- Captures SSID, signal strength, quality, channel, frequency, and more
- Automatic logging at customizable intervals
- Saves logs in CSV format
- Simple and configurable command-line interface


## Example Usage

```bash

airdump --interval 100 --output /var/log/airlog.csv

```

## Credits

This project is eavily inspired by the great tool [wavemon](https://github.com/uoaerg/wavemon)

It provides almost the same information as provided by Info screen of wavemon, but does not have a UI as it task is just to dumo info to a file.

