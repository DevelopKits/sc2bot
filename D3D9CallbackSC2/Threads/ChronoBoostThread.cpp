#include "Main.h"

void ChronoBoostThread::Reset()
{
    _WakeUpTime = GameTime() + 10.0;
    _Priority = 2;
    _Name = "ChronoBoost";
}

void ChronoBoostThread::Update()
{
    _ChronoBoost.Init();
    _ProgramToExecute = &_ChronoBoost;
    _CurAction = "ChronoBoost";
    _Priority = 2;
}

void ChronoBoostThread::ProgramSucceeded()
{
    _WakeUpTime = GameTime() + 5.0;
}

void ChronoBoostThread::ProgramFailed()
{
    _WakeUpTime = GameTime() + 5.0;
}