//
// ProgramArmy.h
//
// Program to order the army
// 

enum ProgArmyState
{
    ProgArmySelectArmy,
    ProgArmyOrderArmy,
    ProgArmySelectScoutGroup,
    ProgArmySelectScout,
    ProgArmySelectArmyForNewScout,
    ProgArmySelectNewScout,
    ProgArmyCenterScout,
    ProgArmyOrderScout,
};

class ProgramArmy : public Program
{
public:
    void Init();
    
    ProgramResultType ExecuteStep();

private:
    ProgArmyState _CurState;
};
