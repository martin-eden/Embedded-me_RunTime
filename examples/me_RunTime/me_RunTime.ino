// [me_RunTime] test

/*
  Author: Martin Eden
  Last mod.: 2025-12-10
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

  CurTime = me_RunTime::GetTime();
  PrintTimestamp("Time after Start()", CurTime);

  for (RunNumber = 1; RunNumber <= NumRuns; ++RunNumber)
  {
    StartTime = me_RunTime::GetTime();

    EndTime = StartTime;
    me_Duration::WrappedAdd(&EndTime, Delay);

    do
    {
      me_Delays::Delay_Ms(1);
      CurTime = me_RunTime::GetTime();
    } while (me_Duration::IsLess(CurTime, EndTime));

    PrintTimestamp("CurTime", CurTime);
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
