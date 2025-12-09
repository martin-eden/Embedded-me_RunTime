// Run time tracking using hardware counter

/*
  Author: Martin Eden
  Last mod.: 2025-12-09
*/

/*
  This implementation uses hardware counter 3 (named TC2 in datasheet).

  Main reason for this are interrupts priorities: counter 3 interrupts
  are served before other counters interrupts. In interrupt handler
  you want to have correct time.

  Also it can tick from external signal and stay alive in most sleep
  modes. And has best range of system clock scaling. Which makes me
  think it's suitable for run time tracking and designed for event
  planning.
*/

/*
  Arduino framework uses:

    * Counter 1 for time tracking

      See <wiring.c>. I don't know why. Ask David Mellis from 2005.

    * Counter 3 for tone() function

      See <Tone.cpp>.

  Our framework uses:

    * Counter 1 for wave generation. See [me_FrequencyGenerator]
    * Counter 2 for signal capture. See [me_DigitalSignalRecorder]
    * Counter 3 for time tracking. See [me_RunTime]
*/

/*
  Counter used here is simple thing

  It advances from from 0 to 255 every "tick". "Tick" is scaled
  system clock. After 255 it goes to 0 and raises "overflow"
  interrupt flag.
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

#include <me_Clock.h>

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
  Get time

  Current period part is not processed.

  So less precision but less overhead and no side effects.
*/
me_Duration::TDuration me_RunTime::GetTime()
{
  return me_Duration::GetVolatile(RunTime);
}

/*
  Get precise time

  Current period part is processed.

  So maximum possible precision but overhead and side effects.

  Side effects is that every call of this function stops
  time tracking for some (small) time. So it increases
  discrepancy between "real" and tracked time.
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
    Sets counter mode to "count till max" and "overflow" event handler.

    Resets counter to zero.
  */

  me_HardwareClockScaling::TClockScaleSetting Spec;

  const TUint_1 SuitableTickDuration_Us = 50;

  me_Counters::TCounter3 Rtc;
  me_HardwareClockScaling::TClockScale ClockScale;

  me_HardwareClockScaling::PrescaleFromTickDuration_Specs(
    &Prescale_PowOfTwo,
    SuitableTickDuration_Us,
    me_HardwareClockScaling::AtMega328::GetSpecs_Counter3()
  );

  me_Counters::Prescale_HwFromSw_Counter3(&PrescaleValue, Prescale_PowOfTwo);

  Spec.Prescale_PowOfTwo = Prescale_PowOfTwo;
  Spec.CounterNumBits = 8;

  me_HardwareClockScaling::SetMaxCounterValue(&ClockScale, Spec);

  TimeAdvancement =
    me_TimerTools::CounterToDuration(ClockScale.CounterLimit, Prescale_PowOfTwo);

  Stop();

  Rtc.SetAlgorithm(me_Counters::TAlgorithm_Counter3::Count_To2Pow8);

  me_Interrupts::On_Counter3_ReachedHardLimit = OnPeriodEnd_I;
  Rtc.Interrupts->OnDone = true;
  Rtc.Status->Done = true; // cleared by one

  *Rtc.Current = 0;

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

/*
  Return internal period duration
*/
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
