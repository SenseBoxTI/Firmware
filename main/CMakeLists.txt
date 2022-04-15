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
    "./app/sensors/impl/dbsensor.cpp"
    # Wifi
    "./app/Wifi/wifi.cpp"
    # SDCard
    "./app/sdcard/file.cpp"
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
