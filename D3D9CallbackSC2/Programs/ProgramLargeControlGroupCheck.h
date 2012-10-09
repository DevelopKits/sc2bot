//
// ProgramLargeControlGroupCheck.h
//
// Program to check a control group with more than 24 units
// 

enum ProgramLargeControlGroupCheckState
{
    ProgramLargeControlGroupCheckSelectGroup,
    ProgramLargeControlGroupCheckSelectLevel,
};

class ProgramLargeControlGroupCheck : public Program
{
public:
    void Init(ControlGroupType GroupType);
    
    ProgramResultType ExecuteStep();

private:
    UINT _CurLevel;
    ControlGroupType _GroupType;
    ProgramLargeControlGroupCheckState _CurState;
};
