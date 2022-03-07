<br>
<center>
<h2><b><i>Sensebox Firmware</i></b></h2>
Data Driven Smart Cities Minor
</center>
<br>
<br>

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
│   │   ├── app.cpp/hpp     # (TODO) application wrapper
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