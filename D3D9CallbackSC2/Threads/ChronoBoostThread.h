//
// ChronoBoostThread.h
//
// AIThread responsible for microing queens
// 

class ChronoBoostThread : public AIThread
{
public:
    void Reset();
    void Update();
    void ProgramSucceeded();
    void ProgramFailed();

private:
    ProgramChronoBoost _ChronoBoost;
};
