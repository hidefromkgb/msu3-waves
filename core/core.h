#include <stdint.h>



#define DEF_UTMR   8 /** Default interval of the state-updating timer **/

#define KEY_RMB    0
#define KEY_LEFT   1
#define KEY_RIGHT  2
#define KEY_UP     3
#define KEY_DOWN   4
#define KEY_W      5
#define KEY_S      6
#define KEY_A      7
#define KEY_D      8
#define KEY_F1     9
#define KEY_F2    10
#define KEY_F3    11
#define KEY_F4    12
#define KEY_F5    13
#define KEY_SPACE 14



typedef struct ENGC ENGC;

intptr_t cGetUserData(ENGC *engc);
void cUpdateState(ENGC *engc);
void cMouseInput(ENGC *engc, long xpos, long ypos, long btns);
void cKbdInput(ENGC *engc, uint8_t code, long down);
void cResizeWindow(ENGC *engc, long xdim, long ydim);
void cRedrawWindow(ENGC *engc);
void cFreeEngine(ENGC **engc);
ENGC *cMakeEngine(intptr_t user);
