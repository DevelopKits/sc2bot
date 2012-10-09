//
// ProgramChronoBoost.h
//
// Program to micro queens
// 

enum ProgramChronoBoostState
{
    ProgramChronoBoostSelectSupportBuildings,
    ProgramChronoBoostSelectSupportBuilding,
    ProgramChronoBoostCenter,
    ProgramChronoBoostSelectNexuses,
    ProgramChronoBoostCastChronoBoost,
};

class ProgramChronoBoost : public Program
{
public:
    void Init();
    
    ProgramResultType ExecuteStep();

private:
    UnitEntry *_ChosenBuilding;
    ProgramChronoBoostState _CurState;
};
