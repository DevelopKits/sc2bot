//
// ProgramProduceUnitUpgrade.h
//
// Program to research upgrades
// 

enum ProgramProduceUnitUpgradeState
{
    ProgramProduceUnitUpgradeSelectSupportBuildings,
    ProgramProduceUnitUpgradeSelectSupportBuilding,
    ProgramProduceUnitUpgradeResearch,
    ProgramProduceUnitUpgradeWaitForSuccess,
};

class ProgramProduceUnitUpgrade : public Program
{
public:
    void Init(UnitEntry *Entry);
    
    ProgramResultType ExecuteStep();

private:
    UnitEntry *_Entry;
    double _TimeoutTime;
    ProgramProduceUnitUpgradeState _CurState;
};
