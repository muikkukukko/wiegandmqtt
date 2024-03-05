# Wiegand to MQTT

## Purpose

This program uses Rasperry Pi Pico W to convert Wiegand card/keypad events into MQTT messages.

## State of the project

- Implement easy way to configure -> DONE
- Implement easy way to generate certificates and keys -> ALMOST DONE
- Connect to Wifi -> DONE
- Connect to MQTT server using certificates -> DONE
- Publish messages -> DONE
- Subscribe messages -> DONE
- Use PIO to decode Wiegand -> TODO

## How to use

1) Install Pico SDK and make sure `PICO_SDK_PATH` enviroment variable to pointing to SDK directory
2) Run `python /certificate_helper/cert_generator.py` to generate certificates and keys 
3) Copy `ca.crt`, `client.crt` and `client.key` to `/certificate_helper/generated` directory
4) Edit `settings.json.example`. Fill in wifi and MQTT server details. Save and rename file to `settings.json`
5) In `build` directory, enter `cmake ..` and after that is completed `make`
6) Copy `wiegandmqtt.uf2` to Raspberry Pi Pico

MQTT code is based on [example project](https://github.com/cniles/picow-iot) by Craig Niles
 

