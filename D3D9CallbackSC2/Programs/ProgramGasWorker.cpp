#include "Main.h"

void ProgramGasWorker::Init(UINT ExtractorIndex)
{
    _ExtractorIndex = ExtractorIndex;
    _CurState = ProgGasWorkerSelectExtractorGroup;
    Name = "GasWorker";
}

ProgramResultType ProgramGasWorker::ExecuteStep()
{
    const ControlGroupManager &ControlGroup = g_Context->Managers.ControlGroup;
    KnowledgeManager &Knowledge = g_Context->Managers.Knowledge;
    const FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    const FrameUnitManager &FrameUnit = g_Context->Managers.FrameUnit;
    if(_CurState == ProgGasWorkerSelectExtractorGroup)
    {
        Vector<ExtractorInfo> &Extractors = Knowledge.Extractors();
        if(_ExtractorIndex >= Extractors.Length())
        {
            LogThreadEvent("ExtractorIndex >= Extractors.Length()", LogError);
            return ProgramResultFail;
        }
        Extractors[_ExtractorIndex].NextCheckTime = GameTime() + 20.0;
        LogThreadEvent("Selecting Extractor Group", LogStep);
        //UINT ControlGroupIndex = ControlGroup.GasControlGroupIndexFromGasIndex(_ExtractorIndex);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + ControlGroup.FindControlGroupIndex(ControlGroupBuildingSupport));
        _CurState = ProgGasWorkerSelectExtractor;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgGasWorkerSelectExtractor)
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
            if(CurTexture != NULL && CurTexture->ID() == Knowledge.ExtractorUnit()->Name)
            {
                ValidIndices.PushEnd(Index);
            }
        }
        if(ValidIndices.Length() <= _ExtractorIndex)
        {
            LogThreadEvent("Not enough extractors found", LogError);
            return ProgramResultFail;
        }
        Vec2i ScreenCoord = FrameHUD.ScreenCoordFromBubblePortraitIndex(ValidIndices[_ExtractorIndex]);
        LogThreadEvent(String("Left Click: ") + ScreenCoord.CommaSeparatedString(), LogStep);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        _CurState = ProgGasWorkerCenterExtractor;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgGasWorkerCenterExtractor)
    {
        if(FrameHUD.SoloPortrait() == NULL || FrameHUD.SoloPortrait()->ID() != Knowledge.ExtractorUnit()->Name)
        {
            LogThreadEvent("Extractor not selected", LogError);
            return ProgramResultFail;
        }
        g_Context->Managers.KeyboardMouse.SendCtrlKey(KEY_F);
        LogThreadEvent("Centering on extractor", LogStep);
        _CurState = ProgGasWorkerSelectGasDrones;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgGasWorkerSelectGasDrones)
    {
        LogThreadEvent("Selecting Gas Group", LogStep);
        UINT GasControlGroupIndex = ControlGroup.GasControlGroupIndexFromGasIndex(_ExtractorIndex);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + ControlGroup.FindControlGroupIndex(ControlGroupBuildingMain));
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + GasControlGroupIndex);
        _CurState = ProgGasWorkerOrderGasDrones;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgGasWorkerOrderGasDrones)
    {
        Vector<const FrameUnitInfo*> AllClickableExtractors;
        for(UINT UnitIndex = 0; UnitIndex < FrameUnit.UnitsThisFrame().Length(); UnitIndex++)
        {
            const FrameUnitInfo &CurUnit = *FrameUnit.UnitsThisFrame()[UnitIndex];
            if(CurUnit.Owner == PlayerSelf && CurUnit.Entry->SecondaryType == UnitSecondaryExtractor && CurUnit.Clickable())
            {
                AllClickableExtractors.PushEnd(&CurUnit);
            }
        }
        if(AllClickableExtractors.Length() == 0)
        {
            LogThreadEvent("No clickable extractors", LogError);
            return ProgramResultFail;
        }
        struct ExtractorSorter
        {
            __forceinline bool operator()(const FrameUnitInfo *Left, const FrameUnitInfo *Right)
            {
                float LeftDistSq = Vec2f::DistSq(Left->ScreenBound.Center(), ScreenCenter);
                float RightDistSq = Vec2f::DistSq(Right->ScreenBound.Center(), ScreenCenter);
                return (LeftDistSq < RightDistSq);
            }
            Vec2f ScreenCenter;
        };
        ExtractorSorter Sorter;
        Sorter.ScreenCenter = Vec2f(g_Context->Constants.GetRectangle2iConstant(ScreenConstantRectangle2iClickableRegion).Center());
        AllClickableExtractors.Sort(Sorter);
        Vec2i ClickPos = AllClickableExtractors[0]->ScreenBound.Center().RoundToVec2i();
        g_Context->Managers.KeyboardMouse.SendKey(KEY_G);
        g_Context->Managers.KeyboardMouse.Click(ClickPos, MouseButtonLeft, ModifierNone);
        LogThreadEvent("Ordering Gas Group", LogDone);
        return ProgramResultSuccess;
    }
    HANDLE_CRITICAL_FAILURE;
}
