// Run time tracking using hardware counter

/*
  Author: Martin Eden
  Last mod.: 2025-12-29
*/

/*
  Historically all time-tracking implementation was here.
  But I rewrote it recently (to almost the same code).
  Now this module scope is interface and handy defaults.
*/

#include <me_RunTime.h>

#include <me_BaseTypes.h>
#include <me_Clock.h>

using namespace me_RunTime;

/*
  Get time
*/
TUint_4 me_RunTime::GetTime_Us()
{
  return me_Clock::GetTime_Us();
}

/*
  Setup for time tracking
*/
void me_RunTime::Init()
{
  /*
    Tick duration

    Longer tick - rarer called update routine.
    Shorter tick - precision for time tracking.

    As general-purpose clock we want longest possible tick - 64 us.

    But currently [me_ModulatedSignalPlayer] relies on us and
    is not happy with 64 us jitter. So staying under 4 us for him.
  */
  const TUint_1 WishedTickDuration_Us = 2;

  me_Clock::Init(WishedTickDuration_Us);
  me_Clock::Start();
}

/*
  Start time tracking
*/
void me_RunTime::Start()
{
  me_Clock::Start();
}

/*
  Stop time tracking
*/
void me_RunTime::Stop()
{
  me_Clock::Stop();
}

/*
  2025 # # #
  2025-10-10 Switched to counter 3
  2025-10-19
  2025-10-28
  2025-12-01
*/
