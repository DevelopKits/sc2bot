//
// ProgramPressKey.h
//
// Program to press a key
// 

enum PressKeyState
{
    PressKeyPressKey,
    PressKeyWait,
};

enum PressKeyType
{
    PressKeyNormal,
    PressKeyMultiple,
    PressKeyMainBase
};

class ProgramPressKey : public Program
{
public:
    void Init(WORD _Key, bool _Shift = false, bool _Ctrl = false);
    void Init(const Vector<WORD> &_Keys);
    ProgramResultType ExecuteStep();

    PressKeyState CurState;
    PressKeyType Type;

    WORD Key;           // Key to press
    bool Shift, Ctrl;   // Key modifiers

    Vector<WORD> Keys;  // All keys to press on multiple key presses

    //
    // Wait variables
    //
    UINT WaitKey;
};
