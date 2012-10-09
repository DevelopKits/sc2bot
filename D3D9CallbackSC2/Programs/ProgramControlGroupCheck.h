//
// ProgramProduceUnitQueen.h
//
// Program to produce queens
// 

class ProgramControlGroupCheck : public Program
{
public:
    void Init();
    
    ProgramResultType ExecuteStep();

private:
    int _ControlGroupIndex;
};
