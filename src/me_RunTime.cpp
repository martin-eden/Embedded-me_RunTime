// Run time tracking using hardware counter

/*
  Author: Martin Eden
  Last mod.: 2025-11-30
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
#include <me_TimerTools.h>

#include <avr/common.h> // SREG
#include <avr/interrupt.h> // cli()

using namespace me_RunTime;

static volatile me_Duration::TDuration RunTime = {};
static TUint_1 Prescale_PowOfTwo;
static TUint_1 PrescaleValue;
static me_Duration::TDuration TimeAdvancement = {};

/*
  [Interrupt handler] Advance tracked time
*/
void OnPeriodEnd_I()
{
  me_Duration::TDuration CurTime;

  CurTime = me_Duration::GetVolatile(RunTime);
  me_Duration::WrappedAdd(&CurTime, TimeAdvancement);
  me_Duration::SetVolatile(RunTime, CurTime);
}

/*
  Get time as duration record

  Microseconds part is not filled, so precision is one millisecond.
*/
me_Duration::TDuration me_RunTime::GetTime()
{
  return me_Duration::GetVolatile(RunTime);
}

/*
  Get time with microsecond precision

  Each use of this functions increases gap between real time and
  tracked time.
*/
me_Duration::TDuration me_RunTime::GetTime_Precise()
{
  me_Counters::TCounter3 Rtc;
  me_Duration::TDuration RoughTime;
  TUint_2 Count;
  TUint_1 PrevSreg;
  me_Duration::TDuration Result;

  PrevSreg = SREG;
  cli();

  Stop();

  if (Rtc.Status->Done)
  {
    OnPeriodEnd_I();
    Rtc.Status->Done = true; // cleared by one
  }

  RoughTime = GetTime();

  Count = *Rtc.Current;

  Start();

  SREG = PrevSreg;

  // At this point we have two parts of time from frozen moment

  Result = RoughTime;
  me_Duration::WrappedAdd(
    &Result,
    me_TimerTools::CounterToDuration(Count, Prescale_PowOfTwo)
  );

  return Result;
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

  const TUint_1 SuitableTickDuration_Us = 1;

  me_Counters::TCounter3 Rtc;
  me_HardwareClockScaling::TClockScale ClockScale;

  me_HardwareClockScaling::PrescaleFromTickDuration_Specs(
    &Prescale_PowOfTwo,
    SuitableTickDuration_Us,
    me_HardwareClockScaling::AtMega328::GetSpecs_Counter3()
  );

  me_Counters::GetPrescaleConst_Counter3(&PrescaleValue, Prescale_PowOfTwo);

  Spec.Prescale_PowOfTwo = Prescale_PowOfTwo;
  Spec.CounterNumBits = 8;

  me_HardwareClockScaling::SetMaxCounterValue(&ClockScale, Spec);

  TimeAdvancement =
    me_TimerTools::CounterToDuration(ClockScale.CounterLimit, Prescale_PowOfTwo);

  Stop();

  Rtc.SetAlgorithm(me_Counters::TAlgorithm_Counter3::Count_To2Pow8);

  *Rtc.Current = 0;
  me_Interrupts::On_Counter3_ReachedHardLimit = OnPeriodEnd_I;
  Rtc.Interrupts->OnDone = true;
  Rtc.Status->Done = true; // cleared by one

  SetVolatile(RunTime, {});
}

/*
  Start time tracking
*/
void me_RunTime::Start()
{
  // Connects counter to clock source

  me_Counters::TCounter3 Rtc;

  Rtc.Control->Speed = PrescaleValue;
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

me_Duration::TDuration me_RunTime::GetPeriodDuration()
{
  return TimeAdvancement;
}

/*
  2025 # # #
  2025-10-10 Switched to counter 3
  2025-10-19
  2025-10-28
  2025-12-01
*/
