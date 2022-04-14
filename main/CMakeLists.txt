idf_component_register(
SRCS
    "main.cpp"
    # Add files separately, no recursives or GLOBs as requested by project members.
    "./app/app.cpp"
    # Sensor stack
    "./app/sensors/sensor.cpp"
    "./app/sensors/sensorstatus.cpp"
    "./app/sensors/sensormanager.cpp"
    "./app/sensors/sensoroutput.cpp"
    "./app/sensors/measurements.cpp"
    # Sensor Impl
    "./app/sensors/impl/adctest.cpp"
    "./app/sensors/impl/i2ctest.cpp"
    "./app/sensors/impl/wifitest.cpp"
    "./app/sensors/impl/uarttest.cpp"
    "./app/sensors/impl/sdtest.cpp"
    # Wifi
    "./app/Wifi/wifi.cpp"
    # SDCard
    "./app/sdcard/file.cpp"
    "./app/sdcard/sd.cpp"
INCLUDE_DIRS
    # Add directories seperately.
    "./app/"
    # Sensor stack
    "./app/sensors/"
    # Sensor Impl
    "./app/sensors/impl/"
    # Wifi
    "./app/Wifi/"
    #
    "./app/sdcard/"
)
