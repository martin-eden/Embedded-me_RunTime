// Delay() demo

/*
  Author: Martin Eden
  Last mod.: 2025-10-23
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
  const TUint_1 TestPinNumber = 7;
  const me_Duration::TDuration Delay = { 0, 0, 80, 0 };

  me_Duration::TDuration CurTime;
  me_Duration::TDuration PrevTime;
  me_Duration::TDuration TimeDiff;
  me_Pins::TOutputPin TestPin;
  TUint_1 RunNumber;

  me_RunTime::Init();
  me_RunTime::Start();
  TestPin.Init(TestPinNumber);
  TestPin.Write(0);

  PrevTime = me_Duration::Zero;

  for (RunNumber = 1; RunNumber <= NumRuns; ++RunNumber)
  {
    TestPin.Write(1);

    do
    {
      CurTime = me_RunTime::GetTime();
      // PrintTimestamp("CurTime", CurTime);
      TimeDiff = CurTime;
      me_Duration::Subtract(&TimeDiff, PrevTime);
    } while (me_Duration::IsLessOrEqual(TimeDiff, Delay));

    TestPin.Write(0);

    PrintTimestamp("Delta", TimeDiff);

    PrevTime = me_RunTime::GetTime();;
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
}

void loop()
{
}

/*
  2025-03
  2025-08-01
  2025-09-12
  2025-09-19
  2025-10-23
*/
