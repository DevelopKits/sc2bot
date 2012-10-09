#include "Main.h"

void ProgramArmy::Init()
{
    _CurState = ProgArmySelectArmy;
    Name = String("Army");
}

ProgramResultType ProgramArmy::ExecuteStep()
{
    FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
	FrameUnitManager &FrameUnit = g_Context->Managers.FrameUnit;
    KnowledgeManager &Knowledge = g_Context->Managers.Knowledge;
    ArmyInfo &CurArmy = g_Context->Managers.Knowledge.Army();
    UINT ScoutGroupIndex = g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupBuildingSupport);
    UINT ArmyGroupIndex = g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupCombatAll);
    if(_CurState == ProgArmySelectArmy)
    {
        LogThreadEvent("Selecting army", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + ArmyGroupIndex);
        _CurState = ProgArmyOrderArmy;
        CurArmy.NextMacroTime = GameTime() + 2.5;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgArmyOrderArmy)
    {
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent("Attack", State))
        {
            LogThreadEvent("Attack button not found", LogError);
            return ProgramResultFail;
        }

        Knowledge.UpdateArmyAction();

        Vec2f BaseLocation;
        if(CurArmy.State == ArmyStateIdleInBase)
        {
            BaseLocation = g_Context->Managers.Knowledge.RandomMusterLocation();
        }
        else if(CurArmy.State == ArmyStateMovingToDefend)
        {
            BaseLocation = CurArmy.AttackTarget;
        }
        else if(CurArmy.State == ArmyStateMovingToAttack)
        {
            BaseLocation = CurArmy.AttackTarget;
        }
        else if(CurArmy.State == ArmyStateGathering)
        {
            BaseLocation = g_Context->Managers.Minimap.ArmyLocation();
        }

        /*if(GameTime() > 450.0f)
        {
            BaseLocation = g_Context->Managers.Knowledge.EnemyLocation();
        }
        else
        {
            BaseLocation = g_Context->Managers.Knowledge.RandomMusterLocation();
        }*/
        Vec2i ScreenCoord = g_Context->Constants.MinimapCoordToScreenCoord(BaseLocation);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_A);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        LogThreadEvent("Ordering army", LogDone);
        if(CurArmy.State == ArmyStateIdleInBase)
        {
            _CurState = ProgArmySelectScoutGroup;
            return ProgramResultStillExecuting;
        }
        else
        {
            return ProgramResultSuccess;
        }
    }
    if(_CurState == ProgArmySelectScoutGroup)
    {
        LogThreadEvent("Selecting Scout Group", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + ScoutGroupIndex);
        _CurState = ProgArmySelectScout;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgArmySelectScout)
    {
        if(FrameHUD.SoloPortrait() != NULL)
        {
            LogThreadEvent("No scouts found", LogStep);
            _CurState = ProgArmySelectArmyForNewScout;
            return ProgramResultStillExecuting;
        }
        Vector<UINT> ValidIndices;
        for(UINT Index = 0; Index < BubblePortraitCount; Index++)
        {
            Texture *CurTexture = FrameHUD.BubblePortraits(Index).Tex;
            if(CurTexture != NULL && CurTexture->ID() == Knowledge.ScoutUnit()->Name)
            {
                ValidIndices.PushEnd(Index);
            }
        }
        if(ValidIndices.Length() >= 2)
        {
            LogThreadEvent("Multiple scouts found", LogError);
        }
        if(ValidIndices.Length() == 0)
        {
            LogThreadEvent("No scouts found", LogStep);
            _CurState = ProgArmySelectArmyForNewScout;
            return ProgramResultStillExecuting;
        }
        else
        {
            Vec2i ScreenCoord = FrameHUD.ScreenCoordFromBubblePortraitIndex(ValidIndices.RandomElement());
            LogThreadEvent(String("Selecting scout: ") + ScreenCoord.CommaSeparatedString(), LogStep);
            g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
            _CurState = ProgArmyCenterScout;
            return ProgramResultStillExecuting;
        }
    }
    if(_CurState == ProgArmySelectArmyForNewScout)
    {
        LogThreadEvent("Selecting army for scout", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + ArmyGroupIndex);
        _CurState = ProgArmySelectNewScout;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgArmySelectNewScout)
    {
        if(FrameHUD.SoloPortrait() != NULL)
        {
            LogThreadEvent("Single unit selected", LogError);
            return ProgramResultFail;
        }
        Vector<UINT> ValidIndices;
        for(UINT Index = 0; Index < BubblePortraitCount; Index++)
        {
            Texture *CurTexture = FrameHUD.BubblePortraits(Index).Tex;
            if(CurTexture != NULL && CurTexture->ID() == Knowledge.ScoutUnit()->Name)
            {
                ValidIndices.PushEnd(Index);
            }
        }
        if(ValidIndices.Length() == 0)
        {
            LogThreadEvent("No scouts found in army", LogError);
            return ProgramResultFail;
        }
        else
        {
            Vec2i ScreenCoord = FrameHUD.ScreenCoordFromBubblePortraitIndex(ValidIndices.RandomElement());
            LogThreadEvent(String("Selecting new scout: ") + ScreenCoord.CommaSeparatedString(), LogStep);
            g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
            _CurState = ProgArmyCenterScout;
            return ProgramResultStillExecuting;
        }
    }
    if(_CurState == ProgArmyCenterScout)
    {
        if(FrameHUD.SoloPortrait() == NULL || FrameHUD.SoloPortrait()->ID() != Knowledge.ScoutUnit()->Name)
        {
            LogThreadEvent("Scout selection failed", LogError);
            return ProgramResultFail;
        }
        LogThreadEvent(String("Adding ") + FrameHUD.SoloPortrait()->ID() + String(" to scout group ") + String(ScoutGroupIndex), LogStep);
        g_Context->Managers.KeyboardMouse.SendShiftKey(KEY_0 + ScoutGroupIndex);
        
        g_Context->Managers.KeyboardMouse.SendCtrlKey(KEY_F);
        LogThreadEvent("Centering on scout", LogStep);
        _CurState = ProgArmyOrderScout;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgArmyOrderScout)
    {
        if(FrameHUD.SoloPortrait() == NULL || FrameHUD.SoloPortrait()->ID() != Knowledge.ScoutUnit()->Name)
        {
            LogThreadEvent("Scout selection failed", LogError);
            return ProgramResultFail;
        }
        Knowledge.ReportCenteredOnScout();
        Vec2i ClickPos = g_Context->Constants.MinimapCoordToScreenCoord(Knowledge.MinimapScoutLocation());

		bool SkipScoutOrder = false;
		UINT AlliedUnitCount = 0;
		UINT EnemyUnitCount = 0;
		for(UINT UnitIndex = 0; UnitIndex < FrameUnit.UnitsThisFrame().Length(); UnitIndex++)
        {
            const FrameUnitInfo &CurUnit = *FrameUnit.UnitsThisFrame()[UnitIndex];
			if(CurUnit.Owner == PlayerAlly && CurUnit.Clickable())
			{
				AlliedUnitCount++;
			}
            if(CurUnit.Owner == PlayerEnemy)
            {
				EnemyUnitCount++;
            }
        }

		String Verb = "Scouting ";
		if(AlliedUnitCount > 5)
		{
			SkipScoutOrder = true;
			LogThreadEvent("Scouting skipped; allies nearby", LogDone);
		}
		else if(EnemyUnitCount > AlliedUnitCount)
		{
			Verb = "Retreating ";
			ClickPos = g_Context->Constants.MinimapCoordToScreenCoord(Knowledge.RandomMusterLocation());
		}

		if(!SkipScoutOrder)
		{
			LogThreadEvent(Verb + FrameHUD.SoloPortrait()->ID() + String(":") + ClickPos.CommaSeparatedString(), LogDone);
			g_Context->Managers.KeyboardMouse.SendKey(KEY_M);
			g_Context->Managers.KeyboardMouse.Click(ClickPos, MouseButtonLeft, ModifierNone);
			return ProgramResultSuccess;
		}
    }
    HANDLE_CRITICAL_FAILURE;
}
