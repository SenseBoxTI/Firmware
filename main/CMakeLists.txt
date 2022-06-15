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
    "./app/sensors/impl/o2sensor.cpp"
    "./app/sensors/impl/scdsensor.cpp"
    "./app/sensors/impl/colorsensor.cpp"
    "./app/sensors/impl/lightintensitysensor.cpp"
    "./app/sensors/impl/particlesensor.cpp"
    "./app/sensors/impl/vocsensor.cpp"
    # Time
    "./app/time/time.cpp"
    # Wifi
    "./app/Wifi/wifi.cpp"
    # SDCard
    "./app/sdcard/file.cpp"
    "./app/sdcard/dir.cpp"
    # Logger
    "./app/logger/log.cpp"
    "./app/logger/logscope.cpp"
    "./app/logger/logtype.cpp"
    # MQTT
    "./app/mqtt/mqtt.cpp"
    # Config
    "./app/config/config.cpp"
    # Timer
    "./app/timer/CTimer.cpp"
    "./app/timer/CTimers.cpp"
INCLUDE_DIRS
    # Add directories seperately.
    "./app/"
    # Sensor stack
    "./app/sensors/"
    # Sensor Impl
    "./app/sensors/impl/"
    # Time
    "./app/time/"
    # Wifi
    "./app/Wifi/"
    # SDCard
    "./app/sdcard/"
    # Logger
    "./app/logger/"
    # MQTT
    "./app/mqtt"
    # Config
    "./app/config/"
    # Timer
    "./app/timer/"
)
