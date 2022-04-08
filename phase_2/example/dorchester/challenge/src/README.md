# Mongoose - Embedded Web Server / Embedded Networking Library

![](https://img.shields.io/badge/license-GPL_2-green.svg "License")

Mongoose is ideal for embedded environments. It has been designed
for connecting devices and bringing them online. On the market since 2004,
used by vast number of open source and
commercial products - it even runs on the International Space station!
Mongoose makes embedded network programming fast, robust, and easy.

- [Download Mongoose Source Code here](https://www.cesanta.com/download.html)

Connectivity is vital for software and embedded devices, but there are many pitfalls to consider when embedding a web server. 
This white paper breaks down on the top 9 things to avoid when embedding a web server.

- [Download “9 Things NOT to do when embedding a web server” white paper here](https://www.cesanta.com/whitepaper.html)

Looking for a complete IoT firmware solution?

Check out [Mongoose OS](https://mongoose-os.com) - open source embedded operating system for low-power connected microcontrollers. Secure, designed for Internet of Things, complete environment for prototyping, development and managing.

# Support
- [Study mongoose example code](https://github.com/cesanta/mongoose/tree/master/examples)
- [Read User Guide and API reference](https://cesanta.com/docs/overview/intro.html)
- [Support Forum - ask your technical questions here](https://community.mongoose-os.com/)
- [Commercial licensing and support available](https://www.cesanta.com/licensing.html)
- [Check our latest releases](https://github.com/cesanta/mongoose/releases)

# Features

* Cross-platform: works on Linux/UNIX, MacOS, QNX, eCos, Windows, Android,
  iPhone, FreeRTOS (TI CC3200, ESP8266), etc
* Supported hardware platforms: TI CC3200, TI MSP432, NRF52, STM32, PIC32, ESP8266, ESP32 and more
* Builtin protocols:
   - plain TCP, plain UDP, SSL/TLS (over TCP, one-way or two-way)
   - HTTP client, HTTP server
   - WebSocket client, WebSocket server
   - MQTT client, MQTT broker
   - CoAP client, CoAP server
   - DNS client, DNS server, async DNS resolver
* Single-threaded, asynchronous, non-blocking core with simple event-based API
* Native support for [PicoTCP embedded TCP/IP stack](http://www.picotcp.com),
  [LWIP embedded TCP/IP stack](https://en.wikipedia.org/wiki/LwIP)
* Tiny static and run-time footprint
* Source code is both ISO C and ISO C++ compliant
* Very easy to integrate: just copy
  [mongoose.c](https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.c) and
  [mongoose.h](https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.h)
  files to your build tree

# Licensing

Mongoose is released under Commercial and [GNU GPL v.2](http://www.gnu.org/licenses/old-licenses/gpl-2.0.html) open source licenses.

Commercial Projects: [Contact us for commercial license.](https://www.cesanta.com/contact.html)

# Dashboard Example

Mongoose is often used to implement device dashboards and real-time
data exchange over Websocket. Here is a dashboard example that illustrates
the functionality:

![](http://www.cesanta.com/images/dashboard.png)

[Developing a new product? Contact us today to discuss how Mongoose can help.](https://www.cesanta.com/contact.html)

# Contributions

Contributions are welcome! Please follow the guidelines below:

- Sign [Cesanta CLA](https://cesanta.com/cla.html) and send GitHub pull request
- When making pull requests, please make sure that it has only one commit,
 and imlements/fixes only one piece of functionality

# Looking for a pre-compiled Mongoose web server Windows or Mac binary?
- [Download pre-compiled Mongoose web server binary.](https://www.cesanta.com/binary.html)
