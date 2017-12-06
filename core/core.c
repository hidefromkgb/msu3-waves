#include <time.h>

#include "ogl_load/ogl_load.h"
#include "vec_math/vec_math.h"
#include "core.h"

#define randf(f) (((float)(f) * 2.0 / (float)RAND_MAX) \
                 * (float)(rand() & RAND_MAX) - (float)(f))

#define DEF_FANG  0.01             /** Default angular step             **/
#define DEF_FTRN  0.05             /** Default translational step       **/

#define DEF_FFOV 45.0              /** Default perspective view field   **/
#define DEF_ZNEA  0.1              /** Default near clipping plane      **/
#define DEF_ZFAR 90.0              /** Default far clipping plane       **/

#define DEF_PDIM  2.0              /** Default pool size along X and Z  **/

#define DEF_AIRF  1.0              /** Default optical density of air   **/
#define DEF_WTRF  1.3              /** Default optical density of water **/
#define DEF_CHEI  0.625            /** Default pool height (relative)   **/
#define DEF_PHEI (0.5 * DEF_PDIM)  /** Default pool height (real)       **/

#define DEF_SCLR ( 90 +  20 +  80) /** (white = 255) Sky color coef.    **/
#define DEF_AWTR (128 +  32 + 160) /** (white = 128) Above-water coef.  **/
#define DEF_UWTR (115 +  50 + 128) /** (white = 128) Underwater coef.   **/
#define DEF_WCLR (128 + 100 + 140) /** (white = 128) Wall inverse coef. **/

typedef union {
    struct {
        uint8_t R, G, B, A;
    };
    uint32_t RGBA;
} RGBA;

typedef struct {
    GLuint frmb, rndb;
    GLuint tfrn, tbak;
    OGL_FVBO *vobj;
} FRBO;

struct ENGC {
    VEC_TMFV  clrs;
    VEC_FMST *view, *proj;

    FRBO *rsur, *rwtr, *rcau;
    OGL_FVBO *csur, *watr, *gcau, *pool, *sphr;

    VEC_T2IV angp;
    VEC_T2FV fang, wdet, cdet, winv, cinv;
    VEC_T3FV dclr, dcam, ldir, ftrn, dims;
    VEC_T4FV cdrp, csph;

    GLfloat shei, rdrp, wsur;
    GLboolean line, halt, keys[KEY_ALL_KEYS];
};



FRBO *MakeRBO(OGL_FVBO *vobj, GLuint tfrn, GLuint tbak) {
    GLuint ftxd = 0, ftyd = 0, btxd = 0, btyd = 0;
    FRBO *retn;

    if (!vobj)
        return 0;

    OGL_EnumTex(*OGL_BindTex(vobj, tfrn, OGL_TEX_NSET),
                 0, 0, 0, &ftxd, &ftyd, 0);
    OGL_EnumTex(*OGL_BindTex(vobj, tbak, OGL_TEX_NSET),
                 0, 0, 0, &btxd, &btyd, 0);
    if (!ftxd || !btxd || !ftyd || !btyd || (ftxd != btxd) || (ftyd != btyd))
        return 0;

    retn = (FRBO*)malloc(sizeof(FRBO));
    retn->vobj = vobj;
    retn->tfrn = tfrn;
    retn->tbak = tbak;

    glGenFramebuffers(1, &retn->frmb);
    glGenRenderbuffers(1, &retn->rndb);
    glBindRenderbuffer(GL_RENDERBUFFER, retn->rndb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ftxd, ftyd);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return retn;
}



void DrawRBO(FRBO *robj, GLuint shad) {
    OGL_FTEX *ttex, **ftex, **btex;
    GLuint xdim = 0, ydim = 0;
    GLint view[4];

    glGetIntegerv(GL_VIEWPORT, view);
    glBindFramebuffer(GL_FRAMEBUFFER, robj->frmb);
    glBindRenderbuffer(GL_RENDERBUFFER, robj->rndb);

    ttex = *(btex = OGL_BindTex(robj->vobj, robj->tbak, OGL_TEX_FRMB));
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, robj->rndb);
    OGL_EnumTex(*btex, 0, 0, 0, &xdim, &ydim, 0);
    glViewport(0, 0, xdim, ydim);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    OGL_DrawVBO(robj->vobj, shad, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(view[0], view[1], view[2], view[3]);

    *btex = *(ftex = OGL_BindTex(robj->vobj, robj->tfrn, OGL_TEX_NSET));
    *ftex = ttex;
}



void FreeRBO(FRBO **robj) {
    if (robj && *robj) {
        glDeleteRenderbuffers(1, &(*robj)->rndb);
        glDeleteFramebuffers(1, &(*robj)->frmb);
        free(*robj);
        *robj = NULL;
    }
}



OGL_FTEX *MakeTileTex(GLuint size, GLuint tile, GLuint tbdr) {
    GLint x, y, u, v;
    RGBA tclr, *bptr;
    OGL_FTEX *retn;

    size = pow(2.0, size);
    tile = pow(2.0, tile);
    bptr = calloc(size * size, sizeof(*bptr));
    for (y = size / tile; y > 0; y--)
        for (x = size / tile; x > 0; x--) {
            tclr.RGBA = ((rand() % 32) + 192) * 0x010101;

            for (v = (y - 1) * tile; v < y * tile; v++)
                for (u = x * tile - tbdr; u < x * tile; u++)
                    bptr[size * v + u].RGBA = tclr.RGBA - 0x505050;
            for (v = y * tile - tbdr; v < y * tile; v++)
                for (u = (x - 1) * tile; u < x * tile - tbdr; u++)
                    bptr[size * v + u].RGBA = tclr.RGBA - 0x505050;

            for (v = (y - 1) * tile + tbdr; v < y * tile - tbdr; v++)
                for (u = (x - 1) * tile; u < (x - 1) * tile + tbdr; u++)
                    bptr[size * v + u].RGBA = tclr.RGBA | 0x202020;
            for (v = (y - 1) * tile; v < (y - 1) * tile + tbdr; v++)
                for (u = (x - 1) * tile; u < x * tile - tbdr; u++)
                    bptr[size * v + u].RGBA = tclr.RGBA | 0x202020;

            for (v = (y - 1) * tile + tbdr; v < y * tile - tbdr; v++)
                for (u = (x - 1) * tile + tbdr; u < x * tile - tbdr; u++)
                    bptr[size * v + u].RGBA = tclr.RGBA;
        }
    retn = OGL_MakeTex(size, size, 0, GL_TEXTURE_2D,
                       GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
                       GL_UNSIGNED_BYTE, GL_RGBA, GL_RGBA, bptr);
    free(bptr);
    return retn;
}



OGL_FTEX *MakeCloudTex(GLuint size, GLfloat dmpf,
                       GLfloat cden, GLfloat cshr, RGBA dclr) {
    size = pow(2.0, size);

    GLint x, y, xbgn, xend, ybgn, yend, oddc, step, sinc = size + 1;
    GLfloat hdef, *farr = (GLfloat*)calloc(sinc * sinc, sizeof(GLfloat));
    hdef = dmpf = pow(2.0, -fabs(dmpf));
    RGBA *data = (RGBA*)malloc(size * size * sizeof(RGBA));
    OGL_FTEX *retn;

    for (step = size >> 1; step; step >>= 1, hdef *= dmpf) {
        for (y = step; y < size; y += step << 1)
            for (x = step; x < size; x += step << 1)
                farr[x + y * sinc] =  hdef * randf(0.500) + 0.250
                                   * (farr[(x - step) + (y - step) * sinc] +
                                      farr[(x + step) + (y - step) * sinc] +
                                      farr[(x - step) + (y + step) * sinc] +
                                      farr[(x + step) + (y + step) * sinc]);

        for (ybgn = yend = oddc = y = 0; y < size;
             ybgn = yend = y += step, oddc = ~oddc) {
            if (y == 0) yend = size; else if (y == size) ybgn = 0;
            for (xbgn = xend = x = (oddc)? 0 : step; x < size;
                 xbgn = xend = x += step << 1) {
                if (x == 0) xend = size; else if (x == size) xbgn = 0;
                farr[x + y * sinc] =  hdef * randf(0.500) + 0.250
                                   * (farr[(xend - step) + y * sinc] +
                                      farr[(xbgn + step) + y * sinc] +
                                      farr[x + (yend - step) * sinc] +
                                      farr[x + (ybgn + step) * sinc]);
                if (x == 0) farr[size + y * sinc] = farr[0 + y * sinc];
                if (y == 0) farr[x + size * sinc] = farr[x + 0 * sinc];
            }
        }
    }
    for (hdef = dmpf = farr[0], x = (size + 1) * (size + 1); x >= 0; x--) {
        hdef = (hdef < farr[x])? hdef : farr[x];
        dmpf = (dmpf > farr[x])? dmpf : farr[x];
    }
    for (dmpf = 0.6 / (dmpf - hdef), x = (size + 1) * (size + 1); x >= 0; x--)
        farr[x] = (farr[x] - hdef) * dmpf;

    for (step = y = 0; y < size; step++, y++)
        for (x = 0; x < size; step++, x++) {
            dmpf = (GLfloat)x / (GLfloat)(size >> 1) - 1.0;
            hdef = (GLfloat)y / (GLfloat)(size >> 1) - 1.0;
            hdef = dmpf * dmpf + hdef * hdef;
            dmpf = sqrt(1.0 - pow(cshr, (0.0 > farr[step] - cden)?
                                         0.0 : farr[step] - cden))
                 * 2.0 * ((0.0 > 1.0 - hdef)? 0.0 : 1.0 - hdef);
            dmpf = (dmpf < 1.0)? dmpf : 1.0;
            data[x + size * y].R = dclr.R + (dmpf * (255.0 - (GLfloat)dclr.R));
            data[x + size * y].G = dclr.G + (dmpf * (255.0 - (GLfloat)dclr.G));
            data[x + size * y].B = dclr.B + (dmpf * (255.0 - (GLfloat)dclr.B));
            data[x + size * y].A = 0xFF;
        }
    retn = OGL_MakeTex(size, size, 0, GL_TEXTURE_CUBE_MAP,
                       GL_REPEAT, GL_LINEAR, GL_LINEAR,
                       GL_UNSIGNED_BYTE, GL_RGBA, GL_RGBA, data);
    free(data);
    free(farr);
    return retn;
}



void RegenerateColors(ENGC *retn) {
    OGL_FTEX **tsky;

    while (!0) { /** DISGUSTING. [TODO:] do something about it. **/
        retn->dclr.x = fabs(randf(1.0));
        retn->dclr.y = fabs(randf(1.0));
        retn->dclr.z = fabs(randf(1.0));
        if ((retn->cdrp.w = retn->dclr.x + retn->dclr.y + retn->dclr.z) > 0) {
            VEC_V3MulC(&retn->dclr, 1.0 / retn->cdrp.w);
            if ((retn->dclr.x * DEF_SCLR <= 0xFF)
            &&  (retn->dclr.y * DEF_SCLR <= 0xFF)
            &&  (retn->dclr.z * DEF_SCLR <= 0xFF)
            &&  (retn->dclr.x * DEF_AWTR <= 0xFF)
            &&  (retn->dclr.y * DEF_AWTR <= 0xFF)
            &&  (retn->dclr.z * DEF_AWTR <= 0xFF)
            &&  (retn->dclr.x * DEF_UWTR <= 0xFF)
            &&  (retn->dclr.y * DEF_UWTR <= 0xFF)
            &&  (retn->dclr.z * DEF_UWTR <= 0xFF)
            &&  (retn->dclr.x * DEF_WCLR <= 0xFF)
            &&  (retn->dclr.y * DEF_WCLR <= 0xFF)
            &&  (retn->dclr.z * DEF_WCLR <= 0xFF))
                break;
        }
    }
    #define CLR_BTOF (1.0 / 128.0)
    retn->clrs[ 0] = CLR_BTOF * DEF_AWTR * retn->dclr.x;
    retn->clrs[ 4] = CLR_BTOF * DEF_AWTR * retn->dclr.y;
    retn->clrs[ 8] = CLR_BTOF * DEF_AWTR * retn->dclr.z;
    retn->clrs[ 1] = CLR_BTOF * DEF_UWTR * retn->dclr.x;
    retn->clrs[ 5] = CLR_BTOF * DEF_UWTR * retn->dclr.y;
    retn->clrs[ 9] = CLR_BTOF * DEF_UWTR * retn->dclr.z;
    retn->clrs[ 2] = CLR_BTOF * DEF_WCLR * retn->dclr.x;
    retn->clrs[ 6] = CLR_BTOF * DEF_WCLR * retn->dclr.y;
    retn->clrs[10] = CLR_BTOF * DEF_WCLR * retn->dclr.z;
    #undef CLR_BTOF

    OGL_FreeTex(tsky = OGL_BindTex(retn->watr, 0, OGL_TEX_NSET));
    *tsky = MakeCloudTex(8, 1.0, 0.35, 0.01,
                        (RGBA){{DEF_SCLR * retn->dclr.x,
                                DEF_SCLR * retn->dclr.y,
                                DEF_SCLR * retn->dclr.z, 255}});
}



void cUpdateState(ENGC *engc) {
    VEC_T3FV vadd;
    VEC_T2FV fang;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (!!engc->keys[KEY_W] != !!engc->keys[KEY_S]) {
        fang = (VEC_T2FV){{engc->fang.x + 0.5 * M_PI, engc->fang.y}};
        VEC_V3FromAng(&vadd, &fang);
        VEC_V3MulC(&vadd, (engc->keys[KEY_W])? DEF_FTRN : -DEF_FTRN);
        VEC_V3AddV(&engc->ftrn, &vadd);
    }
    if (!!engc->keys[KEY_A] != !!engc->keys[KEY_D]) {
        fang = (VEC_T2FV){{engc->fang.x, 0.0}};
        VEC_V3FromAng(&vadd, &fang);
        VEC_V3MulC(&vadd, (engc->keys[KEY_A])? DEF_FTRN : -DEF_FTRN);
        VEC_V3AddV(&engc->ftrn, &vadd);
    }
    if (engc->keys[KEY_RMB]) {
        /** Add a new drop; useless to renormalize here, look below **/
        if (engc->keys[KEY_SPACE])
            engc->keys[KEY_SPACE] = engc->keys[KEY_RMB] = 0;
        DrawRBO(engc->rsur, 2);
        engc->cdrp.z = engc->cdrp.w = 0;
    }
    if (engc->keys[KEY_F4]) {
        /** Clear waves; again, useless to renormalize here **/
        engc->keys[KEY_F4] = 0;
        DrawRBO(engc->rsur, 3);
    }
    if (engc->keys[KEY_F2]) {
        engc->keys[KEY_F2] = 0;
        RegenerateColors(engc);
    }
    if (!engc->halt)
        DrawRBO(engc->rsur, 0); /** Update waves (if allowed to) **/
    DrawRBO(engc->rsur, 1);     /** Renormaize them after all changes above **/
    DrawRBO(engc->rwtr, 2);     /** Update caustics **/
    DrawRBO(engc->rcau, 0);     /** 1D-blur caustics and transpose **/
    DrawRBO(engc->rcau, 0);     /** 1D-blur caustics and transpose **/
}



void cMouseInput(ENGC *engc, long xpos, long ypos, long btns) {
    if (~btns & 1)
        engc->keys[KEY_RMB] = !!(btns & 8);
    else if (btns & 2) { /** 1 for moving state, 2 for LMB **/
        engc->fang.y += DEF_FANG * (GLfloat)(ypos - engc->angp.y);
        if (engc->fang.y < -M_PI) engc->fang.y += 2.0 * M_PI;
        else if (engc->fang.y > M_PI) engc->fang.y -= 2.0 * M_PI;

        engc->fang.x += DEF_FANG * (GLfloat)(xpos - engc->angp.x);
        if (engc->fang.x < -M_PI) engc->fang.x += 2.0 * M_PI;
        else if (engc->fang.x > M_PI) engc->fang.x -= 2.0 * M_PI;
    }
    if (btns & 10)
        engc->angp = (VEC_T2IV){{xpos, ypos}};
}



void cKbdInput(ENGC *engc, uint8_t code, long down) {
    engc->keys[code] = down;
}



void cResizeWindow(ENGC *engc, long xdim, long ydim) {
    GLfloat maty = DEF_ZNEA * tanf(0.5 * DEF_FFOV * VEC_DTOR),
            matx = maty * (GLfloat)xdim / (GLfloat)ydim;

    VEC_PurgeMatrixStack(&engc->proj);
    VEC_PushMatrix(&engc->proj);
    VEC_M4Frustum(engc->proj->curr,
                 -matx, matx, -maty, maty, DEF_ZNEA, DEF_ZFAR);
    VEC_PushMatrix(&engc->proj);

    VEC_PurgeMatrixStack(&engc->view);
    VEC_PushMatrix(&engc->view);
    VEC_PushMatrix(&engc->view);

    VEC_M4Multiply(engc->proj->prev->curr,
                   engc->view->prev->curr, engc->proj->curr);

    glViewport(0, 0, xdim, ydim);
}



void cRedrawWindow(ENGC *engc) {
    VEC_TMFV mmtx, rmtx, tmtx;
    VEC_T3FV vect;
    VEC_T2FV fang;
    GLint vprt[4];

    if (!engc->proj)
        return;

    fang = (VEC_T2FV){{engc->fang.x + 0.5 * M_PI, engc->fang.y}};
    VEC_V3FromAng(&engc->ldir, &fang);
    VEC_M4Translate(tmtx, engc->ftrn.x, engc->ftrn.y, engc->ftrn.z);
    VEC_M4RotOrts(rmtx, engc->fang.y, engc->fang.x, 0.0);
    VEC_M4Multiply(rmtx, tmtx, mmtx);
    VEC_M4Multiply(engc->proj->curr, mmtx, engc->view->curr);

    VEC_T3FV xdir = {{mmtx[ 0], mmtx[ 4], mmtx[ 8]}},
             ydir = {{mmtx[ 1], mmtx[ 5], mmtx[ 9]}},
             zdir = {{mmtx[ 2], mmtx[ 6], mmtx[10]}},
             offs = {{mmtx[ 3], mmtx[ 7], mmtx[11]}};

    engc->dcam = (VEC_T3FV){{-VEC_V3DotProd(&offs, &xdir),
                             -VEC_V3DotProd(&offs, &ydir),
                             -VEC_V3DotProd(&offs, &zdir)}};

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, (engc->line)? GL_LINE : GL_FILL);

    OGL_DrawVBO(engc->pool, 0, 0);  /** Draw the pool **/
    OGL_DrawVBO(engc->sphr, 0, 0);  /** Draw the sphere **/
    glCullFace(GL_FRONT);
    OGL_DrawVBO(engc->watr, 0, 0);  /** Draw the water (inner surface) **/
    glCullFace(GL_BACK);
    OGL_DrawVBO(engc->watr, 1, 0);  /** Draw the water (outer surface) **/

    if (engc->keys[KEY_RMB]) {
        glGetIntegerv(GL_VIEWPORT, vprt);
        vect.x = (GLfloat)engc->angp.x - (GLfloat)vprt[0];
        vect.y = (GLfloat)vprt[3] - (GLfloat)engc->angp.y;
        glReadPixels(vect.x, vect.y, 1, 1,
                     GL_DEPTH_COMPONENT, GL_FLOAT, &vect.z);
        VEC_V3UnProject(&vect, engc->view->curr, vprt);
        if ((fabsf(vect.x) < 0.5 * DEF_PDIM)
        &&  (fabsf(vect.z) < 0.5 * DEF_PDIM) && (vect.y > -engc->csph.w)) {
            engc->cdrp.x = vect.x;
            engc->cdrp.y = vect.z;
            engc->cdrp.z = randf(0.025 * DEF_PDIM);
            engc->cdrp.w = 0.05 * DEF_PDIM;
        }
    }
    if (engc->keys[KEY_SPACE]) {
        engc->cdrp.x = randf(0.5 * DEF_PDIM - engc->csph.w);
        engc->cdrp.y = randf(0.5 * DEF_PDIM - engc->csph.w);
        engc->cdrp.z = randf(0.025 * DEF_PDIM);
        engc->cdrp.w = 0.05 * DEF_PDIM;
        engc->keys[KEY_RMB] = 1;
    }
    if (engc->keys[KEY_F1]) {
        engc->keys[KEY_F1] = 0;
        engc->line = !engc->line;
    }
    if (engc->keys[KEY_F3]) {
        engc->keys[KEY_F3] = 0;
        engc->halt = !engc->halt;
    }
}



GLenum MakeCube(OGL_UNIF *pind, OGL_UNIF *pver, GLfloat pdim) {
    GLfloat hdim = pdim * 0.5;
    VEC_T3FV vert[] = {
        {{-hdim,-hdim,-hdim}}, {{-hdim,-hdim, hdim}},
        {{-hdim, hdim,-hdim}}, {{-hdim, hdim, hdim}},
        {{ hdim,-hdim,-hdim}}, {{ hdim,-hdim, hdim}},
        {{ hdim, hdim,-hdim}}, {{ hdim, hdim, hdim}},
    };
    GLuint indx[] = {
        0, 1, 2,  1, 3, 2,     3, 1, 7,  1, 5, 7, /** 5, 1, 4,  1, 0, 4, **/
        7, 5, 6,  4, 6, 5,     4, 0, 6,  2, 6, 0,     2, 3, 6,  7, 6, 3,
    };
    pind->type = 0;
    pind->pdat = calloc(1, pind->cdat = sizeof(indx));
    memcpy(pind->pdat, indx, pind->cdat);
    pver->type = OGL_UNI_T3FV;
    pver->pdat = calloc(1, pver->cdat = sizeof(vert));
    memcpy(pver->pdat, vert, pver->cdat);
    return GL_TRIANGLES;
}



GLenum MakePlane(OGL_UNIF *pind, OGL_UNIF *pver, GLfloat pdim, GLuint pdet) {
    GLfloat step = pdim / (GLfloat)pdet;
    GLuint x, y;

    GLuint *indx;
    VEC_T3FV *vert;

    pind->type = 0;
    pind->cdat = pdet * pdet * 6;
    pind->pdat = indx = calloc(pind->cdat, sizeof(*indx));

    pver->type = OGL_UNI_T3FV;
    pver->cdat = (pdet + 1) * (pdet + 1);
    pver->pdat = vert = calloc(pver->cdat, sizeof(*vert));

    for (y = 0; y <= pdet; y++)
        for (x = 0; x <= pdet; x++) {
            vert[x + y + y * pdet].x = (GLfloat)x * step - pdim * 0.5;
            vert[x + y + y * pdet].y = (GLfloat)y * step - pdim * 0.5;
        }
    for (y = x = 0; x < pind->cdat; y++, x += 6) {
        if (x && !(x % (pdet * 6))) y++;
        indx[x + 4] = 1 + (indx[x + 5] =
                           indx[x + 2] = pdet + (indx[x + 3] =
                                                 indx[x + 1] =
                                                 1 + (indx[x] = y)));
    }
    pind->cdat *= sizeof(*indx);
    pver->cdat *= sizeof(*vert);
    return GL_TRIANGLES;
}



GLenum MakeSphere(OGL_UNIF *pind, OGL_UNIF *pver, GLuint hdet, GLuint rdet) {
    GLfloat hang, rang, hstp = 1.0 * M_PI / (GLfloat)hdet,
                        rstp = 2.0 * M_PI / (GLfloat)rdet;
    GLuint x, y;

    GLuint *indx;
    VEC_T3FV *vert;

    pind->type = 0;
    pind->cdat = hdet * rdet * 6;
    pind->pdat = indx = calloc(pind->cdat, sizeof(*indx));

    pver->type = OGL_UNI_T3FV;
    pver->cdat = (hdet + 1) * rdet;
    pver->pdat = vert = calloc(pver->cdat, sizeof(*vert));

    for (y = 0; y <= hdet; y++) {
        hang = hstp * (GLfloat)y - 0.5 * M_PI;
        for (x = 0; x < rdet; x++) {
            rang = rstp * (GLfloat)x;
            vert[x + y * rdet].x = cos(rang) * cos(hang);
            vert[x + y * rdet].y = sin(hang);
            vert[x + y * rdet].z = sin(rang) * cos(hang);
        }
    }
    for (y = (x = 0) + 1; x < pind->cdat; y++, x += 6) {
        indx[x + 2] = indx[x + 5] = (indx[x + 4] = y - 1) + rdet;
        if (y % rdet)
            indx[x + 0] = (indx[x + 1] = indx[x + 3] = y) + rdet;
        else
            indx[x + 1] = indx[x + 3] = (indx[x + 0] = y) - rdet;
    }
    pind->cdat *= sizeof(*indx);
    pver->cdat *= sizeof(*vert);
    return GL_TRIANGLES;
}



ENGC *cMakeEngine() {
    const char *func = /** === a set of common shader functions **/
        "#define poolAboveWater (dims.z * 2.0 - 1.0)\n"
        "uniform vec3 ldir;"  /* ^---[ pool coef. height ] */
        "uniform vec3 dims;"
        "uniform vec4 csph;"
        "uniform mat4 clrs;"
        "uniform sampler2D tiles;"
        "uniform sampler2D caust;"
        "uniform sampler2D water;"

        "vec3 v3refract(vec3 I, vec3 N, float eta) {"
            "float k = 1.0 - eta * eta * (1.0 - dot(N, I) * dot(N, I));"

            "if (k < 0.0) return vec3(0.0, 0.0, 0.0);"
            "return eta * I - (eta * dot(N, I) + sqrt(k)) * N;"
        "}"

        "vec3 v3reflect(vec3 I, vec3 N) {"
            "return I - 2.0 * dot(N, I) * N;"
        "}"

        "float intersectSphere(vec3 origin, vec3 ray, vec3 csph, float radius) {"
            "vec3 toSphere = origin - csph;"

            "float a = dot(ray, ray);"
            "float b = 2.0 * dot(toSphere, ray);"
            "float c = dot(toSphere, toSphere) - radius * radius;"
            "float discriminant = b*b - 4.0*a*c;"

            "if (discriminant > 0.0) {"
                "float t = (-b - sqrt(discriminant)) / (2.0 * a);"
                "if (t > 0.0) return t;"
            "}"
            "return 1.0e6;"
        "}"

        "vec3 getSphereColor(vec3 point) {"
            "vec3 color = vec3(0.5);"

            /* ambient occlusion with walls */
            "color *= 1.0 - 0.9 / pow((1.0 + csph.w - abs(point.x)) / csph.w, 3.0);"
            "color *= 1.0 - 0.9 / pow((1.0 + csph.w - abs(point.z)) / csph.w, 3.0);"
            "color *= 1.0 - 0.9 / pow((point.y + 1.0 + csph.w) / csph.w, 3.0);"

            /* caustics                                    [ DEF_AIRF/DEF_WTRF ] */
            "vec3 sphereNormal = (point - csph.xyz) / csph.w;" /*    v           */
            "vec3 refrac = v3refract(-ldir, vec3(0.0, 1.0, 0.0), dims.x);"
            "float diffuse = max(0.0, dot(-refrac, sphereNormal)) * 0.5;"
            "vec4 info = texture2D(water, point.xz * 0.5 + 0.5);"

            "if (point.y < info.r) {"
                "vec4 caustic = texture2D(caust, 0.5 + 0.75 * 0.5"
                                      "* (point.xz - point.y * refrac.xz / refrac.y));"
                "diffuse *= caustic.r * 4.0;"
            "}"
            "color += diffuse;"

            "return color;"
        "}"

        "vec2 intersectCube(vec3 origin, vec3 ray, vec3 cubeMin, vec3 cubeMax) {"
            "vec3 tMin = (cubeMin - origin) / ray;"
            "vec3 tMax = (cubeMax - origin) / ray;"
            "vec3 t1 = min(tMin, tMax);"
            "vec3 t2 = max(tMin, tMax);"
            "float tNear = max(max(t1.x, t1.y), t1.z);"
            "float tFar = min(min(t2.x, t2.y), t2.z);"
            "return vec2(tNear, tFar);"
        "}"

        "vec3 getWallColor(vec3 point) {"
            "float scale = 0.5;"

            "vec3 wallColor;"
            "vec3 normal;"
            "if (abs(point.x) > 0.999) {"
                "wallColor = texture2D(tiles, point.yz * 0.5 + vec2(1.0, 0.5)).rgb;"
                "normal = vec3(-point.x, 0.0, 0.0);"
            "} else if (abs(point.z) > 0.999) {"
                "wallColor = texture2D(tiles, point.yx * 0.5 + vec2(1.0, 0.5)).rgb;"
                "normal = vec3(0.0, 0.0, -point.z);"
            "} else {"
                "wallColor = texture2D(tiles, point.xz * 0.5 + 0.5).rgb;"
                "normal = vec3(0.0, 1.0, 0.0);"
            "}"
            "scale /= length(point);" /* <---[ pool ambient occlusion ] */
            "scale *= 1.0 - 0.9 / pow(length(point - csph.xyz) / csph.w, 4.0);"
            /* ^---[ sphere ambient occlusion ] */
            /* caustics                       [ DEF_AIRF/DEF_WTRF ]---v */
            "vec3 refrac = -v3refract(-ldir, vec3(0.0, 1.0, 0.0), dims.x);"
            "float diffuse = max(0.0, dot(refrac, normal));"

            "vec4 info = texture2D(water, point.xz * 0.5 + 0.5);"

            "if (point.y < info.r) {"
                "vec4 caustic = texture2D(caust, 0.5 + 0.5 * 0.75"
                                      "* (point.xz - point.y * refrac.xz / refrac.y));"
                "scale += diffuse * caustic.r * 2.0 * caustic.g;"
            "} else {"
                /* shadow for the rim of the pool     [ pool height ]--v */
                "vec2 t = intersectCube(point, refrac, vec3(-1.0, -dims.y, -1.0),"
                                       "vec3(1.0, 2.0, 1.0));"
                "diffuse /= 1.0 + exp((point.y + refrac.y * t.y - poolAboveWater)"
                                    "* -200.0 / (1.0 + 10.0 * (t.y - t.x)));"
                "scale += diffuse * 0.5;"
            "}"

            "return wallColor * scale;"
        "}";

    /** CAUTION! Notice the two FREE()`s at the end of _VBO()! **/
    #define _VBO(ctex, elem, patr, puni, pshd, ...)                         \
            OGL_MakeVBO(ctex, elem, sizeof(patr) / sizeof(*patr), patr,     \
                        sizeof(puni) / sizeof(*puni), puni,                 \
                        sizeof(pshd) / sizeof(*pshd), pshd, ##__VA_ARGS__); \
                        free(patr[0].pdat); free(patr[1].pdat)
    ENGC *retn;

    srand(time(0));

    retn = calloc(1, sizeof(*retn));

    retn->ftrn.x =  0.80 * DEF_PDIM;
    retn->ftrn.y = -0.65 * DEF_PDIM;
    retn->ftrn.z = -1.50 * DEF_PDIM;

    retn->fang.x = 30.00 * VEC_DTOR;
    retn->fang.y = 30.00 * VEC_DTOR;

    glClearColor(0.0, 0.0, 0.0, 1.0);

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    retn->wsur = 128.0;
    retn->winv.x = retn->winv.y = 1.0 / (retn->wdet.x = retn->wdet.y = 128.0);
    retn->cinv.x = retn->cinv.y = 1.0 / (retn->cdet.x = retn->cdet.y = 1024.0);
    retn->dims = (VEC_T3FV){{DEF_AIRF / DEF_WTRF, DEF_PHEI, DEF_CHEI}};

    OGL_UNIF attr[] = {{/** indices **/  .draw = GL_STATIC_DRAW},
                       {.name = "vert",  .draw = GL_STATIC_DRAW}};

    retn->csur =
    _VBO(2, MakePlane(&attr[0], &attr[1], DEF_PDIM, 1), attr, ((OGL_UNIF[])
        {{.name = "clrs",  .type = OGL_UNI_TMFI, .pdat = &retn->clrs},
         {.name = "cdrp",  .type = OGL_UNI_T4FV, .pdat = &retn->cdrp},
         {.name = "dims",  .type = OGL_UNI_T3FV, .pdat = &retn->dims},
         {.name = "winv",  .type = OGL_UNI_T2FV, .pdat = &retn->winv},
         {.name = "water", .type = OGL_UNI_T1II, .pdat = (GLvoid*)0}}),
        ((char*[]) { /** Computational surface **/
         "attribute vec3 vert;" /** === main vertex shader **/

         "varying vec2 coord;"

         "void main() {"
             "coord = vert.xy * 0.5 + 0.5;"
             "gl_Position = vec4(vert.xyz, 1.0);"
         "}",

         /** === "udpating" pixel shader **/
         "uniform sampler2D water;"
         "uniform vec2 winv;"

         "varying vec2 coord;"

         "void main() {"
             /* get vertex info */
             "vec4 info = texture2D(water, coord);"

             /* calculate average neighbor height */
             "vec2 dx = vec2(winv.x, 0.0);"
             "vec2 dy = vec2(0.0, winv.y);"
             "float average = (texture2D(water, coord - dx).r +"
                              "texture2D(water, coord - dy).r +"
                              "texture2D(water, coord + dx).r +"
                              "texture2D(water, coord + dy).r) * 0.25;"

             /* change the velocity to move toward the average */
             "info.g += (average - info.r) * 2.0;"

             /* attenuate the velocity a little so waves do not last forever */
             "info.g *= 0.995;"

             /* move the vertex along the velocity */
             "info.r += info.g;"

             "gl_FragColor = info;"
         "}",

         (char*)-1, /** === same vertex shader **/

         "uniform sampler2D water;" /** === "renormalling" pixel shader **/
         "uniform vec2 winv;"

         "varying vec2 coord;"

         "void main() {"
             /* get vertex info */
             "vec4 info = texture2D(water, coord);"

             /* update the normal */
             "vec3 dx = vec3(winv.x,"
                            "texture2D(water,"
                                      "vec2(coord.x + winv.x, coord.y)).r - info.r,"
                            "0.0);"
             "vec3 dy = vec3(0.0,"
                            "texture2D(water,"
                                      "vec2(coord.x, coord.y + winv.y)).r - info.r,"
                            "winv.y);"
             "info.ba = normalize(cross(dy, dx)).xz;"

             "gl_FragColor = info;"
         "}",

         (char*)-1, /** === same vertex shader **/

         "const float PI = 3.141592653589793;" /** === "drop throwing" pixel shader **/

         "uniform sampler2D water;"
         "uniform vec4 cdrp;"

         "varying vec2 coord;"

         "void main() {"
             /* get vertex info */
             "vec4 info = texture2D(water, coord);"

             /* add the drop to the height */
             "float drop = max(1.0 - length(cdrp.xy * 0.5 + 0.5 - coord) / cdrp.w,"
                              "0.0);"
             "drop = 0.5 - cos(drop * PI) * 0.5;"
             "info.r += drop * cdrp.z;"

             "gl_FragColor = info;"
         "}",

         (char*)-1, /** === same vertex shader **/

         "varying vec2 coord;" /** === "zeroing" pixel shader **/

         "void main() {"
             "gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);"
         "}"}));

    retn->gcau =
    _VBO(2, MakePlane(&attr[0], &attr[1], DEF_PDIM, 1), attr, ((OGL_UNIF[])
        {{.name = "clrs",  .type = OGL_UNI_TMFI, .pdat = &retn->clrs},
         {.name = "dims",  .type = OGL_UNI_T3FV, .pdat = &retn->dims},
         {.name = "cinv",  .type = OGL_UNI_T2FV, .pdat = &retn->cinv},
         {.name = "caust", .type = OGL_UNI_T1II, .pdat = (GLvoid*)0}}),
        ((char*[]) { /** Gauss-blur surface **/
         "attribute vec3 vert;" /** === main vertex shader **/

         "varying vec2 coord;"

         "void main() {"
             "coord = vert.xy * 0.5 + 0.5;"
             "gl_Position = vec4(vert.xyz, 1.0);"
         "}",

         "uniform sampler2D caust;" /** === "1D-blur + transposition" pixel shader **/
         "uniform vec2 cinv;"

         "varying vec2 coord;"

         "void main() {"
             "vec4 info = texture2D(caust, coord.yx);"

             "info.r = 0.199676 * info.r"
             "+ 0.176213 * (texture2D(caust, vec2(coord.y - 1.0 * cinv.x, coord.x)).r"
                        "+  texture2D(caust, vec2(coord.y + 1.0 * cinv.x, coord.x)).r)"
             "+ 0.121109 * (texture2D(caust, vec2(coord.y - 2.0 * cinv.x, coord.x)).r"
                        "+  texture2D(caust, vec2(coord.y + 2.0 * cinv.x, coord.x)).r)"
             "+ 0.064825 * (texture2D(caust, vec2(coord.y - 3.0 * cinv.x, coord.x)).r"
                        "+  texture2D(caust, vec2(coord.y + 3.0 * cinv.x, coord.x)).r)"
             "+ 0.027023 * (texture2D(caust, vec2(coord.y - 4.0 * cinv.x, coord.x)).r"
                        "+  texture2D(caust, vec2(coord.y + 4.0 * cinv.x, coord.x)).r)"
             "+ 0.008773 * (texture2D(caust, vec2(coord.y - 5.0 * cinv.x, coord.x)).r"
                        "+  texture2D(caust, vec2(coord.y + 5.0 * cinv.x, coord.x)).r)"
             "+ 0.002218 * (texture2D(caust, vec2(coord.y - 6.0 * cinv.x, coord.x)).r"
                        "+  texture2D(caust, vec2(coord.y + 6.0 * cinv.x, coord.x)).r);"
             "gl_FragColor = info;"
         "}"}));

    retn->watr =
    _VBO(4, MakePlane(&attr[0], &attr[1], DEF_PDIM, retn->wsur), attr, ((OGL_UNIF[])
        {{.name = "mMVP",  .type = OGL_UNI_TMFV, .pdat = &retn->view},
         {.name = "clrs",  .type = OGL_UNI_TMFI, .pdat = &retn->clrs},
         {.name = "csph",  .type = OGL_UNI_T4FV, .pdat = &retn->csph},
         {.name = "dims",  .type = OGL_UNI_T3FV, .pdat = &retn->dims},
         {.name = "ldir",  .type = OGL_UNI_T3FV, .pdat = &retn->ldir},
         {.name = "dcam",  .type = OGL_UNI_T3FV, .pdat = &retn->dcam},
         {.name = "cloud", .type = OGL_UNI_T1II, .pdat = (GLvoid*)0},
         {.name = "caust", .type = OGL_UNI_T1II, .pdat = (GLvoid*)1},
         {.name = "tiles", .type = OGL_UNI_T1II, .pdat = (GLvoid*)2},
         {.name = "water", .type = OGL_UNI_T1II, .pdat = (GLvoid*)3}}),
        ((char*[]) { /** Water surface **/
         "uniform sampler2D water;" /** === main vertex shader **/
         "uniform mat4 mMVP;"
         "attribute vec3 vert;"

         "varying vec3 position;"

         "void main() {"
             "vec4 info = texture2D(water, vert.xy * 0.5 + 0.5);"
             "position = vert.xzy;"
             "position.y += info.r;"
             "gl_Position = mMVP * vec4(position, 1.0);"
         "}",

         "%s%s" /** === above-water pixel shader **/
             "vec3 normal = vec3(info.b, sqrt(1.0 - dot(info.ba, info.ba)), info.a);"
             "vec3 reflec = v3reflect(income, normal);"/* v--[ DEF_AIRF/DEF_WTRF ] */
             "vec3 refrac = v3refract(income, normal, dims.x);"
             "float fresnel = mix(0.25, 1.0, pow(1.0 - dot(normal, -income), 3.0));"
                                                     /* [ above-water color ]---v */
             "vec3 reflectedColor = getSurfaceRayColor(position, reflec, clrs[0].rgb);"
             "vec3 refractedColor = getSurfaceRayColor(position, refrac, clrs[0].rgb);"
                                                     /* [ above-water color ]---^ */
             "gl_FragColor = vec4(mix(refractedColor, reflectedColor, fresnel), 1.0);"
         "}",

         (char*)-1, /** === same vertex shader **/

         "%s%s" /** === below-water pixel shader **/
             "vec3 normal = -vec3(info.b, sqrt(1.0 - dot(info.ba, info.ba)), info.a);"
             "vec3 reflec = v3reflect(income, normal);"/* v--[ DEF_WTRF/DEF_AIRF ] */
             "vec3 refrac = v3refract(income, normal, 1.0 / dims.x);"
             "float fresnel = mix(0.5, 1.0, pow(1.0 - dot(normal, -income), 3.0));"
                                                     /* [ underwater color ]---v */
             "vec3 reflectedColor = getSurfaceRayColor(position, reflec, clrs[1].rgb);"
             "vec3 refractedColor = clrs[2].rgb" /* <---[ wall inverse color ] */
                                 "* getSurfaceRayColor(position, refrac, vec3(1.0));"
             "gl_FragColor = vec4(mix(reflectedColor, refractedColor,"
                                     "(1.0 - fresnel) * length(refrac)), 1.0);"
         "}",

         "%s" /** === caustics vertex shader **/
         "attribute vec3 vert;"

         "varying vec3 oldPos;"
         "varying vec3 newPos;"
         "varying vec3 ray;"

         /* project the ray onto the plane */
         "vec3 project(vec3 origin, vec3 ray, vec3 refrac) {"  /* v--[ pool height ] */
             "vec2 tcube = intersectCube(origin, ray, vec3(-1.0, -dims.y, -1.0),"
                                         "vec3(1.0, 2.0, 1.0));"
             "origin += ray * tcube.y;"
             "float tplane = (-origin.y - 1.0) / refrac.y;"
             "return origin + refrac * tplane;"
         "}"

         "void main() {"
             "vec4 info = texture2D(water, vert.xy * 0.5 + 0.5);"
             "info.ba *= 0.5;"
             "vec3 normal = vec3(info.b, sqrt(1.0 - dot(info.ba, info.ba)), info.a);"

             /* project the vertices along the refracted vertex ray */
             "vec3 refrac = v3refract(-ldir, vec3(0.0, 1.0, 0.0), dims.x);"
             "ray = v3refract(-ldir, normal, dims.x);"/* <------      ^            */
             "oldPos = project(vert.xzy, refrac, refrac);"/* [ DEF_AIRF/DEF_WTRF ] */
             "newPos = project(vert.xzy + vec3(0.0, info.r, 0.0), ray, refrac);"

             "gl_Position = vec4(0.75 * (newPos.xz + refrac.xz / refrac.y), 0.0, 1.0);"
         "}",

         "%s" /** === caustics pixel shader **/
         "varying vec3 oldPos;"
         "varying vec3 newPos;"
         "varying vec3 ray;"

         "void main() {"
             /* if the triangle gets smaller, it gets brighter, and vice versa */
             "float oldArea = length(dFdx(oldPos)) * length(dFdy(oldPos));"
             "float newArea = length(dFdx(newPos)) * length(dFdy(newPos));"
             "gl_FragColor = vec4(oldArea / newArea * 0.2, 1.0, 0.0, 0.0);"
                                             /* [ DEF_AIRF/DEF_WTRF ]---v */
             "vec3 refrac = v3refract(-ldir, vec3(0.0, 1.0, 0.0), dims.x);"

             /* compute a blob shadow and make sure we only
                 draw a shadow if the viewer is blocking the light */
             "vec3 dir = (csph.xyz - newPos) / csph.w;"
             "vec3 area = cross(dir, refrac);"
             "float shadow = dot(area, area);"
             "float dist = dot(dir, -refrac);"
             "shadow = 1.0 + (shadow - 1.0) / (0.05 + dist * 0.025);"
             "shadow = clamp(1.0 / (1.0 + exp(-shadow)), 0.0, 1.0);"
             "shadow = mix(1.0, shadow, clamp(dist * 2.0, 0.0, 1.0));"
             "gl_FragColor.g = shadow;"

             /* shadow for the rim of the pool      [ pool height ]---v */
             "vec2 t = intersectCube(newPos, -refrac, vec3(-1.0, -dims.y, -1.0),"
                                     "vec3(1.0, 2.0, 1.0));"
             "gl_FragColor.r /= 1.0 + exp((newPos.y - poolAboveWater - refrac.y * t.y)"
                                         "* -200.0 / (1.0 + 10.0 * (t.y - t.x)));"
         "}"}), func,
         "uniform vec3 dcam;" /** === above- and below-water shaders` common functions **/
         "uniform samplerCube cloud;"

         "varying vec3 position;"

         "vec3 getSurfaceRayColor(vec3 origin, vec3 ray, vec3 waterColor) {"
             "vec3 color;"

             "float q = intersectSphere(origin, ray, csph.xyz, csph.w);"
             "if (q < 1.0e6) {"
                 "color = getSphereColor(origin + ray * q);"
             "} else if (ray.y < 0.0) {"         /* [ pool height ]---v */
                 "vec2 t = intersectCube(origin, ray, vec3(-1.0, -dims.y, -1.0),"
                                        "vec3(1.0, 2.0, 1.0));"
                 "color = getWallColor(origin + ray * t.y);"
             "} else {"                          /* [ pool height ]---v */
                 "vec2 t = intersectCube(origin, ray, vec3(-1.0, -dims.y, -1.0),"
                                        "vec3(1.0, 2.0, 1.0));"
                 "vec3 hit = origin + ray * t.y;"/* v---[ pool height ] */
                 "if (hit.y < poolAboveWater * dims.y) {"
                     "color = getWallColor(hit);"
                 "} else {"
                     "color = textureCube(cloud, ray).rgb;"
                     "color += vec3(pow(max(0.0, dot(ldir, ray)), 5000.0))"
                            "* vec3(10.0, 8.0, 6.0);"
                 "}"
             "}"
             "if (ray.y < 0.0) color *= waterColor;"
             "return color;"
         "}"

         "void main() {"
             "vec2 coord = position.xz * 0.5 + 0.5;"
             "vec4 info = texture2D(water, coord);"

             /* make water look more 'peaked' */
             "for (int i = 0; i < 5; i++) {"
                 "coord += info.ba * 0.005;"
                 "info = texture2D(water, coord);"
             "}"
             /*    v---[ incoming ray ] */
             "vec3 income = normalize(position - dcam);");

    retn->pool =
    _VBO(3, MakeCube(&attr[0], &attr[1], DEF_PDIM), attr, ((OGL_UNIF[])
        {{.name = "mMVP",  .type = OGL_UNI_TMFV, .pdat = &retn->view},
         {.name = "clrs",  .type = OGL_UNI_TMFI, .pdat = &retn->clrs},
         {.name = "csph",  .type = OGL_UNI_T4FV, .pdat = &retn->csph},
         {.name = "dims",  .type = OGL_UNI_T3FV, .pdat = &retn->dims},
         {.name = "ldir",  .type = OGL_UNI_T3FV, .pdat = &retn->ldir},
         {.name = "tiles", .type = OGL_UNI_T1II, .pdat = (GLvoid*)0},
         {.name = "caust", .type = OGL_UNI_T1II, .pdat = (GLvoid*)1},
         {.name = "water", .type = OGL_UNI_T1II, .pdat = (GLvoid*)2}}),
        ((char*[]) { /** Pool walls **/
         "%s" /** === main vertex shader **/
         "uniform mat4 mMVP;"
         "attribute vec3 vert;"

         "varying vec3 position;"

         "void main(void) {"   /* [ pool coef. height ] -------. */
             "position = vert;"/*v---[ pool height ]           v */
             "position.y = dims.y * ((1.0 - position.y) * dims.z - 1.0);"
             "gl_Position = mMVP * vec4(position, 1.0);"
         "}",

         "%s" /** === main pixel shader **/
         "varying vec3 position;"

         "void main(void) {"
             "gl_FragColor = vec4(getWallColor(position), 1.0);"
             "vec4 info = texture2D(water, position.xz * 0.5 + 0.5);"
             "if (position.y < info.r) {" /* v---[ underwater color ] */
                 "gl_FragColor.rgb *= clrs[1].rgb * 1.2;"
             "}"
         "}"}), func);

    retn->sphr =
    _VBO(2, MakeSphere(&attr[0], &attr[1], 16, 32), attr, ((OGL_UNIF[])
        {{.name = "mMVP",  .type = OGL_UNI_TMFV, .pdat = &retn->view},
         {.name = "clrs",  .type = OGL_UNI_TMFI, .pdat = &retn->clrs},
         {.name = "csph",  .type = OGL_UNI_T4FV, .pdat = &retn->csph},
         {.name = "dims",  .type = OGL_UNI_T3FV, .pdat = &retn->dims},
         {.name = "ldir",  .type = OGL_UNI_T3FV, .pdat = &retn->ldir},
         {.name = "caust", .type = OGL_UNI_T1II, .pdat = (GLvoid*)0},
         {.name = "water", .type = OGL_UNI_T1II, .pdat = (GLvoid*)1}}),
        ((char*[]) { /** Sphere **/
         "%s" /** === main vertex shader **/
         "uniform mat4 mMVP;"
         "attribute vec3 vert;"

         "varying vec3 position;"

         "void main() {"
             "position = csph.xyz + vert.xyz * csph.w;"
             "gl_Position = mMVP * vec4(position, 1.0);"
         "}",

         "%s" /** === main pixel shader **/
         "varying vec3 position;"

         "void main() {"
             "gl_FragColor = vec4(getSphereColor(position), 1.0);"
             "vec4 info = texture2D(water, position.xz * 0.5 + 0.5);"
             "if (position.y < info.r)"   /* v---[ underwater color ] */
                 "gl_FragColor.rgb *= clrs[1].rgb * 1.2;"
         "}"}), func);

    *OGL_BindTex(retn->csur, 0, OGL_TEX_NSET) =
        OGL_MakeTex(retn->wdet.x, retn->wdet.y, 0, GL_TEXTURE_2D,
                    GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR,
                    GL_FLOAT, GL_RGBA32F, GL_RGBA, 0);
    *OGL_BindTex(retn->csur, 1, OGL_TEX_NSET) =
        OGL_MakeTex(retn->wdet.x, retn->wdet.y, 0, GL_TEXTURE_2D,
                    GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR,
                    GL_FLOAT, GL_RGBA32F, GL_RGBA, 0);

    *OGL_BindTex(retn->gcau, 0, OGL_TEX_NSET) =
        OGL_MakeTex(retn->cdet.x, retn->cdet.y, 0, GL_TEXTURE_2D,
                    GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR,
                    GL_UNSIGNED_BYTE, GL_RGBA, GL_RGBA, 0);
    *OGL_BindTex(retn->gcau, 1, OGL_TEX_NSET) =
        OGL_MakeTex(retn->cdet.x, retn->cdet.y, 0, GL_TEXTURE_2D,
                    GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR,
                    GL_UNSIGNED_BYTE, GL_RGBA, GL_RGBA, 0);

    /** water **/
    OGL_LinkTex(retn->sphr, 1, retn->csur, 0);
    OGL_LinkTex(retn->pool, 2, retn->csur, 0);
    OGL_LinkTex(retn->watr, 3, retn->csur, 0);

    /** caust **/
    OGL_LinkTex(retn->sphr, 0, retn->gcau, 0);
    OGL_LinkTex(retn->pool, 1, retn->gcau, 0);
    OGL_LinkTex(retn->watr, 1, retn->gcau, 0);

    /** tiles **/
    OGL_LinkTex(retn->watr, 2, retn->pool, 0);

    retn->rsur = MakeRBO(retn->csur, 0, 1);
    retn->rcau = MakeRBO(retn->gcau, 0, 1);
    retn->rwtr = MakeRBO(retn->watr, 1, 1);

    retn->csph.w =       0.25 * DEF_PHEI;
    retn->csph.x = randf(0.50 * DEF_PDIM - retn->csph.w);
    retn->csph.y =     -(0.50 * DEF_PDIM - retn->csph.w);
    retn->csph.z = randf(0.50 * DEF_PDIM - retn->csph.w);

    *OGL_BindTex(retn->pool, 0, OGL_TEX_NSET) = MakeTileTex(9, 5, 1);
    RegenerateColors(retn);
    return retn;
    #undef _VBO
}



void cFreeEngine(ENGC **engc) {
    FreeRBO(&(*engc)->rsur);
    FreeRBO(&(*engc)->rwtr);
    FreeRBO(&(*engc)->rcau);

    OGL_FreeVBO(&(*engc)->csur);
    OGL_FreeVBO(&(*engc)->watr);
    OGL_FreeVBO(&(*engc)->gcau);
    OGL_FreeVBO(&(*engc)->pool);
    OGL_FreeVBO(&(*engc)->sphr);

    VEC_PurgeMatrixStack(&(*engc)->proj);
    VEC_PurgeMatrixStack(&(*engc)->view);

    free(*engc);
    *engc = 0;
}
