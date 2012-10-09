//
// AIThread.h
//
// General include for threads in the AI.  See README for more information on threads.
// 

//
// Absctract class definining base AIThread behavior
//
class AIThread
{
public:
    friend class ThoughtManager;

    virtual void Reset() = 0;                // Called when the game is loaded
    virtual void StartFrame() {}            // Called at the start of each frame
    virtual void Update() = 0;              // Called each frame
    virtual void ProgramSucceeded() = 0;    // Called when the thread's program completes
    virtual void ProgramFailed() = 0;       // Called when the thread's program fails

protected:
    Program *_ProgramToExecute;   // Program this thread desires to be run after ThoughtManager calls Update on it
    String _Name;                 // This thread's name
    String _CurAction;            // Identifier for current action
    double _WakeUpTime;           // Time this thread will wake up and be ready to update again
    double _LastExecuteTime;      // Last time this thread executed
    UINT _Priority;               // Integer representing the thread priority.  Higher priority executes over low priority.
};
