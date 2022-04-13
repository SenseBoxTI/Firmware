<br>
<center>
<h2><b><i>Sensebox PCB Test Firmware</i></b></h2>
Data Driven Smart Cities Minor
</center>
<br>
<br>

# Test
The following describes how to setup and use the tests written inside this firmware. In all cases the PCB is assumed to be on **during** testing.
## ADC
Setup:
1. Connect potentiometer to the ADC ports through the debug header (AUDIO ADC CHANNEL 7, GPIO 8) (or MOLEX connector if prepared).
2. Try the secondary aswell. Or connect both to the same potentiometer.

Test:
1. Look at the values returned by the 'ADCTest' sensor in the measurements output.

## I2C
Setup:
1. Connect the SCD30 through the debug header (SDA GPIO 47, SCL GPIO 48) (or MOLEX connector (SDC30) if prepared).
2. Try the different Molex Connectors

Test:
1. Look at the values returned by the 'I2CTest' sensor in the measurements output.

## UART
Setup:
1. Connect the UART through the pin header.

Test:
1. Look at the PM25 readout from the I2CTest sensor.

## WiFI
Setup:
1. Add credentials to `wifitest.cpp`, inside the initializer.

Test:
1. Look at the values returned by the 'WifiTest' sensor in the measurements output.

## Development Environment Setup
---
The installation instructions as given by Espressif are very clear.<br>
[**-> Instructions <-**](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#build-your-first-project)

Before using this project, make sure your `PATH` is set up correctly. On Linux this can be done by running the following command: 
```
source <esp-idf-directory>/export.sh
````

You are now able to use `idf.py`
<br><br>

## Building and flashing
---
To build the project, run the following command:
```
idf.py build
```
To flash the project, run the following command:
```
idf.py -p <port> flash
```
`<port>` is the location the esp32 can be found, on linux this is most likely `/dev/ttyUSB0`, while on Windows it could be one of the `COM` _files_.
<br><br>

## Project structure
---
```sh
├── CMakeLists.txt          # project, esp-idf path
├── components              # holds libraries to include
│   ├── <adafruit libs>     # (TODO) libraries for some of the sensors
│   └── Arduino             # arduino esp32 core
├── main                    # source directory
│   ├── app                 # the sensebox implementation 
│   │   ├── app.cpp/hpp     # application wrapper
│   │   ├── <sensors>       # (TODO) sensor implementations
│   │   └── <core>          # (TODO) core component implementations (SD card, etc)
│   ├── CMakeLists.txt      # source include
│   └── main.cpp            # main file (contains app_main(), must be a C function)
└── README.md               # The README you're currently reading
```
<br>

## Project configuration
---
The project can be configured using the `sdkconfig` file, which can be easily edited using the command:
```
idf.py menuconfig
```
`sdkconfig` can be used to enable and disable certain components to speed up compilation time when they are not in use in your branch or fork.