#include "Main.h"

void ProgramBaseSearch::Init()
{
    _CurState = ProgBaseSearchSelectRegion;
    Name = String("BaseSearch");
}

double ProgramBaseSearch::ComputeUnitValue(const UnitEntry *Entry) const
{
	ControlGroupManager &ControlGroup = g_Context->Managers.ControlGroup;
	KnowledgeManager &Knowledge = g_Context->Managers.Knowledge;

	GameLocalUnitInfo& GameLocalUnit = Knowledge.GetGameLocalUnitInfo(Entry);
	
	float ScaleFactor = 5.0;
	if(Entry->PrimaryType == UnitPrimaryBuilding)
	{
		UINT TotalCount = ControlGroup.FindControlGroup(ControlGroupBuildingSupport).CountPrerequisiteUnitsMatchingName(Entry->Name);
		if(GameLocalUnit.TotalBuilt <= TotalCount)
		{
			ScaleFactor = 1.0;
		}
	}
	return ScaleFactor * (GameTime() - GameLocalUnit.LastSelectionTime);
}

ProgramResultType ProgramBaseSearch::ExecuteStep()
{
    ControlGroupManager &ControlGroup = g_Context->Managers.ControlGroup;
	KnowledgeManager &Knowledge = g_Context->Managers.Knowledge;
    if(_CurState == ProgBaseSearchSelectRegion)
    {
        if(g_Context->Managers.Minimap.SelfPixelCoordinates().Length() == 0)
        {
            LogThreadEvent("SelfPixelCoordinates empty", LogError);
            return ProgramResultFail;
        }
        const Vec2f MinimapCoord = g_Context->Managers.Minimap.SelfPixelCoordinates().RandomElement();
        const Vec2i ScreenCoord = g_Context->Constants.MinimapCoordToScreenCoord(MinimapCoord);
        LogThreadEvent(String("SelectRegion: ") + ScreenCoord.CommaSeparatedString(), LogStep);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        _CurState = ProgBaseSearchSelectUnit;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgBaseSearchSelectUnit)
    {
        const FrameUnitManager &FrameUnit = g_Context->Managers.FrameUnit;
        Vector<const FrameUnitInfo*> SelectableUnits;
        for(UINT UnitIndex = 0; UnitIndex < FrameUnit.UnitsThisFrame().Length(); UnitIndex++)
        {
            const FrameUnitInfo &CurUnit = *FrameUnit.UnitsThisFrame()[UnitIndex];
            if(CurUnit.Owner == PlayerSelf && CurUnit.Clickable() && CurUnit.Entry->SecondaryType != UnitSecondaryWorker)
            {
                SelectableUnits.PushEnd(&CurUnit);
            }
        }
        if(SelectableUnits.Length() == 0)
        {
            LogThreadEvent("No units visible", LogError);
            return ProgramResultFail;
        }
		SelectableUnits.Randomize();
        const FrameUnitInfo *BestUnit = SelectableUnits[0];
		double BestUnitValue = 0.0;
		for(UINT Index = 0; Index < SelectableUnits.Length(); Index++)
		{
			double CurUnitValue = ComputeUnitValue(SelectableUnits[Index]->Entry);
			if(CurUnitValue > BestUnitValue)
			{
				BestUnit = SelectableUnits[Index];
				BestUnitValue = CurUnitValue;
			}
		}
		Knowledge.GetGameLocalUnitInfo(BestUnit->Entry).LastSelectionTime = GameTime();
        const Vec2i ScreenCoord = BestUnit->ScreenBound.Center().RoundToVec2i();
        LogThreadEvent(String("SelectUnit: ") + ScreenCoord.CommaSeparatedString() + String("; ") + BestUnit->Entry->Name, LogStep);
        ModifierType Modifier = ModifierCtrl;
        /*if(ChosenUnit.Entry->PrimaryType == UnitPrimaryBuilding)
        {
            Modifier = ModifierNone;
        }*/
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, Modifier);
        _CurState = ProgBaseSearchAddToControlGroup;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgBaseSearchAddToControlGroup)
    {
        bool AddUnits = false;
        const FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
        Texture *CurTexture = NULL;
        if(FrameHUD.SoloPortrait() != NULL)
        {
            CurTexture = FrameHUD.SoloPortrait();
        }
        else
        {
            if(FrameHUD.SingleUnitTypeSelected())
            {
                CurTexture = FrameHUD.BubblePortraits(0).Tex;
            }
            /*if(CurTexture != NULL && CurTexture->Unit()->PrimaryType == UnitPrimaryBuilding)
            {
                LogThreadEvent("Multiple buildings selected", LogError);
                return ProgramResultFail;
            }*/
        }
        if(CurTexture == NULL)
        {
            LogThreadEvent("No unit selected", LogError);
            return ProgramResultFail;
        }
        if(CurTexture->Unit() == NULL)
        {
            LogThreadEvent(String("Unit has no entry: ") + String(CurTexture->ID()), LogError);
            return ProgramResultFail;
        }
        UnitEntry &CurEntry = *CurTexture->Unit();
        RaceType MyRace = g_Context->Managers.Knowledge.MyRace();
        if(CurEntry.Race != MyRace)
        {
            LogThreadEvent("Unit race does not match", LogError);
            return ProgramResultFail;
        }
        int ControlGroupIndex = g_Context->Managers.ControlGroup.FindControlGroupIndex(CurEntry);
        if(ControlGroupIndex == -1)
        {
            LogThreadEvent("Unit has no control group", LogError);
            return ProgramResultFail;
        }
        if(CurEntry.SecondaryType == UnitSecondaryBase)
        {
            LogThreadEvent(String("Adding ") + CurEntry.Name + String(" to all groups"), LogDone);
            for(UINT ControlGroupIndex = 0; ControlGroupIndex < ControlGroupCount; ControlGroupIndex++)
            {
                g_Context->Managers.KeyboardMouse.SendShiftKey(KEY_0 + ControlGroupIndex);
            }
        }
        else
        {
            if(FrameHUD.CurrentUnitUnderConstruction())
            {
                LogThreadEvent("Unit under construction", LogDone);
            }
            else
            {
                LogThreadEvent(String("Adding ") + CurEntry.Name + String(" to group ") + String(ControlGroupIndex), LogDone);
                g_Context->Managers.KeyboardMouse.SendShiftKey(KEY_0 + ControlGroupIndex);
                ControlGroupInfo &CurGroup = ControlGroup.ControlGroups(ControlGroupIndex);
                if(CurGroup.CountUnitsMatchingName(CurEntry.Name) == 0)
                {
                    CurGroup.InsertUnit(&CurEntry);
                }
                if(CurGroup.Type == ControlGroupCombatMicro)
                {
                    UINT ArmyGroupIndex = ControlGroup.FindControlGroupIndex(ControlGroupCombatAll);
                    LogThreadEvent(String("Adding ") + CurEntry.Name + String(" to group ") + String(ArmyGroupIndex), LogDone);
                    g_Context->Managers.KeyboardMouse.SendShiftKey(KEY_0 + ArmyGroupIndex);
                }
            }
        }
        return ProgramResultSuccess;
    }
    HANDLE_CRITICAL_FAILURE;
}
