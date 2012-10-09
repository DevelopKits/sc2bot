//
// WorkerThread.h
//
// AIThread responsible for idle workers
// 

class WorkerThread : public AIThread
{
public:
    void Reset();
    void Update();
    void ProgramSucceeded();
    void ProgramFailed();

private:
    ProgramIdleWorker _IdleWorker;
};
