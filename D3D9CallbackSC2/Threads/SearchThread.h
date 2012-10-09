//
// SearchThread.h
//
// AIThread responsible for randomly clicking on the map for "interesting" things
// 

class SearchThread : public AIThread
{
public:
    void Reset();
    void Update();
    void ProgramSucceeded();
    void ProgramFailed();

private:
    ProgramBaseSearch _BaseSearch;
};
