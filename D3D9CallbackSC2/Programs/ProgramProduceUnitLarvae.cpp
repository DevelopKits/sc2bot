#include "Main.h"

void ProgramProduceUnitLarvae::Init(UnitEntry *Entry)
{
    _CurState = ProgProduceUnitLarvaeSelectHatcheries;
    _Entry = Entry;
    Name = String("Larvae<") + Entry->Name + String(">");
}

ProgramResultType ProgramProduceUnitLarvae::ExecuteStep()
{
    FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    if(_CurState == ProgProduceUnitLarvaeSelectHatcheries)
    {
        LogThreadEvent("Selecting Hatcheries", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupBuildingMain));
        _CurState = ProgProduceUnitLarvaeSelectLarvae;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitLarvaeSelectLarvae)
    {
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent("Larvae", State))
        {
            LogThreadEvent("Larvae button not found", LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent("Larvae button in invalid state", LogError);
            return ProgramResultFail;
        }
        LogThreadEvent("Selecting Larvae", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_S);
        _CurState = ProgProduceUnitLarvaeOrderUnit;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitLarvaeOrderUnit)
    {
        if(!FrameHUD.ValidateSingleUnit("Larvae"))
        {
            LogThreadEvent("Larvae selection failed", LogError, RGBColor::Orange);
            return ProgramResultFail;
        }
        if(_Entry->Hotkey >= 'a' && _Entry->Hotkey <= 'z')
        {
            g_Context->Managers.KeyboardMouse.SendKey(KEY_A + (_Entry->Hotkey - 'a'));
            LogThreadEvent(String("Ordering larvae: ") + String(_Entry->Hotkey), LogStep);
            _TimeoutTime = GameTime() + 1.0;
            _CurState = ProgProduceUnitLarvaeWaitForCocoon;
            return ProgramResultStillExecuting;
        }
        else
        {
            LogThreadEvent("Invalid unit hotkey", LogError);
            return ProgramResultFail;
        }
    }
    if(_CurState == ProgProduceUnitLarvaeWaitForCocoon)
    {
        if(GameTime() > _TimeoutTime)
        {
            LogThreadEvent("Timed out waiting for cocoon", LogError);
            return ProgramResultFail;
        }
        if(FrameHUD.CountUnitsWithName("ZergCocoon") > 0)
        {
            LogThreadEvent("ZergCocoon found", LogDone);
            g_Context->ReportAction(_Entry->Name, RGBColor::Green);
            g_Context->Managers.Knowledge.RecordUnitInProduction(_Entry, GameTime());
            if(_Entry->Name == "Zergling")
            {
                g_Context->Managers.Knowledge.RecordUnitInProduction(_Entry, GameTime());
            }
            return ProgramResultSuccess;
        }
        else
        {
            LogThreadEvent("Waiting for ZergCocoon", LogStep);
            return ProgramResultStillExecuting;
        }
    }
    HANDLE_CRITICAL_FAILURE;
}
