#include "Main.h"

void ProgramProduceUnitUpgrade::Init(UnitEntry *Entry)
{
    _Entry = Entry;
    _CurState = ProgramProduceUnitUpgradeSelectSupportBuildings;
    Name = String("Research<") + Entry->Name + String(">");
}

ProgramResultType ProgramProduceUnitUpgrade::ExecuteStep()
{
    FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    if(_CurState == ProgramProduceUnitUpgradeSelectSupportBuildings)
    {
        LogThreadEvent("Selecting Support Buildings", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupBuildingSupport));
        _CurState = ProgramProduceUnitUpgradeSelectSupportBuilding;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramProduceUnitUpgradeSelectSupportBuilding)
    {
        Vector<UINT> ValidIndices;
        for(UINT Index = 0; Index < BubblePortraitCount; Index++)
        {
            Texture *CurTexture = FrameHUD.BubblePortraits(Index).Tex;
            if(CurTexture != NULL && CurTexture->ID() == _Entry->BuiltAt)
            {
                ValidIndices.PushEnd(Index);
            }
        }
        if(ValidIndices.Length() == 0)
        {
            LogThreadEvent(_Entry->BuiltAt + String(" not found"), LogError);
            return ProgramResultFail;
        }
        Vec2i ScreenCoord = FrameHUD.ScreenCoordFromBubblePortraitIndex(ValidIndices.RandomElement());
        LogThreadEvent(String("Left Click: ") + ScreenCoord.CommaSeparatedString(), LogStep);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        _CurState = ProgramProduceUnitUpgradeResearch;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramProduceUnitUpgradeResearch)
    {
        if(FrameHUD.BuildQueuePresent())
        {
            LogThreadEvent("Build queue already present", LogError);
            return ProgramResultFail;
        }
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent(_Entry->Name, State))
        {
            LogThreadEvent(_Entry->Name + String(" button not found"), LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent(_Entry->Name + String(" button in invalid state"), LogError);
            return ProgramResultFail;
        }
        g_Context->Managers.KeyboardMouse.SendKey(KEY_A + (_Entry->Hotkey - 'a'));
        LogThreadEvent(String("Researching ") + _Entry->Name, LogStep);
        _TimeoutTime = GameTime() + 1.0;
        _CurState = ProgramProduceUnitUpgradeWaitForSuccess;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramProduceUnitUpgradeWaitForSuccess)
    {
        if(GameTime() > _TimeoutTime)
        {
            LogThreadEvent(String("Timed out waiting for ") + _Entry->Name, LogError);
            return ProgramResultFail;
        }
        if(FrameHUD.BuildQueueEntryPresent(_Entry->Name))
        {
            LogThreadEvent(_Entry->Name + String(" found"), LogDone);
            g_Context->ReportAction(_Entry->Name, RGBColor::Green);
            g_Context->Managers.Knowledge.RecordUnitInProduction(_Entry, GameTime());
            return ProgramResultSuccess;
        }
        else
        {
            LogThreadEvent(String("Waiting for ") + _Entry->Name, LogStep);
            return ProgramResultStillExecuting;
        }
    }
    HANDLE_CRITICAL_FAILURE;
}
