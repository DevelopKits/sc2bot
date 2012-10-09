//
// FrameUnitManager.h
//
// Handles the set of all units in a single frame.
// Written by Matthew Fisher
//

struct HealthBarInfo
{
    __forceinline float Health() const
    {
        return float(ColoredBlocks) / float(ColoredBlocks + GrayBlocks);
    }
    bool Energy;
    UINT GrayBlocks;
    UINT ColoredBlocks;
    Rectangle2f ScreenBound;
};

struct FrameUnitInfo
{
    FrameUnitInfo(const UnitEntry *_Entry, PlayerType _Owner, const Rectangle2f &_ScreenBound)
    {
        Entry = _Entry;
        Owner = _Owner;
        ScreenBound = _ScreenBound;
        HealthBar = NULL;
    }
	bool Clickable() const;

    const UnitEntry *Entry;
    HealthBarInfo *HealthBar;
    PlayerType Owner;
    Rectangle2f ScreenBound;
};

struct BattlePositionInfo
{
    float ClosestEnemyHostile;
    Vec2i SafestRetreatLocation;
    Vec2i SafestBlinkLocation;
};

class FrameUnitManager : public Manager
{
public:
    void StartFrame();
    void EndFrame();

    void MatchHealthBars();
    void UpdateBattlePosition();
    void ReportUnitRender(const RenderInfo &Info);
    void ReportHealthBars(const RenderInfo &Info, const Vector<ProcessedVertex> &ProcessedVertices);

    __forceinline const Vector<FrameUnitInfo*>& UnitsThisFrame() const
    {
        return _UnitsThisFrame;
    }
    __forceinline BattlePositionInfo& BattlePosition()
    {
        return _BattlePosition;
    }

private:
    void RecordHealthBar(const HealthBarInfo &CurInfo);

    static PlayerType PlayerFromColor(const Vec4f &Color);
    UINT CountUnits(const String &Name, PlayerType Owner) const;
    float ClosestHostileToPoint(const Vec2f &Pt) const;
    Vec2f BestRetreatLocation(const Vec2f &Center, float Radius) const;

    BattlePositionInfo _BattlePosition;
    Vector<FrameUnitInfo*> _UnitsThisFrame;
    Vector<HealthBarInfo> _HealthBars;
};
