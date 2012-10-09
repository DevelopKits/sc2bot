//
// ProgramCompound.h
//
// Program to execute a series of other programs in order
//

class ProgramCompound : public Program
{
public:
    void Init(Program *P1, Program *P2);
    void Init(Program *P1, Program *P2, Program *P3);
    void Init(Program *P1, Program *P2, Program *P3, Program *P4);
    ProgramResultType ExecuteStep();

    UINT CurProg;
    Vector<Program*> Programs;
};

