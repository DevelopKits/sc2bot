#include "Main.h"
#include "Strats.h"

void ProductionThread::Reset()
{
    _Priority = 0;
    _Name = "Production";
}

void ProductionThread::Update()
{
    UpdateProductionGoals();
    ControlGroupInfo &SupportGroup = g_Context->Managers.ControlGroup.FindControlGroup(ControlGroupBuildingSupport);
    KnowledgeManager &Knowledge = g_Context->Managers.Knowledge;
    for(UINT Index = 0; Index < _ActiveProductionGoals.Length(); Index++)
    {
        UnitEntry *CurEntry = _ActiveProductionGoals[Index];
        if(CurEntry == NULL)
        {
            g_Context->Files.Assert << "CurEntry == NULL\n";
            return;
        }
        if(g_Context->Managers.FrameHUD.HaveResourcesForUnit(*CurEntry))
        {
            if(CurEntry->BuildClass == BuildClassResearch)
            {
                _Upgrade.Init(CurEntry);
                _ProgramToExecute = &_Upgrade;
                _CurAction = String("Research") + CurEntry->Name;
                _Priority = 2;
                return;
            }
            if(CurEntry->BuildClass == BuildClassLarvae)
            {
                _Larvae.Init(CurEntry);
                _ProgramToExecute = &_Larvae;
                _CurAction = String("Produce") + CurEntry->Name;
                _Priority = 2;
                return;
            }
            if(CurEntry->BuildClass == BuildClassWorker)
            {
                if(CurEntry->SecondaryType == UnitSecondaryExtractor)
                {
                    _Extractor.Init();
                    _ProgramToExecute = &_Extractor;
                    _CurAction = String("ProduceExtractor");
                    _Priority = 2;
                    return;
                }
                else if(CurEntry->SecondaryType == UnitSecondaryBase)
                {
                    _Building.Init(CurEntry);
                    _ProgramToExecute = &_Building;
                    _CurAction = String("Produce") + CurEntry->Name;
                    _Priority = 2;
                    return;
                }
                else
                {
                    _Building.Init(CurEntry);
                    _ProgramToExecute = &_Building;
                    _CurAction = String("Produce") + CurEntry->Name;
                    _Priority = 2;
                    return;
                }
            }
            if(CurEntry->BuildClass == BuildClassMorphBuilding)
            {
                if(CurEntry->Name == "Lair" || CurEntry->Name == "Hive")
                {
                    _ZergBaseUpgrade.Init(CurEntry);
                    _ProgramToExecute = &_ZergBaseUpgrade;
                    _CurAction = String("Upgrade") + CurEntry->Name;
                    _Priority = 2;
                    return;
                }
            }
            if(CurEntry->BuildClass == BuildClassQueue)
            {
                //
                // This is flawed; units in production != available production structures
                //
                int ProductionBuildingCount = SupportGroup.CountPrerequisiteUnitsMatchingName(CurEntry->BuiltAt);
                int UnitsInProduction = Knowledge.CountUnitsInProduction(CurEntry->Name);
                if(ProductionBuildingCount - UnitsInProduction > 0)
                {
                    if(CurEntry->Name == "Queen")
                    {
                        _Queen.Init();
                        _ProgramToExecute = &_Queen;
                        _CurAction = "ProduceQueen";
                        _Priority = 2;
                        return;
                    }
                    else
                    {
                        _Queue.Init(CurEntry);
                        _ProgramToExecute = &_Queue;
                        _CurAction = "ProduceQueue";
                        _Priority = 2;
                        return;
                    }
                }
            }
        }
    }
}

UINT SupplyForesightFromSupplyCap(UINT SupplyCap)
{
    if(SupplyCap >= 100)
    {
        return 12;
    }
    if(SupplyCap >= 50)
    {
        return 8;
    }
    if(SupplyCap >= 30)
    {
        return 6;
    }
    if(SupplyCap >= 25)
    {
        return 4;
    }
    if(SupplyCap >= 18)
    {
        return 2;
    }
    if(g_Context->Managers.Knowledge.MyRace() == RaceProtoss)
    {
        return 1;
    }
    return 0;
}

UINT SupplyProductionCapFromSupplyCap(UINT SupplyCap)
{
    if(SupplyCap >= 100)
    {
        return 4;
    }
    if(SupplyCap >= 75)
    {
        return 3;
    }
    if(SupplyCap >= 50)
    {
        return 2;
    }
    if(SupplyCap >= 40)
    {
        return 1;
    }
    return 1;
}

void ProductionThread::AddMultipleBuilding(const String &Building, UINT BuildingIndex, UINT SupplyRequirement, bool &BuildingAdded)
{
    if(g_Context->Managers.FrameHUD.SupplyCurrent() < SupplyRequirement || BuildingAdded)
    {
        return;
    }
    UnitEntry *Entry = g_Context->Managers.Database.GetUnitEntry(Building);
    if(Entry == NULL)
    {
        g_Context->Files.Assert << "UnitEntry not found: " << Building << endl;
        return;
    }
    ControlGroupInfo &SupportGroup = g_Context->Managers.ControlGroup.FindControlGroup(ControlGroupBuildingSupport);
    if(Entry->Requirement != "None")
    {
        UINT PrerequisiteCount = SupportGroup.CountPrerequisiteUnitsMatchingName(Entry->Requirement);
        if(PrerequisiteCount == 0)
        {
            return;
        }
    }
    const UINT BuildingCount = SupportGroup.CountPrerequisiteUnitsMatchingName(Building);
    const UINT BuildingInProduction = g_Context->Managers.Knowledge.CountUnitsInProduction(Building);
    if(BuildingCount + BuildingInProduction >= BuildingIndex + 1)
    {
        return;
    }
    _ActiveProductionGoals.PushEnd(Entry);
    BuildingAdded = true;
}

void ProductionThread::AddResearch(const String &Research, UINT SupplyRequirement, bool &ResearchAdded)
{
    if(g_Context->Managers.FrameHUD.SupplyCurrent() < SupplyRequirement || ResearchAdded)
    {
        return;
    }
    UnitEntry *Entry = g_Context->Managers.Database.GetUnitEntry(Research);
    if(Entry == NULL)
    {
        return;
    }
    if(g_Context->Managers.FrameHUD.Gas() < int(Entry->GasCost))
    {
        return;
    }
    ControlGroupInfo &SupportGroup = g_Context->Managers.ControlGroup.FindControlGroup(ControlGroupBuildingSupport);
    if(Entry->Requirement != "None")
    {
        const UINT PrerequisiteCount = SupportGroup.CountPrerequisiteUnitsMatchingName(Entry->Requirement);
        if(PrerequisiteCount == 0)
        {
            return;
        }
    }
    if(Entry->BuiltAt != "None")
    {
        const UINT BuildAtCount = SupportGroup.CountPrerequisiteUnitsMatchingName(Entry->BuiltAt);
        if(BuildAtCount == 0)
        {
            return;
        }
    }
    const bool ResearchAlreadyDone = g_Context->Managers.Knowledge.ResearchCompleted(Research);
    if(ResearchAlreadyDone)
    {
        return;
    }
    _ActiveProductionGoals.PushEnd(Entry);
    ResearchAdded = true;
}

void ProductionThread::AddSingletonBuilding(const String &Building, UINT SupplyRequirement, bool &BuildingAdded)
{
    if(g_Context->Managers.FrameHUD.SupplyCurrent() < SupplyRequirement || BuildingAdded)
    {
        return;
    }
    UnitEntry *Entry = g_Context->Managers.Database.GetUnitEntry(Building);
    if(Entry == NULL)
    {
        g_Context->Files.Assert << "UnitEntry not found: " << Building << endl;
        return;
    }
    ControlGroupInfo &SupportGroup = g_Context->Managers.ControlGroup.FindControlGroup(ControlGroupBuildingSupport);
    if(Entry->Requirement != "None")
    {
        const UINT PrerequisiteCount = SupportGroup.CountPrerequisiteUnitsMatchingName(Entry->Requirement);
        if(PrerequisiteCount == 0)
        {
            return;
        }
    }
    if(Entry->BuiltAt != "None")
    {
        const UINT BuildAtCount = SupportGroup.CountPrerequisiteUnitsMatchingName(Entry->BuiltAt);
        if(BuildAtCount == 0)
        {
            return;
        }
    }
    if(Entry->Name == "Lair")
    {
        if(SupportGroup.CountUnitsMatchingName("Hive") > 0)
        {
            return;
        }
    }
    if(Entry->Name == "Spire")
    {
        if(SupportGroup.CountUnitsMatchingName("GreaterSpire") > 0)
        {
            return;
        }
    }
    const UINT BuildingCount = SupportGroup.CountUnitsMatchingName(Building);
    const UINT BuildingInProduction = g_Context->Managers.Knowledge.CountUnitsInProduction(Building);
    if(BuildingCount >= 1 || BuildingInProduction >= 1)
    {
        return;
    }
    _ActiveProductionGoals.PushEnd(Entry);
    BuildingAdded = true;
}

bool ProductionThread::AddBuildings()
{
    switch(g_Context->Managers.Knowledge.Strat())
    {
    case StratZergDebug:
        return AddBuildingsZergDebug();
    case StratZergRoaches:
        return AddBuildingsZergRoaches();
    case StratZergMutalisks:
        return AddBuildingsZergMutalisks();
    case StratZergZerglings:
        return AddBuildingsZergZerglings();
    case StratZergRoachesAndHydralisks:
        return AddBuildingsZergRoachesAndHydralisks();
    case StratProtossZealots:
        return AddBuildingsProtossZealots();
	case StratProtossStalkers:
        return AddBuildingsProtossStalkers();
	case StratProtossGroundHybrid:
		return AddBuildingsProtossGroundHybrid();
    default:
        return AddBuildingsZergDebug();
    }
}

bool ProductionThread::AddUnits()
{
    switch(g_Context->Managers.Knowledge.Strat())
    {
    case StratZergDebug:
        return AddUnitsZergDebug();
    case StratZergRoaches:
        return AddUnitsZergRoaches();
    case StratZergMutalisks:
        return AddUnitsZergMutalisks();
    case StratZergZerglings:
        return AddUnitsZergZerglings();
    case StratZergRoachesAndHydralisks:
        return AddUnitsZergRoachesAndHydralisks();
    case StratProtossZealots:
        return AddUnitsProtossZealots();
    case StratProtossStalkers:
        return AddUnitsProtossStalkers();
	case StratProtossGroundHybrid:
		return AddUnitsProtossGroundHybrid();
    default:
        return AddUnitsZergDebug();
    }
}

void ProductionThread::UpdateProductionGoals()
{
    _ActiveProductionGoals.FreeMemory();
    
    if(g_Context->Managers.ControlGroup.WorkerCount() < 7)
    {
        _ActiveProductionGoals.PushEnd(g_Context->Managers.Knowledge.WorkerUnit());
        _ActiveProductionGoals.PushEnd(g_Context->Managers.Knowledge.SupplyUnit());
        return;
    }

    switch(g_Context->Managers.Knowledge.MyRace())
    {
    case RaceZerg:
        UpdateProductionGoalsZerg();
        break;
    case RaceProtoss:
        UpdateProductionGoalsProtoss();
        break;
    }
}

void ProductionThread::UpdateProductionGoalsZerg()
{
    bool QueenAdded = AddQueens();
    if(!QueenAdded)
    {
        bool BuildingAdded = AddBuildings();
        if(!BuildingAdded)
        {
            bool UnitsAdded = AddUnits();
            bool SupplyAdded = AddSupply();
            bool WorkersAdded = AddWorkers();
        }
    }
}

void ProductionThread::UpdateProductionGoalsProtoss()
{
    bool BuildingAdded = AddBuildings();
    bool SupplyAdded = AddSupply();
    if(!BuildingAdded)
    {
        bool WorkersAdded = AddWorkers();
        bool UnitsAdded = AddUnits();
    }
}

String ProductionThread::DescribeProductionGoals()
{
    String Result = "Goals: ";
    for(UINT Index = 0; Index < _ActiveProductionGoals.Length(); Index++)
    {
        Result += _ActiveProductionGoals[Index]->Name;
        if(Index != _ActiveProductionGoals.Length() - 1)
        {
            Result.PushEnd(',');
        }
    }
    return Result;
}

void ProductionThread::AddUnit(const String &Unit, UINT UnitCap, UINT SupplyRequirement, bool &UnitAdded)
{
	KnowledgeManager &Knowledge = g_Context->Managers.Knowledge;

    if(g_Context->Managers.FrameHUD.SupplyCurrent() < SupplyRequirement || UnitAdded)
    {
        return;
    }
    UnitEntry *Entry = g_Context->Managers.Database.GetUnitEntry(Unit);
    if(Entry == NULL)
    {
        g_Context->Files.Assert << "UnitEntry not found: " << Unit << endl;
        return;
    }
    if(g_Context->Managers.FrameHUD.Gas() < int(Entry->GasCost))
    {
        return;
    }
    ControlGroupInfo &SupportGroup = g_Context->Managers.ControlGroup.FindControlGroup(ControlGroupBuildingSupport);
    ControlGroupInfo &UnitGroup = g_Context->Managers.ControlGroup.FindControlGroup(*Entry);
    if(Entry->Requirement != "None")
    {
        const UINT PrerequisiteCount = SupportGroup.CountPrerequisiteUnitsMatchingName(Entry->Requirement);
        if(PrerequisiteCount == 0)
        {
            return;
        }
    }
    if(Entry->BuiltAt != "None")
    {
        UnitEntry *BuildAtEntry = g_Context->Managers.Database.GetUnitEntry(Entry->BuiltAt);
        if(BuildAtEntry != NULL)
        {
            ControlGroupInfo &BuildAtUnitGroup = g_Context->Managers.ControlGroup.FindControlGroup(*BuildAtEntry);
            const UINT BuildAtCount = BuildAtUnitGroup.CountPrerequisiteUnitsMatchingName(Entry->BuiltAt);
            if(BuildAtCount == 0)
            {
                return;
            }
        }
    }
    const UINT UnitCount = UnitGroup.CountUnitsMatchingName(Unit);
    const UINT UnitInProduction = g_Context->Managers.Knowledge.CountUnitsInProduction(Unit);
    if(UnitCount + UnitInProduction >= UnitCap)
    {
        return;
    }
	if(Entry->BuildClass == BuildClassQueue)
    {
        //
        // This is flawed; units in production != available production structures
        //
        int ProductionBuildingCount = SupportGroup.CountPrerequisiteUnitsMatchingName(Entry->BuiltAt);
        int UnitsInProduction = Knowledge.CountUnitsInProduction(Entry->Name);
        if(ProductionBuildingCount - UnitsInProduction <= 0)
		{
			return;
		}
	}
    _ActiveProductionGoals.PushEnd(Entry);
    UnitAdded = true;
}

bool ProductionThread::AddQueens()
{
    if(g_Context->Managers.Knowledge.MyRace() != RaceZerg)
    {
        return false;
    }
    ControlGroupManager &ControlGroup = g_Context->Managers.ControlGroup;
    UINT TotalBaseCount = ControlGroup.FindControlGroup(ControlGroupBuildingMain).LastObservedBaseUnits.Length();
    UINT TotalQueenCount = ControlGroup.FindControlGroup(ControlGroupBuildingSupport).CountUnitsMatchingName("Queen");
    UINT TotalSpawningPoolCount = ControlGroup.FindControlGroup(ControlGroupBuildingSupport).CountUnitsMatchingName("SpawningPool");
    if(TotalSpawningPoolCount >= 1 &&
        g_Context->Managers.FrameHUD.SupplyCurrent() >= 16 &&
        g_Context->Managers.Knowledge.CountUnitsInProduction("Queen") == 0 &&
        TotalQueenCount < TotalBaseCount)
    {
        _ActiveProductionGoals.PushEnd(g_Context->Managers.Database.GetUnitEntry("Queen"));
        return true;
    }
    return false;
}

bool ProductionThread::AddSupply()
{
    RaceType MyRace = g_Context->Managers.Knowledge.MyRace();

    UINT SupplyCurrent = g_Context->Managers.FrameHUD.SupplyCurrent();
    UINT SupplyCap = g_Context->Managers.FrameHUD.SupplyCap();
    UINT SupplyForesight = SupplyForesightFromSupplyCap(SupplyCap);
    UINT SupplyProductionCap = SupplyProductionCapFromSupplyCap(SupplyCap);
    if(SupplyCurrent + SupplyForesight >= SupplyCap)
    {
        UINT SupplyInProduction = g_Context->Managers.Knowledge.CountUnitsInProduction(g_Context->Managers.Knowledge.SupplyUnit()->Name);
        if(SupplyInProduction < SupplyProductionCap)
        {
            _ActiveProductionGoals.PushEnd(g_Context->Managers.Knowledge.SupplyUnit());
            return true;
        }
    }
    return false;
}

bool ProductionThread::AddWorkers()
{
    UINT WorkerCap = Math::Min(UINT(50), g_Context->Managers.Knowledge.MyBaseCount() * 24);
    if(g_Context->Managers.ControlGroup.WorkerCount() < WorkerCap)
    {
        _ActiveProductionGoals.PushEnd(g_Context->Managers.Knowledge.WorkerUnit());
        return true;
    }
    return false;
}

void ProductionThread::ProgramSucceeded()
{
    _WakeUpTime = GameTime() + 0.2f;
}

void ProductionThread::ProgramFailed()
{
    _WakeUpTime = GameTime() + 1.0f;
}
