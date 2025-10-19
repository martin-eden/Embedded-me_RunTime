// Run time tracking using hardware counter

/*
  Author: Martin Eden
  Last mod.: 2025-10-19
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
  one milli-second to reach it (assuming 16 MHz main clock).
*/

/*
  Logically we should set tick scaling, mark A and start value
  in one routine. But here we're setting mark A and start value
  in Init() and tick scaling in Start(). Because we have Stop()
  which just sets tick scaling to infinity.
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

volatile me_Duration::TDuration RunTime = { 0, 0, 0, 0 };
const me_Duration::TDuration OneMs = { 0, 0, 1, 0 };

/*
  Get time as duration record
*/
me_Duration::TDuration me_RunTime::GetTime()
{
  me_Duration::TDuration Result;
  TUint_1 PrevSreg;
  me_Counters::TCounter3 Rtc;
  TBool NeedsAdvancement;

  Stop();

  PrevSreg = SREG;
  cli();

  NeedsAdvancement = Rtc.Status->GotMarkA;

  Result.KiloS = RunTime.KiloS;
  Result.S = RunTime.S;
  Result.MilliS = RunTime.MilliS;
  Result.MicroS = Freetown::GetMicros();

  SREG = PrevSreg;

  Start();

  if (NeedsAdvancement)
  {
    /*
      Damn, we have pending interrupt for our time advancement.

      Most likely we're called from another interrupt handler
      with higher priority.

      Our time needs to be advanced. But it is done in our handler.

      Here we'll fix copy.
    */
    me_Duration::Add(&Result, OneMs);
  }

  return Result;
}

/*
  Set time as duration record
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
  RunTime.MicroS = Ts.MicroS;

  SREG = PrevSreg;
}

/*
  [Interrupt handler] Advance duration by one millisecond
*/
void OnNextMs_I()
{
  me_Duration::TDuration CurTime;

  CurTime = GetTime();
  me_Duration::Add(&CurTime, OneMs);
  SetTime(CurTime);
}

/*
  Setup counter for time tracking

  Sets counter mode, limit and "limit reached" interrupt.
  Sets current value to zero.
*/
void me_RunTime::Init()
{
  const TUint_4 Freq_Hz = 1000;
  const me_Counters::TAlgorithm_Counter3 TickToMarkA =
    me_Counters::TAlgorithm_Counter3::Count_ToMarkA;

  me_HardwareClockScaling::TClockScalingOptions Spec;
  me_HardwareClockScaling::TClockScale ClockScale;
  me_Counters::TCounter3 Rtc;

  Spec.NumPrescalerValues = 1;
  Spec.Prescales_PowOfTwo[0] = 6;
  Spec.CounterNumBits = 8;

  me_HardwareClockScaling::CalculateClockScale(
    &ClockScale,
    Freq_Hz,
    Spec
  );

  Stop();

  Rtc.SetAlgorithm(TickToMarkA);
  *Rtc.MarkA = ClockScale.CounterLimit;
  *Rtc.Current = 0;

  me_Interrupts::On_Counter3_ReachedMarkA = OnNextMs_I;

  Rtc.Interrupts->OnMarkA = true;
}

/*
  Start time tracking

  Starts counter with fixed speed.
*/
void me_RunTime::Start()
{
  const TUint_1 SlowBy64 =
    (TUint_1) me_Counters::TSpeed_Counter3::SlowBy2Pow6;

  me_Counters::TCounter3 Rtc;

  Rtc.Control->Speed = SlowBy64;
}

/*
  Stop time tracking

  Disconnects counter from drive source.
*/
void me_RunTime::Stop()
{
  const TUint_1 Disabled =
    (TUint_1) me_Counters::TSpeed_Counter3::None;

  me_Counters::TCounter3 Rtc;

  Rtc.Control->Speed = Disabled;
}

/*
  Return microseconds part
*/
TUint_2 me_RunTime::Freetown::GetMicros()
{
  me_Counters::TCounter3 Rtc;
  TUint_4 CurrentMsPart;

  // Assert: Current <= Mark A

  CurrentMsPart = (TUint_4) *Rtc.Current;
  CurrentMsPart = CurrentMsPart * 1000;
  CurrentMsPart = CurrentMsPart / ((TUint_4) *Rtc.MarkA + 1);

  return (TUint_2) CurrentMsPart;
}

/*
  2025-03-02
  2025-09-12
  2025-09-24
  2025-10-10 Switched to counter 3
  2025-10-19
*/
