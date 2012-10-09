#include "Main.h"

void Program::LogThreadEvent(const String &Text, LogType Level, RGBColor Color)
{
    g_Context->Files.Thread << Name << ", " << GetLogTypeString(Level) <<  ": " << Text << endl;
    if(DisplayThreadEvents && g_Context->Controller.ConsoleEnabled())
    {
        g_Context->WriteConsole(Name + String(":") + Text, Color, OverlayPanelThreadEvents);
    }
}

void Program::LogThreadEvent(const String &Text, LogType Level)
{
    g_Context->Files.Thread << Name << ", " << GetLogTypeString(Level) <<  ": " << Text << endl;
    if(DisplayThreadEvents && g_Context->Controller.ConsoleEnabled())
    {
        RGBColor Color = RGBColor::Black;
        if(Level == LogStep)
        {
            if(!DisplayThreadSteps)
            {
                return;
            }
            Color = RGBColor::White;
        }
        if(Level == LogError)
        {
            Color = RGBColor::Red;
        }
        if(Level == LogDone)
        {
            Color = RGBColor::Green;
        }
        if(!DisplayBaseSearch && Name == "BaseSearch")
        {
            return;
        }
        g_Context->WriteConsole(Name + String(":") + Text, Color, OverlayPanelThreadEvents);
    }
}

void Program::InitalizeDelay(double DelayTime)
{
    FinishTime = GameTime() + DelayTime;
}

ProgramResultType Program::HandleDelay(const String &WaitName, UINT *StateVariable, UINT NewState)
{
    double CurTime = GameTime();
    if(CurTime > FinishTime)
    {
        LogThreadEvent(WaitName + String(" done"), LogStep);
        *StateVariable = NewState;
    }
    else
    {
        LogThreadEvent("Waiting on " + WaitName + ". Time left: " + String(FinishTime - CurTime), LogStep);
    }
    return ProgramResultStillExecuting;
}

void Program::InitalizeWait(double WaitTime)
{
    FinishTime = GameTime() + WaitTime;
}

ProgramResultType Program::HandleWait(const String &WaitName)
{
    double CurTime = GameTime();
    if(CurTime > FinishTime)
    {
        LogThreadEvent(WaitName + String(" failed"), LogError);
        return ProgramResultFail;
    }
    else
    {
        LogThreadEvent("Waiting on " + WaitName + ". Time left: " + String(FinishTime - CurTime), LogStep);
        return ProgramResultStillExecuting;
    }
}

ProgramResultType Program::HandleWait(UINT &WaitCount, const String &WaitName)
{
    WaitCount--;
    if(WaitCount == 0)
    {
        LogThreadEvent(WaitName + String(" failed"), LogError);
        return ProgramResultFail;
    }
    else
    {
        LogThreadEvent("Waiting on " + WaitName + ": " + String(WaitCount), LogStep);
        return ProgramResultStillExecuting;
    }
}
