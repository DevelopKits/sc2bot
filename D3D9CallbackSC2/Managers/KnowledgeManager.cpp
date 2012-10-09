#include "Main.h"

void KnowledgeManager::Init()
{
    ResetNewGame();
}

void KnowledgeManager::ResetNewGame()
{
    _AllBaseInfo.Allocate(1);
    _AllBaseInfo[0].BaseLocation = Vec2f(0.5f, 0.5f);
    _AllBaseInfo[0].BuildLocation = Vec2f(0.5f, 0.5f);
    _AllBaseInfo[0].Player = PlayerSelf;
    _AllBaseInfo[0].StartLocation = true;
    _AllBaseInfo[0].MyStartLocation = true;
    _AllBaseInfo[0].EnemyStartLocation = false;
    _AllBaseInfo[0].ExtractorCount = 2;
    _AllBaseInfo[0].ExtractorLocations[0] = Vec2f(0.5f, 0.5f);
    _AllBaseInfo[0].ExtractorLocations[1] = Vec2f(0.5f, 0.5f);
    _AllBaseInfo[0].Index = 0;

    _MinimapMatched = false;
    _MyRace = RaceNone;
    _WorkerUnit = g_Context->Managers.Database.GetUnitEntry(WorkerNameFromRace(RaceZerg));
    _SupplyUnit = g_Context->Managers.Database.GetUnitEntry(SupplyNameFromRace(RaceZerg));
    _ExtractorUnit = g_Context->Managers.Database.GetUnitEntry(ExtractorNameFromRace(RaceZerg));
    _ScoutUnit = g_Context->Managers.Database.GetUnitEntry(ScoutNameFromRace(RaceZerg));

    _LastScoutMinimapObservation = Vec2f(0.5f, 0.5f);
    _MinimapScoutBaseIndex = -1;

    _Strat = StratProtossStalkers;

    _Army.NextMacroTime = GameTime() + 20.0;
    _Army.State = ArmyStateIdleInBase;
    _Army.ResourceValue = 0;

	_GameLocalUnits.FreeMemory();
}

void KnowledgeManager::StartFrame()
{
    const double CurTime = GameTime();
    for(int Index = 0; Index < int(_UnitsInProduction.Length()); Index++)
    {
        const UnitInProductionInfo &CurInfo = _UnitsInProduction[Index];
        double SafetyTime = 20.0;
        if(CurInfo.Unit->PrimaryType == UnitPrimaryMobile)
        {
			if(CurInfo.Unit->Race == RaceProtoss)
			{
				if(CurInfo.Unit->BuiltAt == "Gateway" && g_Context->Managers.ControlGroup.FindControlGroup(ControlGroupBuildingSupport).CountUnitsMatchingName("WarpGate") > 0)
				{
					SafetyTime = -8.0;
				}
				else
				{
					//
					// We might have chronoboosted this unit
					//
					SafetyTime = -4.0;
				}
			}
			else
			{
				SafetyTime = 0.0;
			}
        }
        if(CurInfo.Unit->Name == "Overlord")
        {
            SafetyTime = 5.0;
        }
        if(CurInfo.StartTime + CurInfo.Unit->BuildTime + SafetyTime <= CurTime)
        {
            _UnitsInProduction.RemoveSwap(Index);
            Index--;
        }
    }
}

void KnowledgeManager::EndFrame()
{
	if(!g_Context->Controller.ConsoleEnabled())
	{
		return;
	}
    if(ReportExtractors)
    {
        for(UINT ExtractorIndex = 0; ExtractorIndex < _Extractors.Length(); ExtractorIndex++)
        {
            const ExtractorInfo &CurExtractor = _Extractors[ExtractorIndex];
            String S = _ExtractorUnit->Name + String(ExtractorIndex) + String(":") + String(CurExtractor.NextCheckTime - GameTime());
            g_Context->WriteConsole(S, RGBColor::Green, OverlayPanelStatus);
        }
    }
    if(ReportUnitsInProduction)
    {
        const double CurTime = GameTime();
        String S = "Production: ";
        for(UINT Index = 0; Index < _UnitsInProduction.Length(); Index++)
        {
            const UnitInProductionInfo &CurInfo = _UnitsInProduction[Index];
            String TimeString = String(CurInfo.StartTime + CurInfo.Unit->BuildTime - CurTime);
            if(TimeString.Length() > 5)
            {
                TimeString.ReSize(5);
            }
            S += CurInfo.Unit->Name + String("(") + TimeString + String(")");
            if(Index != _UnitsInProduction.Length() - 1)
            {
                S.PushEnd(',');
            }
        }
        g_Context->WriteConsole(S, RGBColor::Orange, OverlayPanelProduction);
    }
    if(DisplayArmyInfo)
    {
        String S = "Unknown";
        if(_Army.State == ArmyStateIdleInBase)
        {
            S = "Idle";
        }
        else if(_Army.State == ArmyStateMovingToDefend)
        {
            S = String("Defending ") + _Army.AttackTarget.CommaSeparatedString();
        }
        else if(_Army.State == ArmyStateMovingToAttack)
        {
            S = String("Attacking ") + _Army.AttackTarget.CommaSeparatedString();
        }
        else if(_Army.State == ArmyStateGathering)
        {
            S = String("Gathering");
        }
        g_Context->WriteConsole(String("Army: ") + S, RGBColor::Orange, OverlayPanelStatus);
        g_Context->WriteConsole(String("Army Value: ") + String(_Army.ResourceValue), RGBColor::Orange, OverlayPanelStatus);
    }
}

void KnowledgeManager::ReportCenteredOnScout()
{
    _LastScoutMinimapObservation = g_Context->Managers.FrameHUD.MinimapViewportCenter();
}

void KnowledgeManager::UpdateArmyAction()
{
    Vector<Vec2f> ConflictLocations;
    MinimapManager &Minimap = g_Context->Managers.Minimap;
    for(UINT Y = 0; Y < Minimap.Cells().Rows(); Y++)
    {
        for(UINT X = 0; X < Minimap.Cells().Cols(); X++)
        {
            const MinimapCell &CurCell = Minimap.Cells()(Y, X);
            if(CurCell.AllyNearby && CurCell.EnemyNearby)
            {
                ConflictLocations.PushEnd(Minimap.MinimapCoordFromCellCoord(Vec2i(X, Y)));
            }
        }
    }

    if(ConflictLocations.Length() == 0)
    {
        if(_Army.ResourceValue < 2000)
        {
            _Army.State = ArmyStateIdleInBase;
            _Army.AttackTarget = Vec2f(0.5f, 0.5f);
        }
        else
        {
            _Army.State = ArmyStateMovingToAttack;
            _Army.AttackTarget = g_Context->Managers.Knowledge.EnemyLocation();

            if(Minimap.ArmySize() >= 10 && Minimap.ArmyScatter() >= 0.2f)
            {
                _Army.State = ArmyStateGathering;
            }
        }
        return;
    }

    struct ConflictSorter
    {
        bool operator()(const Vec2f &A, const Vec2f &B)
        {
            float ADist = g_Context->Managers.Minimap.MinimapCoordDistToBase(A);
            float BDist = g_Context->Managers.Minimap.MinimapCoordDistToBase(B);
            return (ADist < BDist);
        }
    };
    ConflictSorter Sorter;
    ConflictLocations.Sort(Sorter);
	Vec2f ClosestEnemyLocation = Minimap.FindNearestEnemyCoord(ConflictLocations[0]);

    float ClosestConflictDist = Minimap.MinimapCoordDistToBase(ConflictLocations[0]);
    if(ClosestConflictDist < 0.15f)
    {
        _Army.State = ArmyStateMovingToDefend;
        _Army.AttackTarget = ClosestEnemyLocation;
    }
    else
    {
		/*if(_Army.ResourceValue < 2000)
        {
            _Army.State = ArmyStateRetreating;
			const BaseInfo *NearestBase = FindNearestBase(Minimap.ArmyLocation(), PlayerSelf);
			if(NearestBase == NULL)
			{
				_Army.AttackTarget = Vec2f(0.5f, 0.5f);
			}
			else
			{
				_Army.AttackTarget = NearestBase->BuildLocation;
			}
        }
        else*/
		{
			_Army.State = ArmyStateMovingToAttack;
			_Army.AttackTarget = ClosestEnemyLocation;
		}
    }

    /*if(_Army.State == ArmyStateMovingToAttack)
    {
        if(Minimap.ArmySize() >= 10 && Minimap.ArmyScatter() >= 0.2f)
        {
            _Army.State = ArmyStateGathering;
        }
    }*/
}

void KnowledgeManager::MatchMinimap()
{
    if(_MinimapMatched || g_Context->Managers.Minimap.MinimapIsCovered())
    {
        return;
    }
    MinimapEntry *CurEntry = g_Context->Managers.Database.FindClosestMinimap(g_Context->Managers.Minimap.Minimap());
    if(CurEntry == NULL)
    {
        g_Context->Files.Assert << "Minimap match failed\n";
        return;
    }
    
    _MinimapMatched = true;
    MinimapManager &Minimap = g_Context->Managers.Minimap;
    _AllBaseInfo = CurEntry->Bases;
    bool MyBaseFound = false;
    UINT StartLocationCount = 0;
    for(UINT BaseIndex = 0; BaseIndex < _AllBaseInfo.Length(); BaseIndex++)
    {
        BaseInfo &CurBase = _AllBaseInfo[BaseIndex];
        CurBase.Index = BaseIndex;
        if(CurBase.StartLocation)
        {
            StartLocationCount++;
        }
        Vec2i CellCoord = Minimap.CellCoordFromMinimapCoord(CurBase.BaseLocation);
        if(Minimap.Cells()(CellCoord.y, CellCoord.x).AllyNearby)
        {
            if(MyBaseFound)
            {
                g_Context->Files.Assert << "Multiple bases found\n";
            }
            if(!CurBase.StartLocation)
            {
                g_Context->Files.Assert << "Non-start location base found\n";
            }
            CurBase.Player = PlayerSelf;
            CurBase.MyStartLocation = true;
            MyBaseFound = true;
        }
    }
    if(!MyBaseFound)
    {
        g_Context->Files.Assert << "No self start location found\n";
        _AllBaseInfo[0].Player = PlayerSelf;
        _AllBaseInfo[0].MyStartLocation = true;
        return;
    }
    if(StartLocationCount == 2)
    {
        for(UINT BaseIndex = 0; BaseIndex < _AllBaseInfo.Length(); BaseIndex++)
        {
            BaseInfo &CurBase = _AllBaseInfo[BaseIndex];
            if(CurBase.StartLocation && CurBase.Player != PlayerSelf)
            {
                CurBase.EnemyStartLocation = true;
            }
        }
    }
}

void KnowledgeManager::ReportRace(RaceType Race)
{
    if(_MyRace == Race)
    {
        return;
    }
    if(_MyRace != RaceNone)
    {
        g_Context->Files.Assert << "Conflicting races\n";
    }
    g_Context->WriteConsole(String("Race matched: ") + GetRaceTypeString(Race), RGBColor::Yellow, OverlayPanelSystem);
    _MyRace = Race;
	_BaseUnit = g_Context->Managers.Database.GetUnitEntry(BaseNameFromRace(_MyRace));
    _WorkerUnit = g_Context->Managers.Database.GetUnitEntry(WorkerNameFromRace(_MyRace));
    _SupplyUnit = g_Context->Managers.Database.GetUnitEntry(SupplyNameFromRace(_MyRace));
    _ExtractorUnit = g_Context->Managers.Database.GetUnitEntry(ExtractorNameFromRace(_MyRace));
    _ScoutUnit = g_Context->Managers.Database.GetUnitEntry(ScoutNameFromRace(_MyRace));
	GetGameLocalUnitInfo(_BaseUnit).TotalBuilt = 1;
}

void KnowledgeManager::UpdateBaseOwnership()
{
    for(UINT BaseIndex = 0; BaseIndex < _AllBaseInfo.Length(); BaseIndex++)
    {
        BaseInfo &CurBase = _AllBaseInfo[BaseIndex];
        const Vec2i CellCoord = g_Context->Managers.Minimap.CellCoordFromMinimapCoord(CurBase.BaseLocation);
        const MinimapCell &CurCell = g_Context->Managers.Minimap.Cells()(CellCoord.y, CellCoord.x);
        bool BaseOwnerKnown = (CurBase.Player == PlayerSelf);
        if(CurCell.LastExploreTime <= 0.0f && CurBase.EnemyStartLocation)
        {
            CurBase.Player = PlayerEnemy;
            BaseOwnerKnown = true;
        }
        if(!BaseOwnerKnown)
        {
            CurBase.Player = PlayerInvalid;
            for(UINT CoordIndex = 0; CoordIndex < 3; CoordIndex++)
            {
                Vec2f BaseLocation(0.5f, 0.5f);
                if(CoordIndex == 0)
                {
                    BaseLocation = CurBase.BaseLocation;
                }
                else if(CoordIndex == 1)
                {
                    BaseLocation = CurBase.BuildLocation;
                }
                else if(CoordIndex == 2)
                {
                    BaseLocation = CurBase.RampLocation;
                }
                Vec2i CellCoord = g_Context->Managers.Minimap.CellCoordFromMinimapCoord(CurBase.BaseLocation);
                const MinimapCell &CurCell = g_Context->Managers.Minimap.Cells()(CellCoord.y, CellCoord.x);
                if(CurCell.EnemyNearby)
                {
                    CurBase.Player = PlayerEnemy;
                }
            }
        }
    }
}

double ScoutLocationInfo::Value()
{
	double Result = 0.0;
	if(IsEnemyBase)
	{
        g_Context->Files.Assert << "Enemy base should not be a candidate location\n";
	}
    if(!IsStartLocation && IsRamp)
	{
        g_Context->Files.Assert << "Non-start ramps should not be a candidate location\n";
	}
    if(IsStartLocation)
    {
        Result += 1.0;
        if(IsRamp)
        {
            Result += 2.0;
        }
        if(IsBase && BaseRampScouted)
        {
            Result += 4.0;
        }
    }
    return Result;
}

void ScoutLocationInfo::Init(const Vec2f &_MinimapLocation, const BaseInfo &Base)
{
    BaseIndex = Base.Index;
    IsStartLocation = Base.StartLocation;
    BaseLocation = _MinimapLocation;
    Vec2i CellCoord = g_Context->Managers.Minimap.CellCoordFromMinimapCoord(_MinimapLocation);
    const MinimapCell &CurCell = g_Context->Managers.Minimap.Cells()(CellCoord.y, CellCoord.x);
    LastExploreTime = CurCell.LastExploreTime;
    IsEnemyBase = (Base.Player == PlayerEnemy);
    DistToScout = Vec2f::Dist(_MinimapLocation, g_Context->Managers.Knowledge.LastScoutMinimapObservation());
}

bool operator < (const ScoutLocationInfo &L, const ScoutLocationInfo &R)
{
    if(L.LastExploreTime < R.LastExploreTime)
    {
        return true;
    }
    if(L.LastExploreTime > R.LastExploreTime)
    {
        return false;
    }
    if(L.CachedValue > R.CachedValue)
    {
        return true;
    }
    if(L.CachedValue < R.CachedValue)
    {
        return false;
    }
    return (L.DistToScout < R.DistToScout);
}

void KnowledgeManager::UpdateScoutLocation()
{
    if(_MinimapScoutBaseIndex >= 0 && _MinimapScoutBaseIndex < int(_AllBaseInfo.Length()))
    {
        const BaseInfo &CurBase = _AllBaseInfo[_MinimapScoutBaseIndex];
        ScoutLocationInfo LocationInfo;
        LocationInfo.Init(_MinimapScoutLocation, CurBase);
        if(LocationInfo.LastExploreTime == 0.0f && rnd() < 0.9f)
        {
            //
            // Normally, continue on to the destination; avoid bounding between two local minima
            //
            return;
        }
    }
    Vector<ScoutLocationInfo> CandidateScoutLocations;
    for(UINT BaseIndex = 0; BaseIndex < _AllBaseInfo.Length(); BaseIndex++)
    {
        const BaseInfo &CurBase = _AllBaseInfo[BaseIndex];
        if(CurBase.Player != PlayerSelf && CurBase.Player != PlayerEnemy)
        {
            ScoutLocationInfo RampLocationInfo;
            RampLocationInfo.Init(CurBase.RampLocation, CurBase);
            RampLocationInfo.IsRamp = true;
            RampLocationInfo.IsBase = false;
            RampLocationInfo.BaseRampScouted = false;
            
            ScoutLocationInfo BaseLocationInfo;
            BaseLocationInfo.Init(CurBase.BaseLocation, CurBase);
            BaseLocationInfo.IsRamp = false;
            BaseLocationInfo.IsBase = true;
            BaseLocationInfo.BaseRampScouted = (RampLocationInfo.LastExploreTime > 0.0f);

            if(CurBase.StartLocation)
            {
				RampLocationInfo.CachedValue = RampLocationInfo.Value();
                CandidateScoutLocations.PushEnd(RampLocationInfo);
            }
			BaseLocationInfo.CachedValue = BaseLocationInfo.Value();
            CandidateScoutLocations.PushEnd(BaseLocationInfo);
        }
    }
    CandidateScoutLocations.Sort();
    if(CandidateScoutLocations.Length() == 0)
    {
        _MinimapScoutLocation = Vec2f(rnd(), rnd());
    }
    else
    {
        _MinimapScoutLocation = CandidateScoutLocations[0].BaseLocation;
        _MinimapScoutBaseIndex = CandidateScoutLocations[0].BaseIndex;
    }
}

const BaseInfo* KnowledgeManager::FindNearestBase(const Vec2f &MinimapPos, PlayerType Player) const
{
    const BaseInfo *Result = NULL;
    float ClosestDistSq = 10.0f;
    for(UINT BaseIndex = 0; BaseIndex < _AllBaseInfo.Length(); BaseIndex++)
    {
        const BaseInfo &CurBase = _AllBaseInfo[BaseIndex];
        if(CurBase.Player == Player)
        {
            float CurDistSq = Vec2f::DistSq(MinimapPos, CurBase.BaseLocation);
            if(CurDistSq < ClosestDistSq)
            {
                ClosestDistSq = CurDistSq;
                Result = &CurBase;
            }
        }
    }
    return Result;
}

void KnowledgeManager::ReportExtractorCount(UINT ObservedExtractorCount)
{
    if(ObservedExtractorCount != _Extractors.Length())
    {
        _Extractors.Allocate(ObservedExtractorCount);
        for(UINT Index = 0; Index < ObservedExtractorCount; Index++)
        {
            _Extractors[Index].LastObservedRemaining = -1;
            _Extractors[Index].NextCheckTime = GameTime() + 2.0f;
        }
    }
}

const BaseInfo* KnowledgeManager::FindBuildSite() const
{
    for(UINT BaseIndex = 0; BaseIndex < _AllBaseInfo.Length(); BaseIndex++)
    {
        const BaseInfo &CurBase = _AllBaseInfo[BaseIndex];
        if(CurBase.MyStartLocation)
        {
            return &CurBase;
        }
    }
    return NULL;
}

const BaseInfo* KnowledgeManager::RandomBase() const
{
    Vector<const BaseInfo*> MyBases;
    for(UINT BaseIndex = 0; BaseIndex < _AllBaseInfo.Length(); BaseIndex++)
    {
        const BaseInfo &CurBase = _AllBaseInfo[BaseIndex];
        if(CurBase.Player == PlayerSelf)
        {
            MyBases.PushEnd(&CurBase);
        }
    }
    if(MyBases.Length() == 0)
    {
        return NULL;
    }
    else
    {
        return MyBases.RandomElement();
    }
}

const BaseInfo* KnowledgeManager::MainBase() const
{
    for(UINT BaseIndex = 0; BaseIndex < _AllBaseInfo.Length(); BaseIndex++)
    {
        const BaseInfo &CurBase = _AllBaseInfo[BaseIndex];
        if(CurBase.Player == PlayerSelf && CurBase.MyStartLocation)
        {
            return &CurBase;
        }
    }
    return NULL;
}

bool KnowledgeManager::RandomExtractorSite(Vec2f &Result) const
{
    const BaseInfo *MyRandomBase = RandomBase();
    if(MyRandomBase == NULL)
    {
        return false;
    }
    if(MyRandomBase->ExtractorCount == 0)
    {
        return false;
    }
    UINT ExtractorIndex = rand() % MyRandomBase->ExtractorCount;
    Result = MyRandomBase->ExtractorLocations[ExtractorIndex];
    return true;
}

Vec2f KnowledgeManager::EnemyLocation() const
{
    for(UINT BaseIndex = 0; BaseIndex < _AllBaseInfo.Length(); BaseIndex++)
    {
        const BaseInfo &CurBase = _AllBaseInfo[BaseIndex];
        if(CurBase.Player == PlayerEnemy)
        {
            return CurBase.BaseLocation;
        }
    }
    return Vec2f(0.5f, 0.5f);
}

Vec2f KnowledgeManager::RandomMusterLocation() const
{
    const BaseInfo *MyBase = MainBase();
    if(MyBase == NULL)
    {
        return Vec2f(0.5f, 0.5f);
    }
    else
    {
        Vec2f Center = Vec2f::Lerp(MyBase->BaseLocation, MyBase->BuildLocation, rnd() * 0.5f + 0.5f);
        const float Radius = 0.02f + rnd() * 0.01f;
        float Theta = rnd() * Math::PIf * 2.0f;
        Vec2f Result = Center + Radius * Vec2f(cosf(Theta), sinf(Theta));
        Result.x = Utility::Bound(Result.x, 0.0f, 1.0f);
        Result.y = Utility::Bound(Result.y, 0.0f, 1.0f);
        return Result;
    }
}

Vec2f KnowledgeManager::RandomOverlordScoutMinimapLocation() const
{
    const BaseInfo *MyRandomBase = RandomBase();
    if(MyRandomBase == NULL)
    {
        return Vec2f(0.5f, 0.5f);
    }
    else
    {
        Vec2f Center = MyRandomBase->BuildLocation;
        if(rnd() < 0.5f)
        {
            Center = MyRandomBase->BaseLocation;
        }
        const float Radius = 0.1f + rnd() * 0.05f;
        float Theta = rnd() * Math::PIf * 2.0f;
        Vec2f Result = Center + Radius * Vec2f(cosf(Theta), sinf(Theta));
        Result.x = Utility::Bound(Result.x, 0.0f, 1.0f);
        Result.y = Utility::Bound(Result.y, 0.0f, 1.0f);
        return Result;
    }
}

UINT KnowledgeManager::MyBaseCount() const
{
    UINT Result = 0;
    for(UINT BaseIndex = 0; BaseIndex < _AllBaseInfo.Length(); BaseIndex++)
    {
        const BaseInfo &CurBase = _AllBaseInfo[BaseIndex];
        if(CurBase.Player == PlayerSelf)
        {
            Result++;
        }
    }
    return Math::Max(Result, UINT(1));
}

UINT KnowledgeManager::CountUnitsInProduction(const String &Name) const
{
    UINT Result = 0;
    for(UINT Index = 0; Index < _UnitsInProduction.Length(); Index++)
    {
        if(_UnitsInProduction[Index].Unit->Name == Name)
        {
            Result++;
        }
    }
    return Result;
}

bool KnowledgeManager::ResearchCompleted(const String &Name) const
{
    UINT Result = 0;
    for(UINT Index = 0; Index < _AllResearch.Length(); Index++)
    {
        if(_AllResearch[Index].Unit->Name == Name)
        {
            Result++;
        }
    }
    if(Result >= 2)
    {
        g_Context->Files.Assert << "Multiple instances of the same research\n";
    }
    return (Result >= 1);
}

void KnowledgeManager::RecordUnitInProduction(UnitEntry *Entry, double StartTime)
{
	GameLocalUnitInfo& GameLocalUnit = GetGameLocalUnitInfo(Entry);
	GameLocalUnit.TotalBuilt++;
    if(Entry->PrimaryType == UnitPrimaryResearch)
    {
        _AllResearch.PushEnd();
        _AllResearch.Last().StartTime = StartTime;
        _AllResearch.Last().Unit = Entry;
    }
    else
    {
        _UnitsInProduction.PushEnd();
        _UnitsInProduction.Last().StartTime = StartTime;
        _UnitsInProduction.Last().Unit = Entry;
    }
}

GameLocalUnitInfo& KnowledgeManager::GetGameLocalUnitInfo(const UnitEntry *Entry)
{
	for(UINT Index = 0; Index < _GameLocalUnits.Length(); Index++)
	{
		if(_GameLocalUnits[Index].Entry == Entry)
		{
			return _GameLocalUnits[Index];
		}
	}
	_GameLocalUnits.PushEnd( GameLocalUnitInfo(Entry) );
	return (_GameLocalUnits.Last());
}
