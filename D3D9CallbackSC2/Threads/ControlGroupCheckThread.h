//
// ControlGroupCheckThread.h
//
// AIThread responsible for checking all the control groups
// 

class ControlGroupCheckThread : public AIThread
{
public:
    void Reset();
    void Update();
    void ProgramSucceeded();
    void ProgramFailed();

private:
    ProgramControlGroupCheck _ControlGroupCheck;
};
