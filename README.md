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

## Cloning the project
___

To clone the project, it needs to be cloned with submodules. To do this effectively the following command should be used:

```bash
git clone https://github.com/SenseBoxTI/Firmware.git --recursive
```

In case you don't want to clone the project with submodules, but do end up needing them, you should be able to do it like this:

```bash
git clone https://github.com/SenseBoxTI/Firmware.git
cd ./Firmware/
git submodule update --init
```

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
│   ├── <adafruit libs>     # libraries for some of the sensors
│   └── Arduino             # arduino esp32 core
├── main                    # source directory
│   ├── app                 # the sensebox implementation 
│   │   ├── app.cpp/hpp     # application wrapper
│   │   ├── <sensors>       # sensor implementations
│   │   └── <core>          # core component implementations (SD card, etc)
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

## Sensebox Configuration
---
To configure things in an existing sensebox, you should put a `config.toml` file in the root of your sd card. This will let you configure certain offsets / credentials without having to compile a new binary.

### Configuration example:

```toml
[wifi]
    ssid = ""
    eapId = ""
    eapUsername = ""
    password = ""

[calibration]
    [scd30]
    temperatureOffset = 0

    [colorspectrum]
    magentaOffset = 0
    blueOffset = 0
    greenOffset = 0
    yellowOffset = 0
    orangeOffset = 0
    redOffset = 0

    [lightintensity]
    intensityOffset = 0

    [voc]
    factor = 1

    [particles]
    factor = 1

    [db]
    offset = 0
    rc = 1

    [o2]
    offset = 0
    rc = 1

[mqtt]
    url = ""
    deviceId = ""
    accessToken = ""
    provisionKey = ""
    provisionSecret = ""
```

------