// [me_RunTime] test

/*
  Author: Martin Eden
  Last mod.: 2025-12-27
*/

#include <me_RunTime.h>

#include <me_BaseTypes.h>
#include <me_Console.h>

#include <me_Duration.h>
#include <me_DebugPrints.h>
#include <me_Delays.h>

const TUint_1 TestPinNumber = 6;

void PrintTimestamp(
  TAsciiz Annotation,
  TUint_4 Timestamp_Us
)
{
  me_Duration::TDuration Timestamp;

  me_Duration::DurationFromMicros(&Timestamp, Timestamp_Us);
  me_DebugPrints::PrintDuration(Annotation, Timestamp);
  Console.EndLine();
}

void MeasureTime_Test()
{
  const TUint_1 NumRuns = 6;
  const TUint_4 Delay_Us = 1000000;

  TUint_4 StartTime_Us;
  TUint_4 EndTime_Us;
  TUint_4 CurTime_Us;
  TUint_1 RunNumber;

  CurTime_Us = me_RunTime::GetTime_Us();
  PrintTimestamp("Time after Start()", CurTime_Us);

  for (RunNumber = 1; RunNumber <= NumRuns; ++RunNumber)
  {
    StartTime_Us = me_RunTime::GetTime_Us();
    EndTime_Us = StartTime_Us + Delay_Us;

    while (CurTime_Us < EndTime_Us)
    {
      me_Delays::Delay_Ms(1);
      CurTime_Us = me_RunTime::GetTime_Us();
    }

    PrintTimestamp("Current time", CurTime_Us);
  }
}

void setup()
{
  Console.Init();
  me_RunTime::Init();
  me_RunTime::Start();

  Console.Print("( [me_RunTime] test");
  Console.Indent();

  MeasureTime_Test();

  Console.Unindent();
  Console.Print(") Done");
}

void loop()
{
}

/*
  2025 # # # #
  2025-10-23
  2025-10-29
  2025-12-10
*/
