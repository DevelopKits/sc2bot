//
// MinimapManager.cpp
//
// Handles all minimap-related functions.
// Written by Matthew Fisher
//
#include "Main.h"

const RGBColor MinimapResourceColor(126, 191, 241);
const RGBColor MinimapSelfColor0(0, 187, 0);
const RGBColor MinimapSelfColor1(0, 255, 0);
const RGBColor MinimapEnemyColor(255, 0, 0);

const float ArmyPixelThreshold = 0.7f;

void MinimapManager::Init()
{
    ResetNewGame();
}

void MinimapManager::ResetNewGame()
{
    _LastSuccessfulMinimapCapture = GameTime();
    _LastMinimapCapture = GameTime();
    _Cells.Allocate(MinimapGridSize, MinimapGridSize);
    _ArmySize = 0;
    _ArmyScatter = 1.0;
    _ArmyLocation = Vec2f(0.5f, 0.5f);
	_Conflicted = false;
}

void MinimapManager::EndFrame()
{
	if(!g_Context->Controller.ConsoleEnabled())
	{
		return;
	}
    if(DisplayArmyInfo)
    {
        g_Context->WriteConsole(String("Army Size: ") + String(_ArmySize), RGBColor::Green, OverlayPanelStatus);
        g_Context->WriteConsole(String("Army Scatter: ") + String(_ArmyScatter), RGBColor::Green, OverlayPanelStatus);
        g_Context->WriteConsole(String("Army Location: ") + _ArmyLocation.CommaSeparatedString(), RGBColor::Green, OverlayPanelStatus);
    }
}

bool MinimapManager::MinimapCaptureNeeded() const
{
    const double SecondsBetweenSuccessfulMinimapCaptures = 2.0;
    const double SecondsBetweenFailedMinimapCaptures = 0.5;
    double CurTime = GameTime();
    if(CurTime - _LastMinimapCapture < SecondsBetweenFailedMinimapCaptures)
    {
        return false;
    }	
    if(CurTime - _LastSuccessfulMinimapCapture < SecondsBetweenSuccessfulMinimapCaptures)
    {
        return false;
    }
    return true;
}

bool MinimapManager::MinimapIsCovered() const
{
    UINT ResourcePixels = _Minimap.CountPixelsWithColor(MinimapResourceColor);
    UINT SelfPixels = _Minimap.CountPixelsWithColor(MinimapSelfColor0) + _Minimap.CountPixelsWithColor(MinimapSelfColor1);
    return (ResourcePixels < 36 && SelfPixels < 36);
}

float MinimapManager::MinimapCoordDistToBase(const Vec2f &MinimapCoord) const
{
    float BestDist = 2.0f;
    KnowledgeManager &Knowledge = g_Context->Managers.Knowledge;
    for(UINT BaseIndex = 0; BaseIndex < Knowledge.AllBaseInfo().Length(); BaseIndex++)
    {
        const BaseInfo &CurBase = Knowledge.AllBaseInfo()[BaseIndex];
        if(CurBase.Player == PlayerSelf)
        {
            float Dist0 = Vec2f::Dist(CurBase.BaseLocation, MinimapCoord);
            if(Dist0 < BestDist)
            {
                BestDist = Dist0;
            }
            float Dist1 = Vec2f::Dist(CurBase.BuildLocation, MinimapCoord);
            if(Dist1 < BestDist)
            {
                BestDist = Dist1;
            }
        }
    }
    return BestDist;
}

void MinimapManager::ReportMinimapCapture()
{
    //g_Context->Files.Events << "Minimap capture: " << GameTime() << endl;
    _LastMinimapCapture = GameTime();
    if(MinimapIsCovered())
    {
        return;
    }
    _LastSuccessfulMinimapCapture = _LastMinimapCapture;
    _SelfPixelCoordinates.ReSize(0);
    _EnemyPixelCoordinates.ReSize(0);
    const UINT Width = _Minimap.Width();
    const UINT Height = _Minimap.Height();
    const float WidthNormalization = 1.0f / float(Width - 1);
    const float HeightNormalization = 1.0f / float(Height - 1);
    for(UINT Y = 0; Y < Height; Y++)
    {
        for(UINT X = 0; X < Width; X++)
        {
            RGBColor C = _Minimap[Y][X];
            if(C == MinimapSelfColor0 || C == MinimapSelfColor1)
            {
                _SelfPixelCoordinates.PushEnd(Vec2f(float(X) * WidthNormalization, 1.0f - Y * HeightNormalization));
            }
            if(C == MinimapEnemyColor)
            {
                _EnemyPixelCoordinates.PushEnd(Vec2f(float(X) * WidthNormalization, 1.0f - Y * HeightNormalization));
            }
        }
    }
    if(_SelfPixelCoordinates.Length() == 0)
    {
        _SelfPixelCoordinates.PushEnd(Vec2f(0.5f, 0.5f));
    }
    UpdateCells();
    UpdateHistory();
    UpdateArmyStats();
    //_Minimap.SavePNG(g_Context->Parameters.OutputFileDirectory + String("Minimap") + String(g_Context->Controller.FrameIndex()) + String(".png"));
}

void MinimapManager::UpdateHistory()
{
    const UINT Width = _Minimap.Width();
    const UINT Height = _Minimap.Height();
    if(_PixelHistory.Rows() != _Minimap.Height() || _PixelHistory.Cols() != _Minimap.Width())
    {
        _PixelHistory.Allocate(_Minimap.Height(), _Minimap.Width());
        MinimapPixelHistory Pixel;
        Pixel.ObservationFrequency = 0.0f;
        _PixelHistory.Clear(Pixel);
    }
    for(UINT Y = 0; Y < Height; Y++)
    {
        for(UINT X = 0; X < Width; X++)
        {
            RGBColor C = _Minimap[Y][X];
            MinimapPixelHistory &CurHistory = _PixelHistory(Y, X);
            if(C == MinimapSelfColor0 || C == MinimapSelfColor1)
            {
                CurHistory.ObservationFrequency += 0.2f;
                if(CurHistory.ObservationFrequency > 1.0f)
                {
                    CurHistory.ObservationFrequency = 1.0f;
                }
            }
            else
            {
                CurHistory.ObservationFrequency = 0.0f;
            }
        }
    }
}

void MinimapManager::UpdateCells()
{
    //Grid<MinimapCell> CurCells(MinimapGridSize, MinimapGridSize);
    const int Radius = 1;
    for(UINT Y = 0; Y < MinimapGridSize; Y++)
    {
        for(UINT X = 0; X < MinimapGridSize; X++)
        {
            MinimapCell &CurCell = _Cells(Y, X);
            CurCell.AllyNearby = false;
            CurCell.EnemyNearby = false;
        }
    }

    for(UINT PixelIndex = 0; PixelIndex < _SelfPixelCoordinates.Length(); PixelIndex++)
    {
        Vec2i CellCoord = CellCoordFromMinimapCoord(_SelfPixelCoordinates[PixelIndex]);
        for(int RadiusY = -Radius; RadiusY <= Radius; RadiusY++)
        {
            for(int RadiusX = -Radius; RadiusX <= Radius; RadiusX++)
            {
                Vec2i FinalCoord = CellCoord + Vec2i(RadiusX, RadiusY);
                if(_Cells.ValidCoordinates(FinalCoord.y, FinalCoord.x))
                {
                    MinimapCell &CurCell = _Cells(FinalCoord.y, FinalCoord.x);
                    CurCell.AllyNearby = true;
                    CurCell.LastExploreTime = GameTime();
                }
            }
        }
    }

    for(UINT PixelIndex = 0; PixelIndex < _EnemyPixelCoordinates.Length(); PixelIndex++)
    {
        Vec2i CellCoord = CellCoordFromMinimapCoord(_EnemyPixelCoordinates[PixelIndex]);
        for(int RadiusY = -Radius; RadiusY <= Radius; RadiusY++)
        {
            for(int RadiusX = -Radius; RadiusX <= Radius; RadiusX++)
            {
                Vec2i FinalCoord = CellCoord + Vec2i(RadiusX, RadiusY);
                if(_Cells.ValidCoordinates(FinalCoord.y, FinalCoord.x))
                {
                    MinimapCell &CurCell = _Cells(FinalCoord.y, FinalCoord.x);
                    CurCell.EnemyNearby = true;
                }
            }
        }
    }

	_Conflicted = false;
	for(UINT Y = 0; Y < _Cells.Rows(); Y++)
    {
        for(UINT X = 0; X < _Cells.Cols(); X++)
        {
            const MinimapCell &CurCell = _Cells(Y, X);
            if(CurCell.AllyNearby && CurCell.EnemyNearby)
            {
                _Conflicted = true;
            }
        }
    }
}

void MinimapManager::MakeExplorationBitmap(Bitmap &Result) const
{
    Result = _Minimap;
    const UINT Width = _Minimap.Width();
    const UINT Height = _Minimap.Height();
    const float WidthNormalization = 1.0f / float(Width - 1);
    const float HeightNormalization = 1.0f / float(Height - 1);
    for(UINT Y = 0; Y < Height; Y++)
    {
        for(UINT X = 0; X < Width; X++)
        {
            Vec2f MinimapCoord = Vec2f(float(X) * WidthNormalization, 1.0f - Y * HeightNormalization);
            Vec2i CellCoord = CellCoordFromMinimapCoord(MinimapCoord);
            const MinimapCell &CurCell = _Cells(CellCoord.y, CellCoord.x);
            RGBColor Color;
            if(CurCell.LastExploreTime == -1.0)
            {
                Color = RGBColor::Black;
            }
            else
            {
                Color = RGBColor::White;
            }
            Result[Y][X] = Color;
        }
    }
}

Vec2f MinimapManager::FindNearestEnemyCoord(const Vec2f &MinimapCoord) const
{
	if(_EnemyPixelCoordinates.Length() == 0)
	{
		return MinimapCoord;
	}
	Vec2f Result(0.5f, 0.5f);
	float BestDistSq = 0.0f;
	for(UINT PixelIndex = 0; PixelIndex < _EnemyPixelCoordinates.Length(); PixelIndex++)
    {
		float CurDistSq = Vec2f::DistSq(_EnemyPixelCoordinates[PixelIndex], MinimapCoord);
		if(PixelIndex == 0 || CurDistSq < BestDistSq)
		{
			BestDistSq = CurDistSq;
			Result = _EnemyPixelCoordinates[PixelIndex];
		}
	}
	return Result;
}

void MinimapManager::MakeArmyBitmap(Bitmap &Result) const
{
    Result = _Minimap;

    if(_PixelHistory.Rows() != _Minimap.Height() || _PixelHistory.Cols() != _Minimap.Width())
    {
        g_Context->Files.Assert << "Invalid size for PixelHistory\n";
        return;
    }

    const UINT Width = _Minimap.Width();
    const UINT Height = _Minimap.Height();
    const float WidthNormalization = 1.0f / float(Width - 1);
    const float HeightNormalization = 1.0f / float(Height - 1);
    for(UINT Y = 0; Y < Height; Y++)
    {
        for(UINT X = 0; X < Width; X++)
        {
            Vec2f MinimapCoord = Vec2f(float(X) * WidthNormalization, 1.0f - Y * HeightNormalization);
            RGBColor C = _Minimap[Y][X];
            RGBColor Color = RGBColor::Black;
            const MinimapPixelHistory &CurHistory = _PixelHistory(Y, X);
            if(C == MinimapSelfColor0 || C == MinimapSelfColor1)
            {
                if(CurHistory.ObservationFrequency >= ArmyPixelThreshold)
                {
                    Color = RGBColor::Green;
                }
                else
                {
                    if(MinimapCoordDistToBase(MinimapCoord) <= 0.1f)
                    {
                        Color = RGBColor::Yellow;
                    }
                    else
                    {
                        Color = RGBColor::Red;
                    }
                }
            }
            Result[Y][X] = Color;
        }
    }
}

void MinimapManager::UpdateArmyStats()
{
    Vector<Vec2f> ArmyCoordinates;
    const UINT Width = _Minimap.Width();
    const UINT Height = _Minimap.Height();
    const float WidthNormalization = 1.0f / float(Width - 1);
    const float HeightNormalization = 1.0f / float(Height - 1);
    for(UINT Y = 0; Y < Height; Y++)
    {
        for(UINT X = 0; X < Width; X++)
        {
            Vec2f MinimapCoord = Vec2f(float(X) * WidthNormalization, 1.0f - Y * HeightNormalization);
            RGBColor C = _Minimap[Y][X];
            //const MinimapPixelHistory &CurHistory = _PixelHistory(Y, X);
            if(C == MinimapSelfColor0 || C == MinimapSelfColor1)
            {
                //if(CurHistory.ObservationFrequency < ArmyPixelThreshold)
                {
                    if(MinimapCoordDistToBase(MinimapCoord) > 0.15f)
                    {
                        ArmyCoordinates.PushEnd(MinimapCoord);
                    }
                }
            }
        }
    }

    const UINT Length = ArmyCoordinates.Length();
    _ArmySize = Length;
    _ArmyScatter = 1.0;
    _ArmyLocation = Vec2f(0.5f, 0.5f);
    if(Length == 0)
    {
        return;
    }
    
    const float Threshold = 0.075f;
    const float ThresholdSq = Threshold * Threshold;
    
    for(UINT Index0 = 0; Index0 < Length; Index0++)
    {
        UINT Count = 0;
        Vec2f CandidateCenter = ArmyCoordinates[Index0];
        for(UINT Index1 = 0; Index1 < Length; Index1++)
        {
            if(Vec2f::DistSq(CandidateCenter, ArmyCoordinates[Index1]) <= ThresholdSq)
            {
                Count++;
            }
        }
        double CurrentScatter = 1.0 - double(Count) / double(Length);
        if(CurrentScatter < _ArmyScatter)
        {
            _ArmyScatter = CurrentScatter;
            _ArmyLocation = CandidateCenter;
        }
    }
}
