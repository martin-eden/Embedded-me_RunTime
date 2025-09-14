// Delay() demo

/*
  Author: Martin Eden
  Last mod.: 2025-09-14
*/

#include <me_RunTime.h>

#include <me_BaseTypes.h>
#include <me_Console.h>

#include <me_Timestamp.h>
#include <me_Delays.h>
#include <me_DebugPrints.h>

void PrintTimestamp(
  TAsciiz Annotation,
  me_Timestamp::TTimestamp Ts
)
{
  Console.Write(Annotation);
  Console.Write(" ");
  me_DebugPrints::PrintDuration(Ts);
  Console.EndLine();
}

void GetTimeTest()
{
  const me_Timestamp::TTimestamp EndTime = { 0, 24, 0, 0 };
  me_Timestamp::TTimestamp CurTime;

  PrintTimestamp("End time", EndTime);

  me_RunTime::Start();

  while (true)
  {
    CurTime = me_RunTime::GetTime();
    PrintTimestamp("Current time", CurTime);
    if (me_Timestamp::IsGreater(me_RunTime::GetTime(), EndTime))
      break;
    me_Delays::Delay_S(3);
  }

  Console.Print("GetTimeTest() done");
}

void setup()
{
  Console.Init();

  Console.Print("( [me_RunTime] test");
  GetTimeTest();
  Console.Print(") Done");
}

void loop()
{
}

/*
  2025-03
  2025-08-01
  2025-09-12
*/
