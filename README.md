USBtin library for LPC1768
==========================

This is a port of USBtin firmware for LPC1768 using mbed SDK

The original source code can be found at http://www.fischl.de/usbtin/#firmwarepackage
You can find further information about USBtin on the project homepage: http://www.fischl.de/usbtin

Example
-------

```C++
#include "mbed.h"

#define USBTIN_CAN (0)
#include "USBtin.h"

int main(void) {
    Thread USBtin_thread(UT_thread);

    while (1);
}
```

Configuration
-------------

You will find in `USBtin.h` the defined constant for port configuration.
All those default constants can be overwritted.
