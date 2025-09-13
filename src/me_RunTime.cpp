// Run time tracking using hardware counter

/*
  Author: Martin Eden
  Last mod.: 2025-09-13
*/

/*
  We're using "timestamp" record for time keeping. This record
  is only accessed by GetTime() and SetTime().

  We're setting hardware counter to emit interrupt every millisecond.
  At interrupt handler we're advancing run time.

  For special case of fine tracking milliseconds, we're providing
  GetMicros(). It has granularity of 0.5 us.
*/

#include <me_RunTime.h>

#include <me_BaseTypes.h>
#include <me_Timestamp.h>
#include <me_Counters.h>

#include <avr/common.h> // SREG
#include <avr/interrupt.h> // cli()

using namespace me_RunTime;

volatile me_Timestamp::TTimestamp RunTime = { 0, 0, 0, 0 };

/*
  Get time as timestamp record
*/
me_Timestamp::TTimestamp me_RunTime::GetTime()
{
  me_Timestamp::TTimestamp Result;

  TUint_1 PrevSreg = SREG;

  cli();

  Result.KiloS = RunTime.KiloS;
  Result.S = RunTime.S;
  Result.MilliS = RunTime.MilliS;
  Result.MicroS = Freetown::GetMicros();

  SREG = PrevSreg;

  return Result;
}

/*
  Set time as timestamp record
*/
void me_RunTime::SetTime(
  me_Timestamp::TTimestamp Ts
)
{
  TUint_1 PrevSreg = SREG;

  cli();

  RunTime.KiloS = Ts.KiloS;
  RunTime.S = Ts.S;
  RunTime.MilliS = Ts.MilliS;
  RunTime.MicroS = Ts.MicroS;

  SREG = PrevSreg;
}

/*
  Advance timestamp by one millisecond
*/
void OnNextMs()
{
  const me_Timestamp::TTimestamp Ms = { 0, 0, 1, 0 };

  me_Timestamp::TTimestamp CurTime;

  CurTime = GetTime();
  me_Timestamp::Add(&CurTime, Ms);
  SetTime(CurTime);
}

void __vector_11()
{
  OnNextMs();
}

/*
  Start time tracking

  Sets counter speed and limit and "limit reached" interrupt.
  Starts counting from zero.
*/
void me_RunTime::Start()
{
  const me_Counters::TAlgorithm_Counter2 TickToValue =
    me_Counters::TAlgorithm_Counter2::Count_ToMarkA;
  const TUint_2 TicksPerMs = 2000;
  const TUint_1 SlowByEight =
    (TUint_1) me_Counters::TDriveSource_Counter2::Internal_SlowBy2Pow3;

  me_Counters::TCounter2 Rtc;

  Stop();

  Rtc.Control->DriveSource = SlowByEight;

  Rtc.SetAlgorithm(TickToValue);
  *Rtc.Current = 0;
  *Rtc.MarkA = TicksPerMs - 1;

  Rtc.Interrupts->OnMarkA = true;
}

/*
  Stop time tracking

  Does opposite things to Start().
*/
void me_RunTime::Stop()
{
  const TUint_1 Disabled =
    (TUint_1) me_Counters::TDriveSource_Counter2::None;

  me_Counters::TCounter2 Rtc;

  Rtc.Control->DriveSource = Disabled;
  Rtc.Interrupts->OnMarkA = false;
}

/*
  Return microseconds part
*/
TUint_2 me_RunTime::Freetown::GetMicros()
{
  me_Counters::TCounter2 Rtc;

  return *Rtc.Current / 2; // (1)
  /*
    [1]: " / 2" - should be "* 1000 / TicksPerMs". But damned
      GCC will first multiply ui2 by 1000 and then divide by 2000,
      trimming it.
  */
}

/*
  2025-03-02
  2025-09-12
*/
