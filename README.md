# ESPHome AUTOTerm Custom Component
Custom ESPHome component for ESP32 to interface with Autoterm Diesel Air Heaters.

!!! Logic level shifters required. Heater uses 5V logic level, ESP32 3.3V !!!  

Prototype with WT32-ETH01 & ADUM1201 as level shifters

    Control Panel
        Green (RX)	- ADUM1201 VOA  - VIA - ESP32 GPIO17 (TX)
        White (TX)	- ADUM1201 VIB	- VOB	- ESP32 GPIO05 (RX)

    Heater
        Green (TX)	- ADUM1201 VIB  - VOB	- ESP32 GPIO33 (RX)
        White (RX)	- ADUM1201 VOA	- VIA	- ESP32 GPIO32 (TX)

MQTT:

    "state" (operating state):
    0: off
    1: starting
    4: running
    5: shutting down
    6: testing enviroment
    8: ventilation

    "mode" (operating mode):
    1: by heater temperature sensor
    2: by control panel temperature sensor
    3: by external temperature sensor
    4: by power

    "power" (power level)
    0-9

    "vent" (ventilation)
    1: on
    2: off

    "tempset" (temperature setpoint in celsius)
    1-30

    Publishes:
    heater01/autoterm {"state":0,"mode":4,"temp_set":23,"power":3,"vent":2,"temp_heater":0,"temp_ext":8,"temp_panel":-2,"batt":26.9}

    Subscribes:
    heater01/autoterm/cmd

    "cmd" (command)
    1: start heater (only when "state" is off (0))
    2: set settings
    3: shutdown heater (only when "state" is running (4) or ventilation (8))

    Start heater in power mode (4) with power level 3, temperature setpoint 23C and ventilation off
    heater01/autoterm/cmd{"cmd": 1, "mode":4,"temp_set":23,"power":3,"vent":2}
    Each option "mode","temp_set","power" and "vent" is optional.
    When not provided, the system defaults to the current settings.
 
    Start heater with current settigs:
    heater01/autoterm/cmd{"cmd": 1}

    Start heater in power mode (4) with power level 6:
    heater01/autoterm/cmd{"cmd": 1, "mode":4,"power":6}

    Set heater to temperature controlled mode based on external temperature sensor and set temperature setpoint to 22:
    heater01/autoterm/cmd{"cmd": 2, "mode":3,"temp_set":22}

    Shutdown heater: 
    heater01/autoterm/cmd{"cmd": 3}

TODO:

* Publish sensors
* Integrate Home Assistant API



Credits and more info:

https://grimoire314.wordpress.com/2018/08/22/planar-diesel-heater-controller-reverse-engineering/ \
https://grimoire314.wordpress.com/2019/03/21/autoterm-planar-diesel-heater-controller-reverse-engineering-part-2/ \
https://helloworld.schlussdienst.net/blog/hacking-autoterm-planar-2d \
https://github.com/csreades/AutothermDieselRepeater
