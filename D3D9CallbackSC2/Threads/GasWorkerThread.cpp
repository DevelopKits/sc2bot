#include "Main.h"

void GasWorkerThread::Reset()
{
    _WakeUpTime = GameTime() + 10.0;
    _Priority = 2;
    _Name = "GasWorker";
}

void GasWorkerThread::Update()
{
    KnowledgeManager &Knowledge = g_Context->Managers.Knowledge;
    Vector<ExtractorInfo>& Extractors = Knowledge.Extractors();
    for(UINT ExtractorIndex = 0; ExtractorIndex < Extractors.Length(); ExtractorIndex++)
    {
        if(GameTime() >= Extractors[ExtractorIndex].NextCheckTime)
        {
            _GasWorker.Init(ExtractorIndex);
            _ProgramToExecute = &_GasWorker;
            _CurAction = "GasWorker";
            _Priority = 3;
            return;
        }
    }
    
}

void GasWorkerThread::ProgramSucceeded()
{
    _WakeUpTime = GameTime() + 5.0;
}

void GasWorkerThread::ProgramFailed()
{
    _WakeUpTime = GameTime() + 5.0;
}