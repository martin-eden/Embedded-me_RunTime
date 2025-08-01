// Delay() demo

/*
  Author: Martin Eden
  Last mod.: 2025-08-01
*/

#include <me_RunTime.h>
#include <me_Timestamp.h>

#include <me_BaseTypes.h>
#include <me_Uart.h>
#include <me_Console.h>

const TUint_1 DebugPin = 13;

void PrintTimestamp(
  me_Timestamp::TTimestamp Ts
)
{
  Console.Write("Timestamp (");
  Console.Print(Ts.KiloS);
  Console.Print(Ts.S);
  Console.Print(Ts.MilliS);
  Console.Print(Ts.MicroS);
  Console.Write(")");
  Console.EndLine();
}

void setup()
{
  me_Uart::Init(me_Uart::Speed_115k_Bps);

  me_RunTime::Setup();

  pinMode(DebugPin, OUTPUT);

  Console.Print("[me_RunTime] Delay() demo");
  Console.Print("Infinite loop that toggles LED.");
}

void loop()
{
  using
    me_RunTime::Delay;

  Delay({ 0, 1, 760, 50 });

  digitalWrite(DebugPin, !digitalRead(DebugPin));

  PrintTimestamp(me_RunTime::GetTime());
}

/*
  2025-03
  2025-08-01
*/
