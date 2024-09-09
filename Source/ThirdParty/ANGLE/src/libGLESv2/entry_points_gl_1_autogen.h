// GENERATED FILE - DO NOT EDIT.
// Generated by generate_entry_points.py using data from gl.xml.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_gl_1_autogen.h:
//   Defines the Desktop GL 1.x entry points.

#ifndef LIBGLESV2_ENTRY_POINTS_GL_1_AUTOGEN_H_
#define LIBGLESV2_ENTRY_POINTS_GL_1_AUTOGEN_H_

#include <export.h>
#include "angle_gl.h"

extern "C" {

// GL 1.0
ANGLE_EXPORT void GL_APIENTRY GL_Accum(GLenum op, GLfloat value);
ANGLE_EXPORT void GL_APIENTRY GL_Begin(GLenum mode);
ANGLE_EXPORT void GL_APIENTRY GL_Bitmap(GLsizei width,
                                        GLsizei height,
                                        GLfloat xorig,
                                        GLfloat yorig,
                                        GLfloat xmove,
                                        GLfloat ymove,
                                        const GLubyte *bitmap);
ANGLE_EXPORT void GL_APIENTRY GL_CallList(GLuint list);
ANGLE_EXPORT void GL_APIENTRY GL_CallLists(GLsizei n, GLenum type, const void *lists);
ANGLE_EXPORT void GL_APIENTRY GL_ClearAccum(GLfloat red,
                                            GLfloat green,
                                            GLfloat blue,
                                            GLfloat alpha);
ANGLE_EXPORT void GL_APIENTRY GL_ClearDepth(GLdouble depth);
ANGLE_EXPORT void GL_APIENTRY GL_ClearIndex(GLfloat c);
ANGLE_EXPORT void GL_APIENTRY GL_ClipPlane(GLenum plane, const GLdouble *equation);
ANGLE_EXPORT void GL_APIENTRY GL_Color3b(GLbyte red, GLbyte green, GLbyte blue);
ANGLE_EXPORT void GL_APIENTRY GL_Color3bv(const GLbyte *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color3d(GLdouble red, GLdouble green, GLdouble blue);
ANGLE_EXPORT void GL_APIENTRY GL_Color3dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color3f(GLfloat red, GLfloat green, GLfloat blue);
ANGLE_EXPORT void GL_APIENTRY GL_Color3fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color3i(GLint red, GLint green, GLint blue);
ANGLE_EXPORT void GL_APIENTRY GL_Color3iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color3s(GLshort red, GLshort green, GLshort blue);
ANGLE_EXPORT void GL_APIENTRY GL_Color3sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color3ub(GLubyte red, GLubyte green, GLubyte blue);
ANGLE_EXPORT void GL_APIENTRY GL_Color3ubv(const GLubyte *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color3ui(GLuint red, GLuint green, GLuint blue);
ANGLE_EXPORT void GL_APIENTRY GL_Color3uiv(const GLuint *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color3us(GLushort red, GLushort green, GLushort blue);
ANGLE_EXPORT void GL_APIENTRY GL_Color3usv(const GLushort *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
ANGLE_EXPORT void GL_APIENTRY GL_Color4bv(const GLbyte *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color4d(GLdouble red,
                                         GLdouble green,
                                         GLdouble blue,
                                         GLdouble alpha);
ANGLE_EXPORT void GL_APIENTRY GL_Color4dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color4fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color4i(GLint red, GLint green, GLint blue, GLint alpha);
ANGLE_EXPORT void GL_APIENTRY GL_Color4iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color4s(GLshort red, GLshort green, GLshort blue, GLshort alpha);
ANGLE_EXPORT void GL_APIENTRY GL_Color4sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color4ubv(const GLubyte *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha);
ANGLE_EXPORT void GL_APIENTRY GL_Color4uiv(const GLuint *v);
ANGLE_EXPORT void GL_APIENTRY GL_Color4us(GLushort red,
                                          GLushort green,
                                          GLushort blue,
                                          GLushort alpha);
ANGLE_EXPORT void GL_APIENTRY GL_Color4usv(const GLushort *v);
ANGLE_EXPORT void GL_APIENTRY GL_ColorMaterial(GLenum face, GLenum mode);
ANGLE_EXPORT void GL_APIENTRY
GL_CopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
ANGLE_EXPORT void GL_APIENTRY GL_DeleteLists(GLuint list, GLsizei range);
ANGLE_EXPORT void GL_APIENTRY GL_DepthRange(GLdouble n, GLdouble f);
ANGLE_EXPORT void GL_APIENTRY GL_DrawBuffer(GLenum buf);
ANGLE_EXPORT void GL_APIENTRY
GL_DrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
ANGLE_EXPORT void GL_APIENTRY GL_EdgeFlag(GLboolean flag);
ANGLE_EXPORT void GL_APIENTRY GL_EdgeFlagv(const GLboolean *flag);
ANGLE_EXPORT void GL_APIENTRY GL_End();
ANGLE_EXPORT void GL_APIENTRY GL_EndList();
ANGLE_EXPORT void GL_APIENTRY GL_EvalCoord1d(GLdouble u);
ANGLE_EXPORT void GL_APIENTRY GL_EvalCoord1dv(const GLdouble *u);
ANGLE_EXPORT void GL_APIENTRY GL_EvalCoord1f(GLfloat u);
ANGLE_EXPORT void GL_APIENTRY GL_EvalCoord1fv(const GLfloat *u);
ANGLE_EXPORT void GL_APIENTRY GL_EvalCoord2d(GLdouble u, GLdouble v);
ANGLE_EXPORT void GL_APIENTRY GL_EvalCoord2dv(const GLdouble *u);
ANGLE_EXPORT void GL_APIENTRY GL_EvalCoord2f(GLfloat u, GLfloat v);
ANGLE_EXPORT void GL_APIENTRY GL_EvalCoord2fv(const GLfloat *u);
ANGLE_EXPORT void GL_APIENTRY GL_EvalMesh1(GLenum mode, GLint i1, GLint i2);
ANGLE_EXPORT void GL_APIENTRY GL_EvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
ANGLE_EXPORT void GL_APIENTRY GL_EvalPoint1(GLint i);
ANGLE_EXPORT void GL_APIENTRY GL_EvalPoint2(GLint i, GLint j);
ANGLE_EXPORT void GL_APIENTRY GL_FeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer);
ANGLE_EXPORT void GL_APIENTRY GL_Fogi(GLenum pname, GLint param);
ANGLE_EXPORT void GL_APIENTRY GL_Fogiv(GLenum pname, const GLint *params);
ANGLE_EXPORT void GL_APIENTRY GL_Frustum(GLdouble left,
                                         GLdouble right,
                                         GLdouble bottom,
                                         GLdouble top,
                                         GLdouble zNear,
                                         GLdouble zFar);
ANGLE_EXPORT GLuint GL_APIENTRY GL_GenLists(GLsizei range);
ANGLE_EXPORT void GL_APIENTRY GL_GetClipPlane(GLenum plane, GLdouble *equation);
ANGLE_EXPORT void GL_APIENTRY GL_GetDoublev(GLenum pname, GLdouble *data);
ANGLE_EXPORT void GL_APIENTRY GL_GetLightiv(GLenum light, GLenum pname, GLint *params);
ANGLE_EXPORT void GL_APIENTRY GL_GetMapdv(GLenum target, GLenum query, GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_GetMapfv(GLenum target, GLenum query, GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_GetMapiv(GLenum target, GLenum query, GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_GetMaterialiv(GLenum face, GLenum pname, GLint *params);
ANGLE_EXPORT void GL_APIENTRY GL_GetPixelMapfv(GLenum map, GLfloat *values);
ANGLE_EXPORT void GL_APIENTRY GL_GetPixelMapuiv(GLenum map, GLuint *values);
ANGLE_EXPORT void GL_APIENTRY GL_GetPixelMapusv(GLenum map, GLushort *values);
ANGLE_EXPORT void GL_APIENTRY GL_GetPolygonStipple(GLubyte *mask);
ANGLE_EXPORT void GL_APIENTRY GL_GetTexGendv(GLenum coord, GLenum pname, GLdouble *params);
ANGLE_EXPORT void GL_APIENTRY GL_GetTexGenfv(GLenum coord, GLenum pname, GLfloat *params);
ANGLE_EXPORT void GL_APIENTRY GL_GetTexGeniv(GLenum coord, GLenum pname, GLint *params);
ANGLE_EXPORT void GL_APIENTRY
GL_GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels);
ANGLE_EXPORT void GL_APIENTRY GL_IndexMask(GLuint mask);
ANGLE_EXPORT void GL_APIENTRY GL_Indexd(GLdouble c);
ANGLE_EXPORT void GL_APIENTRY GL_Indexdv(const GLdouble *c);
ANGLE_EXPORT void GL_APIENTRY GL_Indexf(GLfloat c);
ANGLE_EXPORT void GL_APIENTRY GL_Indexfv(const GLfloat *c);
ANGLE_EXPORT void GL_APIENTRY GL_Indexi(GLint c);
ANGLE_EXPORT void GL_APIENTRY GL_Indexiv(const GLint *c);
ANGLE_EXPORT void GL_APIENTRY GL_Indexs(GLshort c);
ANGLE_EXPORT void GL_APIENTRY GL_Indexsv(const GLshort *c);
ANGLE_EXPORT void GL_APIENTRY GL_InitNames();
ANGLE_EXPORT GLboolean GL_APIENTRY GL_IsList(GLuint list);
ANGLE_EXPORT void GL_APIENTRY GL_LightModeli(GLenum pname, GLint param);
ANGLE_EXPORT void GL_APIENTRY GL_LightModeliv(GLenum pname, const GLint *params);
ANGLE_EXPORT void GL_APIENTRY GL_Lighti(GLenum light, GLenum pname, GLint param);
ANGLE_EXPORT void GL_APIENTRY GL_Lightiv(GLenum light, GLenum pname, const GLint *params);
ANGLE_EXPORT void GL_APIENTRY GL_LineStipple(GLint factor, GLushort pattern);
ANGLE_EXPORT void GL_APIENTRY GL_ListBase(GLuint base);
ANGLE_EXPORT void GL_APIENTRY GL_LoadMatrixd(const GLdouble *m);
ANGLE_EXPORT void GL_APIENTRY GL_LoadName(GLuint name);
ANGLE_EXPORT void GL_APIENTRY GL_Map1d(GLenum target,
                                       GLdouble u1,
                                       GLdouble u2,
                                       GLint stride,
                                       GLint order,
                                       const GLdouble *points);
ANGLE_EXPORT void GL_APIENTRY
GL_Map1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
ANGLE_EXPORT void GL_APIENTRY GL_Map2d(GLenum target,
                                       GLdouble u1,
                                       GLdouble u2,
                                       GLint ustride,
                                       GLint uorder,
                                       GLdouble v1,
                                       GLdouble v2,
                                       GLint vstride,
                                       GLint vorder,
                                       const GLdouble *points);
ANGLE_EXPORT void GL_APIENTRY GL_Map2f(GLenum target,
                                       GLfloat u1,
                                       GLfloat u2,
                                       GLint ustride,
                                       GLint uorder,
                                       GLfloat v1,
                                       GLfloat v2,
                                       GLint vstride,
                                       GLint vorder,
                                       const GLfloat *points);
ANGLE_EXPORT void GL_APIENTRY GL_MapGrid1d(GLint un, GLdouble u1, GLdouble u2);
ANGLE_EXPORT void GL_APIENTRY GL_MapGrid1f(GLint un, GLfloat u1, GLfloat u2);
ANGLE_EXPORT void GL_APIENTRY
GL_MapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
ANGLE_EXPORT void GL_APIENTRY
GL_MapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
ANGLE_EXPORT void GL_APIENTRY GL_Materiali(GLenum face, GLenum pname, GLint param);
ANGLE_EXPORT void GL_APIENTRY GL_Materialiv(GLenum face, GLenum pname, const GLint *params);
ANGLE_EXPORT void GL_APIENTRY GL_MultMatrixd(const GLdouble *m);
ANGLE_EXPORT void GL_APIENTRY GL_NewList(GLuint list, GLenum mode);
ANGLE_EXPORT void GL_APIENTRY GL_Normal3b(GLbyte nx, GLbyte ny, GLbyte nz);
ANGLE_EXPORT void GL_APIENTRY GL_Normal3bv(const GLbyte *v);
ANGLE_EXPORT void GL_APIENTRY GL_Normal3d(GLdouble nx, GLdouble ny, GLdouble nz);
ANGLE_EXPORT void GL_APIENTRY GL_Normal3dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_Normal3fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_Normal3i(GLint nx, GLint ny, GLint nz);
ANGLE_EXPORT void GL_APIENTRY GL_Normal3iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_Normal3s(GLshort nx, GLshort ny, GLshort nz);
ANGLE_EXPORT void GL_APIENTRY GL_Normal3sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_Ortho(GLdouble left,
                                       GLdouble right,
                                       GLdouble bottom,
                                       GLdouble top,
                                       GLdouble zNear,
                                       GLdouble zFar);
ANGLE_EXPORT void GL_APIENTRY GL_PassThrough(GLfloat token);
ANGLE_EXPORT void GL_APIENTRY GL_PixelMapfv(GLenum map, GLsizei mapsize, const GLfloat *values);
ANGLE_EXPORT void GL_APIENTRY GL_PixelMapuiv(GLenum map, GLsizei mapsize, const GLuint *values);
ANGLE_EXPORT void GL_APIENTRY GL_PixelMapusv(GLenum map, GLsizei mapsize, const GLushort *values);
ANGLE_EXPORT void GL_APIENTRY GL_PixelStoref(GLenum pname, GLfloat param);
ANGLE_EXPORT void GL_APIENTRY GL_PixelTransferf(GLenum pname, GLfloat param);
ANGLE_EXPORT void GL_APIENTRY GL_PixelTransferi(GLenum pname, GLint param);
ANGLE_EXPORT void GL_APIENTRY GL_PixelZoom(GLfloat xfactor, GLfloat yfactor);
ANGLE_EXPORT void GL_APIENTRY GL_PolygonMode(GLenum face, GLenum mode);
ANGLE_EXPORT void GL_APIENTRY GL_PolygonStipple(const GLubyte *mask);
ANGLE_EXPORT void GL_APIENTRY GL_PopAttrib();
ANGLE_EXPORT void GL_APIENTRY GL_PopName();
ANGLE_EXPORT void GL_APIENTRY GL_PushAttrib(GLbitfield mask);
ANGLE_EXPORT void GL_APIENTRY GL_PushName(GLuint name);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos2d(GLdouble x, GLdouble y);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos2dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos2f(GLfloat x, GLfloat y);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos2fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos2i(GLint x, GLint y);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos2iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos2s(GLshort x, GLshort y);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos2sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos3d(GLdouble x, GLdouble y, GLdouble z);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos3dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos3f(GLfloat x, GLfloat y, GLfloat z);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos3fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos3i(GLint x, GLint y, GLint z);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos3iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos3s(GLshort x, GLshort y, GLshort z);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos3sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos4dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos4fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos4i(GLint x, GLint y, GLint z, GLint w);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos4iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w);
ANGLE_EXPORT void GL_APIENTRY GL_RasterPos4sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_Rectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
ANGLE_EXPORT void GL_APIENTRY GL_Rectdv(const GLdouble *v1, const GLdouble *v2);
ANGLE_EXPORT void GL_APIENTRY GL_Rectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
ANGLE_EXPORT void GL_APIENTRY GL_Rectfv(const GLfloat *v1, const GLfloat *v2);
ANGLE_EXPORT void GL_APIENTRY GL_Recti(GLint x1, GLint y1, GLint x2, GLint y2);
ANGLE_EXPORT void GL_APIENTRY GL_Rectiv(const GLint *v1, const GLint *v2);
ANGLE_EXPORT void GL_APIENTRY GL_Rects(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
ANGLE_EXPORT void GL_APIENTRY GL_Rectsv(const GLshort *v1, const GLshort *v2);
ANGLE_EXPORT GLint GL_APIENTRY GL_RenderMode(GLenum mode);
ANGLE_EXPORT void GL_APIENTRY GL_Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
ANGLE_EXPORT void GL_APIENTRY GL_Scaled(GLdouble x, GLdouble y, GLdouble z);
ANGLE_EXPORT void GL_APIENTRY GL_SelectBuffer(GLsizei size, GLuint *buffer);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord1d(GLdouble s);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord1dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord1f(GLfloat s);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord1fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord1i(GLint s);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord1iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord1s(GLshort s);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord1sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord2d(GLdouble s, GLdouble t);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord2dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord2f(GLfloat s, GLfloat t);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord2fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord2i(GLint s, GLint t);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord2iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord2s(GLshort s, GLshort t);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord2sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord3d(GLdouble s, GLdouble t, GLdouble r);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord3dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord3f(GLfloat s, GLfloat t, GLfloat r);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord3fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord3i(GLint s, GLint t, GLint r);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord3iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord3s(GLshort s, GLshort t, GLshort r);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord3sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord4dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord4fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord4i(GLint s, GLint t, GLint r, GLint q);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord4iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);
ANGLE_EXPORT void GL_APIENTRY GL_TexCoord4sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_TexGend(GLenum coord, GLenum pname, GLdouble param);
ANGLE_EXPORT void GL_APIENTRY GL_TexGendv(GLenum coord, GLenum pname, const GLdouble *params);
ANGLE_EXPORT void GL_APIENTRY GL_TexGenf(GLenum coord, GLenum pname, GLfloat param);
ANGLE_EXPORT void GL_APIENTRY GL_TexGenfv(GLenum coord, GLenum pname, const GLfloat *params);
ANGLE_EXPORT void GL_APIENTRY GL_TexGeni(GLenum coord, GLenum pname, GLint param);
ANGLE_EXPORT void GL_APIENTRY GL_TexGeniv(GLenum coord, GLenum pname, const GLint *params);
ANGLE_EXPORT void GL_APIENTRY GL_TexImage1D(GLenum target,
                                            GLint level,
                                            GLint internalformat,
                                            GLsizei width,
                                            GLint border,
                                            GLenum format,
                                            GLenum type,
                                            const void *pixels);
ANGLE_EXPORT void GL_APIENTRY GL_Translated(GLdouble x, GLdouble y, GLdouble z);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex2d(GLdouble x, GLdouble y);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex2dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex2f(GLfloat x, GLfloat y);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex2fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex2i(GLint x, GLint y);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex2iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex2s(GLshort x, GLshort y);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex2sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex3d(GLdouble x, GLdouble y, GLdouble z);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex3dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex3f(GLfloat x, GLfloat y, GLfloat z);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex3fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex3i(GLint x, GLint y, GLint z);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex3iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex3s(GLshort x, GLshort y, GLshort z);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex3sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex4dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex4fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex4i(GLint x, GLint y, GLint z, GLint w);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex4iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex4s(GLshort x, GLshort y, GLshort z, GLshort w);
ANGLE_EXPORT void GL_APIENTRY GL_Vertex4sv(const GLshort *v);

// GL 1.1
ANGLE_EXPORT GLboolean GL_APIENTRY GL_AreTexturesResident(GLsizei n,
                                                          const GLuint *textures,
                                                          GLboolean *residences);
ANGLE_EXPORT void GL_APIENTRY GL_ArrayElement(GLint i);
ANGLE_EXPORT void GL_APIENTRY GL_CopyTexImage1D(GLenum target,
                                                GLint level,
                                                GLenum internalformat,
                                                GLint x,
                                                GLint y,
                                                GLsizei width,
                                                GLint border);
ANGLE_EXPORT void GL_APIENTRY
GL_CopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
ANGLE_EXPORT void GL_APIENTRY GL_EdgeFlagPointer(GLsizei stride, const void *pointer);
ANGLE_EXPORT void GL_APIENTRY GL_IndexPointer(GLenum type, GLsizei stride, const void *pointer);
ANGLE_EXPORT void GL_APIENTRY GL_Indexub(GLubyte c);
ANGLE_EXPORT void GL_APIENTRY GL_Indexubv(const GLubyte *c);
ANGLE_EXPORT void GL_APIENTRY GL_InterleavedArrays(GLenum format,
                                                   GLsizei stride,
                                                   const void *pointer);
ANGLE_EXPORT void GL_APIENTRY GL_PopClientAttrib();
ANGLE_EXPORT void GL_APIENTRY GL_PrioritizeTextures(GLsizei n,
                                                    const GLuint *textures,
                                                    const GLfloat *priorities);
ANGLE_EXPORT void GL_APIENTRY GL_PushClientAttrib(GLbitfield mask);
ANGLE_EXPORT void GL_APIENTRY GL_TexSubImage1D(GLenum target,
                                               GLint level,
                                               GLint xoffset,
                                               GLsizei width,
                                               GLenum format,
                                               GLenum type,
                                               const void *pixels);

// GL 1.2

// GL 1.3
ANGLE_EXPORT void GL_APIENTRY GL_CompressedTexImage1D(GLenum target,
                                                      GLint level,
                                                      GLenum internalformat,
                                                      GLsizei width,
                                                      GLint border,
                                                      GLsizei imageSize,
                                                      const void *data);
ANGLE_EXPORT void GL_APIENTRY GL_CompressedTexSubImage1D(GLenum target,
                                                         GLint level,
                                                         GLint xoffset,
                                                         GLsizei width,
                                                         GLenum format,
                                                         GLsizei imageSize,
                                                         const void *data);
ANGLE_EXPORT void GL_APIENTRY GL_GetCompressedTexImage(GLenum target, GLint level, void *img);
ANGLE_EXPORT void GL_APIENTRY GL_LoadTransposeMatrixd(const GLdouble *m);
ANGLE_EXPORT void GL_APIENTRY GL_LoadTransposeMatrixf(const GLfloat *m);
ANGLE_EXPORT void GL_APIENTRY GL_MultTransposeMatrixd(const GLdouble *m);
ANGLE_EXPORT void GL_APIENTRY GL_MultTransposeMatrixf(const GLfloat *m);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord1d(GLenum target, GLdouble s);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord1dv(GLenum target, const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord1f(GLenum target, GLfloat s);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord1fv(GLenum target, const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord1i(GLenum target, GLint s);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord1iv(GLenum target, const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord1s(GLenum target, GLshort s);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord1sv(GLenum target, const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord2d(GLenum target, GLdouble s, GLdouble t);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord2dv(GLenum target, const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord2f(GLenum target, GLfloat s, GLfloat t);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord2fv(GLenum target, const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord2i(GLenum target, GLint s, GLint t);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord2iv(GLenum target, const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord2s(GLenum target, GLshort s, GLshort t);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord2sv(GLenum target, const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord3dv(GLenum target, const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord3fv(GLenum target, const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord3iv(GLenum target, const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord3sv(GLenum target, const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY
GL_MultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord4dv(GLenum target, const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord4fv(GLenum target, const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord4iv(GLenum target, const GLint *v);
ANGLE_EXPORT void GL_APIENTRY
GL_MultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
ANGLE_EXPORT void GL_APIENTRY GL_MultiTexCoord4sv(GLenum target, const GLshort *v);

// GL 1.4
ANGLE_EXPORT void GL_APIENTRY GL_FogCoordPointer(GLenum type, GLsizei stride, const void *pointer);
ANGLE_EXPORT void GL_APIENTRY GL_FogCoordd(GLdouble coord);
ANGLE_EXPORT void GL_APIENTRY GL_FogCoorddv(const GLdouble *coord);
ANGLE_EXPORT void GL_APIENTRY GL_FogCoordf(GLfloat coord);
ANGLE_EXPORT void GL_APIENTRY GL_FogCoordfv(const GLfloat *coord);
ANGLE_EXPORT void GL_APIENTRY GL_MultiDrawArrays(GLenum mode,
                                                 const GLint *first,
                                                 const GLsizei *count,
                                                 GLsizei drawcount);
ANGLE_EXPORT void GL_APIENTRY GL_MultiDrawElements(GLenum mode,
                                                   const GLsizei *count,
                                                   GLenum type,
                                                   const void *const *indices,
                                                   GLsizei drawcount);
ANGLE_EXPORT void GL_APIENTRY GL_PointParameteri(GLenum pname, GLint param);
ANGLE_EXPORT void GL_APIENTRY GL_PointParameteriv(GLenum pname, const GLint *params);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3bv(const GLbyte *v);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3i(GLint red, GLint green, GLint blue);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3s(GLshort red, GLshort green, GLshort blue);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3ubv(const GLubyte *v);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3ui(GLuint red, GLuint green, GLuint blue);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3uiv(const GLuint *v);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3us(GLushort red, GLushort green, GLushort blue);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColor3usv(const GLushort *v);
ANGLE_EXPORT void GL_APIENTRY GL_SecondaryColorPointer(GLint size,
                                                       GLenum type,
                                                       GLsizei stride,
                                                       const void *pointer);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos2d(GLdouble x, GLdouble y);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos2dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos2f(GLfloat x, GLfloat y);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos2fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos2i(GLint x, GLint y);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos2iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos2s(GLshort x, GLshort y);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos2sv(const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos3d(GLdouble x, GLdouble y, GLdouble z);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos3dv(const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos3f(GLfloat x, GLfloat y, GLfloat z);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos3fv(const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos3i(GLint x, GLint y, GLint z);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos3iv(const GLint *v);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos3s(GLshort x, GLshort y, GLshort z);
ANGLE_EXPORT void GL_APIENTRY GL_WindowPos3sv(const GLshort *v);

// GL 1.5
ANGLE_EXPORT void GL_APIENTRY GL_GetBufferSubData(GLenum target,
                                                  GLintptr offset,
                                                  GLsizeiptr size,
                                                  void *data);
ANGLE_EXPORT void GL_APIENTRY GL_GetQueryObjectiv(GLuint id, GLenum pname, GLint *params);
ANGLE_EXPORT void *GL_APIENTRY GL_MapBuffer(GLenum target, GLenum access);
}  // extern "C"

#endif  // LIBGLESV2_ENTRY_POINTS_GL_1_AUTOGEN_H_
