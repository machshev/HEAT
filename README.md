# HEAT

This project is to convert an existing S-plan heating system to arduino based control.

## Status

The project is driven by the imediate need of getting hotwater again :) The thermostat on the hot water cylinder is functionally impared and requires replacing. I've got a number of ideas where this project could go, but for the moment it's just a quick arduino nano base prototype with bits I've got lying around... cold showers are no fun.

The system is modular with multiple nodes on a CAN bus. This allows flexability, robustness to interferance and not reliant internet or wifi router up time. At some point I'd like to add an ESP32 WiFi node or some ethernet base node as bridge to accept heating setpoint requests or view some stats... but the heating system should be dependent on nothing but running through the power cables. If the internet goes down, then we still get heating.

Anyway, at the moment this is just a bit of a code dump. I'll plan to add KiCAD schematics, and maybe even a custom PCB at some point. Then extend this to ocupancy sensing nodes, room temperature, humidity, door and window open sensing nodes, and other such stuff.

## Disclamer

To build and install this project will require working with mains electricity. Mains electricity can kill you. Please only do this if you are qualified to do so. Any instructions provided in this repository are for information only, if you follow them, it's at your own risk.
