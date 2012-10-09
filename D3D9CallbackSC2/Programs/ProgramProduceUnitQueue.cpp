#include "Main.h"

void ProgramProduceUnitQueue::Init(UnitEntry *Entry)
{
    _CurState = ProgramProduceUnitQueueSelectSupportBuildings;
    _Entry = Entry;
    Name = String("Produce<") + _Entry->Name + String(">");
}

ProgramResultType ProgramProduceUnitQueue::ExecuteStep()
{
    FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    if(_CurState == ProgramProduceUnitQueueSelectSupportBuildings)
    {
        if(_Entry->BuiltAt == "Gateway")
        {
            bool GatewaysFound = (g_Context->Managers.ControlGroup.FindControlGroup(ControlGroupBuildingSupport).CountUnitsMatchingName("Gateway") > 0);
            if(FrameHUD.WarpGateReady() && !GatewaysFound)
            {
                LogThreadEvent("Selecting Warp Gates", LogStep);
                g_Context->Managers.KeyboardMouse.SendKey(KEY_W);
                _CurState = ProgramProduceUnitQueueWarpGateMoveToSpawnPoint;
                return ProgramResultStillExecuting;
            }
        }
        LogThreadEvent("Selecting Support Buildings", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupBuildingSupport));
        _CurState = ProgramProduceUnitQueueSelectSupportBuilding;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramProduceUnitQueueSelectSupportBuilding)
    {
        if(FrameHUD.SoloPortrait() == NULL)
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
                LogThreadEvent("No valid buildings found", LogError);
                return ProgramResultFail;
            }
            Vec2i ScreenCoord = FrameHUD.ScreenCoordFromBubblePortraitIndex(ValidIndices.RandomElement());
            LogThreadEvent(String("Left Click: ") + ScreenCoord.CommaSeparatedString(), LogStep);
            g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        }
        else
        {
            Texture *CurTexture = FrameHUD.SoloPortrait();
            if(CurTexture->Unit()->SecondaryType != UnitSecondaryBase)
            {
                LogThreadEvent("Selected building is not a base", LogError);
                return ProgramResultFail;
            }
            LogThreadEvent("Single base selected", LogStep);
        }
        _CurState = ProgramProduceUnitQueueProduce;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramProduceUnitQueueProduce)
    {
        ActionButtonStateType State;
        if(FrameHUD.BuildQueuePresent())
        {
            LogThreadEvent("Build queue already present", LogError);
            return ProgramResultFail;
        }
        if(_Entry->BuiltAt == "Gateway")
        {
            if(FrameHUD.ActionButtonPresent("ResearchWarpGate", State))
            {
                if(State == ActionButtonStateNormal)
                {
                    LogThreadEvent("Pressing WarpGate button", LogDone);
                    g_Context->Managers.KeyboardMouse.SendKey(KEY_G);
                    return ProgramResultSuccess;
                }
            }
        }
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
        LogThreadEvent("Ordering unit", LogStep);
        _TimeoutTime = GameTime() + 1.0;
        _CurState = ProgramProduceUnitQueueWaitForUnit;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramProduceUnitQueueWaitForUnit)
    {
        if(GameTime() > _TimeoutTime)
        {
            LogThreadEvent("Timed out waiting for unit", LogError);
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
            LogThreadEvent("Waiting for unit", LogStep);
            return ProgramResultStillExecuting;
        }
    }
    if(_CurState == ProgramProduceUnitQueueWarpGateMoveToSpawnPoint)
    {
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent(_Entry->Name, State))
        {
            LogThreadEvent(_Entry->Name + String(" not present"), LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent(_Entry->Name + String(" is in an invalid state"), LogError);
            return ProgramResultFail;
        }
        const BaseInfo* TargetSite = g_Context->Managers.Knowledge.FindBuildSite();
        if(TargetSite == NULL)
        {
            LogThreadEvent("TargetSite not found", LogError);
            return ProgramResultFail;
        }
        Vec2i ScreenCoord = g_Context->Constants.MinimapCoordToScreenCoord(TargetSite->BuildLocation);
        LogThreadEvent(String("MoveToSpawnPoint: ") + ScreenCoord.CommaSeparatedString(), LogStep);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        _CurState = ProgramProduceUnitQueueWarpGateSelectUnit;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramProduceUnitQueueWarpGateSelectUnit)
    {
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent(_Entry->Name, State))
        {
            LogThreadEvent(_Entry->Name + String(" not present"), LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent(_Entry->Name + String(" is in an invalid state"), LogError);
            return ProgramResultFail;
        }
        const BaseInfo* TargetSite = g_Context->Managers.Knowledge.FindBuildSite();
        if(TargetSite == NULL)
        {
            LogThreadEvent("TargetSite not found", LogError);
            return ProgramResultFail;
        }
        LogThreadEvent("Selecting unit hotkey", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_A + (_Entry->Hotkey - 'a'));
        _CurState = ProgramProduceUnitQueueWarpGateFindSpawnLocation;
        _TimeoutTime = GameTime() + 1.0;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramProduceUnitQueueWarpGateFindSpawnLocation)
    {
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent("Cancel", State))
        {
            LogThreadEvent(_Entry->Name + String(" finished"), LogDone);
            g_Context->ReportAction(_Entry->Name, RGBColor::Green);
            g_Context->Managers.Knowledge.RecordUnitInProduction(_Entry, GameTime());
            return ProgramResultSuccess;
        }
        if(GameTime() > _TimeoutTime)
        {
            LogThreadEvent("Timed out searching for unit location", LogError);
            return ProgramResultFail;
        }
        Vec2i ScreenCoord = g_Context->Constants.RandomClickablePoint(0.02f);
        LogThreadEvent(String("RandomSpawnPoint: ") + ScreenCoord.CommaSeparatedString(), LogStep);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        return ProgramResultStillExecuting;
    }
    HANDLE_CRITICAL_FAILURE;
}
