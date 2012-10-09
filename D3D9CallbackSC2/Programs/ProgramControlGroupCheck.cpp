#include "Main.h"

void ProgramControlGroupCheck::Init()
{
    _ControlGroupIndex = -1;
    Name = String("ControlGroupCheck");
}

ProgramResultType ProgramControlGroupCheck::ExecuteStep()
{
    if(_ControlGroupIndex != -1)
    {
        g_Context->Managers.ControlGroup.ControlGroups(_ControlGroupIndex).UpdateFromHUD();
    }
    _ControlGroupIndex++;
    if(_ControlGroupIndex == ControlGroupCount)
    {
        LogThreadEvent(String("All control groups finished"), LogDone);
        return ProgramResultSuccess;
    }
    else
    {
        LogThreadEvent(String("Pressing ") + String(_ControlGroupIndex), LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + _ControlGroupIndex);
        return ProgramResultStillExecuting;
    }
}
