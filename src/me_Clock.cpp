// "Clock" interface implementation

/*
  Author: Martin Eden
  Last mod.: 2026-01-10
*/

/*
  Current implementation is fixed to counter 3

  We wish to be able to work with any counter.
  But that will require generic interface for counters.
  That's not job of this module.
*/

#include <me_Clock.h>

#include <me_BaseTypes.h>
#include <me_Counters.h>
#include <me_HardwareClockScaling.h>
#include <me_Interrupts.h>

using namespace me_Clock;

/*
  We will install counter's interrupt routine to advance time

  It needs to know current time and time advancement.
*/
static volatile TUint_4 ElapsedTime_Us = 0;
static TUint_4 TimeAdvancement_Us = 0;

// [Internal] [Interrupt handler] Called when counter reaches limit
static void OnPeriodEnd_I()
{
  ElapsedTime_Us = ElapsedTime_Us + TimeAdvancement_Us;
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
        GetPrescaleForTickDuration_Specs(
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

  TimeAdvancement_Us = me_HardwareClockScaling::MicrosFromHwDuration(HwDur);
  // )

  // ( Setup counter
  Counter.SetAlgorithm(me_Counters::TAlgorithm_Counter3::Count_To2Pow8);
  me_Interrupts::On_Counter3_ReachedHardLimit = OnPeriodEnd_I;
  Counter.Status->Done = true; // cleared by one
  Counter.Interrupts->OnDone = true;
  // )

  // ( Set time to zero
  *Counter.Current = 0;
  ElapsedTime_Us = 0;
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
TUint_4 me_Clock::GetTime_Us()
{
  /*
    Time consists of two parts: big-endian part from variable and
    little-endian value from hardware counter.

    To get time we need to get little-endian and combine it with main
    record. At the moment of capture we need these parts to be
    consistent. Because hardware timer advances every system clock tick
    and sets interrupt flags, we want to briefly pause it while
    capturing two pieces.

    Side effect is that every call of this function "slows down"
    tracked time increasing gap to real time. Do not call in as delay
    in empty loops, actual delay will be longer.

    Implementation does not bother to restore on/off state
    of counter. It is stopped and then started in this function.
  */
  TUint_4 RoughTime_Us;
  TUint_4 FinePart_Us;
  me_HardwareClockScaling::THardwareDuration HwDur;

  me_Clock::Stop();

  // If needed, execute interrupt handler to advance big-endian
  if (Counter.Status->Done)
  {
    OnPeriodEnd_I();
    Counter.Status->Done = true; // cleared by one
  }

  RoughTime_Us = ElapsedTime_Us;

  HwDur.Scale_BaseOne = *Counter.Current;

  me_Clock::Start();

  // At this point we have two parts of time from frozen moment

  if (
    !me_Counters::Prescale_SwFromHw_Counter3(
      &HwDur.Prescale_PowOfTwo, SpeedValue
    )
  )
    return RoughTime_Us;

  FinePart_Us = me_HardwareClockScaling::MicrosFromHwDuration(HwDur);

  return RoughTime_Us + FinePart_Us;
}

/*
  Get resolution in microseconds

  For sub-microsecond resolution we return 0.
*/
TUint_2 me_Clock::GetPrecision_Us()
{
  me_HardwareClockScaling::THardwareDuration HwDur;

  HwDur.Scale_BaseOne = 0;
  me_Counters::Prescale_SwFromHw_Counter3(&HwDur.Prescale_PowOfTwo, SpeedValue);

  return (TUint_2) me_HardwareClockScaling::MicrosFromHwDuration(HwDur);
}

/*
  2025-12-08
  2025-12-09
*/
