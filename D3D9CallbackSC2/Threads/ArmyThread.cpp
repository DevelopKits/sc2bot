#include "Main.h"

void ArmyThread::Reset()
{
    _WakeUpTime = GameTime() + 10.0;
    _Priority = 4;
    _Name = "Army";
}

void ArmyThread::Update()
{
    KnowledgeManager &Knowledge = g_Context->Managers.Knowledge;
    Vector<ExtractorInfo>& Extractors = Knowledge.Extractors();
    ArmyInfo &Army = Knowledge.Army();
    ControlGroupInfo &CurGroup = g_Context->Managers.ControlGroup.FindControlGroup(ControlGroupCombatAll);
    if(CurGroup.LastObservedNonBaseUnits.Length() > 0 && GameTime() >= Army.NextMacroTime)
    {
        _Army.Init();
        _ProgramToExecute = &_Army;
        _CurAction = "Army";
        _Priority = 4;
        return;
    }
}

void ArmyThread::ProgramSucceeded()
{
    _WakeUpTime = GameTime() + 2.0;
}

void ArmyThread::ProgramFailed()
{
    _WakeUpTime = GameTime() + 2.0;
}