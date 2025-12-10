// Run time tracking using hardware counter

/*
  Author: Martin Eden
  Last mod.: 2025-12-10
*/

/*
  Historically all time-tracking implementation was here.
  But I rewrote it recently (to almost the same code).
  Now this module scope is interface and handy defaults.
*/

#include <me_RunTime.h>

#include <me_BaseTypes.h>
#include <me_Duration.h>
#include <me_Clock.h>

using namespace me_RunTime;

/*
  Get time
*/
me_Duration::TDuration me_RunTime::GetTime()
{
  return me_Clock::GetTime();
}

/*
  Setup for time tracking
*/
void me_RunTime::Init()
{
  const TUint_1 WishedTickDuration_Us = 50;

  me_Clock::Init(WishedTickDuration_Us);
  me_RunTime::Start();
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
