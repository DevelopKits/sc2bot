//
// MinimapManager.cpp
//
// Handles all minimap-related functions.
// Written by Matthew Fisher
//
#include "Main.h"

bool FrameUnitInfo::Clickable() const
{
	return g_Context->Constants.Clickable(ScreenBound.Center().RoundToVec2i());
}

void FrameUnitManager::StartFrame()
{
    _UnitsThisFrame.DeleteMemory();
    _HealthBars.FreeMemory();
}

void FrameUnitManager::EndFrame()
{
	if(!g_Context->Controller.ConsoleEnabled())
	{
		return;
	}
    if(DisplayUnitsOnScreen)
    {
        struct StringColorPair
        {
            String Text;
            RGBColor Color;
        };

        Vector<StringColorPair*> AllLines;

        set<UINT> OutputtedNameHashes;
        for(UINT UnitIndex = 0; UnitIndex < _UnitsThisFrame.Length(); UnitIndex++)
        {
            const FrameUnitInfo &CurUnit = *_UnitsThisFrame[UnitIndex];
            UINT NameHash = CurUnit.Entry->Name.Hash32() + CurUnit.Owner;
            if(OutputtedNameHashes.count(NameHash) == 0)
            {
                OutputtedNameHashes.insert(NameHash);
                UINT Count = CountUnits(CurUnit.Entry->Name, CurUnit.Owner);
                RGBColor Color = RGBColor::Green;
                if(CurUnit.Owner == PlayerEnemy)
                {
                    Color = RGBColor::Red;
                }
                if(CurUnit.Owner == PlayerInvalid)
                {
                    Color = RGBColor::White;
                }
                StringColorPair *NewPair = new StringColorPair;
                NewPair->Text = CurUnit.Entry->Name + String(" ") + String(Count);
                NewPair->Color = Color;
                AllLines.PushEnd(NewPair);
            }
        }

        class StringColorPairComparison
        {
        public:
            bool operator() (const StringColorPair *Left, const StringColorPair *Right)
            {
                if(Left->Color == Right->Color)
                {
                    return String::LexicographicComparison::Compare(Left->Text, Right->Text);
                }
                else
                {
                    return (*((UINT*)&(Left->Color)) < *((UINT*)&(Right->Color)));
                }
            }
        };
        StringColorPairComparison Comparator;
        AllLines.Sort(Comparator);

        for(UINT LineIndex = 0; LineIndex < AllLines.Length(); LineIndex++)
        {
            const StringColorPair &CurLine = *AllLines[LineIndex];
            g_Context->WriteConsole(CurLine.Text, CurLine.Color, OverlayPanelStatus);
        }

        AllLines.DeleteMemory();
    }
    if(DisplayUnitHealthBars)
    {
        struct FrameUnitInfoSorter
        {
            bool operator()(const FrameUnitInfo *Left, const FrameUnitInfo *Right)
            {
                return String::LexicographicComparison::Compare(Left->Entry->Name, Right->Entry->Name);
            }
        };
        FrameUnitInfoSorter Sorter;
        _UnitsThisFrame.Sort(Sorter);

        for(UINT UnitIndex = 0; UnitIndex < _UnitsThisFrame.Length(); UnitIndex++)
        {
            const FrameUnitInfo &CurUnit = *_UnitsThisFrame[UnitIndex];
            String BarDescription = "None";
            if(CurUnit.HealthBar != NULL)
            {
                BarDescription = String(CurUnit.HealthBar->Health());
            }
            g_Context->WriteConsole(CurUnit.Entry->Name + String(" ") + String(BarDescription), RGBColor::Green, OverlayPanelStatus);
        }
        g_Context->WriteConsole(String("Health Bars: ") + String(_HealthBars.Length()), RGBColor::Green, OverlayPanelStatus);
    }
    if(DisplayBattlePosition)
    {
        g_Context->WriteConsole(String("Closest Hostile: ") + String(_BattlePosition.ClosestEnemyHostile), RGBColor::Orange, OverlayPanelStatus);
        g_Context->WriteConsole(String("Blink Location: ") + _BattlePosition.SafestBlinkLocation.CommaSeparatedString(), RGBColor::Orange, OverlayPanelStatus);
        g_Context->WriteConsole(String("Retreat Location: ") + _BattlePosition.SafestRetreatLocation.CommaSeparatedString(), RGBColor::Orange, OverlayPanelStatus);
    }
}

UINT FrameUnitManager::CountUnits(const String &Name, PlayerType Owner) const
{
    UINT Sum = 0;
    for(UINT UnitIndex = 0; UnitIndex < _UnitsThisFrame.Length(); UnitIndex++)
    {
        const FrameUnitInfo &CurUnit = *_UnitsThisFrame[UnitIndex];
        if(CurUnit.Entry->Name == Name && CurUnit.Owner == Owner)
        {
            Sum++;
        }
    }
    return Sum;
}

PlayerType FrameUnitManager::PlayerFromColor(const Vec4f &Color)
{
    Vec3f LocalColor(Color.x, Color.y, Color.z);
    if(Vec3f::DistSq(LocalColor, Vec3f(0.0f, 0.7333f, 0.0f)) < 0.01f)
    {
        return PlayerSelf;
    }
    if(Vec3f::DistSq(LocalColor, Vec3f(1.0f, 0.0f, 0.0f)) < 0.01f)
    {
        return PlayerEnemy;
    }
    return PlayerInvalid;
}

void FrameUnitManager::ReportUnitRender(const RenderInfo &Info)
{
    const float BorderRegion = 2.0f;
    const float MinUnitSize = 5.0f;

    if(g_ReportingEvents)
    {
        g_Context->Files.CurrentFrameAllEvents << "FrameUnitManager::RecordUnitRender: " << Info.Texture0->ID() << endl;
    }

    UnitEntry *Unit = Info.Texture0->Unit();
    if(Unit == NULL)
    {
        if(g_ReportingEvents)
        {
            g_Context->Files.CurrentFrameAllEvents << "Rejected: no unit entry\n";
        }
        return;
    }
    if(Unit->Prims != Info.PrimitiveCount && Unit->Prims != -1)
    {
        if(g_ReportingEvents)
        {
            g_Context->Files.CurrentFrameAllEvents << "Rejected: incorrect number of primitives\n";
        }
        return;
    }
    if(Unit->PrimaryType == UnitPrimaryMobile && Unit->Race == RaceZerg && Unit->Name == "Zergling" && Info.PrimitiveCount != 1056 && Info.PrimitiveCount != 900)
    {
        if(g_ReportingEvents)
        {
            g_Context->Files.CurrentFrameAllEvents << "Rejected: zergling with invalid primitive count\n";
        }
        return;
    }
    if(Unit->PrimaryType == UnitPrimaryResource && (Unit->Name == "MineralBlue" || Unit->Name == "MineralGold") && Info.PrimitiveCount <= 44)
    {
        if(g_ReportingEvents)
        {
            g_Context->Files.CurrentFrameAllEvents << "Rejected: minerals are being carried by worker\n";
        }
        return;
    }
	if(Unit->PrimaryType == UnitPrimaryResource && Unit->Name == "VespeneGeyser" && Info.PrimitiveCount != 553 && Info.PrimitiveCount != 5606)
    {
        if(g_ReportingEvents)
        {
            g_Context->Files.CurrentFrameAllEvents << "Rejected: Vespene Geyser with incorrect primitive count\n";
        }
        return;
    }

    const Vec4f CurColor = g_Context->Managers.State.PShaderFloatConstants[1];
    const PlayerType ActivePlayerColor = PlayerFromColor(CurColor);
    if(ActivePlayerColor == PlayerInvalid && Unit->PrimaryType != UnitPrimaryResource)
    {
        if(g_ReportingEvents)
        {
            g_Context->Files.CurrentFrameAllEvents << "Rejected: ActivePlayerColor != PlayerInvalid\n";
        }
        return;
    }

    const Vec2f Center = Info.ScreenBound.Center();
    if(Center.x < BorderRegion || Center.x > float(g_Context->Graphics.WindowDimensions().x) - BorderRegion)
    {
        if(g_ReportingEvents)
        {
            g_Context->Files.CurrentFrameAllEvents << "Rejected: X coord out of bounds\n";
        }
        return;
    }
    if(Center.y < BorderRegion || Center.y > float(g_Context->Graphics.WindowDimensions().y) - BorderRegion)
    {
        if(g_ReportingEvents)
        {
            g_Context->Files.CurrentFrameAllEvents << "Rejected: Y coord out of bounds\n";
        }
        return;
    }

    const Vec2f Dimensions = Info.ScreenBound.Dimensions();
    if(Dimensions.x < MinUnitSize || Dimensions.y < MinUnitSize)
    {
        if(g_ReportingEvents)
        {
            g_Context->Files.CurrentFrameAllEvents << "Rejected: too small\n";
        }
        return;
    }
    if(g_ReportingEvents)
    {
        g_Context->Files.CurrentFrameAllEvents << "Accepted\n";
    }
    
    _UnitsThisFrame.PushEnd(new FrameUnitInfo(Unit, ActivePlayerColor, Info.ScreenBound));
}

bool IsEnergyColor(RGBColor C)
{
    //g_Context->Files.Thread << "Energy: " << C.CommaSeperatedString() << endl;
    const RGBColor BaseEnergyColor(220, 40, 150); // remember BGR vs. RGB swap
    return (RGBColor::DistL1(BaseEnergyColor, C) < 100);
}

void FrameUnitManager::ReportHealthBars(const RenderInfo &Info, const Vector<ProcessedVertex> &ProcessedVertices)
{
    if((Info.PrimitiveCount & 1) == 1 || Info.PrimitiveType != D3DPT_TRIANGLELIST || ProcessedVertices.Length() != Info.PrimitiveCount * 2)
    {
        g_Context->Files.Assert << "Invalid health bar parameters\n";
        return;
    }

    HealthBarInfo BaseHealthBar;
    BaseHealthBar.ColoredBlocks = 0;
    BaseHealthBar.GrayBlocks = 0;
    const RGBColor HealthBarGray(80, 80, 80);

    for(UINT HealthBarIndex = 0; HealthBarIndex < Info.PrimitiveCount / 2; HealthBarIndex++)
    {
        UINT ColorType = 0;
        Rectangle2f ScreenBound;
        for(UINT VertexIndex = 0; VertexIndex < 3; VertexIndex++)
        {
            const ProcessedVertex &CurVertex = ProcessedVertices[HealthBarIndex * 4 + VertexIndex];
            const Vec2f CurScreenPos = Vec2f(CurVertex.TransformedProjectionPos.x, CurVertex.TransformedProjectionPos.y);
            if(VertexIndex == 0)
            {
                ScreenBound.Min = CurScreenPos;
                ScreenBound.Max = CurScreenPos;
                RGBColor Color = CurVertex.Diffuse;
                if(Color == RGBColor::Black)
                {
                    ColorType = 0;
                }
                else if(RGBColor::DistL1(HealthBarGray, Color) < 25)
                {
                    ColorType = 1;
                }
                else
                {
                    if(IsEnergyColor(Color))
                    {
                        BaseHealthBar.Energy = true;
                    }
                    ColorType = 2;
                }
            }
            else
            {
                ScreenBound.ExpandBoundingBox(CurScreenPos);
            }
        }
        if(ColorType == 0)
        {
            if(BaseHealthBar.ColoredBlocks > 0 || BaseHealthBar.GrayBlocks > 0)
            {
                RecordHealthBar(BaseHealthBar);
            }
            BaseHealthBar.ColoredBlocks = 0;
            BaseHealthBar.GrayBlocks = 0;
            BaseHealthBar.ScreenBound = ScreenBound;
            BaseHealthBar.Energy = false;
        }
        else if(ColorType == 1)
        {
            BaseHealthBar.GrayBlocks++;
        }
        else if(ColorType == 2)
        {
            BaseHealthBar.ColoredBlocks++;
        }
    }

    if(BaseHealthBar.ColoredBlocks > 0 || BaseHealthBar.GrayBlocks > 0)
    {
        RecordHealthBar(BaseHealthBar);
    }
}
    
void FrameUnitManager::RecordHealthBar(const HealthBarInfo &CurInfo)
{
    _HealthBars.PushEnd(CurInfo);
    //g_Context->Files.Thread << CurInfo.ScreenBound.Min.CommaSeparatedString() << ' ' << CurInfo.ScreenBound.Dimensions().CommaSeparatedString() << ' ' << CurInfo.Health() << ' ' << String(CurInfo.Energy) << endl;
}

void FrameUnitManager::MatchHealthBars()
{
    for(UINT UnitIndex = 0; UnitIndex < _UnitsThisFrame.Length(); UnitIndex++)
    {
        FrameUnitInfo &CurUnit = *_UnitsThisFrame[UnitIndex];
        Vec2f BasePt = CurUnit.ScreenBound.Min + Vec2f(CurUnit.ScreenBound.Width() * 0.5f, 0.0f);
        float BestDistSq = 125.0f * 125.0f;
        for(UINT BarIndex = 0; BarIndex < _HealthBars.Length(); BarIndex++)
        {
            HealthBarInfo &CurBar = _HealthBars[BarIndex];
            if(!CurBar.Energy)
            {
                float CurDistSq = Vec2f::DistSq(CurBar.ScreenBound.Center(), BasePt);
                if(CurDistSq < BestDistSq)
                {
                    BestDistSq = CurDistSq;
                    CurUnit.HealthBar = &CurBar;
                }
            }
        }
    }
}

float FrameUnitManager::ClosestHostileToPoint(const Vec2f &Pt) const
{
    float BestDist = 1000.0f;
    for(UINT UnitIndex = 0; UnitIndex < _UnitsThisFrame.Length(); UnitIndex++)
    {
        const FrameUnitInfo &CurUnit = *_UnitsThisFrame[UnitIndex];
		if(CurUnit.Owner == PlayerEnemy && CurUnit.Entry->Hostile && CurUnit.Entry->SecondaryType != UnitSecondaryWorker)
        {
            float CurDist = Vec2f::Dist(CurUnit.ScreenBound.Center(), Pt);
            if(CurDist < BestDist)
            {
                BestDist = CurDist;
            }
        }
    }
    return BestDist;
}

Vec2f FrameUnitManager::BestRetreatLocation(const Vec2f &Center, float Radius) const
{
    const UINT ThetaDivisions = 20;
    float BestValue = -1.0f;
    Vec2f BestLocation = Center + Vec2f(Radius, 0.0f);
    for(UINT ThetaIndex = 0; ThetaIndex < ThetaDivisions; ThetaIndex++)
    {
        float Theta = float(ThetaIndex) / float(ThetaDivisions) * 2.0f * Math::PIf;
        Vec2f CurLocation = Center + Vec2f(cosf(Theta) * Radius, sinf(Theta) * Radius);
        float CurValue = ClosestHostileToPoint(CurLocation);
        if(CurValue > BestValue)
        {
            BestValue = CurValue;
            BestLocation = CurLocation;
        }
    }
    return BestLocation;
}

void FrameUnitManager::UpdateBattlePosition()
{
    Vec2f ScreenCenter = g_Context->Constants.GetVec2fConstant(ScreenConstantVec2fScreenCenter);
    _BattlePosition.ClosestEnemyHostile = ClosestHostileToPoint(ScreenCenter);
    
    const float BlinkRange = 275.0f;
    const float RetreatRange = 150.0f;
    _BattlePosition.SafestRetreatLocation = BestRetreatLocation(ScreenCenter, RetreatRange).RoundToVec2i();
    _BattlePosition.SafestBlinkLocation = BestRetreatLocation(ScreenCenter, BlinkRange).RoundToVec2i();
}
