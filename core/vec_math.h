#ifndef VEC_MATH_H
#define VEC_MATH_H

#include <stdint.h>
#include <math.h>



#define DEG_CRAD (M_PI / 180.0)
#define RAD_CDEG (180.0 / M_PI)



#pragma pack(push, 1)
typedef float TMFV[16];

typedef struct _FMST {
    TMFV curr;
    struct _FMST *prev;
} FMST;

typedef struct {
    float x, y;
} T2FV;

typedef struct {
    float x, y, z;
} T3FV;

typedef struct {
    float x, y, z, w;
} T4FV;

typedef struct {
    int32_t x, y;
} T2IV;

typedef struct {
    int32_t x, y, z;
} T3IV;

typedef struct {
    int32_t x, y, z, w;
} T4IV;
#pragma pack(pop)



static inline void M4Duplicate(TMFV mdst, TMFV msrc) {
    memcpy(mdst, msrc, sizeof(TMFV));
}



static void M4Invert(TMFV minv) {
    float mdet;
    TMFV mtmp;
    int i;

    M4Duplicate(mtmp, minv);

    minv[ 0] = + mtmp[ 5]*mtmp[10]*mtmp[15] - mtmp[ 5]*mtmp[14]*mtmp[11]
               - mtmp[ 6]*mtmp[ 9]*mtmp[15] + mtmp[ 6]*mtmp[13]*mtmp[11]
               + mtmp[ 7]*mtmp[ 9]*mtmp[14] - mtmp[ 7]*mtmp[13]*mtmp[10];
    minv[ 1] = - mtmp[ 1]*mtmp[10]*mtmp[15] + mtmp[ 1]*mtmp[14]*mtmp[11]
               + mtmp[ 2]*mtmp[ 9]*mtmp[15] - mtmp[ 2]*mtmp[13]*mtmp[11]
               - mtmp[ 3]*mtmp[ 9]*mtmp[14] + mtmp[ 3]*mtmp[13]*mtmp[10];
    minv[ 2] = + mtmp[ 1]*mtmp[ 6]*mtmp[15] - mtmp[ 1]*mtmp[14]*mtmp[ 7]
               - mtmp[ 2]*mtmp[ 5]*mtmp[15] + mtmp[ 2]*mtmp[13]*mtmp[ 7]
               + mtmp[ 3]*mtmp[ 5]*mtmp[14] - mtmp[ 3]*mtmp[13]*mtmp[ 6];
    minv[ 3] = - mtmp[ 1]*mtmp[ 6]*mtmp[11] + mtmp[ 1]*mtmp[10]*mtmp[ 7]
               + mtmp[ 2]*mtmp[ 5]*mtmp[11] - mtmp[ 2]*mtmp[ 9]*mtmp[ 7]
               - mtmp[ 3]*mtmp[ 5]*mtmp[10] + mtmp[ 3]*mtmp[ 9]*mtmp[ 6];

    minv[ 4] = - mtmp[ 4]*mtmp[10]*mtmp[15] + mtmp[ 4]*mtmp[14]*mtmp[11]
               + mtmp[ 6]*mtmp[ 8]*mtmp[15] - mtmp[ 6]*mtmp[12]*mtmp[11]
               - mtmp[ 7]*mtmp[ 8]*mtmp[14] + mtmp[ 7]*mtmp[12]*mtmp[10];
    minv[ 5] = + mtmp[ 0]*mtmp[10]*mtmp[15] - mtmp[ 0]*mtmp[14]*mtmp[11]
               - mtmp[ 2]*mtmp[ 8]*mtmp[15] + mtmp[ 2]*mtmp[12]*mtmp[11]
               + mtmp[ 3]*mtmp[ 8]*mtmp[14] - mtmp[ 3]*mtmp[12]*mtmp[10];
    minv[ 6] = - mtmp[ 0]*mtmp[ 6]*mtmp[15] + mtmp[ 0]*mtmp[14]*mtmp[ 7]
               + mtmp[ 2]*mtmp[ 4]*mtmp[15] - mtmp[ 2]*mtmp[12]*mtmp[ 7]
               - mtmp[ 3]*mtmp[ 4]*mtmp[14] + mtmp[ 3]*mtmp[12]*mtmp[ 6];
    minv[ 7] = + mtmp[ 0]*mtmp[ 6]*mtmp[11] - mtmp[ 0]*mtmp[10]*mtmp[ 7]
               - mtmp[ 2]*mtmp[ 4]*mtmp[11] + mtmp[ 2]*mtmp[ 8]*mtmp[ 7]
               + mtmp[ 3]*mtmp[ 4]*mtmp[10] - mtmp[ 3]*mtmp[ 8]*mtmp[ 6];

    minv[ 8] = + mtmp[ 4]*mtmp[ 9]*mtmp[15] - mtmp[ 4]*mtmp[13]*mtmp[11]
               - mtmp[ 5]*mtmp[ 8]*mtmp[15] + mtmp[ 5]*mtmp[12]*mtmp[11]
               + mtmp[ 7]*mtmp[ 8]*mtmp[13] - mtmp[ 7]*mtmp[12]*mtmp[ 9];
    minv[ 9] = - mtmp[ 0]*mtmp[ 9]*mtmp[15] + mtmp[ 0]*mtmp[13]*mtmp[11]
               + mtmp[ 1]*mtmp[ 8]*mtmp[15] - mtmp[ 1]*mtmp[12]*mtmp[11]
               - mtmp[ 3]*mtmp[ 8]*mtmp[13] + mtmp[ 3]*mtmp[12]*mtmp[ 9];
    minv[10] = + mtmp[ 0]*mtmp[ 5]*mtmp[15] - mtmp[ 0]*mtmp[13]*mtmp[ 7]
               - mtmp[ 1]*mtmp[ 4]*mtmp[15] + mtmp[ 1]*mtmp[12]*mtmp[ 7]
               + mtmp[ 3]*mtmp[ 4]*mtmp[13] - mtmp[ 3]*mtmp[12]*mtmp[ 5];
    minv[11] = - mtmp[ 0]*mtmp[ 5]*mtmp[11] + mtmp[ 0]*mtmp[ 9]*mtmp[ 7]
               + mtmp[ 1]*mtmp[ 4]*mtmp[11] - mtmp[ 1]*mtmp[ 8]*mtmp[ 7]
               - mtmp[ 3]*mtmp[ 4]*mtmp[ 9] + mtmp[ 3]*mtmp[ 8]*mtmp[ 5];

    minv[12] = - mtmp[ 4]*mtmp[ 9]*mtmp[14] + mtmp[ 4]*mtmp[13]*mtmp[10]
               + mtmp[ 5]*mtmp[ 8]*mtmp[14] - mtmp[ 5]*mtmp[12]*mtmp[10]
               - mtmp[ 6]*mtmp[ 8]*mtmp[13] + mtmp[ 6]*mtmp[12]*mtmp[ 9];
    minv[13] = + mtmp[ 0]*mtmp[ 9]*mtmp[14] - mtmp[ 0]*mtmp[13]*mtmp[10]
               - mtmp[ 1]*mtmp[ 8]*mtmp[14] + mtmp[ 1]*mtmp[12]*mtmp[10]
               + mtmp[ 2]*mtmp[ 8]*mtmp[13] - mtmp[ 2]*mtmp[12]*mtmp[ 9];
    minv[14] = - mtmp[ 0]*mtmp[ 5]*mtmp[14] + mtmp[ 0]*mtmp[13]*mtmp[ 6]
               + mtmp[ 1]*mtmp[ 4]*mtmp[14] - mtmp[ 1]*mtmp[12]*mtmp[ 6]
               - mtmp[ 2]*mtmp[ 4]*mtmp[13] + mtmp[ 2]*mtmp[12]*mtmp[ 5];
    minv[15] = + mtmp[ 0]*mtmp[ 5]*mtmp[10] - mtmp[ 0]*mtmp[ 9]*mtmp[ 6]
               - mtmp[ 1]*mtmp[ 4]*mtmp[10] + mtmp[ 1]*mtmp[ 8]*mtmp[ 6]
               + mtmp[ 2]*mtmp[ 4]*mtmp[ 9] - mtmp[ 2]*mtmp[ 8]*mtmp[ 5];

    if ((mdet = mtmp[ 0]*minv[ 0] + mtmp[ 1]*minv[ 4]
              + mtmp[ 2]*minv[ 8] + mtmp[ 3]*minv[12]) != 0.0)
        mdet = 1.0 / mdet;
    for (i = 0; i < 16; i++)
        minv[i] *= mdet;
}



static void M4Multiply(TMFV mul1, TMFV mul2, TMFV mres) {
    mres[ 0] = mul1[ 0]*mul2[ 0] + mul1[ 1]*mul2[ 4]
             + mul1[ 2]*mul2[ 8] + mul1[ 3]*mul2[12];
    mres[ 1] = mul1[ 0]*mul2[ 1] + mul1[ 1]*mul2[ 5]
             + mul1[ 2]*mul2[ 9] + mul1[ 3]*mul2[13];
    mres[ 2] = mul1[ 0]*mul2[ 2] + mul1[ 1]*mul2[ 6]
             + mul1[ 2]*mul2[10] + mul1[ 3]*mul2[14];
    mres[ 3] = mul1[ 0]*mul2[ 3] + mul1[ 1]*mul2[ 7]
             + mul1[ 2]*mul2[11] + mul1[ 3]*mul2[15];

    mres[ 4] = mul1[ 4]*mul2[ 0] + mul1[ 5]*mul2[ 4]
             + mul1[ 6]*mul2[ 8] + mul1[ 7]*mul2[12];
    mres[ 5] = mul1[ 4]*mul2[ 1] + mul1[ 5]*mul2[ 5]
             + mul1[ 6]*mul2[ 9] + mul1[ 7]*mul2[13];
    mres[ 6] = mul1[ 4]*mul2[ 2] + mul1[ 5]*mul2[ 6]
             + mul1[ 6]*mul2[10] + mul1[ 7]*mul2[14];
    mres[ 7] = mul1[ 4]*mul2[ 3] + mul1[ 5]*mul2[ 7]
             + mul1[ 6]*mul2[11] + mul1[ 7]*mul2[15];

    mres[ 8] = mul1[ 8]*mul2[ 0] + mul1[ 9]*mul2[ 4]
             + mul1[10]*mul2[ 8] + mul1[11]*mul2[12];
    mres[ 9] = mul1[ 8]*mul2[ 1] + mul1[ 9]*mul2[ 5]
             + mul1[10]*mul2[ 9] + mul1[11]*mul2[13];
    mres[10] = mul1[ 8]*mul2[ 2] + mul1[ 9]*mul2[ 6]
             + mul1[10]*mul2[10] + mul1[11]*mul2[14];
    mres[11] = mul1[ 8]*mul2[ 3] + mul1[ 9]*mul2[ 7]
             + mul1[10]*mul2[11] + mul1[11]*mul2[15];

    mres[12] = mul1[12]*mul2[ 0] + mul1[13]*mul2[ 4]
             + mul1[14]*mul2[ 8] + mul1[15]*mul2[12];
    mres[13] = mul1[12]*mul2[ 1] + mul1[13]*mul2[ 5]
             + mul1[14]*mul2[ 9] + mul1[15]*mul2[13];
    mres[14] = mul1[12]*mul2[ 2] + mul1[13]*mul2[ 6]
             + mul1[14]*mul2[10] + mul1[15]*mul2[14];
    mres[15] = mul1[12]*mul2[ 3] + mul1[13]*mul2[ 7]
             + mul1[14]*mul2[11] + mul1[15]*mul2[15];
}



static inline void M4Identity(TMFV load) {
    load[ 1] = load[ 2] = load[ 3] =
    load[ 4] = load[ 6] = load[ 7] =
    load[ 8] = load[ 9] = load[11] =
    load[12] = load[13] = load[14] = 0.0;
    load[ 0] = load[ 5] = load[10] = load[15] = 1.0;
}



static inline void M4Transpose(TMFV mtrn) {
    #define fswp(a, b) (ftmp = (a), (a) = (b), (b) = ftmp)
    float ftmp;

    fswp(mtrn[ 1], mtrn[ 4]);
    fswp(mtrn[ 2], mtrn[ 8]);
    fswp(mtrn[ 3], mtrn[12]);
    fswp(mtrn[ 6], mtrn[ 9]);
    fswp(mtrn[ 7], mtrn[13]);
    fswp(mtrn[11], mtrn[14]);
    #undef fswp
}



static inline void M4Frustum(TMFV mfru, float xmin, float xmax,
                             float ymin, float ymax, float znea, float zfar) {
    mfru[ 0] = 2.0 * znea / (xmax - xmin);
    mfru[ 1] = 0.0;
    mfru[ 2] = (xmax + xmin) / (xmax - xmin);
    mfru[ 3] = 0.0;

    mfru[ 4] = 0.0;
    mfru[ 5] = 2.0 * znea / (ymax - ymin);
    mfru[ 6] = (ymax + ymin) / (ymax - ymin);
    mfru[ 7] = 0.0;

    mfru[ 8] = 0.0;
    mfru[ 9] = 0.0;
    mfru[10] = (znea + zfar) / (znea - zfar);
    mfru[11] = 2.0 * znea * zfar / (znea - zfar);

    mfru[12] = 0.0;
    mfru[13] = 0.0;
    mfru[14] = -1.0;
    mfru[15] = 0.0;
}



static inline void M4Ortho(TMFV mort, float xmin, float xmax,
                           float ymin, float ymax, float znea, float zfar) {
    mort[ 0] =   2.0 / (xmax - xmin);
    mort[ 1] =   0.0;
    mort[ 2] =   0.0;
    mort[ 3] = - (xmax + xmin) / (xmax - xmin);

    mort[ 4] =   0.0;
    mort[ 5] =   2.0 / (ymax - ymin);
    mort[ 6] =   0.0;
    mort[ 7] = - (ymax + ymin) / (ymax - ymin);

    mort[ 8] =   0.0;
    mort[ 9] =   0.0;
    mort[10] =   2.0 / (znea - zfar);
    mort[11] =   (znea + zfar) / (znea - zfar);

    mort[12] =   0.0;
    mort[13] =   0.0;
    mort[14] =   0.0;
    mort[15] =   1.0;
}



static inline void M4Scale(TMFV mscl, float x, float y, float z) {
    mscl[ 0] =  x;
    mscl[ 1] = 0.0;
    mscl[ 2] = 0.0;
    mscl[ 3] = 0.0;

    mscl[ 4] = 0.0;
    mscl[ 5] =  y;
    mscl[ 6] = 0.0;
    mscl[ 7] = 0.0;

    mscl[ 8] = 0.0;
    mscl[ 9] = 0.0;
    mscl[10] =  z;
    mscl[11] = 0.0;

    mscl[12] = 0.0;
    mscl[13] = 0.0;
    mscl[14] = 0.0;
    mscl[15] = 1.0;
}



static inline void M4Translate(TMFV mscl, float x, float y, float z) {
    mscl[ 0] = 1.0;
    mscl[ 1] = 0.0;
    mscl[ 2] = 0.0;
    mscl[ 3] =  x;

    mscl[ 4] = 0.0;
    mscl[ 5] = 1.0;
    mscl[ 6] = 0.0;
    mscl[ 7] =  y;

    mscl[ 8] = 0.0;
    mscl[ 9] = 0.0;
    mscl[10] = 1.0;
    mscl[11] =  z;

    mscl[12] = 0.0;
    mscl[13] = 0.0;
    mscl[14] = 0.0;
    mscl[15] = 1.0;
}



static inline void M4RotAxis(TMFV mrot, float a, float x, float y, float z) {
    float t, s, c, d;

    if (a != 0.0 && (x != 0.0 || y != 0.0 || z != 0.0)) {
        d = 1.0 / sqrtf(x*x + y*y + z*z);
        x *= d; y *= d; z *= d;
        s = sinf(a), c = cosf(a), t = 1.0 - c;

        mrot[ 0] = x * x * t + c;
        mrot[ 1] = x * y * t - z * s;
        mrot[ 2] = x * z * t + y * s;
        mrot[ 3] = 0.0;

        mrot[ 4] = y * x * t + z * s;
        mrot[ 5] = y * y * t + c;
        mrot[ 6] = y * z * t - x * s;
        mrot[ 7] = 0.0;

        mrot[ 8] = z * x * t - y * s;
        mrot[ 9] = z * y * t + x * s;
        mrot[10] = z * z * t + c;
        mrot[11] = 0.0;

        mrot[12] = 0.0;
        mrot[13] = 0.0;
        mrot[14] = 0.0;
        mrot[15] = 1.0;
    }
    else
        M4Identity(mrot);
}



static inline void M4RotOrts(TMFV mrot, float x, float y, float z) {
    float a = cos(x), b = sin(x),
          c = cos(y), d = sin(y),
          e = cos(z), f = sin(z),
          g = a * d,   h = b * d;

    mrot[ 0] =   c * e;
    mrot[ 1] = - c * f;
    mrot[ 2] =   d;
    mrot[ 3] =   0.0;

    mrot[ 4] =   h * e + a * f;
    mrot[ 5] = - h * f + a * e;
    mrot[ 6] = - b * c;
    mrot[ 7] =   0.0;

    mrot[ 8] = - g * e + b * f;
    mrot[ 9] =   g * f + b * e;
    mrot[10] =   a * c;
    mrot[11] =   0.0;

    mrot[12] =   0.0;
    mrot[13] =   0.0;
    mrot[14] =   0.0;
    mrot[15] =   1.0;
}



static inline void PushMatrix(FMST **crst) {
    FMST *temp = (FMST*)malloc(sizeof(FMST));

    if (*crst)
        M4Duplicate(temp->curr, (*crst)->curr);
    else
        M4Identity(temp->curr);
    temp->prev = *crst;
    *crst = temp;
}



static inline void PopMatrix(FMST **crst) {
    FMST *temp = (*crst)->prev;

    free(*crst);
    *crst = temp;
}



static inline void PurgeMatrixStack(FMST **crst) {
    while (*crst)
        PopMatrix(crst);
    PushMatrix(crst);
}



static inline void V3AddC(T3FV *vres, float cadd) {
    vres->x += cadd;
    vres->y += cadd;
    vres->z += cadd;
}



static inline void V3AddV(T3FV *vres, T3FV *vadd) {
    vres->x += vadd->x;
    vres->y += vadd->y;
    vres->z += vadd->z;
}



static inline void V3SubC(T3FV *vres, float csub) {
    vres->x -= csub;
    vres->y -= csub;
    vres->z -= csub;
}



static inline void V3SubV(T3FV *vres, T3FV *vsub) {
    vres->x -= vsub->x;
    vres->y -= vsub->y;
    vres->z -= vsub->z;
}



static inline void V3MulC(T3FV *vres, float cmul) {
    vres->x *= cmul;
    vres->y *= cmul;
    vres->z *= cmul;
}



static inline void V3MulV(T3FV *vres, T3FV *vmul) {
    vres->x *= vmul->x;
    vres->y *= vmul->y;
    vres->z *= vmul->z;
}



static inline void V3DivC(T3FV *vres, float cdiv) {
    vres->x /= cdiv;
    vres->y /= cdiv;
    vres->z /= cdiv;
}



static inline void V3DivV(T3FV *vres, T3FV *vdiv) {
    vres->x /= vdiv->x;
    vres->y /= vdiv->y;
    vres->z /= vdiv->z;
}



static inline void V3CrsProd(T3FV *vec1, T3FV *vec2, T3FV *vecr) {
    vecr->x = vec1->y * vec2->z - vec1->z * vec2->y;
    vecr->y = vec1->z * vec2->x - vec1->x * vec2->z;
    vecr->z = vec1->x * vec2->y - vec1->y * vec2->x;
}



static inline void V3LInterp(T3FV *vec1, T3FV *vec2, T3FV *vecr, float intr) {
    vecr->x = vec2->x;
    vecr->y = vec2->y;
    vecr->z = vec2->z;
    V3SubV(vecr, vec1);
    V3MulC(vecr, intr);
    V3AddV(vecr, vec1);
}



static inline long V3Equ(T3FV *vec1, T3FV *vec2) {
    return (vec1->x == vec2->x && vec1->y == vec2->y && vec1->z == vec2->z);
}



static inline float V3DotProd(T3FV *vec1, T3FV *vec2) {
    return vec1->x * vec2->x + vec1->y * vec2->y + vec1->z * vec2->z;
}



static inline float V3Len(T3FV *vect) {
    return sqrtf(V3DotProd(vect, vect));
}



static inline void V3Normalize(T3FV *vect) {
    V3MulC(vect, 1.0 / V3Len(vect));
}



static inline void V3ToAng(T2FV *vang, T3FV *vect) {
    vang->x = atan2f(vect->z, vect->x);
    vang->y = asinf(vect->y / V3Len(vect));
};



static inline void V3FromAng(T3FV *vect, float angu, float angv) {
    vect->x = cos(angu) * cos(angv);
    vect->y = sin(angv);
    vect->z = sin(angu) * cos(angv);
};



static inline void M4MulVect(TMFV matx, T4FV *vect) {
    T4FV temp = *vect;

    vect->x = matx[ 0] * temp.x + matx[ 1] * temp.y
            + matx[ 2] * temp.z + matx[ 3] * temp.w;
    vect->y = matx[ 4] * temp.x + matx[ 5] * temp.y
            + matx[ 6] * temp.z + matx[ 7] * temp.w;
    vect->z = matx[ 8] * temp.x + matx[ 9] * temp.y
            + matx[10] * temp.z + matx[11] * temp.w;
    vect->w = matx[12] * temp.x + matx[13] * temp.y
            + matx[14] * temp.z + matx[15] * temp.w;
}



static void V3UnProject(T3FV *vect, TMFV matx, int view[4]) {
    TMFV minv;
    T4FV vdot;

    vdot.x = (vect->x - (float)view[0]) / (float)view[2] * 2.0 - 1.0;
    vdot.y = (vect->y - (float)view[1]) / (float)view[3] * 2.0 - 1.0;
    vdot.z = 2.0 * vect->z - 1.0;
    vdot.w = 1.0;

    M4Duplicate(minv, matx);
    M4Invert(minv);
    M4MulVect(minv, &vdot);
    if(!vdot.w)
        return;

    vect->x = vdot.x;
    vect->y = vdot.y;
    vect->z = vdot.z;
    V3MulC(vect, 1.0 / vdot.w);
}

#endif /** VEC_MATH_H **/
