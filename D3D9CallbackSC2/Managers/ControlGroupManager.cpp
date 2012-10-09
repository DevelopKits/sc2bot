//
// ControlGroupManager.cpp
//
// Handles all control group functions.
// Written by Matthew Fisher
//
#include "Main.h"

UINT ControlGroupUnitInfo::ComputeResourceValue(const Vector<ControlGroupUnitInfo> &Units)
{
    UINT Result = 0;
    for(UINT UnitIndex = 0; UnitIndex < Units.Length(); UnitIndex++)
    {
        UnitEntry *CurEntry = Units[UnitIndex].Entry;
        if(CurEntry->Name == "Zergling")
        {
            Result += 25;
        }
        else
        {
            Result += CurEntry->MineralCost + CurEntry->GasCost;
        }
    }
    return Result;
}

String ControlGroupUnitInfo::Describe(const Vector<ControlGroupUnitInfo> &Units)
{
    map<UnitEntry*, UINT> UnitCounts;
    for(UINT UnitIndex = 0; UnitIndex < Units.Length(); UnitIndex++)
    {
        UnitEntry *CurEntry = Units[UnitIndex].Entry;
        if(UnitCounts.find(CurEntry) == UnitCounts.end())
        {
            UnitCounts[CurEntry] = 1;
        }
        else
        {
            UnitCounts[CurEntry]++;
        }
    }
    String Result;
    for(map<UnitEntry*, UINT>::const_iterator Iterator = UnitCounts.begin(); Iterator != UnitCounts.end(); Iterator++)
    {
        UnitEntry *CurEntry = Iterator->first;
        UINT Count = Iterator->second;
        Result += CurEntry->Name + String(Count) + String(' ');
    }
    return Result;
}
void ControlGroupInfo::Reset()
{
    Type = ControlGroupUnused;
    WorkersAssigned = 0;
    ControlGroupLevel = 0;
    LastObservedNonBaseUnits.FreeMemory();
    LastObservedBaseUnits.FreeMemory();
    LastObservedAllLevelUnits.FreeMemory();
}

void ControlGroupInfo::UpdateFromHUD()
{
    LastObservedNonBaseUnits.FreeMemory();
    LastObservedBaseUnits.FreeMemory();
    const FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    UINT ExtractorCount = 0;
    for(UINT Index = 0; Index < BubblePortraitCount; Index++)
    {
        Texture *Tex = FrameHUD.BubblePortraits(Index).Tex;
        if(Tex != NULL)
        {
            UnitEntry *Entry = Tex->Unit();
            if(Entry != NULL)
            {
                InsertUnit(Entry);
                if(Entry->Name == g_Context->Managers.Knowledge.ExtractorUnit()->Name)
                {
                    ExtractorCount++;
                }
            }
        }
    }
    Texture *Tex = FrameHUD.SoloPortrait();
    if(Tex != NULL)
    {
        UnitEntry *Entry = Tex->Unit();
        if(Entry != NULL)
        {
            InsertUnit(Entry);
        }
    }
    if(Type == ControlGroupBuildingSupport)
    {
        g_Context->Managers.Knowledge.ReportExtractorCount(ExtractorCount);
    }
    ControlGroupLevel = FrameHUD.ControlGroupLevel();
    if(ControlGroupLevel < 2)
    {
        LastObservedAllLevelUnits.FreeMemory();
        if(Type == ControlGroupCombatAll)
        {
            g_Context->Managers.Knowledge.Army().ResourceValue = ControlGroupUnitInfo::ComputeResourceValue(LastObservedNonBaseUnits);
        }
    }
}

void ControlGroupInfo::AppendAllFromHUD()
{
    const FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    for(UINT Index = 0; Index < BubblePortraitCount; Index++)
    {
        Texture *Tex = FrameHUD.BubblePortraits(Index).Tex;
        if(Tex != NULL)
        {
            UnitEntry *Entry = Tex->Unit();
            if(Entry != NULL && Entry->SecondaryType != UnitSecondaryBase)
            {
                LastObservedAllLevelUnits.PushEnd(Entry);
            }
        }
    }
}

void ControlGroupInfo::InsertUnit(UnitEntry *Entry)
{
    if(Entry->SecondaryType == UnitSecondaryBase)
    {
        LastObservedBaseUnits.PushEnd(Entry);
    }
    else
    {
        LastObservedNonBaseUnits.PushEnd(Entry);
    }
}

UINT ControlGroupInfo::CountPrerequisiteUnitsMatchingName(const String &Name) const
{
    if(Name == "Hatchery")
    {
        return CountUnitsMatchingName("Hatchery") + CountUnitsMatchingName("Lair") + CountUnitsMatchingName("Hive");
    }
    if(Name == "Lair")
    {
        return CountUnitsMatchingName("Lair") + CountUnitsMatchingName("Hive");
    }
    if(Name == "Gateway")
    {
        return CountUnitsMatchingName("Gateway") + CountUnitsMatchingName("WarpGate");
    }
    return CountUnitsMatchingName(Name);
}

UINT ControlGroupInfo::CountUnitsMatchingName(const String &Name) const
{
    UINT Result = 0;
    for(UINT Index = 0; Index < LastObservedNonBaseUnits.Length(); Index++)
    {
        if(LastObservedNonBaseUnits[Index].Entry == NULL)
        {
            g_Context->Files.Assert << "NULL Unit\n";
        }
        else if(LastObservedNonBaseUnits[Index].Entry->Name == Name)
        {
            Result++;
        }
    }
    for(UINT Index = 0; Index < LastObservedBaseUnits.Length(); Index++)
    {
        if(LastObservedBaseUnits[Index].Entry == NULL)
        {
            g_Context->Files.Assert << "NULL Unit\n";
        }
        else if(LastObservedBaseUnits[Index].Entry->Name == Name)
        {
            Result++;
        }
    }
    return Result;
}

void ControlGroupManager::Init()
{
    ResetNewGame();
}

void ControlGroupManager::ResetNewGame()
{
    for(UINT Index = 0; Index < ControlGroupCount; Index++)
    {
        _ControlGroups[Index].Reset();
    }
    _ControlGroups[0].Type = ControlGroupSupply;
    _ControlGroups[1].Type = ControlGroupCombatMicro;
    _ControlGroups[2].Type = ControlGroupBuildingMain;
    _ControlGroups[3].Type = ControlGroupBuildingSupport;
    _ControlGroups[4].Type = ControlGroupCombatAll;
    _ControlGroups[5].Type = ControlGroupWorkerMineral;
    _ControlGroups[6].Type = ControlGroupWorkerGas0;
    _ControlGroups[7].Type = ControlGroupWorkerGas1;
    _ControlGroups[8].Type = ControlGroupWorkerGas2;
    _ControlGroups[9].Type = ControlGroupWorkerGas3;
}

void ControlGroupManager::EndFrame()
{
	if(!g_Context->Controller.ConsoleEnabled())
	{
		return;
	}
    if( DisplayControlGroups || DisplayArmyInfo )
    {
        for(UINT ControlGroupIndex = 0; ControlGroupIndex < ControlGroupCount; ControlGroupIndex++)
        {
            const ControlGroupInfo &CurInfo = _ControlGroups[ControlGroupIndex];
            if(DisplayControlGroups)
            {
                String S = String(ControlGroupIndex) + String(':');
                for(UINT UnitIndex = 0; UnitIndex < CurInfo.LastObservedBaseUnits.Length(); UnitIndex++)
                {
                    S += CurInfo.LastObservedBaseUnits[UnitIndex].Entry->Name;
                    if(UnitIndex != CurInfo.LastObservedBaseUnits.Length() - 1)
                    {
                        S.PushEnd(',');
                    }
                }
                S.PushEnd('|');
                for(UINT UnitIndex = 0; UnitIndex < CurInfo.LastObservedNonBaseUnits.Length(); UnitIndex++)
                {
                    S += CurInfo.LastObservedNonBaseUnits[UnitIndex].Entry->Name;
                    if(UnitIndex != CurInfo.LastObservedNonBaseUnits.Length() - 1)
                    {
                        S.PushEnd(',');
                    }
                }
                g_Context->WriteConsole(S, RGBColor::Orange, OverlayPanelStatus);
            }
            if(DisplayArmyInfo)
            {
                if(CurInfo.LastObservedAllLevelUnits.Length() > 0)
                {
                    String AllDescription = ControlGroupUnitInfo::Describe(CurInfo.LastObservedAllLevelUnits);
                    g_Context->WriteConsole(String(ControlGroupIndex) + String(':') + AllDescription, RGBColor::Yellow, OverlayPanelStatus);
                }
            }
        }
    }
}

UINT ControlGroupManager::BaseCount() const
{
    UINT Result = 0;
    int BaseIndex = FindControlGroupIndex(ControlGroupBuildingMain);
    if(BaseIndex == -1)
    {
        return 0;
    }
    else
    {
        return _ControlGroups[BaseIndex].LastObservedBaseUnits.Length();
    }
    return Result;
}

UINT ControlGroupManager::WorkerCount() const
{
    UINT Result = 0;
    for(UINT Index = 0; Index < ControlGroupCount; Index++)
    {
        Result += _ControlGroups[Index].CountUnitsMatchingName(g_Context->Managers.Knowledge.WorkerUnit()->Name);
    }
    return Result;
}

ControlGroupInfo& ControlGroupManager::FindControlGroup(const UnitEntry &Unit)
{
    int ControlGroupIndex = FindControlGroupIndex(Unit);
    if(ControlGroupIndex == -1)
    {
        g_Context->Files.Assert << "Control group not found\n";
        return _ControlGroups[0];
    }
    return _ControlGroups[ControlGroupIndex];
}
ControlGroupInfo& ControlGroupManager::FindControlGroup(ControlGroupType Type)
{
    int ControlGroupIndex = FindControlGroupIndex(Type);
    if(ControlGroupIndex == -1)
    {
        g_Context->Files.Assert << "Control group not found\n";
        return _ControlGroups[0];
    }
    return _ControlGroups[ControlGroupIndex];
}

int ControlGroupManager::FindControlGroupIndex(ControlGroupType Type) const
{
    for(UINT Index = 0; Index < ControlGroupCount; Index++)
    {
        if(Type == _ControlGroups[Index].Type)
        {
            return Index;
        }
    }
    return -1;
}

int ControlGroupManager::FindControlGroupIndex(const UnitEntry &Unit) const
{
    if(Unit.Name.Length() == 0 || Unit.SecondaryType == UnitSecondaryWorker)
    {
        return -1;
    }
    if(Unit.SecondaryType == UnitSecondaryBase)
    {
        return FindControlGroupIndex(ControlGroupBuildingMain);
    }
    if(Unit.SecondaryType == UnitSecondarySupply)
    {
        return FindControlGroupIndex(ControlGroupSupply);
    }
    if(Unit.Name == "CreepTumor")// || Unit.Name == "SpineCrawler" || Unit.Name == "SporeCrawler")
    {
        return -1;
    }
    if(Unit.PrimaryType == UnitPrimaryBuilding || Unit.Name == "Queen")
    {
        return FindControlGroupIndex(ControlGroupBuildingSupport);
    }
    if(Unit.PrimaryType == UnitPrimaryMobile)
    {
        if(Unit.SecondaryType == UnitSecondaryCombatMicro)
        {
            return FindControlGroupIndex(ControlGroupCombatMicro);
        }
        return FindControlGroupIndex(ControlGroupCombatAll);
    }
    return -1;
}

UINT ControlGroupManager::GasControlGroupIndexFromGasIndex(UINT GasIndex) const
{
    int BaseIndex = FindControlGroupIndex(ControlGroupWorkerGas0);
    return BaseIndex + GasIndex;
}

int ControlGroupManager::FindNewWorkerControlGroupIndex() const
{
    UINT ActiveGasGroups = ActiveGasGroupCount();
    for(UINT GasIndex = 0; GasIndex < ActiveGasGroups; GasIndex++)
    {
        UINT ControlGroupIndex = GasControlGroupIndexFromGasIndex(GasIndex);
        const ControlGroupInfo &CurGroup = _ControlGroups[ControlGroupIndex];
        if(CurGroup.WorkersAssigned < 3 || CurGroup.LastObservedNonBaseUnits.Length() <= 1)
        {
            return ControlGroupIndex;
        }
    }
    return FindControlGroupIndex(ControlGroupWorkerMineral);
}

UINT ControlGroupManager::ActiveGasGroupCount() const
{
    int ExtractorGroupIndex = FindControlGroupIndex(ControlGroupBuildingSupport);
    if(ExtractorGroupIndex == -1)
    {
        return 2;
    }
    const String ExtractorName = ExtractorNameFromRace(g_Context->Managers.Knowledge.MyRace());
    UINT ExtractorCount = _ControlGroups[ExtractorGroupIndex].CountUnitsMatchingName(ExtractorName);
    UINT DesiredActiveGasGroupCount = Math::Max(UINT(2), ExtractorCount);
    return Math::Min(DesiredActiveGasGroupCount, UINT(4));
}

void ControlGroupManager::ReportWorkerAssigned(UINT ControlGroupIndex)
{
    _ControlGroups[ControlGroupIndex].WorkersAssigned++;
    UnitEntry *WorkerEntry = g_Context->Managers.Knowledge.WorkerUnit();
    if(WorkerEntry == NULL)
    {
        g_Context->Files.Assert << "WorkerEntry == NULL\n";
    }
    else
    {
        _ControlGroups[ControlGroupIndex].LastObservedNonBaseUnits.PushEnd(WorkerEntry);
    }
}
