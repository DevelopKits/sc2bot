//
// Program.h
//
// General include for all programs
//

//
// See README or online help for a general description of how programs and threads work
//
// WaitXXX variables in program mean the number of steps left to wait for a certain event
// to happen.  For example in ProgramBuildStructure, WaitOnStructureAction is the number of
// steps the program will wait for the structure to become an avaliable action after pressing
// the "Build Structure" button.
//
// The <<Program Name>>State enum is the set of states the program can be in and every program
// with have an instance of this enum called CurState
//
// The <<Program Name>>Type enum is a property of some programs that can have different functions.
// For example ProgramSelectGroupUnit has SelectGroupUnitType which specifies whether it is selecting
// a building, a unit, or an index into a unit group.  Program which have this associated enum will
// have an instance called Type.
//
class Program
{
public:
    virtual ProgramResultType ExecuteStep() = 0;                       // Executes a single frame in this program
    void LogThreadEvent(const String &Text, LogType Level);
    void LogThreadEvent(const String &Text, LogType Level, RGBColor Color);

    void InitalizeDelay(double DelayTime);
    void InitalizeWait(double WaitTime);
    ProgramResultType HandleDelay(const String &WaitName, UINT *StateVariable, UINT NewState);
    ProgramResultType HandleWait(const String &WaitName);
    ProgramResultType HandleWait(UINT &WaitCount, const String &WaitName); // Decrements WaitCount and waits for it to hit 0.
                                                                           // WaitName is used for logging purposes only

    double FinishTime;  // Time at which wait timer expires
    String Name;        // Name of this program
};

#define HANDLE_PROGRAM_EXECUTION(Prog, NewState)                  \
    LogThreadEvent("Executing: " + Prog.Name, LogExecuteStep);    \
    ProgramResultType Status = Prog.ExecuteStep();                \
    if(Status == ProgramResultStillExecuting)                     \
    {                                                             \
        return ProgramResultStillExecuting;                       \
    }                                                             \
    else if(Status == ProgramResultFail)                          \
    {                                                             \
        LogThreadEvent(Prog.Name + " failed", LogError);          \
        return ProgramResultFail;                                 \
    }                                                             \
    else                                                          \
    {                                                             \
        LogThreadEvent(Prog.Name + " done", LogStep);             \
        CurState = NewState;                                      \
    }

#define HANDLE_CRITICAL_FAILURE                                   \
    LogThreadEvent("Critical Error", LogError);                   \
    g_Context->Files.Assert << Name << " Critical Error\n";       \
    return ProgramResultFail;