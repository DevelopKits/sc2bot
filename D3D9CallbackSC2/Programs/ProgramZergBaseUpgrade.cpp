#include "Main.h"

void ProgramZergBaseUpgrade::Init(UnitEntry *Entry)
{
    _Entry = Entry;
    _CurState = ProgramZergBaseUpgradeSelectBaseLocation;
    Name = String("BaseUpgrade<") + _Entry->Name + String(">");
}

ProgramResultType ProgramZergBaseUpgrade::ExecuteStep()
{
    const FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    const FrameUnitManager &FrameUnit = g_Context->Managers.FrameUnit;
    if(_CurState == ProgramZergBaseUpgradeSelectBaseLocation)
    {
        const BaseInfo *MainBase = g_Context->Managers.Knowledge.MainBase();
        if(MainBase == NULL)
        {
            LogThreadEvent("No main base", LogError);
            return ProgramResultFail;
        }
        const Vec2i ScreenCoord = g_Context->Constants.MinimapCoordToScreenCoord(MainBase->BaseLocation);
        LogThreadEvent("Selecting Main Base", LogStep);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        _CurState = ProgramZergBaseUpgradeSelectBase;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramZergBaseUpgradeSelectBase)
    {
        Vector<const FrameUnitInfo*> AllClickableBases;
        for(UINT UnitIndex = 0; UnitIndex < FrameUnit.UnitsThisFrame().Length(); UnitIndex++)
        {
            const FrameUnitInfo &CurUnit = *FrameUnit.UnitsThisFrame()[UnitIndex];
            if(CurUnit.Owner == PlayerSelf && CurUnit.Entry->SecondaryType == UnitSecondaryBase && g_Context->Constants.Clickable(CurUnit.ScreenBound.Center().RoundToVec2i()))
            {
                AllClickableBases.PushEnd(&CurUnit);
            }
        }
        
        if(AllClickableBases.Length() == 0)
        {
            LogThreadEvent("No base building at main base", LogError);
            return ProgramResultFail;
        }
        Vec2i RandomOffset(rand() % 150 - 75, rand() % 150 - 75);
        const Vec2i ScreenCoord = AllClickableBases.RandomElement()->ScreenBound.Center().RoundToVec2i() + RandomOffset;
        LogThreadEvent(String("Clicking on base: ") + ScreenCoord.CommaSeparatedString(), LogStep);
        _CurState = ProgramZergBaseUpgradeUpgradeBase;
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramZergBaseUpgradeUpgradeBase)
    {
        if(FrameHUD.BuildQueuePresent())
        {
            LogThreadEvent("Build queue present", LogError);
            return ProgramResultFail;
        }
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent(_Entry->Name, State))
        {
            LogThreadEvent("Upgrade button not found", LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent("Upgrade button in invalid state", LogError);
            return ProgramResultFail;
        }
        if(!FrameHUD.ActionButtonPresent("Queen", State))
        {
            LogThreadEvent("Queen button not found", LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent("Queen button in invalid state", LogError);
            return ProgramResultFail;
        }
        g_Context->Managers.KeyboardMouse.SendKey(KEY_A + (_Entry->Hotkey - 'a'));
        LogThreadEvent("Ordering upgrade", LogStep);
        _TimeoutTime = GameTime() + 1.0;
        _CurState = ProgramZergBaseUpgradeWaitForSuccess;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramZergBaseUpgradeWaitForSuccess)
    {
        if(GameTime() > _TimeoutTime)
        {
            LogThreadEvent("Timed out waiting for success", LogError);
            return ProgramResultFail;
        }
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent("Queen", State))
        {
            LogThreadEvent("Queen button not found", LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent("Upgrade successful", LogDone);
            g_Context->ReportAction(_Entry->Name, RGBColor::Green);
            g_Context->Managers.Knowledge.RecordUnitInProduction(_Entry, GameTime());
            return ProgramResultSuccess;
        }
        LogThreadEvent("Waiting for upgrade", LogStep);
        return ProgramResultStillExecuting;
    }
    HANDLE_CRITICAL_FAILURE;
}
