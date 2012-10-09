#include "Main.h"

void ThoughtManager::ResetAll()
{
    _ActiveProgram = NULL;
    _ActiveThread = NULL;
    _AllThreads.FreeMemory();
}

void ThoughtManager::StartFrame()
{
    for(UINT ThreadIndex = 0; ThreadIndex < _AllThreads.Length(); ThreadIndex++)
    {
        _AllThreads[ThreadIndex]->StartFrame();
    }
}

void ThoughtManager::EndFrame()
{
	if(DisplayProductionGoals)
    {
        g_Context->WriteConsole(_RealGameThreads.Production.DescribeProductionGoals(), RGBColor::Green, OverlayPanelStatus);
    }
	if(!g_Context->Controller.ConsoleEnabled())
	{
		return;
	}
    if(DisplayThreadState && g_Context->Controller.AIEnabled())
    {
        g_Context->Graphics.Overlay().ClearPanel(OverlayPanelThreadState);
        double CurTime = GameTime();
        for(UINT ThreadIndex = 0; ThreadIndex < _AllThreads.Length(); ThreadIndex++)
        {
            AIThread &CurThread = *_AllThreads[ThreadIndex];
            if(CurThread._WakeUpTime > CurTime)
            {
                g_Context->WriteConsole(CurThread._Name + String(":Asleep (") + String(CurThread._WakeUpTime - CurTime) + String("s)"), RGBColor::Gray, OverlayPanelThreadState);
            }
            else if(CurThread._ProgramToExecute)
            {
                RGBColor Color = RGBColor::Red;
                if(&CurThread == _ActiveThread)
                {
                    Color = RGBColor::Green;
                }
                g_Context->WriteConsole(CurThread._Name + String(':') + CurThread._CurAction, Color, OverlayPanelThreadState);
            }
            else
            {
                g_Context->WriteConsole(CurThread._Name + String(":No work ready"), RGBColor::Yellow, OverlayPanelThreadState);
            }
        }
    }
}

void ThoughtManager::FindWork()
{
    if(_ActiveProgram)
    {
        g_Context->Files.Thought << "Executing " << _ActiveThread->_Name << " Action=" << _ActiveThread->_CurAction << " Prog=" << _ActiveProgram->Name << endl;
    }
    else
    {
        ActionButtonStateType State;
        bool SingleBaseSelected = (g_Context->Managers.FrameHUD.SoloPortrait() != NULL && g_Context->Managers.FrameHUD.SoloPortrait()->Unit()->SecondaryType == UnitSecondaryBase);
        if(g_Context->Managers.FrameHUD.ActionButtonPresent("Cancel", State) && !SingleBaseSelected)
        {
            g_Context->Files.Thought << "Pressing supply control group to remove cancel button\n";
            g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupSupply));
            //return;
        }
        double CurTime = GameTime();
        
        //
        // Threads must not update while they have an active program!
        //
        g_Context->Files.Thought << "Finding work. Frame=" << g_Context->Controller.FrameIndex() << endl;

        Vector<AIThread*> ActiveThreadCanidates;
        UINT HighestActivePriority = 0;

        //
        // Find the set of all awake threads with the highest priority and put them in ActiveThreadCanidates
        //
        for(UINT ThreadIndex = 0; ThreadIndex < _AllThreads.Length(); ThreadIndex++)
        {
            AIThread &CurThread = *_AllThreads[ThreadIndex];
            if(CurThread._WakeUpTime < CurTime)
            {
                CurThread.Update();
                if(CurThread._ProgramToExecute == NULL)
                {
                    g_Context->Files.Thought << CurThread._Name << ": No work ready\n";
                }
                else
                {
                    g_Context->Files.Thought << CurThread._Name << ": " << CurThread._CurAction << ", Pri=" << CurThread._Priority << endl;
                    
                    if(CurThread._Priority > HighestActivePriority)
                    {
                        HighestActivePriority = CurThread._Priority;
                        ActiveThreadCanidates.FreeMemory();
                    }

                    if(CurThread._Priority == HighestActivePriority)
                    {
                        ActiveThreadCanidates.PushEnd(&CurThread);
                    }
                }
            }
            else
            {
                g_Context->Files.Thought << CurThread._Name << ": Asleep\n";
            }
        }

        if(ActiveThreadCanidates.Length() == 0)
        {
            g_Context->Files.Thought << "No work ready\n";
        }
        else
        {
            //
            // Choose the thread that executed the longest time ago
            //
            double SmallestThreadValue = ActiveThreadCanidates[0]->_LastExecuteTime;
            UINT SmallestThreadIndex = 0;
            for(UINT ActiveThreadIndex = 1; ActiveThreadIndex < ActiveThreadCanidates.Length(); ActiveThreadIndex++)
            {
                const AIThread &CurThread = *ActiveThreadCanidates[ActiveThreadIndex];
                if(CurThread._LastExecuteTime < SmallestThreadValue)
                {
                    SmallestThreadValue = CurThread._LastExecuteTime;
                    SmallestThreadIndex = ActiveThreadIndex;
                }
            }
            _ActiveThread = ActiveThreadCanidates[SmallestThreadIndex];
            _ActiveProgram = _ActiveThread->_ProgramToExecute;
            g_Context->Files.Thought << "Executing: " << _ActiveThread->_Name << " Action=" << _ActiveThread->_CurAction << endl;
        }
    }
}

//
// Decide and perform the AI's next action
//
void ThoughtManager::ExecuteStep()
{
    Debugger("ThoughtManager::ExecuteStep");

    //
    // If there are no threads yet, add them if we know what type of game we're playing
    //
    const RaceType Race = g_Context->Managers.Knowledge.MyRace();
    if(_AllThreads.Length() == 0 && Race != RaceNone)
    {
        g_Context->WriteConsole("Threads reset", RGBColor::Yellow, OverlayPanelSystem);
        _AllThreads.PushEnd(&_RealGameThreads.Search);
        _AllThreads.PushEnd(&_RealGameThreads.Worker);
        _AllThreads.PushEnd(&_RealGameThreads.Production);
        _AllThreads.PushEnd(&_RealGameThreads.ControlGroupCheck);
        _AllThreads.PushEnd(&_RealGameThreads.GasWorker);
        _AllThreads.PushEnd(&_RealGameThreads.Army);
        _AllThreads.PushEnd(&_RealGameThreads.ArmyCheck);
        _AllThreads.PushEnd(&_RealGameThreads.Micro);

        if(Race == RaceZerg)
        {
            _AllThreads.PushEnd(&_RealGameThreads.Overlord);
            _AllThreads.PushEnd(&_RealGameThreads.Queen);
        }
        if(Race == RaceProtoss)
        {
            _AllThreads.PushEnd(&_RealGameThreads.ChronoBoost);
        }
        
        for(UINT ThreadIndex = 0; ThreadIndex < _AllThreads.Length(); ThreadIndex++)
        {
            AIThread &CurThread = *_AllThreads[ThreadIndex];
            CurThread._WakeUpTime = GameTime();
            CurThread._LastExecuteTime = GameTime();
            CurThread._ProgramToExecute = NULL;
            CurThread._Priority = 1;
            CurThread._CurAction = "Error";
            CurThread.Reset();
        }

        _ActiveProgram = NULL;
        _ActiveThread = NULL;
    }

    //
    // Find a thread with work to do
    //
    FindWork();

    if(_ActiveProgram)
    {
        //
        // Execute the program's next action
        //
        ProgramResultType Status = _ActiveProgram->ExecuteStep();

        //
        // Check if the program has completed
        //
        if(Status == ProgramResultSuccess || Status == ProgramResultFail)
        {
            g_Context->Files.Thread << _ActiveProgram->Name << " returned " << GetProgramResultTypeString(Status) << endl;

            _ActiveThread->_LastExecuteTime = GameTime();

            //
            // Inform the thread their program finished
            //
            if(Status == ProgramResultSuccess)
            {
                g_Context->Files.Thought << "Success. " << _ActiveThread->_Name << ": " << _ActiveThread->_CurAction << endl;
                _ActiveThread->ProgramSucceeded();
            }
            else if(Status == ProgramResultFail)
            {
                g_Context->Files.Thought << "Failure. " << _ActiveThread->_Name << ": " << _ActiveThread->_CurAction << endl;
                _ActiveThread->ProgramFailed();
            }
            _ActiveThread->_CurAction = "Action Reset";
            _ActiveThread->_ProgramToExecute = NULL;

            _ActiveProgram = NULL;
            _ActiveThread = NULL;
        }
    }
}
