#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>

#include <gdk/x11/gdkglx.h>
#include <X11/extensions/Xrender.h>

#include "../core/core.h"



static inline GdkGLDrawable *gtk_widget_gl_begin(GtkWidget *gwnd) {
    GdkGLDrawable *pGLD = gtk_widget_get_gl_drawable(gwnd);

    if (!gdk_gl_drawable_gl_begin(pGLD, gtk_widget_get_gl_context(gwnd)))
        return 0;
    return pGLD;
}



gboolean DrawFunc(gpointer user) {
    GdkWindow *hwnd = gtk_widget_get_window(user);

    gdk_window_invalidate_rect(hwnd, 0, FALSE);
    gdk_window_process_updates(hwnd, FALSE);
    return TRUE;
}



gboolean UpdateFunc(gpointer user) {
    GdkGLDrawable *pGLD;

    pGLD = gtk_widget_gl_begin(GTK_WIDGET(cGetUserData((ENGC*)user)));
    cUpdateState((ENGC*)user);
    gdk_gl_drawable_gl_end(pGLD);
}



gboolean OnDestroy(GtkWidget *gwnd, gpointer user) {
    gtk_main_quit();
    return TRUE;
}



gboolean OnMouseMove(GtkWidget *gwnd, GdkEventMotion *emov, gpointer user) {
    cMouseInput(*(ENGC**)user, emov->x, emov->y,
               ((emov->state & GDK_BUTTON1_MASK)? 2 : 0) |
               ((emov->state & GDK_BUTTON2_MASK)? 4 : 0) |
               ((emov->state & GDK_BUTTON3_MASK)? 8 : 0) | 1);
    return TRUE;
}



gboolean OnMousePress(GtkWidget *gwnd, GdkEventButton *ebtn, gpointer user) {
    long down = (ebtn->type == GDK_BUTTON_PRESS)? 1 : 0;

    cMouseInput(*(ENGC**)user, ebtn->x, ebtn->y,
               ((ebtn->button == 1)? down << 1 : 0) |
               ((ebtn->button == 2)? down << 2 : 0) |
               ((ebtn->button == 3)? down << 3 : 0));
    return TRUE;
}



gboolean OnKeyPress(GtkWidget *gwnd, GdkEventKey *ekey, gpointer user) {
    GLboolean down = (ekey->type == GDK_KEY_PRESS)? TRUE : FALSE;

    switch (ekey->keyval) {
        case GDK_KEY_Left:
            cKbdInput(*(ENGC**)user, KEY_LEFT, down);
            break;

        case GDK_KEY_Right:
            cKbdInput(*(ENGC**)user, KEY_RIGHT, down);
            break;

        case GDK_KEY_Up:
            cKbdInput(*(ENGC**)user, KEY_UP, down);
            break;

        case GDK_KEY_Down:
            cKbdInput(*(ENGC**)user, KEY_DOWN, down);
            break;

        case GDK_KEY_F1:
            cKbdInput(*(ENGC**)user, KEY_F1, down);
            break;

        case GDK_KEY_F2:
            cKbdInput(*(ENGC**)user, KEY_F2, down);
            break;

        case GDK_KEY_F3:
            cKbdInput(*(ENGC**)user, KEY_F3, down);
            break;

        case GDK_KEY_F4:
            cKbdInput(*(ENGC**)user, KEY_F4, down);
            break;

        case GDK_KEY_F5:
            cKbdInput(*(ENGC**)user, KEY_F5, down);
            break;

        case GDK_KEY_space:
            cKbdInput(*(ENGC**)user, KEY_SPACE, down);
            break;

        case GDK_KEY_w:
        case GDK_KEY_W:
            cKbdInput(*(ENGC**)user, KEY_W, down);
            break;

        case GDK_KEY_s:
        case GDK_KEY_S:
            cKbdInput(*(ENGC**)user, KEY_S, down);
            break;

        case GDK_KEY_a:
        case GDK_KEY_A:
            cKbdInput(*(ENGC**)user, KEY_A, down);
            break;

        case GDK_KEY_d:
        case GDK_KEY_D:
            cKbdInput(*(ENGC**)user, KEY_D, down);
            break;
    }
    return TRUE;
}



gboolean OnChange(GtkWidget *gwnd, GdkScreen *scrn, gpointer user) {
    GdkColormap *cmap;

    cmap = gdk_screen_get_rgba_colormap(gtk_widget_get_screen(gwnd));
    gtk_widget_set_colormap(gwnd, cmap);
    return TRUE;
}



gboolean OnResize(GtkWidget *gwnd, GdkEventConfigure *ecnf, gpointer user) {
    GdkGLDrawable *pGLD;

    pGLD = gtk_widget_gl_begin(gwnd);
    cResizeWindow(*(ENGC**)user, ecnf->width, ecnf->height);
    gdk_gl_drawable_gl_end(pGLD);
    return FALSE;
}



gboolean OnRedraw(GtkWidget *gwnd, GdkEventExpose *eexp, gpointer user) {
    GdkGLDrawable *pGLD;

    pGLD = gtk_widget_gl_begin(gwnd);
    cRedrawWindow(*(ENGC**)user);
    gdk_gl_drawable_swap_buffers(pGLD);
    gdk_gl_drawable_gl_end(pGLD);
    return TRUE;
}



GdkGLConfig *GetGDKGL(GtkWidget *gwnd) {
    GdkScreen *scrn = gtk_window_get_screen(GTK_WINDOW(gwnd));
    GdkGLConfig *pGGL = 0;

    Display *disp = GDK_SCREEN_XDISPLAY(scrn);
    int iter, numc,
        attr[] = {
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_RENDER_TYPE,   GLX_RGBA_BIT,
            GLX_DOUBLEBUFFER,  True,
            GLX_RED_SIZE,      1,
            GLX_GREEN_SIZE,    1,
            GLX_BLUE_SIZE,     1,
            GLX_ALPHA_SIZE,    1,
            None
        };

    if (XRenderQueryExtension(disp, &iter, &iter)) {
        GLXFBConfig *pFBC =
            glXChooseFBConfig(disp, GDK_SCREEN_XNUMBER(scrn), attr, &numc);
        if (pFBC) {
            for (iter = 0; !pGGL && iter < numc; iter++) {
                XVisualInfo *pvis = glXGetVisualFromFBConfig(disp, pFBC[iter]);
                if (pvis) {
                    XRenderPictFormat *pfmt =
                    XRenderFindVisualFormat(disp, pvis->visual);
                    if (pfmt && pfmt->direct.alphaMask > 0)
                        pGGL =
                        gdk_x11_gl_config_new_from_visualid(pvis->visualid);
                    XFree(pvis);
                }
            }
            XFree(pFBC);
        }
    }
    if (!pGGL)
        pGGL = gdk_gl_config_new_by_mode(GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE
                                       | GDK_GL_MODE_ALPHA | GDK_GL_MODE_RGBA);
    return pGGL;
}



int main(int argc, char *argv[]) {
    GdkGLDrawable *pGLD;
    GtkWidget *gwnd;
    guint tmru, tmrd;
    ENGC *engc = 0;

    gtk_init(0, 0);
    gtk_gl_init(0, 0);
    gwnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    OnChange(gwnd, 0, 0);

    g_signal_connect(G_OBJECT(gwnd), "expose-event",
                     G_CALLBACK(OnRedraw), &engc);
    g_signal_connect(G_OBJECT(gwnd), "delete-event",
                     G_CALLBACK(OnDestroy), &engc);
    g_signal_connect(G_OBJECT(gwnd), "screen-changed",
                     G_CALLBACK(OnChange), &engc);
    g_signal_connect(G_OBJECT(gwnd), "configure-event",
                     G_CALLBACK(OnResize), &engc);
    g_signal_connect(G_OBJECT(gwnd), "key-press-event",
                     G_CALLBACK(OnKeyPress), &engc);
    g_signal_connect(G_OBJECT(gwnd), "key-release-event",
                     G_CALLBACK(OnKeyPress), &engc);
    g_signal_connect(G_OBJECT(gwnd), "button-press-event",
                     G_CALLBACK(OnMousePress), &engc);
    g_signal_connect(G_OBJECT(gwnd), "button-release-event",
                     G_CALLBACK(OnMousePress), &engc);
    g_signal_connect(G_OBJECT(gwnd), "motion-notify-event",
                     G_CALLBACK(OnMouseMove), &engc);
    gtk_widget_set_events(gwnd, gtk_widget_get_events(gwnd)
                              | GDK_KEY_PRESS_MASK    | GDK_KEY_RELEASE_MASK
                              | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
                              | GDK_POINTER_MOTION_MASK);

    gtk_widget_set_gl_capability(gwnd, GetGDKGL(gwnd), 0,
                                 TRUE, GDK_GL_RGBA_TYPE);
    gtk_widget_realize(gwnd);

    pGLD = gtk_widget_gl_begin(gwnd);
    engc = cMakeEngine((intptr_t)gwnd);
    gdk_gl_drawable_gl_end(pGLD);

    gtk_widget_set_app_paintable(gwnd, TRUE);
    gtk_widget_set_size_request(gwnd, 800, 600);
    gtk_window_set_position(GTK_WINDOW(gwnd), GTK_WIN_POS_CENTER);
    gtk_widget_show(gwnd);

    tmru = g_timeout_add(DEF_UTMR, UpdateFunc, engc);
    tmrd = g_idle_add(DrawFunc, gwnd);

    gtk_main();

    pGLD = gtk_widget_gl_begin(gwnd);
    cFreeEngine(&engc);
    gdk_gl_drawable_gl_end(pGLD);

    g_source_remove(tmrd);
    g_source_remove(tmru);
    gtk_widget_destroy(gwnd);
}
