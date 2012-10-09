#include "Main.h"

void ProgramOverlord::Init()
{
    _CurState = ProgOverlordSelectOverlords;
    Name = String("Overlord");
}

ProgramResultType ProgramOverlord::ExecuteStep()
{
    FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    if(_CurState == ProgOverlordSelectOverlords)
    {
        LogThreadEvent("Selecting Overlords", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupSupply));
        _CurState = ProgOverlordSelectSingleOverlord;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgOverlordSelectSingleOverlord)
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
            if(CurTexture != NULL && CurTexture->ID() == "Overlord")
            {
                ValidIndices.PushEnd(Index);
            }
        }
        if(ValidIndices.Length() == 0)
        {
            LogThreadEvent("No overlords found", LogError);
            return ProgramResultFail;
        }
        Vec2i ScreenCoord = FrameHUD.ScreenCoordFromBubblePortraitIndex(ValidIndices.RandomElement());
        LogThreadEvent(String("Left Click: ") + ScreenCoord.CommaSeparatedString(), LogStep);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        _CurState = ProgOverlordOrderOverlord;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgOverlordOrderOverlord)
    {
        if(FrameHUD.SoloPortrait() == NULL || FrameHUD.SoloPortrait()->ID() != "Overlord")
        {
            LogThreadEvent("Overlord not selected", LogError);
            return ProgramResultFail;
        }
        const Vec2f BaseLocation = g_Context->Managers.Knowledge.RandomOverlordScoutMinimapLocation();
        Vec2i ScreenCoord = g_Context->Constants.MinimapCoordToScreenCoord(BaseLocation);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonRight, ModifierNone);
        LogThreadEvent("Ordering overlord", LogStep);
        return ProgramResultSuccess;
    }
    HANDLE_CRITICAL_FAILURE;
}
