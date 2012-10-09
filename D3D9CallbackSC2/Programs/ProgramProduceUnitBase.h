//
// ProgramProduceUnitBase.h
//
// Program to produce bases
// 

enum ProgProduceUnitBaseState
{
    ProgProduceUnitBaseSelectWorkers,
    ProgProduceUnitBaseMoveToBuildingSite,
    ProgProduceUnitBaseSelectBuildStructure,
    ProgProduceUnitBaseSelectBuilding,
    ProgProduceUnitBaseFindBuildingPlacement,
    ProgProduceUnitBaseWaitForSuccess,
};

struct BaseInfo;

class ProgramProduceUnitBase : public Program
{
public:
    void Init(BaseInfo *Base);
    
    ProgramResultType ExecuteStep();

private:
    UINT _AttemptsMade;
    double _TimeoutTime0;
    double _TimeoutTime1;
    BaseInfo *_Base;
    ProgProduceUnitBaseState _CurState;
};
