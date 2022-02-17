// GENERATED FILE - DO NOT EDIT.
// Generated by generate_entry_points.py using data from gl.xml.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Context_gl_1_3_autogen.h: Creates a macro for interfaces in Context.

#ifndef ANGLE_CONTEXT_GL_1_3_AUTOGEN_H_
#define ANGLE_CONTEXT_GL_1_3_AUTOGEN_H_

#define ANGLE_GL_1_3_CONTEXT_API                                                                \
    void compressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, \
                              GLint border, GLsizei imageSize, const void *data);               \
    void compressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width,      \
                                 GLenum format, GLsizei imageSize, const void *data);           \
    void getCompressedTexImage(GLenum target, GLint level, void *img);                          \
    void loadTransposeMatrixd(const GLdouble *m);                                               \
    void loadTransposeMatrixf(const GLfloat *m);                                                \
    void multTransposeMatrixd(const GLdouble *m);                                               \
    void multTransposeMatrixf(const GLfloat *m);                                                \
    void multiTexCoord1d(GLenum target, GLdouble s);                                            \
    void multiTexCoord1dv(GLenum target, const GLdouble *v);                                    \
    void multiTexCoord1f(GLenum target, GLfloat s);                                             \
    void multiTexCoord1fv(GLenum target, const GLfloat *v);                                     \
    void multiTexCoord1i(GLenum target, GLint s);                                               \
    void multiTexCoord1iv(GLenum target, const GLint *v);                                       \
    void multiTexCoord1s(GLenum target, GLshort s);                                             \
    void multiTexCoord1sv(GLenum target, const GLshort *v);                                     \
    void multiTexCoord2d(GLenum target, GLdouble s, GLdouble t);                                \
    void multiTexCoord2dv(GLenum target, const GLdouble *v);                                    \
    void multiTexCoord2f(GLenum target, GLfloat s, GLfloat t);                                  \
    void multiTexCoord2fv(GLenum target, const GLfloat *v);                                     \
    void multiTexCoord2i(GLenum target, GLint s, GLint t);                                      \
    void multiTexCoord2iv(GLenum target, const GLint *v);                                       \
    void multiTexCoord2s(GLenum target, GLshort s, GLshort t);                                  \
    void multiTexCoord2sv(GLenum target, const GLshort *v);                                     \
    void multiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r);                    \
    void multiTexCoord3dv(GLenum target, const GLdouble *v);                                    \
    void multiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r);                       \
    void multiTexCoord3fv(GLenum target, const GLfloat *v);                                     \
    void multiTexCoord3i(GLenum target, GLint s, GLint t, GLint r);                             \
    void multiTexCoord3iv(GLenum target, const GLint *v);                                       \
    void multiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r);                       \
    void multiTexCoord3sv(GLenum target, const GLshort *v);                                     \
    void multiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);        \
    void multiTexCoord4dv(GLenum target, const GLdouble *v);                                    \
    void multiTexCoord4fv(GLenum target, const GLfloat *v);                                     \
    void multiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q);                    \
    void multiTexCoord4iv(GLenum target, const GLint *v);                                       \
    void multiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);            \
    void multiTexCoord4sv(GLenum target, const GLshort *v);

#endif  // ANGLE_CONTEXT_API_1_3_AUTOGEN_H_
