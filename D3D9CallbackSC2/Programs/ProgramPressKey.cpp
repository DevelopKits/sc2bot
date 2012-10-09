#include "Main.h"

void ProgramPressKey::Init(WORD _Key, bool _Shift, bool _Ctrl)
{
    CurState = PressKeyPressKey;
    Type = PressKeyNormal;

    WaitKey = 0;
    Key = _Key;
    Shift = _Shift;
    Ctrl = _Ctrl;
    Name = String("PressKey <") + String(Key);
    if(Shift)
    {
        Name = Name + " Shift";
    }
    if(Ctrl)
    {
        Name = Name + " Ctrl";
    }
    Name = Name + String(">");
}

void ProgramPressKey::Init(const Vector<WORD> &_Keys)
{
    CurState = PressKeyPressKey;
    Type = PressKeyMultiple;

    WaitKey = 0;
    Keys = _Keys;
    Name = String("PressKey <");
    for(UINT i = 0; i < Keys.Length(); i++)
    {
        Name = Name + " " + String(Keys[i]) + " ";
    }
    Name = Name + String(">");
}

ProgramResultType ProgramPressKey::ExecuteStep()
{
    if(CurState == PressKeyPressKey)
    {
        LogThreadEvent("Key pressed", LogStep);
        if(Type == PressKeyMultiple)
        {
            for(UINT i = 0; i < Keys.Length(); i++)
            {
                g_Context->Managers.KeyboardMouse.SendKey(Keys[i]);
            }
        }
        else if(Shift)
        {
            g_Context->Managers.KeyboardMouse.SendShiftKey(Key);
        }
        else if(Ctrl)
        {
            g_Context->Managers.KeyboardMouse.SendCtrlKey(Key);
        }
        else
        {
            g_Context->Managers.KeyboardMouse.SendKey(Key);
        }
        CurState = PressKeyWait;
        return ProgramResultStillExecuting;
    }
    if(CurState == PressKeyWait)
    {
        if(WaitKey)
        {
            LogThreadEvent("Key wait", LogStep);
            WaitKey--;
            return ProgramResultStillExecuting;
        }
        else
        {
            LogThreadEvent("Key done", LogDone);
            return ProgramResultSuccess;
        }
    }
    HANDLE_CRITICAL_FAILURE;
}
