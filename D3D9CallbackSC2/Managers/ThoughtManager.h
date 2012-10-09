//
// ThoughtManager.h
//
// Class responsible for updating threads in the AI and scheduling their programs to run
//

//
// Threads used in the real game
//
struct RealGameThreads
{
    SearchThread Search;
    WorkerThread Worker;
    ProductionThread Production;
    ControlGroupCheckThread ControlGroupCheck;
    ArmyCheckThread ArmyCheck;
    OverlordThread Overlord;
    QueenThread Queen;
    GasWorkerThread GasWorker;
    ArmyThread Army;
    ChronoBoostThread ChronoBoost;
    MicroThread Micro;
};

//
// Class that stores all threads used by the current game and handles their execution.
//
class ThoughtManager : public Manager
{
public:
    ThoughtManager() {}

    void ResetAll();
    void StartFrame();
    void EndFrame();

    void ExecuteStep();

private:
    void FindWork();

    Vector<AIThread*> _AllThreads;    // All threads in the AI

    Program *_ActiveProgram;          // Currently executing program
    AIThread *_ActiveThread;          // Currently executing thread
    
    RealGameThreads _RealGameThreads; // Threads for multiplayer Starcraft II
};
