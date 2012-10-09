#include "Main.h"

void ProgramCompound::Init(Program *P1, Program *P2)
{
    CurProg = 0;
    Programs.ReSize(2);
    Programs[0] = P1;
    Programs[1] = P2;
    Name = String("Compound Program " + String(CurProg) + " " + Programs[CurProg]->Name);
}

void ProgramCompound::Init(Program *P1, Program *P2, Program *P3)
{
    CurProg = 0;
    Programs.ReSize(3);
    Programs[0] = P1;
    Programs[1] = P2;
    Programs[2] = P3;
    Name = String("Compound Program " + String(CurProg) + " " + Programs[CurProg]->Name);
}

void ProgramCompound::Init(Program *P1, Program *P2, Program *P3, Program *P4)
{
    CurProg = 0;
    Programs.ReSize(4);
    Programs[0] = P1;
    Programs[1] = P2;
    Programs[2] = P3;
    Programs[3] = P4;
    Name = String("Compound Program " + String(CurProg) + " " + Programs[CurProg]->Name);
}

ProgramResultType ProgramCompound::ExecuteStep()
{
    LogThreadEvent("Executing: " + Programs[CurProg]->Name, LogExecuteStep);
    ProgramResultType Status = Programs[CurProg]->ExecuteStep();
    if(Status == ProgramResultStillExecuting)
    {
        return ProgramResultStillExecuting;
    }
    else if(Status == ProgramResultFail)
    {
        LogThreadEvent(Programs[CurProg]->Name + " failed.", LogError);
        return ProgramResultFail;
    }
    else
    {
        LogThreadEvent(Programs[CurProg]->Name + " done.", LogStep);
        CurProg++;
        if(CurProg == Programs.Length())
        {
            Name = String("Compound Program Done " + String(CurProg));
            return ProgramResultSuccess;
        }
        else
        {
            Name = String("Compound Program " + String(CurProg) + " " + Programs[CurProg]->Name);
            return ExecuteStep();
        }
    }
}
