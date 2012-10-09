//
// ProgramMicroAll.h
//
// Program to micromanage all units
// 

enum ProgramMicroAllState
{
    ProgramMicroAllSelectGroup,
    ProgramMicroAllSelectUnit,
    ProgramMicroAllCenterUnit,
    ProgramMicroAllOrderUnit,
};

class ProgramMicroAll : public Program
{
public:
    void Init();
    
    ProgramResultType ExecuteStep();

private:
    int _UnitIndex;
    ProgramMicroAllState _CurState;
};
