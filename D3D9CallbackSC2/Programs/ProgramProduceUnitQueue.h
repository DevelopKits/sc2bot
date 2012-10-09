//
// ProgramProduceUnitQueue.h
//
// Program to produce units in a standard unit queue
// 

enum ProgramProduceUnitQueueState
{
    ProgramProduceUnitQueueSelectSupportBuildings,
    ProgramProduceUnitQueueSelectSupportBuilding,
    ProgramProduceUnitQueueProduce,
    ProgramProduceUnitQueueWaitForUnit,

    ProgramProduceUnitQueueWarpGateMoveToSpawnPoint,
    ProgramProduceUnitQueueWarpGateSelectUnit,
    ProgramProduceUnitQueueWarpGateFindSpawnLocation,
};

class ProgramProduceUnitQueue : public Program
{
public:
    void Init(UnitEntry *Entry);
    
    ProgramResultType ExecuteStep();

private:
    UnitEntry *_Entry;
    double _TimeoutTime;
    ProgramProduceUnitQueueState _CurState;
};
