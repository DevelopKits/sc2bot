#include "Main.h"

void SearchThread::Reset()
{
    _Priority = 2;
    _Name = "Search";
}

void SearchThread::Update()
{
    if(g_Context->Managers.Minimap.SelfPixelCoordinates().Length() == 0)
    {
        _ProgramToExecute = NULL;
    }
    else
    {
        //const Vec2f MinimapCoord = g_Context->Managers.Minimap.SelfPixelCoordinates().RandomElement();
        //const Vec2i ScreenCoord = g_Context->Constants.MinimapCoordToScreenCoord(MinimapCoord);
        _BaseSearch.Init();
        _ProgramToExecute = &_BaseSearch;
        _CurAction = "RandomLook";
        _Priority = 1;
    }
}

void SearchThread::ProgramSucceeded()
{
    _WakeUpTime = GameTime() + 0.01f;
}

void SearchThread::ProgramFailed()
{
    _WakeUpTime = GameTime() + 0.01f;
}