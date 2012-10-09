#include "Main.h"

void ProgramMicroAll::Init()
{
    _CurState = ProgramMicroAllSelectGroup;
    _UnitIndex = -1;
    Name = String("MicroAll");
}

ProgramResultType ProgramMicroAll::ExecuteStep()
{
    FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    FrameUnitManager &FrameUnit = g_Context->Managers.FrameUnit;
	MinimapManager &Minimap = g_Context->Managers.Minimap;
    KnowledgeManager &Knowledge = g_Context->Managers.Knowledge;
    KeyboardMouseManager &KeyboardMouse = g_Context->Managers.KeyboardMouse;
    if(_CurState == ProgramMicroAllSelectGroup)
    {
        LogThreadEvent("Selecting Army", LogStep);
        KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupCombatMicro));
        _CurState = ProgramMicroAllSelectUnit;

        if(_UnitIndex + 1 < BubblePortraitCount)
        {
            Vec2i ScreenCoord = FrameHUD.ScreenCoordFromBubblePortraitIndex(_UnitIndex + 1);
            LogThreadEvent(String("Preemptive MouseMove: ") + String(_UnitIndex + 1), LogStep);
            KeyboardMouse.PreemptiveMouseMove(ScreenCoord);
        }

        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramMicroAllSelectUnit)
    {
        _UnitIndex++;
        bool UnitFound = false;
        while(_UnitIndex < BubblePortraitCount && !UnitFound)
        {
            Texture *Tex = FrameHUD.BubblePortraits(_UnitIndex).Tex;
            if(Tex != NULL && Tex->Unit() != NULL && Tex->Unit()->SecondaryType != UnitSecondaryBase)
            {
                UnitFound = true;
            }
            else
            {
                _UnitIndex++;
            }
        }
        if(_UnitIndex == BubblePortraitCount)
        {
            LogThreadEvent("All units finished", LogDone);
            return ProgramResultSuccess;
        }
        Vec2i ScreenCoord = FrameHUD.ScreenCoordFromBubblePortraitIndex(_UnitIndex);
        LogThreadEvent(String("Selecting unit ") + String(_UnitIndex), LogStep);
        KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        _CurState = ProgramMicroAllCenterUnit;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramMicroAllCenterUnit)
    {
        if(FrameHUD.SoloPortrait() == NULL)
        {
            LogThreadEvent("Unit selection failed", LogStep);
            _CurState = ProgramMicroAllSelectGroup;
            return ProgramResultStillExecuting;
        }
        KeyboardMouse.SendCtrlKey(KEY_F);
        LogThreadEvent(String("Centering on ") + FrameHUD.SoloPortrait()->ID(), LogStep);
        _CurState = ProgramMicroAllOrderUnit;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgramMicroAllOrderUnit)
    {
        if(FrameHUD.SoloPortrait() == NULL)
        {
            LogThreadEvent("Unit selection failed", LogStep);
            _CurState = ProgramMicroAllSelectGroup;
            return ProgramResultStillExecuting;
        }
        
        UnitEntry *MyUnit = FrameHUD.SoloPortrait()->Unit();
        if(MyUnit == NULL)
        {
            g_Context->Files.Assert << "Selected unit has no entry\n";
            _CurState = ProgramMicroAllSelectGroup;
            return ProgramResultStillExecuting;
        }

        LogThreadEvent(String("Ordering ") + FrameHUD.SoloPortrait()->ID(), LogStep);
		
        const BaseInfo* NearestBase = Knowledge.FindNearestBase(FrameHUD.MinimapViewportCenter(), PlayerSelf);
        if(NearestBase == NULL)
        {
            g_Context->Files.Assert << "No base found\n";
            return ProgramResultFail;
        }

		float Health = FrameHUD.CurrentHealthPercentage();
		bool RetreatUnitLocal = false;
        bool RetreatUnitToBase = false;
        bool FindNewTarget = false;
        UINT AlliedUnitCount = 0;
        UINT EnemyUnitCount = 0;
        
        double TotalEnemyValue = 0;
        double EnemyArmyValue = 0, AlliedArmyValue = 0;
        for(UINT UnitIndex = 0; UnitIndex < FrameUnit.UnitsThisFrame().Length(); UnitIndex++)
		{
			const FrameUnitInfo &CurUnit = *FrameUnit.UnitsThisFrame()[UnitIndex];
            if(CurUnit.Owner == PlayerEnemy && CurUnit.Entry->Hostile && CurUnit.Entry->SecondaryType != UnitSecondaryWorker)
            {
                TotalEnemyValue += CurUnit.Entry->MineralCost + CurUnit.Entry->GasCost;
            }
            if(CurUnit.Owner == PlayerEnemy && CurUnit.Clickable() && CurUnit.Entry->Hostile && CurUnit.Entry->SecondaryType != UnitSecondaryWorker)
			{
                if( (CurUnit.Entry->AttackAir && MyUnit->Flying) ||
                    (CurUnit.Entry->AttackGround && !MyUnit->Flying) )
                {
                    EnemyUnitCount++;
                    EnemyArmyValue += CurUnit.Entry->MineralCost + CurUnit.Entry->GasCost;
                }
			}
            if( (CurUnit.Owner == PlayerSelf || CurUnit.Owner == PlayerAlly)  && CurUnit.Entry->Hostile && CurUnit.Entry->SecondaryType != UnitSecondaryWorker)
			{
                AlliedUnitCount++;
                AlliedArmyValue += CurUnit.Entry->MineralCost + CurUnit.Entry->GasCost;
			}
		}

        if(AlliedArmyValue < 2.0 * EnemyArmyValue && AlliedUnitCount < 4 && EnemyUnitCount > 6)
        {
            if(Vec2f::Dist(NearestBase->BaseLocation, FrameHUD.MinimapViewportCenter()) > 0.1f)
            {
                RetreatUnitToBase = true;
            }
        }

		//if(Health > 0.0f && Health < 0.6f && FrameUnit.BattlePosition().ClosestEnemyHostile < 500.0f) RetreatUnit = true;
        if(MyUnit->Range >= 2 && !MyUnit->Flying && FrameUnit.BattlePosition().ClosestEnemyHostile < 150.0f)
		{
			RetreatUnitLocal = true;
		}

        if(MyUnit->Range >= 2)
        {
            FindNewTarget = true;
        }
        
        if(MyUnit->Range < 2 && FrameUnit.BattlePosition().ClosestEnemyHostile > 200.0f)
        {
            FindNewTarget = true;
        }

		ActionButtonStateType BlinkState;
		bool BlinkReady = FrameHUD.ActionButtonPresent("Blink", BlinkState);
		if(BlinkState != ActionButtonStateNormal)
		{
			BlinkReady = false;
		}

		if(RetreatUnitLocal || RetreatUnitToBase)
		{
            if(RetreatUnitToBase)
            {
                RetreatUnitLocal = false;
            }

            WORD Key = KEY_M;
            String Verb;
            if(BlinkReady)
            {
                Key = KEY_B;
            }
            Vec2i Target;
            if(RetreatUnitLocal)
            {
                if(BlinkReady)
                {
                    Verb = "BlinkLocal";
                    Target = FrameUnit.BattlePosition().SafestBlinkLocation;
                }
                else
                {
                    Verb = "RetreatLocal";
                    Target = FrameUnit.BattlePosition().SafestRetreatLocation;
                }
            }
            else
            {
                Verb = "RetreatBase";
                Target = g_Context->Constants.MinimapCoordToScreenCoord(NearestBase->BaseLocation);
            }
			g_Context->ReportAction(Verb + String(" ") + FrameHUD.SoloPortrait()->ID(), RGBColor::Magenta);
            KeyboardMouse.SendKey(Key);
            KeyboardMouse.Click(Target, MouseButtonLeft, ModifierNone);
		}
		else if(FindNewTarget)
		{
			const FrameUnitInfo *BestUnit = NULL;
			float BestUnitValue = -1.0f;
			Vec2f ScreenCenter = g_Context->Constants.GetVec2fConstant(ScreenConstantVec2fScreenCenter);
			for(UINT UnitIndex = 0; UnitIndex < FrameUnit.UnitsThisFrame().Length(); UnitIndex++)
			{
				const FrameUnitInfo &CurUnit = *FrameUnit.UnitsThisFrame()[UnitIndex];
				float CurDist = Vec2f::Dist(CurUnit.ScreenBound.Center(), ScreenCenter);
				if(CurUnit.Owner == PlayerEnemy && CurDist < 400.0f && CurUnit.Clickable())
				{
                    if( (MyUnit->AttackAir && CurUnit.Entry->Flying) ||
                        (MyUnit->AttackGround && !CurUnit.Entry->Flying) )
                    {
					    float CurUnitValue = 0.0f;
					    if(CurUnit.HealthBar != NULL)
					    {
						    CurUnitValue += 1.0f - CurUnit.HealthBar->Health();
					    }
					    if(CurUnit.Entry->Hostile)
					    {
						    CurUnitValue += 10.0f;
					    }
					    if(CurUnitValue > BestUnitValue)
					    {
						    BestUnitValue = CurUnitValue;
						    BestUnit = &CurUnit;
					    }
                    }
				}
			}
			if(BestUnit == NULL)
			{
				g_Context->ReportAction("No enemy units found, attacking nearest enemy location", RGBColor::Magenta);
				Vec2f NearestEnemyMinimapCoord = Minimap.FindNearestEnemyCoord(FrameHUD.MinimapViewportCenter());
				KeyboardMouse.SendKey(KEY_A);
				KeyboardMouse.Click(g_Context->Constants.MinimapCoordToScreenCoord(NearestEnemyMinimapCoord), MouseButtonLeft, ModifierNone);
			}
			else
			{
				KeyboardMouse.SendKey(KEY_A);
				KeyboardMouse.Click(BestUnit->ScreenBound.Center().RoundToVec2i(), MouseButtonLeft, ModifierNone);
				float HealthValue = 0.0f;
				if(BestUnit->HealthBar != NULL)
				{
					HealthValue = BestUnit->HealthBar->Health();
				}
				g_Context->ReportAction(FrameHUD.SoloPortrait()->ID() + String(">") + BestUnit->Entry->Name + String(", ") + String(HealthValue), RGBColor::Magenta);
			}
		}
        _CurState = ProgramMicroAllSelectGroup;
        return ProgramResultStillExecuting;
    }
    HANDLE_CRITICAL_FAILURE;
}
