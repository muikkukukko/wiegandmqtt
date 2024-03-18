# Wiegand to MQTT

## Purpose

This program uses Rasperry Pi Pico W to convert Wiegand card/keypad events into MQTT messages.

## Wiegand implemententation

Wiegand bits are received by the PIO hw in two bit chunks. When two bits have been received, bits are
pushed to PIO fifo. DMA transfers a word containing these 2 bits and when completed triggers an interrupt.
DMA interrupt constructs bytes from incoming DMA transfers.

2-bit transfer was selected as my reader sends card info as 58-bit Wiegand and keypad as 8-bit chunks. Only
common factor for these is 2, so therefore 2-bit transfer was selected.

## State of the project

- Implement easy way to configure -> DONE
- Implement easy way to generate certificates and keys -> ALMOST DONE
- Connect to Wifi -> DONE
- Connect to MQTT server using certificates -> DONE
- Publish messages -> DONE
- Subscribe messages -> DONE
- Use PIO to decode Wiegand -> ALMOST DONE
- Messaging between Wiegand and MQTT blocks -> TODO

## Bugs

- DMA irq not triggering if Wifi has been initialized

## How to use

1) Install Pico SDK and make sure `PICO_SDK_PATH` enviroment variable to pointing to SDK directory
2) Run `python /certificate_helper/cert_generator.py` to generate certificates and keys 
3) Copy `ca.crt`, `client.crt` and `client.key` to `/certificate_helper/generated` directory
4) Edit `settings.json.example`. Fill in wifi and MQTT server details. Save and rename file to `settings.json`
5) In `build` directory, enter `cmake ..` and after that is completed `make`
6) Copy `wiegandmqtt.uf2` to Raspberry Pi Pico

MQTT code is based on [example project](https://github.com/cniles/picow-iot) by Craig Niles
 

