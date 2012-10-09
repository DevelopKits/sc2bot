//
// MicroThread.h
//
// AIThread responsible for microing the army
// 

class MicroThread : public AIThread
{
public:
    void Reset();
    void Update();
    void ProgramSucceeded();
    void ProgramFailed();

private:
    ProgramMicroAll _MicroAll;
};
