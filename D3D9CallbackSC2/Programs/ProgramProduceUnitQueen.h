//
// ProgramProduceUnitQueen.h
//
// Program to produce queens
// 

enum ProgProduceUnitQueenState
{
    ProgProduceUnitQueenSelectHatcheries,
    ProgProduceUnitQueenSelectRandomHatchery,
    ProgProduceUnitQueenProduce,
    ProgProduceUnitQueenWaitForQueen,
};

class ProgramProduceUnitQueen : public Program
{
public:
    void Init();
    
    ProgramResultType ExecuteStep();

private:
    double _TimeoutTime;
    ProgProduceUnitQueenState _CurState;
};
