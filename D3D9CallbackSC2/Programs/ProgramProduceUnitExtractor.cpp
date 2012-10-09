#include "Main.h"

void ProgramProduceUnitExtractor::Init()
{
    _CurState = ProgProduceUnitExtractorSelectWorkers;
    _AttemptsMade = 0;
    Name = "BuildingExtractor";
}

ProgramResultType ProgramProduceUnitExtractor::ExecuteStep()
{
    UnitEntry *Entry = g_Context->Managers.Knowledge.ExtractorUnit();
    FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    FrameUnitManager &FrameUnit = g_Context->Managers.FrameUnit;
    const String ActionButtonName = GetRaceTypeString(Entry->Race) + String("BuildBasicStructure");
    if(_CurState == ProgProduceUnitExtractorSelectWorkers)
    {
        LogThreadEvent("Selecting Workers", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupWorkerMineral));
        _CurState = ProgProduceUnitExtractorMoveToBuildingSite;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitExtractorMoveToBuildingSite)
    {
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent(ActionButtonName, State))
        {
            LogThreadEvent("BuildBasicStructure not present", LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent("BuildBasicStructure is in an invalid state", LogError);
            return ProgramResultFail;
        }
        Vec2f MinimapExtractorLocation;
        bool Success = g_Context->Managers.Knowledge.RandomExtractorSite(MinimapExtractorLocation);
        if(!Success)
        {
            LogThreadEvent("RandomExtractorSite failed", LogError);
            return ProgramResultFail;
        }
        const Vec2i ScreenCoord = g_Context->Constants.MinimapCoordToScreenCoord(MinimapExtractorLocation);
        LogThreadEvent(String("MoveToBuildingSite: ") + ScreenCoord.CommaSeparatedString(), LogStep);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        _CurState = ProgProduceUnitExtractorSelectBuildStructure;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitExtractorSelectBuildStructure)
    {
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent(ActionButtonName, State))
        {
            LogThreadEvent("BuildBasicStructure not present", LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent("BuildBasicStructure is in an invalid state", LogError);
            return ProgramResultFail;
        }
        g_Context->Managers.KeyboardMouse.SendKey(KEY_B);
        LogThreadEvent("Selecting BuildBasicStructure", LogStep);
        _CurState = ProgProduceUnitExtractorSelectBuilding;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitExtractorSelectBuilding)
    {
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent(Entry->Name, State))
        {
            LogThreadEvent(Entry->Name + String(" not present"), LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent(Entry->Name + String(" is in an invalid state"), LogError);
            return ProgramResultFail;
        }
        g_Context->Managers.KeyboardMouse.SendKey(KEY_A + (Entry->Hotkey - 'a'));
        LogThreadEvent(String("Selecting building: ") + String(Entry->Hotkey), LogStep);
        _CurState = ProgProduceUnitExtractorFindBuildingPlacement;
        _TimeoutTime0 = GameTime() + 1.0;
    }
    if(_CurState == ProgProduceUnitExtractorFindBuildingPlacement)
    {
        ActionButtonStateType State;
        if(FrameHUD.ActionButtonPresent(ActionButtonName, State))
        {
            if(_AttemptsMade == 0)
            {
                LogThreadEvent("BuildBasicStructure present but no attempts made", LogError);
                return ProgramResultFail;
            }
            _TimeoutTime1 = GameTime() + 1.5;
            _CurState = ProgProduceUnitExtractorWaitForSuccess;
            return ProgramResultStillExecuting;
        }
        if(!FrameHUD.ActionButtonPresent("Cancel", State))
        {
            LogThreadEvent("Cancel not present", LogError);
            return ProgramResultFail;
        }
        if(GameTime() > _TimeoutTime0)
        {
            LogThreadEvent("Timed out searching for building location", LogError);
            return ProgramResultFail;
        }
        Vector<const FrameUnitInfo*> AllGeysers;
        for(UINT UnitIndex = 0; UnitIndex < FrameUnit.UnitsThisFrame().Length(); UnitIndex++)
        {
            const FrameUnitInfo &CurUnit = *FrameUnit.UnitsThisFrame()[UnitIndex];
            if(CurUnit.Owner == PlayerInvalid && CurUnit.Entry->Name == "VespeneGeyser" && CurUnit.Clickable())
            {
                AllGeysers.PushEnd(&CurUnit);
            }
        }
        if(AllGeysers.Length() == 0)
        {
            LogThreadEvent("No clickable geysers", LogError);
            return ProgramResultFail;
        }
        
        Vec2i ScreenCoord = AllGeysers.RandomElement()->ScreenBound.Center().RoundToVec2i();
        LogThreadEvent(String("GeyserPoint: ") + ScreenCoord.CommaSeparatedString(), LogStep);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        _AttemptsMade++;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitExtractorWaitForSuccess)
    {
        if(GameTime() > _TimeoutTime1)
        {
            LogThreadEvent("Timed out waiting for building success", LogError);
            return ProgramResultFail;
        }
        ActionButtonStateType State;
        if(FrameHUD.ActionButtonPresent(ActionButtonName, State))
        {
            if(State == ActionButtonStateSelected)
            {
                LogThreadEvent("BuildBasicStructure selected, success", LogDone);
                g_Context->ReportAction(Entry->Name, RGBColor::Green);
                g_Context->Managers.Knowledge.RecordUnitInProduction(Entry, GameTime());
                return ProgramResultSuccess;
            }
            else
            {
                LogThreadEvent("BuildBasicStructure is in an invalid state", LogStep);
                return ProgramResultStillExecuting;
            }
        }
    }
    HANDLE_CRITICAL_FAILURE;
}
