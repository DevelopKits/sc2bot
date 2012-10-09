//
// ProgramProduceUnitBuilding.h
//
// Program to produce buildings
// 

enum ProgProduceUnitBuildingState
{
    ProgProduceUnitBuildingSelectWorkers,
    ProgProduceUnitBuildingMoveToBuildingSite,
    ProgProduceUnitBuildingSelectBuildStructure,
    ProgProduceUnitBuildingSelectBuilding,
    ProgProduceUnitBuildingFindBuildingPlacement,
    ProgProduceUnitBuildingWaitForSuccess,
};

class ProgramProduceUnitBuilding : public Program
{
public:
    void Init(UnitEntry *Entry);
    
    ProgramResultType ExecuteStep();

private:
    UINT _AttemptsMade;
    double _TimeoutTime0;
    double _TimeoutTime1;
    UnitEntry *_Entry;
    ProgProduceUnitBuildingState _CurState;
};
