#pragma once

#define DEBUG_ENABLED 1

#define MQTT_URL "mqtts://test.ddss-sensebox.nl"

#define PROVISION_KEY ""
#define PROVISION_SECRET ""

// public key
#define MQTT_TLS_CERT \
"-----BEGIN CERTIFICATE-----\n" \
"MIICFzCCAb2gAwIBAgIUEoiTWh4AEEYq+uvJvf8azjDIjtQwCgYIKoZIzj0EAwIw\n" \
"YTELMAkGA1UEBhMCTkwxFjAUBgNVBAgMDU5vb3JkLUhvbGxhbmQxEDAOBgNVBAcM\n" \
"B0Fsa21hYXIxDTALBgNVBAoMBEREU1MxGTAXBgNVBAMMEGRkc3Mtc2Vuc2Vib3gu\n" \
"bmwwHhcNMjIwNjAxMTQxNDQ0WhcNMjMwNjAxMTQxNDQ0WjBhMQswCQYDVQQGEwJO\n" \
"TDEWMBQGA1UECAwNTm9vcmQtSG9sbGFuZDEQMA4GA1UEBwwHQWxrbWFhcjENMAsG\n" \
"A1UECgwERERTUzEZMBcGA1UEAwwQZGRzcy1zZW5zZWJveC5ubDBZMBMGByqGSM49\n" \
"AgEGCCqGSM49AwEHA0IABCn2NFpxwn+2xDQo30NgflYLvbK7wsJGHpmD6GU7gIgk\n" \
"wxfPbholHq1yKFyFUOp8RD4Bh/upIbBrZ7Qjpkd4dzOjUzBRMB0GA1UdDgQWBBSH\n" \
"myR1dTSlIceMaeUzqlRJ6fYPPzAfBgNVHSMEGDAWgBSHmyR1dTSlIceMaeUzqlRJ\n" \
"6fYPPzAPBgNVHRMBAf8EBTADAQH/MAoGCCqGSM49BAMCA0gAMEUCIQCt3cbj6sUC\n" \
"OYnFQSSGD7LZgLQeRvRQ5Cm3HJZpRwZqBAIgL981nWhPEf4p2BYsIAt6xsO6i7eC\n" \
"LiNPVRT2RvZKS58=\n" \
"-----END CERTIFICATE-----"

#define SEND_INTERVAL_US                60 * 1000 * 1000

#define MIX8410_MEASURE_INTERVAL_US     10 * 1000 * 1000 // O2 sensor
#define MAX4466_MEASURE_INTERVAL_US     .5 * 1000 * 1000 // dB sensor
#define TSL2591_MEASURE_INTERVAL_US     2  * 1000 * 1000 // light sensor
#define AS7262_MEASURE_INTERVAL_US      2  * 1000 * 1000 // color sensor
#define CCS811_MEASURE_INTERVAL_US      2  * 1000 * 1000 // tVOC sensor
#define PMSA003I_MEASURE_INTERVAL_US    10 * 1000 * 1000 // particle sensor
#define SCD30_MEASURE_INTERVAL_US       10 * 1000 * 1000 // CO2

#define FILE_CLOSE_TIMEOUT              120 * 1000 * 1000
