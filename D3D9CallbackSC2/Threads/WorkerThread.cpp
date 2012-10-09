#include "Main.h"

void WorkerThread::Reset()
{
    _Priority = 0;
    _Name = "Worker";
}

void WorkerThread::Update()
{
    if(g_Context->Managers.FrameHUD.IdleWorker())
    {
        _IdleWorker.Init();
        _ProgramToExecute = &_IdleWorker;
        _CurAction = "IdleWorker";
        _Priority = 2;
    }
}

void WorkerThread::ProgramSucceeded()
{
    _WakeUpTime = GameTime() + 0.2f;
}

void WorkerThread::ProgramFailed()
{
    _WakeUpTime = GameTime() + 0.2f;
}
