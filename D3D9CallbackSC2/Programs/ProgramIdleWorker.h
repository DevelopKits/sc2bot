//
// ProgramIdleWorker.h
//
// Program to do something with idle workers
// 

enum ProgIdleWorkerState
{
    ProgIdleWorkerSelectWorker,
    ProgIdleWorkerAssignAction,
};

class ProgramIdleWorker : public Program
{
public:
    void Init();
    
    ProgramResultType ExecuteStep();

private:
    ProgIdleWorkerState _CurState;
};
