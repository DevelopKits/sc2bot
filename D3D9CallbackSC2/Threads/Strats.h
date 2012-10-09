bool ProductionThread::AddBuildingsZergDebug()
{
    bool BuildingAdded = false;
    AddSingletonBuilding("Lair", 8, BuildingAdded);
    AddSingletonBuilding("Hive", 10, BuildingAdded);
    return BuildingAdded;
}

bool ProductionThread::AddUnitsZergDebug()
{
    bool UnitAdded = false;
    AddUnit("Roach", 100, 12, UnitAdded);
    AddUnit("Zergling", 4, 12, UnitAdded);
    return UnitAdded;
}

bool ProductionThread::AddBuildingsZergRoaches()
{
    bool BuildingAdded = false;
    AddSingletonBuilding("SpawningPool", 14, BuildingAdded);
    AddMultipleBuilding("Extractor", 0, 15, BuildingAdded);
    AddSingletonBuilding("RoachWarren", 16, BuildingAdded);
    AddSingletonBuilding("Lair", 30, BuildingAdded);
    AddResearch("RoachSpeed", 31, BuildingAdded);
    AddMultipleBuilding("Extractor", 1, 36, BuildingAdded);
    AddSingletonBuilding("EvolutionChamber", 36, BuildingAdded);
    AddResearch("ZergRangedAttack", 36, BuildingAdded);
    //AddResearch("RoachRegen", 28, BuildingAdded);
    //AddSingletonBuilding("EvolutionChamber", 45, BuildingAdded);
    //AddSingletonBuilding("InfestationPit", 45, BuildingAdded);
    //AddSingletonBuilding("Hive", 50, BuildingAdded);
    return BuildingAdded;
}

bool ProductionThread::AddUnitsZergRoaches()
{
    bool UnitAdded = false;
    AddUnit("Zergling", 4, 12, UnitAdded);
    AddUnit("Roach", 100, 12, UnitAdded);
    return UnitAdded;
}

bool ProductionThread::AddBuildingsZergZerglings()
{
    bool BuildingAdded = false;
    AddSingletonBuilding("SpawningPool", 13, BuildingAdded);
    AddMultipleBuilding("Extractor", 0, 16, BuildingAdded);
    AddResearch("MetabolicBoost", 18, BuildingAdded);
    AddMultipleBuilding("Hatchery", 1, 20, BuildingAdded);
    AddSingletonBuilding("EvolutionChamber", 26, BuildingAdded);
    AddResearch("ZergMeleeAttack", 27, BuildingAdded);
    return BuildingAdded;
}

bool ProductionThread::AddUnitsZergZerglings()
{
    bool UnitAdded = false;
    AddUnit("Zergling", 400, 22, UnitAdded);
    return UnitAdded;
}

bool ProductionThread::AddBuildingsZergRoachesAndHydralisks()
{
    bool BuildingAdded = false;
    AddSingletonBuilding("SpawningPool", 14, BuildingAdded);
    AddMultipleBuilding("Extractor", 0, 15, BuildingAdded);
    AddSingletonBuilding("RoachWarren", 16, BuildingAdded);
    AddSingletonBuilding("Lair", 30, BuildingAdded);
    AddMultipleBuilding("Extractor", 1, 36, BuildingAdded);
    AddSingletonBuilding("EvolutionChamber", 45, BuildingAdded);
    AddSingletonBuilding("InfestationPit", 45, BuildingAdded);
    AddSingletonBuilding("Hive", 50, BuildingAdded);
    return BuildingAdded;
}

bool ProductionThread::AddUnitsZergRoachesAndHydralisks()
{
    bool UnitAdded = false;
    AddUnit("Roach", 100, 12, UnitAdded);
    AddUnit("Zergling", 4, 12, UnitAdded);
    return UnitAdded;
}

bool ProductionThread::AddBuildingsZergMutalisks()
{
    bool BuildingAdded = false;
    AddSingletonBuilding("SpawningPool", 14, BuildingAdded);
    AddMultipleBuilding("Extractor", 0, 15, BuildingAdded);
    AddMultipleBuilding("Extractor", 1, 18, BuildingAdded);
    AddSingletonBuilding("Lair", 19, BuildingAdded);
    AddSingletonBuilding("Spire", 20, BuildingAdded);
    return BuildingAdded;
}

bool ProductionThread::AddUnitsZergMutalisks()
{
    bool UnitAdded = false;
    if(g_Context->Managers.FrameHUD.Gas() >= 100)
    {
        AddUnit("Mutalisk", 100, 12, UnitAdded);
    }
    AddUnit("Zergling", 6, 15, UnitAdded);
    AddUnit("Zergling", 80, 32, UnitAdded);
    return UnitAdded;
}

bool ProductionThread::AddBuildingsProtossZealots()
{
    bool BuildingAdded = false;
    AddMultipleBuilding("Gateway", 0, 11, BuildingAdded);
    AddMultipleBuilding("Gateway", 1, 13, BuildingAdded);
    AddMultipleBuilding("Gateway", 2, 18, BuildingAdded);
    AddMultipleBuilding("Gateway", 3, 26, BuildingAdded);
    //AddMultipleBuilding("Assimilator", 0, 15, BuildingAdded);
    //AddMultipleBuilding("Assimilator", 1, 18, BuildingAdded);
    return BuildingAdded;
}

bool ProductionThread::AddUnitsProtossZealots()
{
    bool UnitAdded = false;
    AddUnit("Zealot", 100, 15, UnitAdded);
    return UnitAdded;
}

bool ProductionThread::AddBuildingsProtossStalkers()
{
    bool BuildingAdded = false;
    AddResearch("ResearchWarpGate", 8, BuildingAdded);
	AddResearch("Blink", 8, BuildingAdded);
    AddMultipleBuilding("Gateway", 0, 11, BuildingAdded);
    AddSingletonBuilding("CyberneticsCore", 15, BuildingAdded);
    AddMultipleBuilding("Assimilator", 0, 16, BuildingAdded);
    AddMultipleBuilding("Gateway", 1, 18, BuildingAdded);
    AddMultipleBuilding("Assimilator", 1, 23, BuildingAdded);
	AddSingletonBuilding("TwilightCouncil", 31, BuildingAdded);
    AddMultipleBuilding("Gateway", 2, 32, BuildingAdded);
    AddMultipleBuilding("Gateway", 3, 42, BuildingAdded);
    return BuildingAdded;
}

bool ProductionThread::AddUnitsProtossStalkers()
{
    bool UnitAdded = false;
    AddUnit("Sentry", 1, 15, UnitAdded);
    AddUnit("Stalker", 100, 15, UnitAdded);
    AddUnit("Zealot", 100, 15, UnitAdded);
	return UnitAdded;
}

bool ProductionThread::AddBuildingsProtossGroundHybrid()
{
    bool BuildingAdded = false;
    AddResearch("ResearchWarpGate", 7, BuildingAdded);
    AddMultipleBuilding("Gateway", 0, 11, BuildingAdded);
	AddMultipleBuilding("Assimilator", 0, 14, BuildingAdded);
    AddSingletonBuilding("CyberneticsCore", 15, BuildingAdded);
    AddMultipleBuilding("Gateway", 1, 16, BuildingAdded);
    AddMultipleBuilding("Assimilator", 1, 22, BuildingAdded);
    AddSingletonBuilding("RoboticsFacility", 30, BuildingAdded);
    AddMultipleBuilding("Gateway", 2, 36, BuildingAdded);
    return BuildingAdded;
}

bool ProductionThread::AddUnitsProtossGroundHybrid()
{
    bool UnitAdded = false;
	AddUnit("Sentry", 1, 15, UnitAdded);
	AddUnit("Immortal", 10, 15, UnitAdded);
	AddUnit("Stalker", 100, 15, UnitAdded);
	AddUnit("Zealot", 100, 15, UnitAdded);
	return UnitAdded;
}