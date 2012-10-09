#include "Main.h"

void QueenThread::Reset()
{
    _WakeUpTime = GameTime() + 5.0;
    _Priority = 2;
    _Name = "Queen";
}

void QueenThread::Update()
{
    ControlGroupManager &ControlGroup = g_Context->Managers.ControlGroup;
    if(ControlGroup.ControlGroups(ControlGroup.FindControlGroupIndex(ControlGroupBuildingSupport)).CountUnitsMatchingName("Queen") > 0)
    {
        _Queen.Init();
        _ProgramToExecute = &_Queen;
        _CurAction = "Queen";
        _Priority = 2;
		return;
    }
}

void QueenThread::ProgramSucceeded()
{
    _WakeUpTime = GameTime() + 1.5;
}

void QueenThread::ProgramFailed()
{
    _WakeUpTime = GameTime() + 1.5;
}