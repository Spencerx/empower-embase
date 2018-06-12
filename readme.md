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
Running the compiled application will print the help screen on the console, which will guide you through the required arguments. For a quick start is enough to run the command `embase --id 1` to start a simulator instance with agent id 1 and Physical Cell Id 1; to enrich the simulation provide additional arguments which better describe your cell.

The simulator just needs a console to run and draw the interface, so it can be easily used though an SSH shell.

**Connect to remote controller:** An important option of the simulator is the possibility to specify a remote controller rather than the default, local one. In fact, if no options are specified, the simulator tries to attach to the address 127.0.0.1, on the port 2210. By specifying the options `--ctrl_addr <ip>` and `--ctrl_port <port>` with a custom IP address and port number, you will be able to instruct the simulator to attach to another EmPOWER controller. 

**Customizing cells:** It is possible to create up to 6 cells for a single eNB instance, and this is done by issuing the right argument during application launch. The syntax for such operation is `--cell <pci:DL_earfcn:UL_earfcn:DL_prbs:UL_prbs>`. As you can see you need to specify the Physical Cell Id (PCI), the DL frequency (EARFCN), the DL number of Physical Resource Blocks (PRBs), the Uplink EARFCN and the UL number of PRBs.

**Running multiple instances:** It is possible to run multiple instance of the simulator on the same machine by selecting a proper X2 interface port number during application launch. Default X2 interface for the simulator eNB is `9999`, but by using the command `--x2p <port>` you can actually force the simulator to switch on another one.

**Headless:** It is possible to run the simulator in headless mode, but such functionality now is limited. If you specify the `--hl` command as an argument, the interface will be supressed.

**Scenarios:** This feature allows to start the simulator in a known state without having to repeat all the configuration steps at startup. `--scenario <path>` option allow to specify a formatted text file containing all the necessary information. To save the initial state run the simulator and adds neighbor eNB and User Equipments. Then from UE interface (option F2), press 's' to save the current status into ./scenario.ems file. You can later load it or further modify the file as you wish to change the setup of the eNB.

### License
Code is released under the Apache License, Version 2.0.
