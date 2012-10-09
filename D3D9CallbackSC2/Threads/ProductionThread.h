//
// ProductionThread.h
//
// AIThread responsible for producing units and buildings
// 

class ProductionThread : public AIThread
{
public:
    void Reset();
    void Update();
    void ProgramSucceeded();
    void ProgramFailed();
    String DescribeProductionGoals();

private:
    void UpdateProductionGoals();
    void UpdateProductionGoalsZerg();
    void UpdateProductionGoalsProtoss();
    bool AddBuildings();
    bool AddQueens();
    bool AddUnits();
    bool AddSupply();
    bool AddWorkers();

    bool AddUnitsZergDebug();
    bool AddBuildingsZergDebug();
    bool AddUnitsZergRoaches();
    bool AddBuildingsZergRoaches();
    bool AddUnitsZergHydralisks();
    bool AddBuildingsZergHydralisks();
    bool AddUnitsZergMutalisks();
    bool AddBuildingsZergMutalisks();
    bool AddUnitsZergZerglings();
    bool AddBuildingsZergZerglings();
    bool AddUnitsZergRoachesAndHydralisks();
    bool AddBuildingsZergRoachesAndHydralisks();
    
    bool AddUnitsProtossZealots();
    bool AddBuildingsProtossZealots();
    bool AddUnitsProtossStalkers();
    bool AddBuildingsProtossStalkers();
	bool AddUnitsProtossGroundHybrid();
    bool AddBuildingsProtossGroundHybrid();

    void AddSingletonBuilding(const String &Building, UINT SupplyRequirement, bool &BuildingAdded);
    void AddResearch(const String &Research, UINT SupplyRequirement, bool &ResearchAdded);
    void AddMultipleBuilding(const String &Building, UINT BuildingIndex, UINT SupplyRequirement, bool &BuildingAdded);
    void AddUnit(const String &Unit, UINT UnitCap, UINT SupplyRequirement, bool &UnitAdded);

    Vector<UnitEntry*> _ActiveProductionGoals;
    ProgramProduceUnitLarvae _Larvae;
    ProgramProduceUnitQueue _Queue;
    ProgramProduceUnitQueen _Queen;
    ProgramProduceUnitExtractor _Extractor;
    ProgramProduceUnitBase _Base;
    ProgramProduceUnitBuilding _Building;
    ProgramProduceUnitUpgrade _Upgrade;
    ProgramZergBaseUpgrade _ZergBaseUpgrade;
};
