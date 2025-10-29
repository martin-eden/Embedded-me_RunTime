// Run time tracking using hardware counter

/*
  Author: Martin Eden
  Last mod.: 2025-10-28
*/

/*
  This implementation uses hardware counter 3 (named TC2 in datasheet).

  Main reason for this are interrupts priorities: counter 3 interrupts
  are served before counter 2 (and counter 1) interrupts. In interrupt
  handler you want to have correct time.

  Also it can tick from external signal and stay alive in most sleep
  modes. And has best range of system clock scaling. Which makes me
  think it's really designed for run time tracking.

  Arduino framework uses:

    * Counter 1 for time tracking

      See <wiring.c>. I don't know why. Ask David Mellis from 2005.

    * Counter 3 for tone() function

      See <Tone.cpp>.
*/

/*
  We're using "duration" record for time keeping. This record
  is only accessed by GetTime() and SetTime().

  We're setting hardware counter to emit interrupt every millisecond.
  At interrupt handler we're advancing run time.

  For special case of fine tracking milliseconds, we're providing
  GetMicros(). It has granularity of 4 us.

  Mind that hardware counter runs even when interrupts are disabled.
  So GetMicros() will advance if counter is not stopped.
*/

/*
  Counter is simple thing

  It advances from from 0 to "Mark A" every "tick". "Tick" is scaled
  system clock. When it reaches mark A we're getting hardware interrupt.

  We want to set mark A to such value that it takes exactly
  one milli-second to reach it.
*/

#include <me_RunTime.h>

#include <me_BaseTypes.h>
#include <me_Duration.h>
#include <me_Counters.h>
#include <me_Interrupts.h>
#include <me_HardwareClockScaling.h>

#include <avr/common.h> // SREG
#include <avr/interrupt.h> // cli()

using namespace me_RunTime;

volatile me_Duration::TDuration RunTime = me_Duration::Zero;
const TUint_4 TimerFreq_Hz = 1000;
const me_Duration::TDuration TimeAdvancement = { 0, 0, 1, 0 };

/*
  Get time as duration record

  Microseconds part is not filled, so precision is one millisecond.
*/
me_Duration::TDuration me_RunTime::GetTime()
{
  me_Duration::TDuration Result;
  TUint_1 PrevSreg;
  TBool NeedsAdvancement;
  me_Counters::TCounter3 Rtc;

  PrevSreg = SREG;
  cli();

  NeedsAdvancement = Rtc.Status->GotMarkA;

  Result.KiloS = RunTime.KiloS;
  Result.S = RunTime.S;
  Result.MilliS = RunTime.MilliS;
  Result.MicroS = 0;

  SREG = PrevSreg;

  if (NeedsAdvancement)
  {
    /*
      Damn, we have pending interrupt for our time advancement.

      Most likely we're called from another interrupt handler
      with higher priority.

      Our time needs to be advanced. But it is done in our handler.

      Here we'll fix copy.
    */
    me_Duration::Add(&Result, TimeAdvancement);
  }

  return Result;
}

/*
  Get time with microsecond precision

  Each use of this functions increases gap between real time and
  tracked time.
*/
me_Duration::TDuration me_RunTime::GetTime_Precise()
{
  me_Duration::TDuration Result;
  TUint_1 PrevSreg;
  TUint_1 CounterValue;
  TUint_1 CounterLimit;
  me_Counters::TCounter3 Rtc;

  PrevSreg = SREG;
  cli();

  Stop();

  Result = GetTime();
  CounterValue = *Rtc.Current;

  Start();

  SREG = PrevSreg;

  CounterLimit = *Rtc.MarkA;

  Result.MicroS = Freetown::CalcMicros(CounterValue, CounterLimit);

  return Result;
}

/*
  Set time as duration record

  Microseconds part is ignored.
*/
void me_RunTime::SetTime(
  me_Duration::TDuration Ts
)
{
  TUint_1 PrevSreg;

  PrevSreg = SREG;
  cli();

  RunTime.KiloS = Ts.KiloS;
  RunTime.S = Ts.S;
  RunTime.MilliS = Ts.MilliS;
  RunTime.MicroS = 0;

  SREG = PrevSreg;
}

/*
  [Interrupt handler] Advance tracked time
*/
void OnNextMs_I()
{
  me_Duration::TDuration CurTime;

  CurTime = GetTime();
  me_Duration::Add(&CurTime, TimeAdvancement);
  SetTime(CurTime);
}

/*
  Setup counter for time tracking
*/
void me_RunTime::Init()
{
  /*
    Sets counter mode, limit and "limit reached" interrupt.
    Sets current value to zero.
  */

  me_HardwareClockScaling::TClockScaleSetting Spec;
  me_HardwareClockScaling::TClockScale ClockScale;
  me_Counters::TCounter3 Rtc;

  Spec.Prescale_PowOfTwo = 6;
  Spec.CounterNumBits = 8;

  if (
    !me_HardwareClockScaling::CalculateClockScale_Spec(
      &ClockScale, TimerFreq_Hz, Spec
    )
  )
    return;

  Stop();

  Rtc.SetAlgorithm(me_Counters::TAlgorithm_Counter3::Count_ToMarkA);
  *Rtc.MarkA = ClockScale.CounterLimit;
  me_Interrupts::On_Counter3_ReachedMarkA = OnNextMs_I;

  *Rtc.Current = 0;

  Rtc.Interrupts->OnMarkA = true;
}

/*
  Start time tracking
*/
void me_RunTime::Start()
{
  // Connects counter to clock source

  me_Counters::TCounter3 Rtc;

  Rtc.Control->Speed = (TUint_1) me_Counters::TSpeed_Counter3::SlowBy2Pow6;
}

/*
  Stop time tracking
*/
void me_RunTime::Stop()
{
  // Disconnects counter from clock source

  me_Counters::TCounter3 Rtc;

  Rtc.Control->Speed = (TUint_1) me_Counters::TSpeed_Counter3::None;
}

/*
  Return microseconds part
*/
TUint_2 me_RunTime::Freetown::CalcMicros(
  TUint_1 CounterValue,
  TUint_1 CounterLimit
)
{
  /*
  TUint_4 CurrentMsPart;

  // Assert: Current <= Mark A

  CurrentMsPart = CounterValue;
  CurrentMsPart = CurrentMsPart * TimerFreq_Hz;
  CurrentMsPart = CurrentMsPart / ((TUint_4) CounterLimit + 1);

  return (TUint_2) CurrentMsPart;

  //*/
  return (TUint_4) TimerFreq_Hz * CounterValue / (CounterLimit + 1);
  // return (TUint_2) CounterValue << 2;
}

/*
  2025 # # #
  2025-10-10 Switched to counter 3
  2025-10-19
  2025-10-28
*/
