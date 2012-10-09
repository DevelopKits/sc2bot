//
// ProgramGasWorker.h
//
// Program to tell gas workers to harvest from extractors
// 

enum ProgGasWorkerState
{
    ProgGasWorkerSelectExtractorGroup,
    ProgGasWorkerSelectExtractor,
    ProgGasWorkerCenterExtractor,
    ProgGasWorkerSelectGasDrones,
    ProgGasWorkerOrderGasDrones,
};

class ProgramGasWorker : public Program
{
public:
    void Init(UINT ExtractorIndex);
    
    ProgramResultType ExecuteStep();

private:
    UINT _ExtractorIndex;
    ProgGasWorkerState _CurState;
};
