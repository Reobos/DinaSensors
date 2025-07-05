# DinaSensors

Sensors and Server for Dinacon 2025

## Smell Sensor Setup

The sample code is for ESP32 Arduino platform.io project publishing to a local MQTT server. A bunch of gas and atmospheric sensors are currently published to topics.

Below is a table of configuration for the local server. You can subscribe to the topics, which are currently published every 2 seconds. You can configure the publish rate or process the data however you want! Check out the source code at [src/dina_smell.cpp](src/dina_smell.cpp). This can also serve as a nice template for adding other sensors and publishing to MQTT.


| Setting                | Value                        |
|------------------------|------------------------------|
| **WiFi SSID**          | DinaCam                      |
| **WiFi Password**      | bigheadsim                   |
| **MQTT Server**        | 192.168.8.174                |
| **MQTT Port**          | 1883                         |
| **MQTT Username**      | dinacam                      |
| **MQTT Password**      | bigheadsim                   |
| **MQTT Topic (Temp)**  | dinacam/feeds/dinasmell.temp |
| **MQTT Topic (Humd)**  | dinacam/feeds/dinasmell.humd |
| **MQTT Topic (Pres)**  | dinacam/feeds/dinasmell.pres |
| **MQTT Topic (CO₂)**   | dinacam/feeds/dinasmell.co2  |
| **MQTT Topic (VOC1)**  | dinacam/feeds/dinasmell.voc1 |
| **MQTT Topic (VOC2)**  | dinacam/feeds/dinasmell.voc2 |
| **MQTT Topic (NO₂)**   | dinacam/feeds/dinasmell.no2  |
| **MQTT Topic (Eth)**   | dinacam/feeds/dinasmell.eth  |
| **MQTT Topic (CO)**    | dinacam/feeds/dinasmell.co   |

## Mosquitto (MQTT) Setup

Tutorial: [https://randomnerdtutorials.com/cloud-mqtt-mosquitto-broker-access-anywhere-digital-ocean/](https://randomnerdtutorials.com/cloud-mqtt-mosquitto-broker-access-anywhere-digital-ocean/)

First install the broker server:

```sh
sudo apt update && sudo apt upgrade -y
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto.service
```

User and password

```sh
sudo mosquitto_passwd -c /etc/mosquitto/passwd YOUR_USERNAME
sudo chown mosquitto /etc/mosquitto/passwd
```

Change config

```sh
sudo vim /etc/mosquitto/mosquitto.conf
```

Change file contents to this:

```sh
# Place your local configuration in /etc/mosquitto/conf.d/
#
# A full description of the configuration file is at
# /usr/share/doc/mosquitto/examples/mosquitto.conf.example

per_listener_settings true

pid_file /run/mosquitto/mosquitto.pid

persistence true
persistence_location /var/lib/mosquitto/

log_dest file /var/log/mosquitto/mosquitto.log

include_dir /etc/mosquitto/conf.d
allow_anonymous false
listener 1883
password_file /etc/mosquitto/passwd
```

Restart server

```sh
sudo systemctl restart mosquitto
```

