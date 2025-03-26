// Run-time wallclock

/*
  Author: Martin Eden
  Last mod.: 2025-03-02
*/

#pragma once

#include <me_BaseTypes.h>
#include <me_Timestamp.h>

namespace me_RunTime
{
  void Setup();
  void Start();
  void Stop();

  me_Timestamp::TTimestamp GetTime();
  void SetTime(me_Timestamp::TTimestamp);

  void Delay(me_Timestamp::TTimestamp);

  namespace Freetown
  {
    TUint_2 GetMicros();
  }
}

/*
  2025-03-02
*/
