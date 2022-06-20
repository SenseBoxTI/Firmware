#pragma once

#include "sensoroutput.hpp"

// The sensor output is made up of a map, this contains all possible outputs by the system.
// Think of it as "Device : [Unit : Value]". Very, JSON-like!
typedef std::map<std::string, SensorOutput> Measurements;