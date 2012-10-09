//
// ProgramClick.h
//
// Program to click on the screen
// 

enum ProgClickState
{
    ProgClickClickScreen,
    ProgClickWait,
};

class ProgramClick : public Program
{
public:
    void InternalInit();
    void Init(const Vec2i &_ScreenCoord);
    void Init(const Vec2i &_ScreenCoord, bool _RightClick);
    void Init(const String &_PreStatement, const Vec2i &_ScreenCoord);
    void Init(WORD _PreKey, const Vec2i &_ScreenCoord);
    ProgramResultType ExecuteStep();

    ProgClickState CurState;

    String PreStatement;        // String to type frame before click
    WORD PreKey;                // Key to press frame before click

    Vec2i ScreenCoord;          // Location to click at
    bool RightClick;            // if True a right-click is performed instead

    //
    // Wait Variables
    //
    UINT WaitClick;
};
