//
// FrameHUDManager.cpp
//
// Handles the set of all relevant HUD events in a single frame.
// Written by Matthew Fisher
//
#include "Main.h"

void FrameHUDManager::StartFrame()
{
    _Minerals = -1;
    _Gas = -1;
    _SupplyCurrent = -1;
    _SupplyCap = -1;
    _CurHP = -1;
    _MaxHP = -1;
    _CurShields = -1;
    _MaxShields = -1;
    _CurEnergy = -1;
    _MaxEnergy = -1;
    _ResourcesRemaining = -1;
    _ControlGroupLevel = 0;
    _SoloPortrait = NULL;
    
    _WarpGatePresent = false;
    _WarpGateCount = 0;
    
    _IdleWorker = false;
    _BuildQueuePresent = false;
    
    for(UINT PortraitIndex = 0; PortraitIndex < BubblePortraitCount; PortraitIndex++)
    {
        _BubblePortraits[PortraitIndex].Tex = NULL;
    }

    for(UINT ActionButtonIndex = 0; ActionButtonIndex < ActionButtonCount; ActionButtonIndex++)
    {
        _ActionButtons[ActionButtonIndex].Tex = NULL;
        _ActionButtons[ActionButtonIndex].State = ActionButtonStateInvalid;
    }

    for(UINT BuildQueueIndex = 0; BuildQueueIndex < BuildQueueCount; BuildQueueIndex++)
    {
        _BuildQueue[BuildQueueIndex].Tex = NULL;
    }

    _MinimapViewportObserved = false;
    _MinimapViewportCenter = Vec2f(0.5f, 0.5f);
}

void FrameHUDManager::EndFrame()
{
	if(!g_Context->Controller.ConsoleEnabled())
	{
		return;
	}
    if(DisplayHUDDetails)
    {
        g_Context->WriteConsole(String("ControlGroupLevel: ") + String(_ControlGroupLevel), RGBColor::White, OverlayPanelStatus);
        g_Context->WriteConsole(String("WarpGateCount: ") + String(_WarpGateCount), RGBColor::White, OverlayPanelStatus);
    }
    if(DisplayViewport)
    {
        g_Context->WriteConsole(String("Viewport: ") + _MinimapViewportCenter.CommaSeparatedString(), RGBColor::White, OverlayPanelStatus);
    }
    if(DisplayUnitInfo)
    {
        g_Context->WriteConsole(String("Shields: ") + String(_CurShields) + String("/") + String(_MaxShields), RGBColor::Blue, OverlayPanelStatus);
        g_Context->WriteConsole(String("HP: ") + String(_CurHP) + String("/") + String(_MaxHP), RGBColor::Green, OverlayPanelStatus);
        g_Context->WriteConsole(String("Energy: ") + String(_CurEnergy) + String("/") + String(_MaxEnergy), RGBColor::Purple, OverlayPanelStatus);
        g_Context->WriteConsole(String("Resources remaining: ") + String(_ResourcesRemaining), RGBColor::Green, OverlayPanelStatus);
        if(_ResourcesRemaining > 0)
        {
            g_Context->Files.Events << GameTime() << '\t' << _ResourcesRemaining << endl;
        }
    }
    if(DisplayResources)
    {
        g_Context->WriteConsole(String("Minerals: ") + String(_Minerals), RGBColor::Blue, OverlayPanelStatus);
        g_Context->WriteConsole(String("Gas: ") + String(_Gas), RGBColor::Green, OverlayPanelStatus);
        g_Context->WriteConsole(String("Supply: ") + String(_SupplyCurrent) + String("/") + String(_SupplyCap), RGBColor::White, OverlayPanelStatus);
    }
    if(DisplayPortraits)
    {
        if(_SoloPortrait != NULL)
        {
            g_Context->WriteConsole(String("Solo: ") + String(_SoloPortrait->ID()), RGBColor::Orange, OverlayPanelStatus);
        }
        for(UINT Y = 0; Y < 3; Y++)
        {
            String S = String("Row ") + String(Y) + String(": ");
            for(UINT X = 0; X < 8; X++)
            {
                Texture *CurTexture = _BubblePortraits[Y * 8 + X].Tex;
                if(CurTexture == NULL)
                {
                    S += "E,";
                }
                else
                {
                    S += CurTexture->ID() + String(",");
                }
            }
            g_Context->WriteConsole(S, RGBColor::Orange, OverlayPanelStatus);
        }
    }
    if(DisplayActionButtons)
    {
        for(UINT Y = 0; Y < 3; Y++)
        {
            String S = String("Row ") + String(Y) + String(": ");
            for(UINT X = 0; X < 5; X++)
            {
                const FrameActionButton &CurButton = _ActionButtons[Y * 5 + X];
                String Modifier;
                if(CurButton.State == ActionButtonStateInvalid)
                {
                    Modifier = "I";
                }
                if(CurButton.State == ActionButtonStateNormal)
                {
                    Modifier = "N";
                }
                if(CurButton.State == ActionButtonStateDisabled)
                {
                    Modifier = "D";
                }
                if(CurButton.State == ActionButtonStateSelected)
                {
                    Modifier = "S";
                }
                if(CurButton.State == ActionButtonStateNotEnoughEnergy)
                {
                    Modifier = "O";
                }
                if(CurButton.State == ActionButtonCoolingDown)
                {
                    Modifier = "C";
                }
                if(CurButton.Tex == NULL)
                {
                    S += "E,";
                }
                else
                {
                    S += Modifier + CurButton.Tex->ID() + String(",");
                }
            }
            if(S.Length() > 0)
            {
                S.PopEnd();
            }
            g_Context->WriteConsole(S, RGBColor::Orange, OverlayPanelStatus);
        }
    }
}

UINT FrameHUDManager::CountUnitsWithName(const String &Name) const
{
    if(_SoloPortrait != NULL)
    {
        if(_SoloPortrait->ID() == Name)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        UINT Result = 0;
        for(UINT PortraitIndex = 0; PortraitIndex < BubblePortraitCount; PortraitIndex++)
        {
            Texture *CurTexture = _BubblePortraits[PortraitIndex].Tex;
            if(CurTexture != NULL && CurTexture->ID() == Name)
            {
                Result++;
            }
        }
        return Result;
    }
}

void FrameHUDManager::ReportMinimapViewport(const RenderInfo &Info, const Vector<ProcessedVertex> &ProcessedVertices)
{
    _MinimapViewportObserved = true;
    Vec2f AverageScreenPos = Vec2f::Origin;
    for(UINT Index = 0; Index < 8; Index++)
    {
        const Vec4f &Vertex = ProcessedVertices[Index].TransformedProjectionPos;
        AverageScreenPos += Vec2f(Vertex.x, Vertex.y);
    }
    AverageScreenPos *= 0.125f;
    _MinimapViewportCenter = g_Context->Constants.ScreenCoordToMinimapCoord(AverageScreenPos);
}

void FrameHUDManager::ReportWarpGatePresent()
{
    _WarpGatePresent = true;
}

void FrameHUDManager::ReportIdleWorker()
{
    _IdleWorker = true;
}

void FrameHUDManager::ReportBuildQueue()
{
    _BuildQueuePresent = true;
}

bool FrameHUDManager::CurrentUnitUnderConstruction() const
{
    for(UINT ActionIndex = 0; ActionIndex < ActionButtonCount - 1; ActionIndex++)
    {
        const FrameActionButton &CurAction = _ActionButtons[ActionIndex];
        if(CurAction.Tex != NULL)
        {
            return false;
        }
    }
    return (_ActionButtons[ActionButtonCount - 1].Tex != NULL && _ActionButtons[ActionButtonCount - 1].Tex->ID() == "Cancel");
}

void FrameHUDManager::ReportFont(const RenderInfo &Info)
{
    const Rectangle2i &UnitInfoRegion = g_Context->Constants.GetRectangle2iConstant(ScreenConstantRectangle2iUnitInfoRegion);
	const Rectangle2i &ResourceRegion = g_Context->Constants.GetRectangle2iConstant(ScreenConstantRectangle2iResourceRegion);
    const Rectangle2i &WarpGateRegion = g_Context->Constants.GetRectangle2iConstant(ScreenConstantRectangle2iWarpGateRegion);

    if(ScreenBoundInsideRegion(Info.ScreenBound, ResourceRegion))
    {
        ProcessResourceString(Info.Text);
    }
    if(ScreenBoundInsideRegion(Info.ScreenBound, UnitInfoRegion))
    {
        ProcessUnitInfoString(Info.Text);
    }
    if(ScreenBoundInsideRegion(Info.ScreenBound, WarpGateRegion))
    {
        int Value = Info.Text.ConvertToInteger();
        if(Value >= 0 && Value < 20)
        {
            _WarpGateCount = Value;
        }
    }
}

void FrameHUDManager::ReportControlGroupLevel(const RenderInfo &Info)
{
    UINT NewValue = 0;
    if(Info.PrimitiveCount == 18 * 1)
    {
        NewValue = 1;
    }
    else if(Info.PrimitiveCount == 18 * 2)
    {
        NewValue = 2;
    }
    else if(Info.PrimitiveCount == 18 * 3)
    {
        NewValue = 3;
    }
    else if(Info.PrimitiveCount == 18 * 4)
    {
        NewValue = 4;
    }
    else if(Info.PrimitiveCount == 18 * 5)
    {
        NewValue = 5;
    }
    else
    {
        g_Context->Files.Assert << "Unexpected ControlGroupLevel\n";
        return;
    }
    if(_ControlGroupLevel != 0 && _ControlGroupLevel != NewValue)
    {
        g_Context->Files.Assert << "Conflicting ControlGroupLevel: " << _ControlGroupLevel << ' ' << NewValue << endl;
        return;
    }
    _ControlGroupLevel = NewValue;
}

void FrameHUDManager::ReportBubbleIcon(const RenderInfo &Info)
{
    if(Info.PrimitiveCount != 2 || Info.PrimitiveType != D3DPT_TRIANGLELIST)
    {
        g_Context->Files.Assert << "Invalid icon parameters\n";
    }
    for(UINT Index = 0; Index < BuildQueueCount; Index++)
    {
        FrameBuildQueue &CurBuildQueue = _BuildQueue[Index];
        if(CurBuildQueue.Tex == NULL)
        {
            CurBuildQueue.Tex = Info.Texture1;
            return;
        }
    }
}

void FrameHUDManager::ReportPortrait(const RenderInfo &Info, const Vector<ProcessedVertex> &ProcessedVertices)
{
    if(Info.PrimitiveCount > 48 || (Info.PrimitiveCount & 1) == 1 || Info.PrimitiveType != D3DPT_TRIANGLELIST || ProcessedVertices.Length() != Info.PrimitiveCount * 2)
    {
        g_Context->Files.Assert << "Invalid bubble portrait parameters\n";
        return;
    }
    for(UINT BubbleIndex = 0; BubbleIndex < Info.PrimitiveCount / 2; BubbleIndex++)
    {
        Rectangle2f ScreenPosBounds;
        for(UINT VertexIndex = 0; VertexIndex < 3; VertexIndex++)
        {
            const ProcessedVertex &CurVertex = ProcessedVertices[BubbleIndex * 4 + VertexIndex];
            const Vec2f CurScreenPos = Vec2f(CurVertex.TransformedProjectionPos.x, CurVertex.TransformedProjectionPos.y);
            if(VertexIndex == 0)
            {
                ScreenPosBounds.Min = CurScreenPos;
                ScreenPosBounds.Max = CurScreenPos;
            }
            else
            {
                ScreenPosBounds.ExpandBoundingBox(CurScreenPos);
            }
        }
        if(Info.PrimitiveCount == 2 && Math::Round(ScreenPosBounds.Width()) == 89 && Math::Round(ScreenPosBounds.Height()) == 89)
        {
            if(_SoloPortrait == NULL)
            {
                if(Info.Texture0 == NULL || Info.Texture0->Unit() == NULL)
                {
                    g_Context->Files.Assert << "No unit info found for portrait\n";
                }
                else
                {
                    _SoloPortrait = Info.Texture0;
                }
            }
            else
            {
                g_Context->Files.Assert << "Multiple solo portraits\n";
            }
        }
        else
        {
            int PortraitIndex = BubblePortraitIndexFromScreenBounds(ScreenPosBounds);
            if(PortraitIndex != -1)
            {
                FrameBubblePortrait &CurPortrait = _BubblePortraits[PortraitIndex];
                if(Info.Texture0->ID() == "PortraitBubble")
                {
                    CurPortrait.Tex = Info.Texture1;
                }
                else
                {
                    CurPortrait.Tex = Info.Texture0;
                }
                if(CurPortrait.Tex != NULL && CurPortrait.Tex->Unit() == NULL)
                {
                    g_Context->Files.Assert << "No unit info found for portrait\n";
                    CurPortrait.Tex = NULL;
                }
            }
        }
    }
}

bool FrameHUDManager::ValidateSingleUnit(const String &Name) const
{
    if(_SoloPortrait != NULL)
    {
        return (Name == _SoloPortrait->ID());
    }
    else
    {
        if(_BubblePortraits[0].Tex == NULL)
        {
            return false;
        }
        for(UINT PortraitIndex = 1; PortraitIndex < BubblePortraitCount; PortraitIndex++)
        {
            if(_BubblePortraits[PortraitIndex].Tex != NULL && _BubblePortraits[PortraitIndex].Tex->ID() != Name)
            {
                return false;
            }
        }
        return true;
    }
}

bool BuildingsEquivalent(const String &A, const String &B)
{
	return (A == "Gateway" && B == "WarpGate");
}

bool FrameHUDManager::SingleUnitTypeSelected() const
{
    if(_BubblePortraits[0].Tex == NULL)
    {
        return false;
    }
    for(UINT PortraitIndex = 1; PortraitIndex < BubblePortraitCount; PortraitIndex++)
    {
        if(_BubblePortraits[PortraitIndex].Tex != NULL)
        {
			if(_BubblePortraits[PortraitIndex].Tex == _BubblePortraits[0].Tex ||
			   BuildingsEquivalent(_BubblePortraits[PortraitIndex].Tex->ID(), _BubblePortraits[0].Tex->ID()) ||
			   BuildingsEquivalent(_BubblePortraits[0].Tex->ID(), _BubblePortraits[PortraitIndex].Tex->ID()))
			{
				
			}
			else
			{
				return false;
			}
        }
    }
    return true;
}

bool FrameHUDManager::HaveResourcesForUnit(const UnitEntry &Entry) const
{
    return (_Minerals >= int(Entry.MineralCost) && _Gas >= int(Entry.GasCost) && _SupplyCap - _SupplyCurrent >= int(Entry.SupplyCost));
}

Vec2i FrameHUDManager::ScreenCoordFromControlGroupLevel(UINT Level) const
{
    const Vec2f TopLeft = g_Context->Constants.GetVec2fConstant(ScreenConstantVec2fControlGroupLevelTopLeft);
    const Vec2f Displacement = g_Context->Constants.GetVec2fConstant(ScreenConstantVec2fControlGroupLevelDisplacement);

    return (TopLeft + Displacement * float(Level)).RoundToVec2i();
}

Vec2i FrameHUDManager::ScreenCoordFromBubblePortraitIndex(UINT Index) const
{
    const Vec2f TopLeft = g_Context->Constants.GetVec2fConstant(ScreenConstantVec2fBubblePortraitTopLeft);
    const Vec2f ExpectedDimensions = g_Context->Constants.GetVec2fConstant(ScreenConstantVec2fBubblePortraitDimensions);
    UINT Row = Index / 8;
    UINT Col = Index - Row * 8;
    Vec2f ScreenCoord = TopLeft + Vec2f(ExpectedDimensions.x * (Col + 0.5f), ExpectedDimensions.y * (Row + 0.5f));
    return ScreenCoord.RoundToVec2i();
}

int FrameHUDManager::BubblePortraitIndexFromScreenBounds(const Rectangle2f &ScreenBounds) const
{
    const Vec2f ObservedDimensions = ScreenBounds.Dimensions();
    const Vec2f TopLeft = g_Context->Constants.GetVec2fConstant(ScreenConstantVec2fBubblePortraitTopLeft);
    const Vec2f ExpectedDimensions = g_Context->Constants.GetVec2fConstant(ScreenConstantVec2fBubblePortraitDimensions);
    if((ExpectedDimensions - ObservedDimensions).LengthSq() >= 4.0f)
    {
        g_Context->Files.Assert << "Invalid bubble portrait dimensions: Observed=" << ObservedDimensions.CommaSeparatedString() << " Expected=" << ExpectedDimensions.CommaSeparatedString() << endl;
        return -1;
    }
    Vec2f Offset = ScreenBounds.Min - TopLeft;
    int XIndex = Math::Round(Offset.x / ExpectedDimensions.x);
    int YIndex = Math::Round(Offset.y / ExpectedDimensions.y);
    if(XIndex < 0 || YIndex < 0 || XIndex >= 8 || YIndex >= 3)
    {
        g_Context->Files.Assert << "Invalid bubble portrait coordinates\n";
        return -1;
    }
    return (YIndex * 8 + XIndex);
}

int FrameHUDManager::ActionButtonIndexFromScreenBounds(const Rectangle2f &ScreenBounds) const
{
    const Vec2f ObservedDimensions = ScreenBounds.Dimensions();
    const Vec2f TopLeft = g_Context->Constants.GetVec2fConstant(ScreenConstantVec2fActionButtonTopLeft);
    const Vec2f ExpectedDimensions = g_Context->Constants.GetVec2fConstant(ScreenConstantVec2fActionButtonDimensions);
    if((ExpectedDimensions - ObservedDimensions).LengthSq() >= 4.0f)
    {
        //g_Context->Files.Assert << "Invalid action button dimensions: Observed=" << ObservedDimensions.CommaSeparatedString() << " Expected=" << ExpectedDimensions.CommaSeparatedString() << " Center=" << ScreenBounds.Center().CommaSeparatedString() << endl;
        return -1;
    }
    Vec2f Offset = ScreenBounds.Min - TopLeft;
    int XIndex = Math::Round(Offset.x / ExpectedDimensions.x);
    int YIndex = Math::Round(Offset.y / ExpectedDimensions.y);
    if(XIndex < 0 || YIndex < 0 || XIndex >= 5 || YIndex >= 3)
    {
        g_Context->Files.Assert << "Invalid action button coordinates\n";
        return -1;
    }
    return (YIndex * 5 + XIndex);
}

void FrameHUDManager::ReportActionButtonBackground(const RenderInfo &Info, const Vector<ProcessedVertex> &ProcessedVertices)
{
    if(Info.PrimitiveCount > 30 || (Info.PrimitiveCount & 1) == 1 || Info.PrimitiveType != D3DPT_TRIANGLELIST || ProcessedVertices.Length() != Info.PrimitiveCount * 2)
    {
        g_Context->Files.Assert << "Invalid action button background parameters\n";
        return;
    }
    ActionButtonStateType State = ActionButtonStateNormal;
    if(Info.Texture0->ID() == "ActionButtonSelectedBubble")
    {
        State = ActionButtonStateSelected;
    }

    const Vec4f CurColor = g_Context->Managers.State.PShaderFloatConstants[0];
    if(Math::Abs(CurColor.x - 0.50196f) < 0.01f && 
       Math::Abs(CurColor.y - 0.50196f) < 0.01f &&
       Math::Abs(CurColor.z - 0.50196f) < 0.01f)
    {
        State = ActionButtonStateDisabled;
        g_Context->Managers.State.PShaderFloatConstants[0] = Vec4f::Origin;
    }

    for(UINT ButtonIndex = 0; ButtonIndex < Info.PrimitiveCount / 2; ButtonIndex++)
    {
        Rectangle2f ScreenPosBounds;
        for(UINT VertexIndex = 0; VertexIndex < 3; VertexIndex++)
        {
            const ProcessedVertex &CurVertex = ProcessedVertices[ButtonIndex * 4 + VertexIndex];
            const Vec2f CurScreenPos = Vec2f(CurVertex.TransformedProjectionPos.x, CurVertex.TransformedProjectionPos.y);
            if(VertexIndex == 0)
            {
                ScreenPosBounds.Min = CurScreenPos;
                ScreenPosBounds.Max = CurScreenPos;
            }
            else
            {
                ScreenPosBounds.ExpandBoundingBox(CurScreenPos);
            }
        }
        int ButtonIndex = ActionButtonIndexFromScreenBounds(ScreenPosBounds);
        if(ButtonIndex != -1)
        {
            FrameActionButton &CurActionButton = _ActionButtons[ButtonIndex];
            CurActionButton.State = State;
        }
    }
}

void FrameHUDManager::ReportActionButtonIcon(const RenderInfo &Info, const Vector<ProcessedVertex> &ProcessedVertices)
{
    if(Info.PrimitiveCount != 2 || Info.PrimitiveType != D3DPT_TRIANGLELIST || ProcessedVertices.Length() != 4)
    {
        g_Context->Files.Assert << "Invalid action button icon parameters\n";
    }
    Rectangle2f ScreenPosBounds;
    for(UINT VertexIndex = 0; VertexIndex < 3; VertexIndex++)
    {
        const ProcessedVertex &CurVertex = ProcessedVertices[VertexIndex];
        const Vec2f CurScreenPos = Vec2f(CurVertex.TransformedProjectionPos.x, CurVertex.TransformedProjectionPos.y);
        if(VertexIndex == 0)
        {
            ScreenPosBounds.Min = CurScreenPos;
            ScreenPosBounds.Max = CurScreenPos;
        }
        else
        {
            ScreenPosBounds.ExpandBoundingBox(CurScreenPos);
        }
    }
    int ButtonIndex = ActionButtonIndexFromScreenBounds(ScreenPosBounds);
    if(ButtonIndex != -1)
    {
        FrameActionButton &CurActionButton = _ActionButtons[ButtonIndex];
        CurActionButton.Tex = Info.Texture0;
        if(Info.Texture1->Type() == RenderSpecial && Info.Texture1->ID() == "CooldownGrid")
        {
            CurActionButton.State = ActionButtonCoolingDown;
        }
        else //if(Info.PShaderHash == 84762009 || Info.PShaderHash == 200960479)
        {
            const Vec4f CurColor = g_Context->Managers.State.PShaderFloatConstants[0];
            if(Math::Abs(CurColor.x - 0.462744f) < 0.01f && 
               Math::Abs(CurColor.y - 0.286274f) < 0.01f)
            {
                if(CurActionButton.State == ActionButtonStateNormal)
                {
                    CurActionButton.State = ActionButtonStateNotEnoughEnergy;
                    g_Context->Managers.State.PShaderFloatConstants[0] = Vec4f::Origin;
                }
            }
        }
    }
}

bool FrameHUDManager::BuildQueueEntryPresent(const String &Name) const
{
    for(UINT Index = 0; Index < BuildQueueCount; Index++)
    {
        if(_BuildQueue[Index].Tex != NULL && _BuildQueue[Index].Tex->ID() == Name)
        {
            return true;
        }
    }
    return false;
}

bool FrameHUDManager::ActionButtonPresent(const String &Name, ActionButtonStateType &State) const
{
    for(UINT Index = 0; Index < ActionButtonCount; Index++)
    {
        if(_ActionButtons[Index].Tex != NULL && _ActionButtons[Index].Tex->ID() == Name)
        {
            State = _ActionButtons[Index].State;
            return true;
        }
    }
    return false;
}

void FrameHUDManager::ProcessResourceString(const String &S)
{
    Vector<String> Words, SupplyWords;
    S.Partition('@', Words);
    if(Words.Length() != 3)
    {
        return;
    }
    Words[0].Partition('/', SupplyWords);
    if(SupplyWords.Length() != 2)
    {
        return;
    }
    _Minerals = Words[2].ConvertToInteger();
    _Gas = Words[1].ConvertToInteger();
    _SupplyCurrent = SupplyWords[0].ConvertToInteger();
    _SupplyCap = SupplyWords[1].ConvertToInteger();
}

float FrameHUDManager::CurrentHealthPercentage() const
{
	if(_CurHP <= 0 || _MaxHP <= 0)
	{
		return -1.0f;
	}
	int CurrentHealth = _CurHP;
	int TotalHealth = _MaxHP;
	if(_CurShields > 0 && _MaxShields > 0)
	{
		CurrentHealth += _CurShields;
		TotalHealth += _MaxShields;
	}
	return float(CurrentHealth) / float(TotalHealth);
}

bool IsBarStatString(const String &S, int &Current, int &Max)
{
    Vector<String> Words;
    S.Partition('/', Words);
    if(Words.Length() != 2)
    {
        return false;
    }
    Current = Words[0].ConvertToInteger();
    Max = Words[1].ConvertToInteger();
    return true;
}

void FrameHUDManager::ProcessUnitInfoString(const String &S)
{
    Vector<String> Words, HPWords;
    S.Partition('@', Words);
    if(Words.Length() == 0)
    {
        return;
    }

	/*if(Words.Length() >= 3 && Words[2].StartsWith("Remaining:"))
    {
        _ResourcesRemaining = Words[2].RemovePrefix("Remaining:").ConvertToInteger();
    }*/
    
    UINT BarCount = 0;
    int BarCurrent[3], BarMax[3];

	for(UINT WordIndex = 0; WordIndex < Words.Length(); WordIndex++)
	{
		int Current, Max;
		const String &CurWord = Words[WordIndex];
		if(IsBarStatString(CurWord, Current, Max))
		{
			if(BarCount < 3)
			{
				BarCurrent[BarCount] = Current;
				BarMax[BarCount] = Max;
				BarCount++;
			}
		}
	}

	if(BarCount == 0)
	{
		return;
	}

    bool Protoss = (g_Context->Managers.Knowledge.MyRace() == RaceProtoss);
    if(BarCount == 1)
    {
        _CurHP = BarCurrent[0];
        _MaxHP = BarMax[0];
    }
    else if(BarCount == 2)
    {
        if(Protoss)
        {
            _CurShields = BarCurrent[0];
            _MaxShields = BarMax[0];
            _CurHP = BarCurrent[1];
            _MaxHP = BarMax[1];
        }
        else
        {
            _CurHP = BarCurrent[0];
            _MaxHP = BarMax[0];
            _CurEnergy = BarCurrent[1];
            _MaxEnergy = BarMax[1];
        }
    }
    else
    {
        _CurShields = BarCurrent[0];
        _MaxShields = BarMax[0];
        _CurHP = BarCurrent[1];
        _MaxHP = BarMax[1];
        _CurEnergy = BarCurrent[2];
        _MaxEnergy = BarMax[2];
    }
}
