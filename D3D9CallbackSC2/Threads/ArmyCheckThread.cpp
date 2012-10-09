#include "Main.h"

void ArmyCheckThread::Reset()
{
    _WakeUpTime = GameTime() + 10.0;
    _Priority = 2;
    _Name = "ArmyCheck";
}

void ArmyCheckThread::Update()
{
    ControlGroupManager &ControlGroup = g_Context->Managers.ControlGroup;
    ControlGroupInfo &ArmyGroup = ControlGroup.FindControlGroup(ControlGroupCombatAll);
    if(ArmyGroup.ControlGroupLevel >= 2) // && g_Context->Managers.Knowledge.Army().State == ArmyStateIdleInBase
    {
        _LargeControlGroupCheck.Init(ControlGroupCombatAll);
        _ProgramToExecute = &_LargeControlGroupCheck;
        _CurAction = "ArmyCheck";
        _Priority = 2;
		return;
    }
}

void ArmyCheckThread::ProgramSucceeded()
{
    _WakeUpTime = GameTime() + 10.0;
}

void ArmyCheckThread::ProgramFailed()
{
    _WakeUpTime = GameTime() + 10.0;
}
