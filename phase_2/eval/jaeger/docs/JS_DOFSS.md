# Introduction to the Jaeger System for Dev Ops Flight System Simulation (JS-DOFSS)

The Jaeger System Modernization Program is moving to microservices. The Jaeger System for Dev Ops Flight System Simulation (JS-DOFSS) is our first step towards modernizing the Jaeger System. The JS-DOFSS comprises a multi-device, RISC-V simulator capable of running multiple RISC-V microservices simultaneously (hereforth referred to as, "Devices"). The JS-DOFSS is a revolutionary design in Flight System design, providing unprecedented security at the system-bus level.

While many devices are planned, only a few are currently implemented. The devices currently available for demonstration are:

* The Access Control Device (ACD)
* The Maintenance Control Device (MCD)
* The RF Communications Device (RCD)
* The File Storage Device (FSD)

Additionally, the JS-DOFSS contains three separate busses. They are:

* The Privileged Bus.
* The Maintenance Bus.
* The RF Comms Bus.

A variety of external peripherals are available to devices within the JS-DOFSS. They are:

* The Bus Device
* The Debug Device
* The File Storage Device
* The Serial TCP Device
* The Test Device

Peripherals are memory-mapped into the the various devices. Because externally-driven interrupts are not yet implemented in JS-DOFSS, all devices run on regularly scheduled timer interrupts. Devices poll their periperhals for updated information, and interact with them according to their unique memory-mapped interfaces.

Finally, a variety of protocols are used between the JS-DOFSS systems. The, "Bus Protocol," is the standard protocol for inter-device communication. Each device normally presents its service via a unique protocol, which operates within the Bus Protocol. Those unique service protocols are:

* The Access Control Protocol.
* The RF Communications Protocol.
* The File Storage Protocol.
