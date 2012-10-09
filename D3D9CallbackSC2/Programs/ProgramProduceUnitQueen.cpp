#include "Main.h"

void ProgramProduceUnitQueen::Init()
{
    _CurState = ProgProduceUnitQueenSelectHatcheries;
    Name = String("Queen");
}

ProgramResultType ProgramProduceUnitQueen::ExecuteStep()
{
    FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    if(_CurState == ProgProduceUnitQueenSelectHatcheries)
    {
        LogThreadEvent("Selecting Hatcheries", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupBuildingMain));
        _CurState = ProgProduceUnitQueenSelectRandomHatchery;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitQueenSelectRandomHatchery)
    {
        ActionButtonStateType State;
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
        if(FrameHUD.SoloPortrait() == NULL)
        {
            Vector<UINT> ValidIndices;
            for(UINT Index = 0; Index < BubblePortraitCount; Index++)
            {
                Texture *CurTexture = FrameHUD.BubblePortraits(Index).Tex;
                if(CurTexture != NULL && CurTexture->Unit()->SecondaryType == UnitSecondaryBase)
                {
                    ValidIndices.PushEnd(Index);
                }
            }
            if(ValidIndices.Length() == 0)
            {
                LogThreadEvent("No bases found", LogError);
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
            LogThreadEvent("Single base already selected", LogStep);
        }
        _CurState = ProgProduceUnitQueenProduce;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitQueenProduce)
    {
        if(FrameHUD.BuildQueuePresent())
        {
            LogThreadEvent("Build queue already present", LogError);
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
            LogThreadEvent("Queen button in invalid state", LogError);
            return ProgramResultFail;
        }
        g_Context->Managers.KeyboardMouse.SendKey(KEY_Q);
        LogThreadEvent("Ordering queen", LogStep);
        _TimeoutTime = GameTime() + 1.0;
        _CurState = ProgProduceUnitQueenWaitForQueen;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitQueenWaitForQueen)
    {
        if(GameTime() > _TimeoutTime)
        {
            LogThreadEvent("Timed out waiting for queen", LogError);
            return ProgramResultFail;
        }
        if(FrameHUD.BuildQueueEntryPresent("Queen"))
        {
            LogThreadEvent("Queen found", LogDone);
            g_Context->ReportAction(String("Queen"), RGBColor::Green);
            g_Context->Managers.Knowledge.RecordUnitInProduction(g_Context->Managers.Database.GetUnitEntry("Queen"), GameTime());
            return ProgramResultSuccess;
        }
        else
        {
            LogThreadEvent("Waiting for queen", LogStep);
            return ProgramResultStillExecuting;
        }
    }
    HANDLE_CRITICAL_FAILURE;
}
