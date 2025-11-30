// [me_RunTime] test

/*
  Author: Martin Eden
  Last mod.: 2025-11-30
*/

#include <me_RunTime.h>

#include <me_BaseTypes.h>
#include <me_Console.h>

#include <me_Duration.h>
#include <me_DebugPrints.h>
#include <me_Pins.h>

#include <avr/common.h>
#include <avr/interrupt.h>

const TUint_1 TestPinNumber = 6;
me_Pins::TOutputPin TestPin;

void PrintTimestamp(
  TAsciiz Annotation,
  me_Duration::TDuration Ts
)
{
  Console.Write(Annotation);
  me_DebugPrints::PrintDuration(Ts);
  Console.EndLine();
}

void MeasureTime_Test()
{
  const TUint_1 NumRuns = 6;
  const me_Duration::TDuration Delay = { 0, 1, 0, 0 };

  me_Duration::TDuration StartTime;
  me_Duration::TDuration EndTime;
  me_Duration::TDuration CurTime;
  TUint_1 RunNumber;

  TestPin.Write(0);

  me_RunTime::Init();
  me_RunTime::Start();
  CurTime = me_RunTime::GetTime_Precise();
  PrintTimestamp("Time after Start()", CurTime);

  for (RunNumber = 1; RunNumber <= NumRuns; ++RunNumber)
  {
    StartTime = me_RunTime::GetTime();

    EndTime = StartTime;
    me_Duration::WrappedAdd(&EndTime, Delay);

    TestPin.Write(1);
    do
    {
      CurTime = me_RunTime::GetTime();
    } while (me_Duration::IsLess(CurTime, EndTime));
    TestPin.Write(0);

    PrintTimestamp("CurTime", CurTime);
    Console.Print("");
  }

  me_RunTime::Stop();
}

void DetectLargeDelays_InfTest()
{
  /*
    Check that current time is not jumping forward more than
    expected.

    We're using GetTime_Precise() here. In almost empty loop.
    It will significantly slow down our time tracking vs
    real-world time. But we don't care.

    We expect that counter runs at designed speed.

    Then loop

      do
      {
        CurTime = me_RunTime::GetTime_Precise();
      } while (me_Duration::IsLess(CurTime, EndTime));

    spends time on GetTime_Precise() and on IsLess().
    They are heavy functions in terms of CPU cycles.

    We're disabling interrupts for this cycle.

    <CurTime> after loop can be used to calculate overshoot
    over <EndTime>.

    Tuning is done by tweaking <AcceptedDiscrepancy> till
    overshoot is less than it. This way you can understand
    how much time GetTime_Precise() and IsLess() are taking.
  */

  Console.Print("Starting infinite test to detect large delays..");

  const me_Duration::TDuration Delay = { 0, 0, 8, 0 };
  const me_Duration::TDuration AcceptedDiscrepancy = { 0, 0, 0, 192 };
  const TUint_2 Heartbeat_S = 15;

  me_Duration::TDuration StartTime;
  me_Duration::TDuration EndTime;
  me_Duration::TDuration CurTime;
  me_Duration::TDuration Discrepancy;
  TUint_1 OrigSreg;
  TUint_2 LastHeartbeat_S = 1;

  me_RunTime::Init();
  me_RunTime::Start();

  TestPin.Write(0);

  while (true)
  {
    StartTime = me_RunTime::GetTime_Precise();

    EndTime = StartTime;
    me_Duration::WrappedAdd(&EndTime, Delay);

    TestPin.Write(1);

    OrigSreg = SREG;
    cli();

    do
    {
      CurTime = me_RunTime::GetTime_Precise();
    } while (me_Duration::IsLess(CurTime, EndTime));

    SREG = OrigSreg;

    TestPin.Write(0);

    if ((CurTime.S % Heartbeat_S == 0) && (CurTime.S != LastHeartbeat_S))
    {
      LastHeartbeat_S = CurTime.S;

      PrintTimestamp("Heartbeat", CurTime);
      Console.Print("");
    }

    Discrepancy = CurTime;
    me_Duration::CappedSub(&Discrepancy, EndTime);

    if (me_Duration::IsGreater(Discrepancy, AcceptedDiscrepancy))
    {
      PrintTimestamp("Time discrepancy", Discrepancy);
      PrintTimestamp("Current time", CurTime);
      Console.Print("");
    }
  }

  me_RunTime::Stop();
}

void setup()
{
  Console.Init();

  TestPin.Init(TestPinNumber);

  Console.Print("( [me_RunTime] test");
  Console.Indent();
  MeasureTime_Test();
  Console.Unindent();
  Console.Print(") Done");

  DetectLargeDelays_InfTest();
}

void loop()
{
}

/*
  2025 # # # #
  2025-10-23
  2025-10-29
*/
