// "Clock" time abstraction

/*
  Author: Martin Eden
  Last mod.: 2025-12-27
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

namespace me_Clock
{
  /*
    Clock - get current time

    Also we provide our internal precision.
  */
  TBool Init(TUint_2 WishedPrecision_Us);
  void Start();
  void Stop();

  TUint_4 GetTime_Us();

  TUint_2 GetPrecision_Us();
}

/*
  2025-12-08
  2025-12-10
  2025-12-27
*/
