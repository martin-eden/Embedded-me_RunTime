// Run-time wallclock

/*
  Author: Martin Eden
  Last mod.: 2025-09-20
*/

/*
  Implementation occupies timer 2 (16-bit counter)
*/

#pragma once

#include <me_BaseTypes.h>
#include <me_Duration.h>

namespace me_RunTime
{
  void Start();
  void Stop();

  me_Duration::TDuration GetTime();
  void SetTime(me_Duration::TDuration);

  namespace Freetown
  {
    TUint_2 GetMicros();
  }
}

// Time update routine (counter 2 mark A event)
extern "C" void __vector_11() __attribute__((signal, used));

/*
  2025-03-02
  2025-09-12 Removed Delay(), we have [me_Delays] now
*/
