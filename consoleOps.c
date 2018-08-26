#include <windows.h>


const int KEYCODE_UPARROW = 38;
const int KEYCODE_DOWNARROW = 40;
const int KEYCODE_LEFTARROW = 37;
const int KEYCODE_RIGHTARROW = 39;
const int KEYCODE_PGUP = 33;
const int KEYCODE_PGDOWN = 34;
const int KEYCODE_ENTER = 13;
const int KEYCODE_ESCAPE = 27;
const int KEYCODE_BACKSPACE = 9;
const int KEYCODE_D = 68;
const int KEYCODE_DELETE = 46;


HANDLE hConsole;
INPUT_RECORD keypresses[1];
static const INPUT_RECORD NoEvent;

void clearScreen(){
    system("cls");
}

int getKeypress()
{
    DWORD eventsRead;
    hConsole = GetStdHandle(STD_INPUT_HANDLE);

    ReadConsoleInput(hConsole, keypresses, 1, &eventsRead);

    while(keypresses[0].EventType != KEY_EVENT || !keypresses[0].Event.KeyEvent.bKeyDown)
    {
        ReadConsoleInput(hConsole, keypresses, 1, &eventsRead);
    }

    int x = keypresses[0].Event.KeyEvent.wVirtualKeyCode;
    keypresses[0] = NoEvent;
    return x;
}
