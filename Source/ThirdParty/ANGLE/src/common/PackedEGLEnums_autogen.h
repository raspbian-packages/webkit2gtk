// GENERATED FILE - DO NOT EDIT.
// Generated by gen_packed_gl_enums.py using data from packed_egl_enums.json.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PackedEGLEnums_autogen.h:
//   Declares ANGLE-specific enums classes for EGLenums and functions operating
//   on them.

#ifndef COMMON_PACKEDEGLENUMS_AUTOGEN_H_
#define COMMON_PACKEDEGLENUMS_AUTOGEN_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <angle_gl.h>

#include <cstdint>
#include <ostream>

namespace egl
{

template <typename Enum>
Enum FromEGLenum(EGLenum from);

enum class CompositorTiming : uint8_t
{
    CompositeDeadline        = 0,
    CompositInterval         = 1,
    CompositToPresentLatency = 2,

    InvalidEnum = 3,
    EnumCount   = 3,
};

template <>
CompositorTiming FromEGLenum<CompositorTiming>(EGLenum from);
EGLenum ToEGLenum(CompositorTiming from);
std::ostream &operator<<(std::ostream &os, CompositorTiming value);

enum class ContextPriority : uint8_t
{
    Low    = 0,
    Medium = 1,
    High   = 2,

    InvalidEnum = 3,
    EnumCount   = 3,
};

template <>
ContextPriority FromEGLenum<ContextPriority>(EGLenum from);
EGLenum ToEGLenum(ContextPriority from);
std::ostream &operator<<(std::ostream &os, ContextPriority value);

enum class MessageType : uint8_t
{
    Critical = 0,
    Error    = 1,
    Warn     = 2,
    Info     = 3,

    InvalidEnum = 4,
    EnumCount   = 4,
};

template <>
MessageType FromEGLenum<MessageType>(EGLenum from);
EGLenum ToEGLenum(MessageType from);
std::ostream &operator<<(std::ostream &os, MessageType value);

enum class ObjectType : uint8_t
{
    Thread  = 0,
    Display = 1,
    Context = 2,
    Surface = 3,
    Image   = 4,
    Sync    = 5,
    Stream  = 6,

    InvalidEnum = 7,
    EnumCount   = 7,
};

template <>
ObjectType FromEGLenum<ObjectType>(EGLenum from);
EGLenum ToEGLenum(ObjectType from);
std::ostream &operator<<(std::ostream &os, ObjectType value);

enum class TextureFormat : uint8_t
{
    NoTexture = 0,
    RGB       = 1,
    RGBA      = 2,

    InvalidEnum = 3,
    EnumCount   = 3,
};

template <>
TextureFormat FromEGLenum<TextureFormat>(EGLenum from);
EGLenum ToEGLenum(TextureFormat from);
std::ostream &operator<<(std::ostream &os, TextureFormat value);

enum class Timestamp : uint8_t
{
    RequestedPresentTime            = 0,
    RenderingCompleteTime           = 1,
    CompositionLatchTime            = 2,
    FirstCompositionStartTime       = 3,
    LastCompositionStartTime        = 4,
    FirstCompositionGPUFinishedTime = 5,
    DisplayPresentTime              = 6,
    DequeueReadyTime                = 7,
    ReadsDoneTime                   = 8,

    InvalidEnum = 9,
    EnumCount   = 9,
};

template <>
Timestamp FromEGLenum<Timestamp>(EGLenum from);
EGLenum ToEGLenum(Timestamp from);
std::ostream &operator<<(std::ostream &os, Timestamp value);

}  // namespace egl

#endif  // COMMON_PACKEDEGLENUMS_AUTOGEN_H_
