// [me_RunTime] test

/*
  Author: Martin Eden
  Last mod.: 2025-11-28
*/

#include <me_RunTime.h>

#include <me_BaseTypes.h>
#include <me_Console.h>

#include <me_Duration.h>
#include <me_DebugPrints.h>
#include <me_Pins.h>

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
  const TUint_1 NumRuns = 12;
  const TUint_1 TestPinNumber = 6;
  const me_Duration::TDuration Delay = { 0, 1, 0, 0 };

  me_Duration::TDuration StartTime;
  me_Duration::TDuration EndTime;
  me_Duration::TDuration CurTime;
  me_Pins::TOutputPin TestPin;
  TUint_1 RunNumber;

  me_RunTime::Init();
  me_RunTime::Start();
  TestPin.Init(TestPinNumber);
  TestPin.Write(0);

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

    PrintTimestamp("StartTime", StartTime);
    PrintTimestamp("EndTime", EndTime);
    PrintTimestamp("CurTime", CurTime);
    Console.Print("");
  }

  me_RunTime::Stop();
}

void DetectLargeDelays_InfTest()
{
  /*
    Check that current time is not jumping forward more than
    expected. (Time continuity test.)

    We're using GetTime_Precise() here. In empty loop.
    It will significantly slow down our time tracking vs
    real-world time. But we don't care.
  */

  Console.Print("Starting infinite test to detect large delays..");

  const me_Duration::TDuration Delay = { 0, 0, 1, 333 };
  const me_Duration::TDuration AcceptedDiscrepancy = { 0, 0, 0, 220 };
  const TUint_1 TestPinNumber = 6;

  me_Duration::TDuration StartTime;
  me_Duration::TDuration EndTime;
  me_Duration::TDuration CurTime;
  me_Duration::TDuration TimeDiscrepancy;
  me_Pins::TOutputPin TestPin;

  me_RunTime::Init();
  me_RunTime::Start();
  TestPin.Init(TestPinNumber);
  TestPin.Write(0);

  while (true)
  {
    StartTime = me_RunTime::GetTime_Precise();

    EndTime = StartTime;
    me_Duration::WrappedAdd(&EndTime, Delay);

    TestPin.Write(1);
    do
    {
      CurTime = me_RunTime::GetTime_Precise();
    } while (me_Duration::IsLess(CurTime, EndTime));
    TestPin.Write(0);

    TimeDiscrepancy = CurTime;
    me_Duration::CappedSub(&TimeDiscrepancy, EndTime);

    if (me_Duration::IsGreater(TimeDiscrepancy, AcceptedDiscrepancy))
    {
      PrintTimestamp("Time discrepancy", TimeDiscrepancy);
      PrintTimestamp("Start time", StartTime);
      PrintTimestamp("End time", EndTime);
      PrintTimestamp("Current time", CurTime);
      Console.Print("");
    }
  }

  me_RunTime::Stop();
}

void setup()
{
  Console.Init();

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
