//
// ProgramProduceUnitLarvae.h
//
// Program to transform larvae
// 

enum ProgProduceUnitLarvaeState
{
    ProgProduceUnitLarvaeSelectHatcheries,
    ProgProduceUnitLarvaeSelectLarvae,
    ProgProduceUnitLarvaeOrderUnit,
    ProgProduceUnitLarvaeWaitForCocoon,
};

class ProgramProduceUnitLarvae : public Program
{
public:
    void Init(UnitEntry *Entry);
    
    ProgramResultType ExecuteStep();

private:
    double _TimeoutTime;
    UnitEntry *_Entry;
    ProgProduceUnitLarvaeState _CurState;
};
