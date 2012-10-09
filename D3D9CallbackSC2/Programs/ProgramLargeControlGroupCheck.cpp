#include "Main.h"

void ProgramLargeControlGroupCheck::Init(ControlGroupType GroupType)
{
    _CurState = ProgramLargeControlGroupCheckSelectGroup;
    _GroupType = GroupType;
    String GroupDescription = "Error";
    if(GroupType == ControlGroupCombatAll)
    {
        GroupDescription = "Army";
    }
    _CurLevel = 0;
    Name = String("Check") + GroupDescription;
}

ProgramResultType ProgramLargeControlGroupCheck::ExecuteStep()
{
    FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    ControlGroupInfo &CurGroup = g_Context->Managers.ControlGroup.FindControlGroup(_GroupType);
    if(_CurState == ProgramLargeControlGroupCheckSelectGroup)
    {
        LogThreadEvent("Selecting Group", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(_GroupType));
        CurGroup.LastObservedAllLevelUnits.FreeMemory();
        _CurState = ProgramLargeControlGroupCheckSelectLevel;
        _CurLevel = 0;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramLargeControlGroupCheckSelectLevel)
    {
        CurGroup.AppendAllFromHUD();
        _CurLevel++;
        if(_CurLevel < FrameHUD.ControlGroupLevel())
        {
            LogThreadEvent(String("Selecting Level ") + String(_CurLevel), LogStep);
            Vec2i ScreenPos = g_Context->Managers.FrameHUD.ScreenCoordFromControlGroupLevel(_CurLevel);
            g_Context->Managers.KeyboardMouse.Click(ScreenPos, MouseButtonLeft, ModifierNone);
            return ProgramResultStillExecuting;
        }
        else
        {
            LogThreadEvent(String("Done, Level ") + String(_CurLevel), LogDone);
            if(_GroupType == ControlGroupCombatAll)
            {
                g_Context->Managers.Knowledge.Army().ResourceValue = ControlGroupUnitInfo::ComputeResourceValue(CurGroup.LastObservedAllLevelUnits);
            }
            return ProgramResultSuccess;
        }
    }
    HANDLE_CRITICAL_FAILURE;
}
