//
// ArmyThread.h
//
// AIThread responsible for moving the army
// 

class ArmyThread : public AIThread
{
public:
    void Reset();
    void Update();
    void ProgramSucceeded();
    void ProgramFailed();

private:
    ProgramArmy _Army;
};
