#include "Main.h"

void ProgramQueen::Init()
{
    _CurState = ProgQueenSelectQueens;
    Name = String("Queen");
}

ProgramResultType ProgramQueen::ExecuteStep()
{
    FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    FrameUnitManager &FrameUnit = g_Context->Managers.FrameUnit;
    if(_CurState == ProgQueenSelectQueens)
    {
        LogThreadEvent("Selecting Queens", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupBuildingSupport));
        _CurState = ProgQueenSelectSingleQueen;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgQueenSelectSingleQueen)
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
            if(CurTexture != NULL && CurTexture->ID() == "Queen")
            {
                ValidIndices.PushEnd(Index);
            }
        }
        if(ValidIndices.Length() == 0)
        {
            LogThreadEvent("No queens found", LogError);
            return ProgramResultFail;
        }
        Vec2i ScreenCoord = FrameHUD.ScreenCoordFromBubblePortraitIndex(ValidIndices.RandomElement());
        LogThreadEvent(String("Left Click: ") + ScreenCoord.CommaSeparatedString(), LogStep);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        _CurState = ProgQueenOrderQueen;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgQueenOrderQueen)
    {
        if(FrameHUD.SoloPortrait() == NULL || FrameHUD.SoloPortrait()->ID() != "Queen")
        {
            LogThreadEvent("Queen not selected", LogError);
            return ProgramResultFail;
        }
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent("Larvae", State))
        {
            LogThreadEvent("Larvae button not found", LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent("Queen needs more energy", LogError, RGBColor(Vec3f(0.462744f, 0.286274f, 0.901959f)));
            return ProgramResultFail;
        }

        Vector<const FrameUnitInfo*> AllClickableBases;
        for(UINT UnitIndex = 0; UnitIndex < FrameUnit.UnitsThisFrame().Length(); UnitIndex++)
        {
            const FrameUnitInfo &CurUnit = *FrameUnit.UnitsThisFrame()[UnitIndex];
            if(CurUnit.Owner == PlayerSelf && CurUnit.Entry->SecondaryType == UnitSecondaryBase && g_Context->Constants.Clickable(CurUnit.ScreenBound.Center().RoundToVec2i()))
            {
                AllClickableBases.PushEnd(&CurUnit);
            }
        }
        
        Vec2i ClickPos;
        MouseButtonType Button;
        if(AllClickableBases.Length() == 0)
        {
            const BaseInfo *NearestBase = g_Context->Managers.Knowledge.FindNearestBase(FrameHUD.MinimapViewportCenter(), PlayerSelf);
            if(NearestBase == NULL)
            {
                LogThreadEvent(String("No allied bases"), LogError);
                return ProgramResultFail;
            }
            else
            {
                Button = MouseButtonRight;
                ClickPos = g_Context->Constants.MinimapCoordToScreenCoord(NearestBase->BaseLocation);
                LogThreadEvent(String("Clicking on nearest base: ") + ClickPos.CommaSeparatedString(), LogDone);
            }
        }
        else
        {
            Button = MouseButtonLeft;
            ClickPos = AllClickableBases.RandomElement()->ScreenBound.Center().RoundToVec2i();
            g_Context->Managers.KeyboardMouse.SendKey(KEY_V);
            LogThreadEvent(String("Clicking on nearest base: ") + ClickPos.CommaSeparatedString(), LogDone);
        }
        
        g_Context->Managers.KeyboardMouse.Click(ClickPos, Button, ModifierNone);
        return ProgramResultSuccess;
    }
    HANDLE_CRITICAL_FAILURE;
}
