#include "Main.h"

void ControlGroupCheckThread::Reset()
{
    _WakeUpTime = GameTime() + 2.0;
    _Priority = 3;
    _Name = "ControlGroupCheck";
}

void ControlGroupCheckThread::Update()
{
    _ControlGroupCheck.Init();
    _ProgramToExecute = &_ControlGroupCheck;
    _CurAction = "ControlGroupCheck";
    _Priority = 3;
}

void ControlGroupCheckThread::ProgramSucceeded()
{
    _WakeUpTime = GameTime() + 6.0;
}

void ControlGroupCheckThread::ProgramFailed()
{
    _WakeUpTime = GameTime() + 6.0;
}