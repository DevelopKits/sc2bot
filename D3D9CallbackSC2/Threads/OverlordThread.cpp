#include "Main.h"

void OverlordThread::Reset()
{
    _WakeUpTime = GameTime() + 5.0;
    _Priority = 2;
    _Name = "Overlord";
}

void OverlordThread::Update()
{
    _Overlord.Init();
    _ProgramToExecute = &_Overlord;
    _CurAction = "Overlord";
    _Priority = 2;
}

void OverlordThread::ProgramSucceeded()
{
    _WakeUpTime = GameTime() + 5.0;
}

void OverlordThread::ProgramFailed()
{
    _WakeUpTime = GameTime() + 5.0;
}