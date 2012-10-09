//
// QueenThread.h
//
// AIThread responsible for microing queens
// 

class QueenThread : public AIThread
{
public:
    void Reset();
    void Update();
    void ProgramSucceeded();
    void ProgramFailed();

private:
    ProgramQueen _Queen;
};
