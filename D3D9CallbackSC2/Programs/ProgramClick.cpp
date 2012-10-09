#include "Main.h"

void ProgramClick::InternalInit()
{
    CurState = ProgClickClickScreen;
    WaitClick = 0;
    PreStatement.FreeMemory();
    PreKey = 0;
}

void ProgramClick::Init(const Vec2i &_ScreenCoord)
{
    InternalInit();
    ScreenCoord = _ScreenCoord;
    RightClick = false;
    Name = String("Click <") + ScreenCoord.CommaSeparatedString() + String(">");
}

void ProgramClick::Init(const Vec2i &_ScreenCoord, bool _RightClick)
{
    InternalInit();
    ScreenCoord = _ScreenCoord;
    RightClick = _RightClick;
    if(RightClick)
    {
        Name = String("Right Click <") + ScreenCoord.CommaSeparatedString() + String(">");
    }
    else
    {
        Name = String("Click <") + ScreenCoord.CommaSeparatedString() + String(">");
    }
}

void ProgramClick::Init(const String &_PreStatement, const Vec2i &_ScreenCoord)
{
    InternalInit();
    ScreenCoord = _ScreenCoord;
    RightClick = false;
    PreStatement = _PreStatement;
    Name = String("Click <") + ScreenCoord.CommaSeparatedString() + String("PreStatement: ") + PreStatement + String(">");
}

void ProgramClick::Init(WORD _PreKey, const Vec2i &_ScreenCoord)
{
    InternalInit();
    ScreenCoord = _ScreenCoord;
    RightClick = false;
    PreKey = _PreKey;
    Name = String("Click <") + ScreenCoord.CommaSeparatedString() + String("PreKey: ") + String(PreKey) + String(">");
}

ProgramResultType ProgramClick::ExecuteStep()
{
    if(CurState == ProgClickClickScreen)
    {
        if(PreStatement.Length() > 0)
        {
            g_Context->Managers.KeyboardMouse.SendString(PreStatement);
        }
        if(PreKey != 0)
        {
            g_Context->Managers.KeyboardMouse.SendKey(PreKey);
        }
        if(RightClick)
        {
            LogThreadEvent(String("Right Click: ") + ScreenCoord.CommaSeparatedString(), LogStep);
            g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonRight, ModifierNone);
        }
        else
        {
            LogThreadEvent(String("Left Click: ") + ScreenCoord.CommaSeparatedString(), LogStep);
            g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        }
        CurState = ProgClickWait;
        return ProgramResultStillExecuting;
    }
    if(CurState == ProgClickWait)
    {
        if(WaitClick)
        {
            LogThreadEvent("Click wait.", LogStep);
            WaitClick--;
            return ProgramResultStillExecuting;
        }
        else
        {
            LogThreadEvent("Click done.", LogDone);
            return ProgramResultSuccess;
        }
    }
    HANDLE_CRITICAL_FAILURE;
}
