#include "Main.h"

void MicroThread::Reset()
{
    _WakeUpTime = GameTime() + 10.0;
    _Priority = 2;
    _Name = "Micro";
}

void MicroThread::Update()
{
    ControlGroupManager &ControlGroup = g_Context->Managers.ControlGroup;
    ControlGroupInfo &MicroGroup = ControlGroup.FindControlGroup(ControlGroupCombatMicro);
	if(MicroGroup.LastObservedNonBaseUnits.Length() > 0 && g_Context->Managers.Minimap.Conflicted())
    {
        _MicroAll.Init();
        _ProgramToExecute = &_MicroAll;
        _CurAction = "MicroAll";
        _Priority = 1;
		return;
    }
}

void MicroThread::ProgramSucceeded()
{
    _WakeUpTime = GameTime() + 1.0;
}

void MicroThread::ProgramFailed()
{
    _WakeUpTime = GameTime() + 1.0;
}
