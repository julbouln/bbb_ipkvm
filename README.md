# BBB KVM over IP VPN Server

Low power IP KVM VNC server solution using Beaglebone Black and HDMI to USB Macrosilicon MS2109 dongle.

## How it works
The server software capture video coming from the HDMI to USB dongle and send captured frame in realtime to VNC client.
It will also automaticaly create required files using configfs USB gadget to transmit input events coming from the client to the target computer.
ikvm_vnc_server will work with any VNC client, however the included SDL based ikvm_vnc_client support a custom RFB audio extension to allow a one program solution including video and audio desktop sharing.

## Harware requirement:
- beaglebone black
- HDMI to USB dongle
- mini usb cable

Plug the target computer HDMI to the dongle, and the dongle to the BBB USB host port. Plug the mini USB cable between the USB OTG of beaglebone to the target computer.

## Software requirement
- libvncserver >= 0.9.13 https://github.com/LibVNC/libvncserver
- libasound
- SDL >= 2.0.4 for client

## TODO:
- usb hid gadget do not wake up host

## Sources
- https://github.com/openbmc/obmc-ikvm
- https://github.com/Nuvoton-Israel/obmc-ikvm

## See also
- https://pikvm.org/
- https://mtlynch.io/tinypilot/

## Notes
- MS2109 audio is not 96000/mono https://patchwork.kernel.org/project/alsa-devel/patch/20200620135202.213447-1-marcan@marcan.st/
- There is no host wakeup with hid usb gadget https://stackoverflow.com/questions/44337151/how-do-you-get-a-composite-usb-linux-device-with-hid-to-wake-up-a-suspended-host
