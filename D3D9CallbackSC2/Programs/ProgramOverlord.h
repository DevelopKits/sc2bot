//
// ProgramOverlord.h
//
// Program to micro overlords
// 

enum ProgOverlordState
{
    ProgOverlordSelectOverlords,
    ProgOverlordSelectSingleOverlord,
    ProgOverlordOrderOverlord,
};

class ProgramOverlord : public Program
{
public:
    void Init();
    
    ProgramResultType ExecuteStep();

private:
    ProgOverlordState _CurState;
};
