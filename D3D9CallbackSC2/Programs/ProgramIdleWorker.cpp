#include "Main.h"

void ProgramIdleWorker::Init()
{
    _CurState = ProgIdleWorkerSelectWorker;
    Name = "IdleWorker";
}

ProgramResultType ProgramIdleWorker::ExecuteStep()
{
    if(_CurState == ProgIdleWorkerSelectWorker)
    {
        LogThreadEvent("Selecting Idle Worker", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(VK_F1);
        _CurState = ProgIdleWorkerAssignAction;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgIdleWorkerAssignAction)
    {
        const FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
        const FrameUnitManager &FrameUnit = g_Context->Managers.FrameUnit;
        
        const Texture *CurPortrait = FrameHUD.SoloPortrait();
        if(CurPortrait == NULL || CurPortrait->Unit()->SecondaryType != UnitSecondaryWorker)
        {
            LogThreadEvent("Worker selection failed", LogError);
            return ProgramResultFail;
        }
        else
        {
            int NewGroupIndex = g_Context->Managers.ControlGroup.FindNewWorkerControlGroupIndex();
            if(NewGroupIndex != -1)
            {
                String Description = String("Worker added to group ") + String(NewGroupIndex);
                LogThreadEvent(Description, LogStep);
                //g_Context->ReportAction(Description, RGBColor::Blue);
                g_Context->Managers.ControlGroup.ReportWorkerAssigned(NewGroupIndex);
                g_Context->Managers.KeyboardMouse.SendShiftKey(KEY_0 + NewGroupIndex);
            }

            Vector<const FrameUnitInfo*> AllClickableMinerals;
            for(UINT UnitIndex = 0; UnitIndex < FrameUnit.UnitsThisFrame().Length(); UnitIndex++)
            {
                const FrameUnitInfo &CurUnit = *FrameUnit.UnitsThisFrame()[UnitIndex];
                if((CurUnit.Entry->Name == "MineralBlue" || CurUnit.Entry->Name == "MineralGold") && CurUnit.Clickable())
                {
                    AllClickableMinerals.PushEnd(&CurUnit);
                }
            }
            Vec2i ClickPos;
            if(AllClickableMinerals.Length() == 0)
            {
                const BaseInfo *NearestBase = g_Context->Managers.Knowledge.FindNearestBase(FrameHUD.MinimapViewportCenter(), PlayerSelf);
                if(NearestBase == NULL)
                {
                    LogThreadEvent(String("No allied bases"), LogError);
                    return ProgramResultFail;
                }
                else
                {
                    ClickPos = g_Context->Constants.MinimapCoordToScreenCoord(NearestBase->BaseLocation);
                    g_Context->Managers.KeyboardMouse.SendKey(KEY_M);
                    LogThreadEvent(String("Clicking on nearest base: ") + ClickPos.CommaSeparatedString(), LogDone);
                }
            }
            else
            {
                ClickPos = AllClickableMinerals.RandomElement()->ScreenBound.Center().RoundToVec2i();
                g_Context->Managers.KeyboardMouse.SendKey(KEY_G);
                LogThreadEvent(String("Clicking on minerals: ") + ClickPos.CommaSeparatedString(), LogDone);
            }
            g_Context->Managers.KeyboardMouse.Click(ClickPos, MouseButtonLeft, ModifierNone);
            return ProgramResultSuccess;
        }
    }
    HANDLE_CRITICAL_FAILURE;
}
