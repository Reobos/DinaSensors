# DinaSensors

Local MQTT and OSC Server for Dinacon 2025

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

