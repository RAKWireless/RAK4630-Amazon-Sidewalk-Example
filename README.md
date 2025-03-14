RAK Sidewalk SDK
===

In order to make things simple and to keep all developer on the same page,
this project serve as `west` manifest repo, to take advantage of west to
manage the set of repositories.


Please check out our [Sidewalk Quick Start Guide](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK4631/Sidewalk) as well.

## Getting started

Before getting started, make sure you have a proper nRF Connect SDK development environment.
Follow the official
[Getting started guide](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/getting_started.html).

Or using `Docker` to build the application. To build the `ncs-builder` docker image
run the `build-docker-image` in the docker folder.
```
./docker/build-docker-image
```

### Initialization
The first step is to initialize the workspace folder. Run the following command:
```shell
west init -m https://github.com/RAKWireless/this.git rak-sid-workspace
cd rak-sid-workspace
west update
```

### Patch
To patch sidewalk subsys for RAK4631, run the following command:
```shell
cd rak-sid
west patch -a
```

### Build
To build the application, run the following command:

#### RAK4631 EVB
- Sidewalk sensor_monitoring demo
```
west build -p -b rak4631 app/rak4631_rak1901_demo/ -- -DOVERLAY_CONFIG="lora.conf"
```

### Build with docker(`ncs-builder`)
To build application via docker, run the following command:
```
./docker/dock-run west build -p -b rak4631 app/rak4631_rak1901_demo/ -- -DOVERLAY_CONFIG="lora.conf"
```


