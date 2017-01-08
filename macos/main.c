#include "mac_load/mac_load.h"
#include "../core/ogl_load/ogl_load.h"
#include "../core/core.h"



#define VAR_ENGC "engc"

typedef struct {
    ENGC *engc;
    NSView *view;
    NSWindow *mwnd;
    uint32_t pbtn;
} DATA;



bool OnTrue(void *this, SEL name) {
    return true;
}

bool OnFalse(void *this, SEL name) {
    return false;
}

bool OnClose(void *this, SEL name) {
    stop_(sharedApplication(NSApplication()), this);
    return true;
}

void OnSize(void *this, SEL name) {
    CGRect rect;
    ENGC *engc;

    rect = frame(this);
    MAC_GET_IVAR(this, VAR_ENGC, &engc);
    cResizeWindow(engc, rect.size.width, rect.size.height);
}

void OnDraw(void *this, SEL name, CGRect rect) {
    ENGC *engc;

    MAC_GET_IVAR(this, VAR_ENGC, &engc);
    cRedrawWindow(engc);
    flushBuffer(openGLContext(this));
}

void OnKbd(void *this, SEL name, NSEvent *ekbd) {
    CFStringRef temp;
    ENGC *engc;
    long down;

    if (!CFStringGetLength(temp = charactersIgnoringModifiers(ekbd)))
        return;
    MAC_GET_IVAR(this, VAR_ENGC, &engc);
    down = (type(ekbd) == NSKeyDown)? 1 : 0;
    switch (CFStringGetCharacterAtIndex(temp, 0)) {
        case NSLeftArrowFunctionKey:
            cKbdInput(engc, KEY_LEFT, down);
            break;

        case NSRightArrowFunctionKey:
            cKbdInput(engc, KEY_RIGHT, down);
            break;

        case NSUpArrowFunctionKey:
            cKbdInput(engc, KEY_UP, down);
            break;

        case NSDownArrowFunctionKey:
            cKbdInput(engc, KEY_DOWN, down);
            break;

        case NSF1FunctionKey:
            cKbdInput(engc, KEY_F1, down);
            break;

        case NSF2FunctionKey:
            cKbdInput(engc, KEY_F2, down);
            break;

        case NSF3FunctionKey:
            cKbdInput(engc, KEY_F3, down);
            break;

        case NSF4FunctionKey:
            cKbdInput(engc, KEY_F4, down);
            break;

        case NSF5FunctionKey:
            cKbdInput(engc, KEY_F5, down);
            break;

        case ' ':
            cKbdInput(engc, KEY_SPACE, down);
            break;

        case 'w': case 'W':
            cKbdInput(engc, KEY_W, down);
            break;

        case 's': case 'S':
            cKbdInput(engc, KEY_S, down);
            break;

        case 'a': case 'A':
            cKbdInput(engc, KEY_A, down);
            break;

        case 'd': case 'D':
            cKbdInput(engc, KEY_D, down);
            break;
    }
}

void OnInvalidate(CFRunLoopTimerRef tmrp, void *user) {
    setNeedsDisplay_((NSView*)user, true);
}

void OnUpdate(CFRunLoopTimerRef tmrp, void *user) {
    DATA *data = (DATA*)user;
    uint32_t pbtn, attr, here;
    CGRect wdim, vdim;
    CGPoint mptr;

    if (isKeyWindow(data->mwnd)) {
        wdim = frame(data->mwnd);
        vdim = frame(data->view);
        mptr = mouseLocation(NSEvent());
        pbtn = pressedMouseButtons(NSEvent());
        mptr.x -= wdim.origin.x + vdim.origin.x;
        mptr.y -= wdim.origin.y + vdim.origin.y;

        attr = (((data->pbtn ^ pbtn) & 3)? 0 : 1) |
                ((pbtn & 1)? 2 : 0) | ((pbtn & 2)? 8 : 0);
        data->pbtn = (data->pbtn & -0x100) | (pbtn & 0xFF);

        here = !!((mptr.x < vdim.size.width) && (mptr.y < vdim.size.height)
               && (mptr.x >= 0) && (mptr.y >= 0));
        if ((~attr & 1) && (here || ((data->pbtn >> 8) & ~attr & 14)))
            data->pbtn = (data->pbtn & 0xFF) | ((attr & 14) << 8);
        if ((data->pbtn & (14 << 8)))
            cMouseInput(data->engc, mptr.x, -mptr.y, attr);
    }
    cUpdateState(data->engc);
}



int main(int argc, char *argv[]) {
    GLint attr[] = {NSOpenGLPFADoubleBuffer, NSOpenGLPFADepthSize, 32, 0};
    CFRunLoopTimerRef tmru, tmri;
    NSOpenGLPixelFormat *pfmt;
    NSAutoreleasePool *pool;
    NSApplication *thrd;
    CGSize size;
    CGRect dims;
    Class vogl;

    DATA data = {};

    pool = init(alloc(NSAutoreleasePool()));
    thrd = sharedApplication(NSApplication());
    setActivationPolicy_(thrd, NSApplicationActivationPolicyAccessory);

    size = (CGSize){800, 600};
    dims = visibleFrame(mainScreen(NSScreen()));
    dims = (CGRect){{dims.origin.x + (dims.size.width - size.width) * 0.5,
                     dims.origin.y + (dims.size.height - size.height) * 0.5},
                     size};

    vogl = MAC_MakeClass("NSO", NSOpenGLView(), MAC_TempArray(VAR_ENGC),
                         MAC_TempArray(drawRect_(), OnDraw,
                                       windowDidResize_(), OnSize,
                                       keyDown_(), OnKbd, keyUp_(), OnKbd,
                                       windowShouldClose_(), OnClose,
                                       acceptsFirstResponder(), OnTrue));

    pfmt = initWithAttributes_(alloc(NSOpenGLPixelFormat()), attr);
    data.view = (NSView*)initWithFrame_pixelFormat_(alloc(vogl), dims, pfmt);
    makeCurrentContext(openGLContext(data.view));
    release(pfmt);

    MAC_SET_IVAR(data.view, VAR_ENGC, data.engc = cMakeEngine(0));
    data.mwnd = initWithContentRect_styleMask_backing_defer_
                    (alloc(NSWindow()), dims, NSTitledWindowMask
                                            | NSClosableWindowMask
                                            | NSResizableWindowMask
                                            | NSMiniaturizableWindowMask,
                     kCGBackingStoreBuffered, false);
    setContentView_(data.mwnd, data.view);
    setDelegate_(data.mwnd, data.view);

    activateIgnoringOtherApps_(thrd, true);
    makeKeyWindow(data.mwnd);
    orderFront_(data.mwnd, thrd);

    OnSize(data.view, 0);
    tmru = MAC_MakeTimer(DEF_UTMR, OnUpdate, &data);
    tmri = MAC_MakeTimer(1, OnInvalidate, data.view);
    run(thrd);
    MAC_FreeTimer(tmru);
    MAC_FreeTimer(tmri);
    cFreeEngine(&data.engc);

    release(pool);
    MAC_FreeClass(vogl);
    return 0;
}
