# No Network MQTT-SN Demo

Simplest possible MQTT-SN client demo for MCUs without any network interface.
MCU sends MQTT-SN "quick publish" (QoS -1) packets over a serial (SLIP) connection.

Tested on STM32F103C8 (aka the "Blue Pill") but should be easily portable to
any board supported by the Arduino library.

# Build

This project uses PlatformIO for building.

Connect your Pill to an ST-LINK and run:

```sh
$ pio run -t upload
```

You can configure some options in `include/app_config.h` and `platformio.ini`
files.

# Wiring

1. Default: just use Pill's native USB as a SLIP interface.

2. Using Serial1 and Serial2 (must be enabled in `platformio.ini`)

- SLIP TX = PA9
- SLIP RX = PA10
- DBG TX = PA2
- DBG RX = PA3

# Running

Connect your Pill USB to a Linux computer and run:

```sh
$ sudo slattach -L -s 115200 -p slip -d /dev/ttyACM0
```

And in another terminal:

```sh
$ sudo ifconfig sl0 192.168.100.1 dstaddr 192.168.100.2
$ ping 192.168.100.2
```

The LED on the pill should blink with every received packet. Pill detects the
echo request packets but does not send echo reply so you will see "100% packet
loss" in ping, that's ok.

If everything went well, you can start an MQTT-SN gateway
(e.g. [this one][arobenko-mqtt-sn] ) on 1883/udp, configure predefined topic 1
and you should see `Hello!` messages sent by the Pill on the configured topic.

[arobenko-mqtt-sn]: https://github.com/arobenko/mqtt-sn
