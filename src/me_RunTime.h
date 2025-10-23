// Run-time wallclock

/*
  Author: Martin Eden
  Last mod.: 2025-10-23
*/

/*
  Implementation uses timer 3 (TC2)
*/

#pragma once

#include <me_BaseTypes.h>
#include <me_Duration.h>

namespace me_RunTime
{
  void Init();
  void Start();
  void Stop();

  me_Duration::TDuration GetTime();
  me_Duration::TDuration GetTime_Precise();

  void SetTime(me_Duration::TDuration);

  namespace Freetown
  {
    TUint_2 CalcMicros(TUint_1 CounterValue, TUint_1 CounterLimit);
  }
}

/*
  2025-03-02
  2025-09-12 Removed Delay(), we have [me_Delays] now
  2025-09-23
  2025-09-24
  2025-10-23
*/
