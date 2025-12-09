// "Clock" time abstraction

/*
  Author: Martin Eden
  Last mod.: 2025-12-08
*/

/*
  Taxonomy

    Hardware unit

      Hardware magic that microcontroller supplies.

    Counter

      Hardware unit specialization. Uses clock scaling. Can toggle pins.
      Raises interrupts.

    Clock

      Counter specialization. Uses durations. Provides elapsed time.
*/

#pragma once

#include <me_BaseTypes.h>
#include <me_Duration.h>

namespace me_Clock
{
  /*
    Clock - get current time

    Point of this interface is use "Duration" time record.

    Also we provide our internal and external precision.
  */
  TBool Init(TUint_2 WishedPrecision_Us);
  void Start();
  void Stop();

  me_Duration::TDuration GetTime();
  me_Duration::TDuration GetTickDuration();
  TUint_2 GetPrecision_Us();
}

/*
  2025-12-08
*/
