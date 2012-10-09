//
// GasWorkerThread.h
//
// AIThread responsible for putting workers on gas
// 

class GasWorkerThread : public AIThread
{
public:
    void Reset();
    void Update();
    void ProgramSucceeded();
    void ProgramFailed();

private:
    ProgramGasWorker _GasWorker;
};
