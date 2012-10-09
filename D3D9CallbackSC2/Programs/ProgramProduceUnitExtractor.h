//
// ProgramProduceUnitExtractor.h
//
// Program to produce extractors
// 

enum ProgProduceUnitExtractorState
{
    ProgProduceUnitExtractorSelectWorkers,
    ProgProduceUnitExtractorMoveToBuildingSite,
    ProgProduceUnitExtractorSelectBuildStructure,
    ProgProduceUnitExtractorSelectBuilding,
    ProgProduceUnitExtractorFindBuildingPlacement,
    ProgProduceUnitExtractorWaitForSuccess,
};

class ProgramProduceUnitExtractor : public Program
{
public:
    void Init();
    
    ProgramResultType ExecuteStep();

private:
    UINT _AttemptsMade;
    double _TimeoutTime0, _TimeoutTime1;
    ProgProduceUnitExtractorState _CurState;
};
