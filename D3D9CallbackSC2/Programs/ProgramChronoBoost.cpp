#include "Main.h"

void ProgramChronoBoost::Init()
{
    _CurState = ProgramChronoBoostSelectSupportBuildings;
    _ChosenBuilding = NULL;
    Name = String("ChronoBoost");
}

bool IsBoostableBuilding(const String &S)
{
    return (//S == "Gateway" ||
            S == "Nexus" ||
            S == "RoboticsFacility" ||
            S == "RoboticsBay" ||
            S == "Forge" ||
            S == "CyberneticsCore" ||
            S == "Stargate" ||
            S == "DarkShrine" ||
            S == "FleetBeacon" ||
            S == "TwilightCouncil" ||
            S == "TemplarArchives");
}

ProgramResultType ProgramChronoBoost::ExecuteStep()
{
    FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    FrameUnitManager &FrameUnit = g_Context->Managers.FrameUnit;
    if(_CurState == ProgramChronoBoostSelectSupportBuildings)
    {
        LogThreadEvent("Selecting Support Buildings", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupBuildingSupport));
        _CurState = ProgramChronoBoostSelectSupportBuilding;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramChronoBoostSelectSupportBuilding)
    {
        if(FrameHUD.SoloPortrait() != NULL)
        {
            if(FrameHUD.SoloPortrait()->ID() == "Nexus")
            {
                _ChosenBuilding = FrameHUD.SoloPortrait()->Unit();
                _CurState = ProgramChronoBoostCenter;
                return ProgramResultStillExecuting;
            }
            else
            {
                LogThreadEvent("Single non-nexus unit selected", LogError);
                return ProgramResultFail;
            }
        }
        Vector<UINT> ValidIndices;
        for(UINT Index = 0; Index < BubblePortraitCount; Index++)
        {
            Texture *CurTexture = FrameHUD.BubblePortraits(Index).Tex;
            if(CurTexture != NULL && IsBoostableBuilding(CurTexture->ID()))
            {
                ValidIndices.PushEnd(Index);
            }
        }
        if(ValidIndices.Length() == 0)
        {
            LogThreadEvent("No boostable buildings found", LogError);
            return ProgramResultFail;
        }
        UINT SelectedIndex = ValidIndices.RandomElement();
        Vec2i ScreenCoord = FrameHUD.ScreenCoordFromBubblePortraitIndex(SelectedIndex);
        _ChosenBuilding = FrameHUD.BubblePortraits(SelectedIndex).Tex->Unit();
        if(_ChosenBuilding != NULL)
        {
            LogThreadEvent(String("Selecting ") + _ChosenBuilding->Name + String(": ") + ScreenCoord.CommaSeparatedString(), LogStep);
            g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        }
        _CurState = ProgramChronoBoostCenter;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramChronoBoostCenter)
    {
        if(FrameHUD.SoloPortrait() == NULL || _ChosenBuilding == NULL || FrameHUD.SoloPortrait()->ID() != _ChosenBuilding->Name)
        {
            LogThreadEvent("Boostable building selection failed", LogError);
            return ProgramResultFail;
        }
        if(!FrameHUD.BuildQueuePresent())
        {
            LogThreadEvent("Selected building has no build queue", LogError, RGBColor::Orange);
            return ProgramResultFail;
        }
        g_Context->Managers.KeyboardMouse.SendCtrlKey(KEY_F);
        LogThreadEvent("Centering on boostable building", LogStep);
        _CurState = ProgramChronoBoostSelectNexuses;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramChronoBoostSelectNexuses)
    {
        LogThreadEvent("Selecting Nexuses", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupBuildingMain));
        _CurState = ProgramChronoBoostCastChronoBoost;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramChronoBoostCastChronoBoost)
    {
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent("ChronoBoost", State))
        {
            LogThreadEvent("Chrono Boost button not found", LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent("Not enough energy for Chrono Boost", LogError, RGBColor(Vec3f(0.462744f, 0.286274f, 0.901959f)));
            return ProgramResultFail;
        }
        
        Vector<const FrameUnitInfo*> AllTargets;
        for(UINT UnitIndex = 0; UnitIndex < FrameUnit.UnitsThisFrame().Length(); UnitIndex++)
        {
            const FrameUnitInfo &CurUnit = *FrameUnit.UnitsThisFrame()[UnitIndex];
            if(CurUnit.Owner == PlayerSelf && CurUnit.Entry->Name == _ChosenBuilding->Name && CurUnit.Clickable())
            {
                AllTargets.PushEnd(&CurUnit);
            }
        }

        if(AllTargets.Length() == 0)
        {
            LogThreadEvent(String("No Chrono Boost targets on-screen"), LogError);
            return ProgramResultFail;
        }
        
        Vec2i ClickPos = AllTargets.RandomElement()->ScreenBound.Center().RoundToVec2i();
        g_Context->ReportAction(String("Chrono Boost: ") + _ChosenBuilding->Name, RGBColor(20, 225, 225));
        LogThreadEvent(String("Chrono Boost on ") + _ChosenBuilding->Name + String(" at ") + ClickPos.CommaSeparatedString(), LogDone, RGBColor(20, 225, 225));
        g_Context->Managers.KeyboardMouse.SendKey(KEY_C);
        g_Context->Managers.KeyboardMouse.Click(ClickPos, MouseButtonLeft, ModifierNone);
        return ProgramResultSuccess;
    }
    HANDLE_CRITICAL_FAILURE;
}
