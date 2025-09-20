// Delay() demo

/*
  Author: Martin Eden
  Last mod.: 2025-09-20
*/

#include <me_RunTime.h>

#include <me_BaseTypes.h>
#include <me_Console.h>

#include <me_Duration.h>
#include <me_Delays.h>
#include <me_DebugPrints.h>

void PrintTimestamp(
  TAsciiz Annotation,
  me_Duration::TDuration Ts
)
{
  Console.Write(Annotation);
  me_DebugPrints::PrintDuration(Ts);
  Console.EndLine();
}

void GetTimeTest()
{
  const me_Duration::TDuration EndTime = { 0, 18, 0, 0 };
  me_Duration::TDuration CurTime;

  PrintTimestamp("End time", EndTime);

  me_RunTime::Start();

  while (true)
  {
    CurTime = me_RunTime::GetTime();
    PrintTimestamp("Current time", CurTime);
    if (me_Duration::IsGreater(me_RunTime::GetTime(), EndTime))
      break;
    me_Delays::Delay_S(3);
  }
}

void setup()
{
  Console.Init();

  Console.Print("( [me_RunTime] test");
  Console.Indent();
  GetTimeTest();
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
*/
