// Run time tracking using hardware counter

/*
  Author: Martin Eden
  Last mod.: 2025-03-26
*/

/*
  We're using "timestamp" record for time keeping. This record
  is only accessed by GetTime() and SetTime().

  We're setting hardware counter to emit interrupt every millisecond.
  At interrupt handler we're advancing run time.

  For special case of fine tracking milliseconds, we're providing
  GetMicros(). There is trade-off between overhead for time-keeping
  and time resolution. GetMicros() at current implementation
  has granularity of half us.
*/

#include <me_RunTime.h>

#include <me_BaseTypes.h>
#include <me_Counters.h>
#include <me_Timestamp.h>

#include <Arduino.h>

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
  Updating timestamp on new millisecond

  Just adds 1 ms to timestamp.
*/
void OnNextMs()
{
  using
    me_Timestamp::TTimestamp,
    me_Timestamp::Add;

  const TTimestamp Ms = { 0, 0, 1, 0 };

  TTimestamp CurTime = GetTime();

  Add(&CurTime, Ms);

  SetTime(CurTime);
}

// Interrupt 11 is for counter 2 mark A event. Expected to trigger every ms
extern "C" void __vector_11() __attribute__((signal, used));

void __vector_11()
{
  OnNextMs();
}

/*
  Setup counter for milliseconds tracking

  Sets counter mark A to 1 ms (for 16 MHz at slowdown by 8)
*/
void me_RunTime::Setup()
{
  const TUint_2 TicksPerMs = 2000;

  using
    me_Counters::TCounter2,
    me_Counters::TAlgorithm_Counter2;

  TCounter2 Rtc;

  Stop();

  Rtc.SetAlgorithm(TAlgorithm_Counter2::Count_ToMarkA);
  *Rtc.Current = 0;
  *Rtc.MarkA = TicksPerMs - 1;

  Start();
}

/*
  Start time tracking

  Sets counter speed and enables mark A interrupt.
*/
void me_RunTime::Start()
{
  using
    me_Counters::TCounter2,
    me_Counters::TDriveSource_Counter2;

  TCounter2 Rtc;
  Rtc.Control->DriveSource = (TUint_1) TDriveSource_Counter2::Internal_SlowBy2Pow3;
  Rtc.Interrupts->OnMarkA = true;
}

/*
  Stop time tracking

  Does opposite things to Start().
*/
void me_RunTime::Stop()
{
  using
    me_Counters::TCounter2,
    me_Counters::TDriveSource_Counter2;

  TCounter2 Rtc;
  Rtc.Control->DriveSource = (TUint_1) TDriveSource_Counter2::None;
  Rtc.Interrupts->OnMarkA = false;
}

/*
  Delay for given interval of time

  Actually it's derived function that should be in separate
  module but it's damn handy to be left in example and we
  don't want to create separate module for it now.
*/
void me_RunTime::Delay(
  me_Timestamp::TTimestamp DeltaTs
)
{
  using
    me_Timestamp::TTimestamp,
    me_Timestamp::Add,
    me_Timestamp::Compare;

  TTimestamp EndTs = GetTime();
  TBool IsWrapped = !Add(&EndTs, DeltaTs);

  if (IsWrapped)
    while (Compare(GetTime(), EndTs) >= 0);

  while (Compare(GetTime(), EndTs) < 0);
}

/*
  Return microseconds part
*/
TUint_2 me_RunTime::Freetown::GetMicros()
{
  me_Counters::TCounter2 Rtc;

  return *Rtc.Current / 2; // (1)
  /*
    [1]: " / 2" - hardcoded const for 2 kHz. Actually it depends of
      <TicksPerMs> and should be "* 1000 / TicksPerMs". But damned
      GCC will first multiply ui2 by 1000 and then divide by 2000,
      trimming it.
  */
}

/*
  2025-03-02
*/
