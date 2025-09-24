// Run-time wallclock

/*
  Author: Martin Eden
  Last mod.: 2025-09-24
*/

/*
  Implementation occupies timer 2 (16-bit counter)
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
  void SetTime(me_Duration::TDuration);

  namespace Freetown
  {
    TUint_2 GetMicros();
  }
}

/*
  2025-03-02
  2025-09-12 Removed Delay(), we have [me_Delays] now
  2025-09-23
  2025-09-24
*/
