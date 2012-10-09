//
// KnowledgeManager.h
//
// Stores long-term facts tha AI knows, such as the coordinates of all allied and enemy bases.
// Written by Matthew Fisher
//

struct BaseInfo
{
    UINT Index;
    Vec2f BaseLocation;
    Vec2f BuildLocation;
    UINT ExtractorCount;
    Vec2f ExtractorLocations[2];
    Vec2f RampLocation;
    Vec2f AssaultLocation;
    bool StartLocation;
    bool Debris;
    bool Island;
    
    PlayerType Player;
    bool MyStartLocation;
    bool EnemyStartLocation;
};

struct UnitInProductionInfo
{
    UnitEntry *Unit;
    double StartTime;
};

struct ExtractorInfo
{
    double NextCheckTime;
    double LastObservedRemainingTime;
    int LastObservedRemaining;
};

struct ArmyInfo
{
    ArmyStateType State;
    Vec2f AttackTarget;
    double NextMacroTime;
    UINT ResourceValue;
};


struct ScoutLocationInfo
{
    void Init(const Vec2f &_MinimapLocation, const BaseInfo &Base);
	double Value();

	double CachedValue;
    UINT BaseIndex;
    bool IsEnemyBase;
    bool IsRamp;
    bool IsBase;
    bool IsStartLocation;
    bool BaseRampScouted;
    double LastExploreTime;
    float DistToScout;
    Vec2f BaseLocation;
};

struct GameLocalUnitInfo
{
	GameLocalUnitInfo() {}
	GameLocalUnitInfo(const UnitEntry *_Entry)
	{
		Entry = _Entry;
		TotalBuilt = 0;
		LastSelectionTime = 0.0f;
	}
	const UnitEntry *Entry;
	UINT TotalBuilt;
	double LastSelectionTime;
};

class KnowledgeManager : public Manager
{
public:
    void Init();
    void ResetNewGame();
    void StartFrame();
    void EndFrame();
    
    const BaseInfo* FindNearestBase(const Vec2f &MinimapPos, PlayerType Player) const;
    const BaseInfo* FindBuildSite() const;
    UINT CountUnitsInProduction(const String &Name) const;
    bool ResearchCompleted(const String &Name) const;
    UINT MyBaseCount() const;
    void ReportExtractorCount(UINT ObservedExtractorCount);
    void UpdateScoutLocation();
    void UpdateBaseOwnership();
    void ReportCenteredOnScout();

    const BaseInfo* MainBase() const;
    const BaseInfo* RandomBase() const;
    void RecordUnitInProduction(UnitEntry *Entry, double StartTime);
    Vec2f RandomOverlordScoutMinimapLocation() const;
    Vec2f RandomMusterLocation() const;
    Vec2f EnemyLocation() const;
    bool RandomExtractorSite(Vec2f &Result) const;

    void ReportRace(RaceType Race);
    void MatchMinimap();
    void UpdateArmyAction();

	GameLocalUnitInfo& GetGameLocalUnitInfo(const UnitEntry *Entry);

    __forceinline RaceType MyRace() const
    {
        return _MyRace;
    }
    __forceinline UnitEntry* WorkerUnit() const
    {
        return _WorkerUnit;
    }
    __forceinline UnitEntry* SupplyUnit() const
    {
        return _SupplyUnit;
    }
    __forceinline UnitEntry* ExtractorUnit() const
    {
        return _ExtractorUnit;
    }
    __forceinline UnitEntry* ScoutUnit() const
    {
        return _ScoutUnit;
    }
    __forceinline Vector<ExtractorInfo>& Extractors()
    {
        return _Extractors;
    }
    __forceinline ArmyInfo& Army()
    {
        return _Army;
    }
    __forceinline StratType Strat() const
    {
        return _Strat;
    }
    __forceinline const Vec2f& LastScoutMinimapObservation() const
    {
        return _LastScoutMinimapObservation;
    }
    __forceinline const Vec2f& MinimapScoutLocation() const
    {
        return _MinimapScoutLocation;
    }
    __forceinline const Vector<BaseInfo>& AllBaseInfo() const
    {
        return _AllBaseInfo;
    }

private:
    Vector<BaseInfo> _AllBaseInfo;
    Vector<UnitInProductionInfo> _AllResearch;
    Vector<UnitInProductionInfo> _UnitsInProduction;
    Vector<ExtractorInfo> _Extractors;
	Vector<GameLocalUnitInfo> _GameLocalUnits; 
    
    ArmyInfo _Army;

    bool _MinimapMatched;
    Vec2f _LastScoutMinimapObservation;
    Vec2f _MinimapScoutLocation;
    int _MinimapScoutBaseIndex;

    StratType _Strat;
    RaceType _MyRace;
	UnitEntry *_BaseUnit;
    UnitEntry *_WorkerUnit;
    UnitEntry *_ExtractorUnit;
    UnitEntry *_SupplyUnit;
    UnitEntry *_ScoutUnit;
};
