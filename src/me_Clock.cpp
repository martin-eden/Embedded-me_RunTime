// "Clock" interface implementation

/*
  Author: Martin Eden
  Last mod.: 2025-12-14
*/

/*
  Current implementation is fixed to counter 3

  We wish to be able to work with any counter.
  But that will require generic interface for counters.
  That's not job of this module.
*/

#include <me_Clock.h>

#include <me_BaseTypes.h>
#include <me_Duration.h>
#include <me_Counters.h>
#include <me_HardwareClockScaling.h>
#include <me_Interrupts.h>

using namespace me_Clock;

/*
  We will install counter's interrupt routine to advance time

  It needs to know current time and time advancement.
*/
static volatile me_Duration::TDuration ElapsedTime = {};
static me_Duration::TDuration TimeAdvancement = {};

static void OnPeriodEnd_I()
{
  me_Duration::TDuration CurTime;

  CurTime = me_Duration::GetVolatile(ElapsedTime);
  me_Duration::WrappedAdd(&CurTime, TimeAdvancement);
  me_Duration::SetVolatile(ElapsedTime, CurTime);
}

// Counter's hardware interface. Global here for less variable declarations.
static me_Counters::TCounter3 Counter;

/*
  Calculated speed value to resume clock

  We're storing hardware-specific integer (three bits actually)
  to resume counting as fast as possible.
*/
static TUint_1 SpeedValue;

/*
  Setup clock with wished precision
*/
TBool me_Clock::Init(
  TUint_2 WishedPrecision_Us
)
{
  /*
    Calculate and export time advancement and speed setting.
    Setup counter for our needs. Set time to zero.
  */

  TUint_1 Prescale_PowOfTwo;
  me_HardwareClockScaling::THardwareDuration HwDur;

  // ( Calculate speed value
  if
    (
      !
        me_HardwareClockScaling::
        PrescaleFromTickDuration_Specs(
          &Prescale_PowOfTwo,
          WishedPrecision_Us,
          me_HardwareClockScaling::AtMega328::GetSpecs_Counter3()
        )
    )
    return false;

  if
    (
      !
        me_Counters::
        Prescale_HwFromSw_Counter3(&SpeedValue, Prescale_PowOfTwo)
    )
    return false;
  // )

  // ( Calculate time advancement
  HwDur.Prescale_PowOfTwo = Prescale_PowOfTwo;
  HwDur.Scale_BaseOne = TUint_1_Max;

  TimeAdvancement = me_HardwareClockScaling::HwToSwDuration(HwDur);
  // )

  // ( Setup counter
  Counter.SetAlgorithm(me_Counters::TAlgorithm_Counter3::Count_To2Pow8);
  me_Interrupts::On_Counter3_ReachedHardLimit = OnPeriodEnd_I;
  Counter.Status->Done = true; // cleared by one
  Counter.Interrupts->OnDone = true;
  // )

  // ( Set time to zero
  *Counter.Current = 0;
  me_Duration::SetVolatile(ElapsedTime, {});
  // )

  return true;
}

/*
  Start (resume) time tracking
*/
void me_Clock::Start()
{
  Counter.Control->Speed = SpeedValue;
}

/*
  Stop (pause) time tracking
*/
void me_Clock::Stop()
{
  Counter.Control->Speed = (TUint_1) me_Counters::TSpeed_Counter3::None;
}

/*
  Return elapsed time
*/
me_Duration::TDuration me_Clock::GetTime()
{
  /*
    Time consists of two parts: big-endian TDuration record and
    little-endian Current TUint_2 value from counter.

    To get time we need to get Current and combine it with TDuration.
    At the moment of capture we need these parts be consistent.
    Because hardware timer advances up to every system clock tick and
    setting interrupt flags, we want to briefly pause it when capturing
    two pieces.

    Side effect is that every call of this function "slows down"
    tracked time increasing gap to real time. Do not call in as delay
    in empty loops, actual delay will be longer.

    Implementation does not bother to restore on/off state
    of counter. It is stopped and then started in this function.
  */
  me_Duration::TDuration RoughTime;
  TUint_2 Count;
  TUint_1 Prescale_PowOfTwo;
  me_Duration::TDuration Result;

  me_Clock::Stop();

  if (Counter.Status->Done)
  {
    OnPeriodEnd_I();
    Counter.Status->Done = true; // cleared by one
  }

  RoughTime = me_Duration::GetVolatile(ElapsedTime);

  Count = *Counter.Current;

  me_Clock::Start();

  // At this point we have two parts of time from frozen moment

  if (!me_Counters::Prescale_SwFromHw_Counter3(&Prescale_PowOfTwo, SpeedValue))
    return RoughTime;

  Result = RoughTime;
  me_Duration::WrappedAdd(
    &Result,
    me_HardwareClockScaling::CounterToDuration(Count, Prescale_PowOfTwo)
  );

  return Result;
}

/*
  Get resolution in microseconds

  For sub-microsecond resolution we return 0.
*/
TUint_2 me_Clock::GetPrecision_Us()
{
  TUint_1 Prescale_PowOfTwo;
  me_Duration::TDuration Tick;
  TUint_4 Tick_Us;

  me_Counters::Prescale_SwFromHw_Counter3(&Prescale_PowOfTwo, SpeedValue);
  Tick = me_HardwareClockScaling::CounterToDuration(1, Prescale_PowOfTwo);

  me_Duration::DurationToMicros(&Tick_Us, Tick);

  return (TUint_2) Tick_Us;
}

/*
  2025-12-08
  2025-12-09
*/
