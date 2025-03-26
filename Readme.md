# What

(2025-03)

System run-time clock implemented via interrupt from counter
of ATmega328/P.

We use fancy record to track time. But it's from other module.
This module sets 16-bit hardware counter to emit interrupt
every millisecond and attaches handler to that interrupt.

## Code

* [Interface][Interface]
* [Implementation][Implementation]
* [Examples][Examples]

## Requirements

  * arduino-cli
  * bash

## Install/remove

This is low-level library which depends only of `me_BaseTypes` and
`me_Timestamp`. However examples depend on my other libraries.
To save us time I recommend to clone [GetLibs][GetLibs] repo
and run it's code to get all my stuff.

## See also

* [My other embedded C++ libraries][Embedded]
* [My other repositories][Repos]

[Interface]: src/me_RunTime.h
[Implementation]: src/me_RunTime.cpp
[Examples]: examples/

[GetLibs]: https://github.com/martin-eden/Embedded-Framework-GetLibs

[Embedded]: https://github.com/martin-eden/Embedded_Crafts/tree/master/Parts
[Repos]: https://github.com/martin-eden/contents
