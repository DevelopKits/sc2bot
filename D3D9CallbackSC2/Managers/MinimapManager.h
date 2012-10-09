//
// MinimapManager.h
//
// Handles all minimap-related functions.
// Written by Matthew Fisher
//

const UINT MinimapGridSize = 32;
const float MinimapGridSizeInv = 1.0f / float(MinimapGridSize);
extern const RGBColor MinimapResourceColor;

struct MinimapCell
{
    MinimapCell()
    {
        LastExploreTime = -1.0;
        AllyNearby = false;
        EnemyNearby = false;
    }
    double LastExploreTime;
    bool AllyNearby;
    bool EnemyNearby;
};

struct MinimapPixelHistory
{
    float ObservationFrequency;
};

class MinimapManager : public Manager
{
public:
    void Init();
    void ResetNewGame();
    void EndFrame();

    bool MinimapCaptureNeeded() const;
    void MakeExplorationBitmap(Bitmap &Result) const;
    void MakeArmyBitmap(Bitmap &Result) const;
    bool MinimapIsCovered() const;

    void ReportMinimapCapture();

    float MinimapCoordDistToBase(const Vec2f &MinimapCoord) const;
	Vec2f FindNearestEnemyCoord(const Vec2f &MinimapCoord) const;

    __forceinline Bitmap& Minimap()
    {
        return _Minimap;
    }
    __forceinline const Vector<Vec2f>& SelfPixelCoordinates() const
    {
        return _SelfPixelCoordinates;
    }
    __forceinline const Grid<MinimapCell>& Cells() const
    {
        return _Cells;
    }
    __forceinline double ArmySize() const
    {
        return _ArmySize;
    }
    __forceinline double ArmyScatter() const
    {
        return _ArmyScatter;
    }
    __forceinline const Vec2f& ArmyLocation() const
    {
        return _ArmyLocation;
    }
	__forceinline bool Conflicted() const
	{
		return _Conflicted;
	}

    __forceinline static Vec2i CellCoordFromMinimapCoord(const Vec2f &MinimapCoord)
    {
        Vec2i Result;
        Result.x = Utility::Bound(int(MinimapCoord.x * MinimapGridSize), 0, int(MinimapGridSize) - 1);
        Result.y = Utility::Bound(int(MinimapCoord.y * MinimapGridSize), 0, int(MinimapGridSize) - 1);
        return Result;
    }
    __forceinline static Vec2f MinimapCoordFromCellCoord(const Vec2i &CellCoord)
    {
        Vec2f Result;
        Result.x = (CellCoord.x + 0.5f) * MinimapGridSizeInv;
        Result.y = (CellCoord.y + 0.5f) * MinimapGridSizeInv;
        return Result;
    }

private:
    void UpdateCells();
    void UpdateHistory();
    void UpdateArmyStats();

    Grid<MinimapPixelHistory> _PixelHistory;
    Grid<MinimapCell> _Cells;
    Vector<Vec2f> _SelfPixelCoordinates;
    Vector<Vec2f> _EnemyPixelCoordinates;
    Bitmap _Minimap;
    double _LastSuccessfulMinimapCapture;
    double _LastMinimapCapture;

    double _ArmySize;
    double _ArmyScatter;
    Vec2f _ArmyLocation;
	bool _Conflicted;
};
