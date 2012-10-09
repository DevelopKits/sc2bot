//
// ProgramQueen.h
//
// Program to micro queens
// 

enum ProgQueenState
{
    ProgQueenSelectQueens,
    ProgQueenSelectSingleQueen,
    ProgQueenOrderQueen,
};

class ProgramQueen : public Program
{
public:
    void Init();
    
    ProgramResultType ExecuteStep();

private:
    ProgQueenState _CurState;
};
