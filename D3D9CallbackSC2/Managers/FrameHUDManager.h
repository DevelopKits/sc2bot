//
// FrameHUDManager.h
//
// Handles the set of all relevant HUD events in a single frame.
// Written by Matthew Fisher
//

struct FrameBubblePortrait
{
    Texture *Tex;
};

struct FrameActionButton
{
    ActionButtonStateType State;
    Texture *Tex;
};

struct FrameBuildQueue
{
    Texture *Tex;
};

class FrameHUDManager : public Manager
{
public:
    void StartFrame();
    void EndFrame();

    void ReportFont(const RenderInfo &Info);
    void ReportPortrait(const RenderInfo &Info, const Vector<ProcessedVertex> &ProcessedVertices);
    void ReportMinimapViewport(const RenderInfo &Info, const Vector<ProcessedVertex> &ProcessedVertices);
    void ReportIdleWorker();
    void ReportWarpGatePresent();
    void ReportBuildQueue();
    void ReportBubbleIcon(const RenderInfo &Info);
    void ReportControlGroupLevel(const RenderInfo &Info);
    void ReportActionButtonIcon(const RenderInfo &Info, const Vector<ProcessedVertex> &ProcessedVertices);
    void ReportActionButtonBackground(const RenderInfo &Info, const Vector<ProcessedVertex> &ProcessedVertices);
    bool SingleUnitTypeSelected() const;
    bool HaveResourcesForUnit(const UnitEntry &Entry) const;
    bool ActionButtonPresent(const String &Name, ActionButtonStateType &State) const;
    bool BuildQueueEntryPresent(const String &Name) const;
    bool ValidateSingleUnit(const String &Name) const;
    bool CurrentUnitUnderConstruction() const;
    UINT CountUnitsWithName(const String &Name) const;

    Vec2i ScreenCoordFromBubblePortraitIndex(UINT Index) const;
    Vec2i ScreenCoordFromControlGroupLevel(UINT Level) const;

	float CurrentHealthPercentage() const;

    __forceinline bool IdleWorker() const
    {
        return _IdleWorker;
    }
    __forceinline bool WarpGateReady() const
    {
        return (_WarpGatePresent && _WarpGateCount >= 1);
    }
    __forceinline bool BuildQueuePresent() const
    {
        return _BuildQueuePresent;
    }
    __forceinline Texture* SoloPortrait() const
    {
        return _SoloPortrait;
    }
    __forceinline bool MinimapViewportObserved() const
    {
        return _MinimapViewportObserved;
    }
    __forceinline const Vec2f& MinimapViewportCenter() const
    {
        return _MinimapViewportCenter;
    }
    __forceinline const FrameBubblePortrait& BubblePortraits(UINT Index) const
    {
        return _BubblePortraits[Index];
    }
    __forceinline const FrameActionButton& ActionButtons(UINT Index) const
    {
        return _ActionButtons[Index];
    }
    __forceinline UINT SupplyCurrent() const
    {
        return _SupplyCurrent;
    }
    __forceinline UINT SupplyCap() const
    {
        return _SupplyCap;
    }
    __forceinline int Gas() const
    {
        return _Gas;
    }
    __forceinline int Minerals() const
    {
        return _Minerals;
    }
    __forceinline UINT ControlGroupLevel() const
    {
        return _ControlGroupLevel;
    }
private:

    void ProcessResourceString(const String &S);
    void ProcessUnitInfoString(const String &S);
    int BubblePortraitIndexFromScreenBounds(const Rectangle2f &ScreenBounds) const;
    int ActionButtonIndexFromScreenBounds(const Rectangle2f &ScreenBounds) const;

    bool _IdleWorker;
    int _Minerals, _Gas, _SupplyCurrent, _SupplyCap;

    int _CurHP, _MaxHP;
    int _CurShields, _MaxShields;
    int _CurEnergy, _MaxEnergy;
    int _ResourcesRemaining;
    
    bool _WarpGatePresent;
    int _WarpGateCount;
    bool _MinimapViewportObserved;
    Vec2f _MinimapViewportCenter;

    bool _BuildQueuePresent;
    UINT _ControlGroupLevel;

    Texture *_SoloPortrait;
    FrameBubblePortrait _BubblePortraits[BubblePortraitCount];
    FrameActionButton _ActionButtons[ActionButtonCount];
    FrameBuildQueue _BuildQueue[BuildQueueCount];
};
