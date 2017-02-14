CP210xManufacturing Library for CP210x
--------------------------------------

Release packages contain binaries built using Ubuntu 11.10 Desktop 32-bit and
Ubuntu 12.04.1 Desktop 64-bit. These binaries may not be compatible with your
system, in which case it may be necessary to build the CP210xManufacturing
binaries using make.

CP210xManufacturing linux library release contents
--------------------------------------------------

- /Common/              - Shared/common source
- /CP210xManufacturing/ - CP210xManufacturing library source code
- /nbproject/           - Netbeans project that uses the library and test main
                          app makefiles
- /Release/Linux/       - CP210xManufacturing library header file and output
                          directory for main and libslabhidtouart.so.1.0
                          (released with prebuilt 32-bit Ubuntu 11.10 binaries)
- /Release/Linux/x86_64 - Contains 64-bit binaries (released with prebuilt
                          64-bit Ubuntu 12.04.1 binaries)
- /main.cpp             - Basic test app that compiles CP210xManufacturing
                          source w/o using CP210xManufacturing.so
- /Makefile             - CP210xManufacturing.so.1.0 makefile
- /Makefile-main-linux  - main makefile
- /FAQ.txt              - Frequently asked questions regarding building and
                          using the CP210xManufacturing library.
- /ReadMe.txt           - Explains how to build and use the CP210xManufacturing
                          library
- /SharedObjectLibraryUsage.txt - Explains how to use CP210xManufacturing.so
- /SiliconLabs.rules    - udev rules file for Silicon Labs USB devices
                          (VID = 0x10c4)

Dependencies
------------

1. libusb-1.0

On Ubuntu:

  $ sudo apt-get install libusb-1.0-0-dev

2. g++

On Ubuntu:

  $ sudo apt-get install g++

Build CP210xManufacturing
-------------------------

Build an example console app, main, that uses the CP210xManufacturing source files to
communicate with a CP210x:

  $ make -f Makefile-main-linux

  Builds /Release/Linux/main

Build CP210xManufacturing.so.1.0, a shared object library that exports the
CP210xManufacturing library API

  $ make

  Builds /Release/Linux/libslabhidtouart.so.1.0

  See SharedObjectLibraryUsage.txt for more information on how to use a shared
  object library.

Configure Your System to Allow libusb to Access Your Device
-----------------------------------------------------------

CP210xManufacturing uses libusb-1.0 to communicate with the CP210x devices over
USB. In order for CP210xManufacturing to recognize your device, you must first
copy a udev file to /etc/udev/rules.d/. This udev file will automatically
enable read and write permissions for a device with the specified USB vendor
ID. Unplug and replug your device after copying a udev file for the permissions
to take effect. On some systems, it may be necessary to reboot the machine
before the new rules are applied.

CP2101 (VID: 0x10c4 PID: 0xea60):
- Copy SiliconLabs.rules to /etc/udev/rules.d/

Other:
- Modify an existing udev rules file for the required vendor ID

  SUBSYSTEM=="usb", ATTRS{idVendor}=="10c4", MODE="0666"
  SUBSYSTEM=="usb_device", ATTRS{idVendor}=="10c4", MODE="0666"
  
  - or - (optionally add a required product ID)

  SUBSYSTEM=="usb", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea80", MODE="0666"
  SUBSYSTEM=="usb_device", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea80", MODE="0666"
  
- SiliconLabs.rules does not specify the product ID and will therefore allow
  read/write access to all Silicon Labs USB devices with the Silicon Labs VID (0x10c4).
  The product ID is optional and will further restrict which devices the rules file
  will affect.

