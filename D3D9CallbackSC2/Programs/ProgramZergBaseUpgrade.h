//
// ProgramZergBaseUpgrade.h
//
// Program to do something with idle workers
// 

enum ProgramZergBaseUpgradeState
{
    ProgramZergBaseUpgradeSelectBaseLocation,
    ProgramZergBaseUpgradeSelectBase,
    ProgramZergBaseUpgradeUpgradeBase,
    ProgramZergBaseUpgradeWaitForSuccess,
};

class ProgramZergBaseUpgrade : public Program
{
public:
    void Init(UnitEntry *Entry);
    
    ProgramResultType ExecuteStep();

private:
    double _TimeoutTime;
    ProgramZergBaseUpgradeState _CurState;
    UnitEntry *_Entry;
};
