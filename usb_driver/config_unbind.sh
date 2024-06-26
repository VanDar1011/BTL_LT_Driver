#!/bin/bash
echo -n "4-1:1.0" | sudo tee /sys/bus/usb/drivers/usb_driver/unbind
echo -n "4-1:1.0" | sudo tee /sys/bus/usb/drivers/usb-storage/bind
