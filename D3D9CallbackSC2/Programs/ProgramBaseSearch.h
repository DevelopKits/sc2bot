//
// ProgramBaseSearch.h
//
// Program to investigate a random location on the minimap
// 

enum ProgBaseSearchState
{
    ProgBaseSearchSelectRegion,
    ProgBaseSearchSelectUnit,
    ProgBaseSearchAddToControlGroup,
};

class ProgramBaseSearch : public Program
{
public:
    void Init();
    
    ProgramResultType ExecuteStep();

private:
	double ComputeUnitValue(const UnitEntry *Entry) const;

    ProgBaseSearchState _CurState;
};
