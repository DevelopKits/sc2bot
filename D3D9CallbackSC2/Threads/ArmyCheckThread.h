//
// ArmyCheckThread.h
//
// AIThread responsible for microing queens
// 

class ArmyCheckThread : public AIThread
{
public:
    void Reset();
    void Update();
    void ProgramSucceeded();
    void ProgramFailed();

private:
    ProgramLargeControlGroupCheck _LargeControlGroupCheck;
};
