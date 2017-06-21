# EmPOWER Base station simulator

The EmBASE software provides a simulated LTE base station which interacts with an EMPOWER controller through the relative agent. It is not an LTE stack emulator, but provides a working fake base station which respond to the action issued by a controller. The tools was created as a debugging utility for the EMPOWER LTE part and later it has evolved in a more complex and useful tool.

With EmBASE is possible to design, develop and test an application which uses EMPOWER controller (on the LTE side) without having to use a LTE cell. By doing this you can speed up your design and development process before testing the software on real devices.

### Compatibility
This software has been developed and tested for Linux.

### Pre-requisites
In order to successfully build EMAge you need:
* empower-enb-agent and relative pre-requisites installed in your system.
* Ncurses library for simple user interfaces (libncurses5-dev).

### Build instructions
Once you installed all the necessary pre-requisites in your system, is enough to invoke the `make` directly from the root folder. As a default action the project will fall down into the right subdirectory and will compile the application.

### Run the simulator
There are limited features actually in the project. Running the compiled application will print the help screen on the console, which will guide you through the required arguments. For a quick start is enough to run the command `embase --id 1` to start a simulator instance with agent id 1 and Physical Cell Id 0; to enrich the simulation provide additional arguments which better describe your cell.

The simulator just needs a console to run and draw the interface, so it can be easily used though an SSH shell.

### The Team
Here a list of the maintainers of such project:
* Rausch Kewin, <krausch@fbk.eu>

### License
Code is released under the Apache License, Version 2.0.
