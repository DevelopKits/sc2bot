//
// ControlGroupManager.h
//
// Handles all control group functions.
// Written by Matthew Fisher
//

struct ControlGroupUnitInfo
{
    ControlGroupUnitInfo() {}
    ControlGroupUnitInfo(UnitEntry *_Entry)
    {
        Entry = _Entry;
    }
    static String Describe(const Vector<ControlGroupUnitInfo> &Units);
    static UINT ComputeResourceValue(const Vector<ControlGroupUnitInfo> &Units);

    UnitEntry *Entry;
};

struct ControlGroupInfo
{
    void Reset();
    void UpdateFromHUD();
    void AppendAllFromHUD();
    UINT CountUnitsMatchingName(const String &Name) const;
    UINT CountPrerequisiteUnitsMatchingName(const String &Name) const;
    void InsertUnit(UnitEntry *Entry);

    ControlGroupType Type;
    UINT WorkersAssigned;
    UINT ControlGroupLevel;
    Vector<ControlGroupUnitInfo> LastObservedNonBaseUnits;
    Vector<ControlGroupUnitInfo> LastObservedBaseUnits;

    Vector<ControlGroupUnitInfo> LastObservedAllLevelUnits;
};

const UINT ControlGroupCount = 10;

class ControlGroupManager : public Manager
{
public:
    void Init();
    void ResetNewGame();
    void EndFrame();
    int FindControlGroupIndex(const UnitEntry &Unit) const;
    int FindControlGroupIndex(ControlGroupType Type) const;
    ControlGroupInfo& FindControlGroup(ControlGroupType Type);
    ControlGroupInfo& FindControlGroup(const UnitEntry &Unit);
    int FindNewWorkerControlGroupIndex() const;
    UINT GasControlGroupIndexFromGasIndex(UINT GasIndex) const;
    UINT WorkerCount() const;
    UINT BaseCount() const;

    void ReportWorkerAssigned(UINT ControlGroupIndex);

    __forceinline ControlGroupInfo& ControlGroups(UINT Index)
    {
        return _ControlGroups[Index];
    }

private:
    UINT ActiveGasGroupCount() const;

    ControlGroupInfo _ControlGroups[ControlGroupCount];
};