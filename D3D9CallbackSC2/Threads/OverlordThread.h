//
// OverlordThread.h
//
// AIThread responsible for moving overlords around
// 

class OverlordThread : public AIThread
{
public:
    void Reset();
    void Update();
    void ProgramSucceeded();
    void ProgramFailed();

private:
    ProgramOverlord _Overlord;
};
