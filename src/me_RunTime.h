// Run-time wallclock

/*
  Author: Martin Eden
  Last mod.: 2025-12-27
*/

/*
  Implementation uses timer 3 (TC2)
*/

#pragma once

#include <me_BaseTypes.h>

namespace me_RunTime
{
  void Init();
  void Start();
  void Stop();

  TUint_4 GetTime_Us();
}

/*
  2025-03-02
  2025-09-12 Removed Delay(), we have [me_Delays] now
  2025-09-23
  2025-09-24
  2025-10-23
  2025-11-28 Removed SetTime()
  2025-12-01 Added function to get period duration
  2025-12-10 Interface reduction, implementation delegation
*/
