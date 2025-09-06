////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef SFML_VECTOR2_H
#define SFML_VECTOR2_H

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.h>


////////////////////////////////////////////////////////////
/// \brief 2-component vector of floats
///
////////////////////////////////////////////////////////////
typedef struct
{
    float x;
    float y;
} sfVector2f;

sfVector2f sfVec2f_add(sfVector2f a, sfVector2f b);
sfVector2f sfVec2f_sub(sfVector2f a, sfVector2f b);
sfVector2f sfVec2f_scale(sfVector2f a, float b);
float sfVec2f_dot(sfVector2f a, sfVector2f b);
float sfVec2f_lenSquared(sfVector2f v);
float sfVec2f_len(sfVector2f v);
sfVector2f sfVec2f_norm(sfVector2f v);
sfVector2f sfVec2f_compDiv(sfVector2f a, sfVector2f b);
sfVector2f sfVec2f_compMul(sfVector2f a, sfVector2f b);
sfVector2f sfVec2f_perpCw(sfVector2f v);
sfVector2f sfVec2f_perpCcw(sfVector2f v);
float sfVec2f_cmp(sfVector2f a, sfVector2f b);
float sfVec2f_angle(sfVector2f a, sfVector2f b);


////////////////////////////////////////////////////////////
/// \brief 2-component vector of integers
///
////////////////////////////////////////////////////////////
typedef struct
{
    int x;
    int y;
} sfVector2i;

sfVector2i sfVec2i_add(sfVector2i a, sfVector2i b);
sfVector2i sfVec2i_sub(sfVector2i a, sfVector2i b);
sfVector2f sfVec2i_scale(sfVector2i a, float b);
int   sfVec2i_dot(sfVector2i a, sfVector2i b);
int   sfVec2i_lenSquared(sfVector2i v);
float sfVec2i_len(sfVector2i v);
sfVector2f sfVec2i_norm(sfVector2i v);
sfVector2f sfVec2i_compDiv(sfVector2i a, sfVector2i b);
sfVector2i sfVec2i_compDivFloor(sfVector2i a, sfVector2i b);
sfVector2i sfVec2i_compMul(sfVector2i a, sfVector2i b);
sfVector2i sfVec2i_perpCw(sfVector2i v);
sfVector2i sfVec2i_perpCcw(sfVector2i v);
int   sfVec2i_cmp(sfVector2i a, sfVector2i b);
float sfVec2i_angle(sfVector2i a, sfVector2i b);


////////////////////////////////////////////////////////////
/// \brief 2-component vector of unsigned integers
///
////////////////////////////////////////////////////////////
typedef struct
{
    unsigned int x;
    unsigned int y;
} sfVector2u;

sfVector2u sfVec2u_add(sfVector2u a, sfVector2u b);
sfVector2u sfVec2u_sub(sfVector2u a, sfVector2u b);
sfVector2f sfVec2u_scale(sfVector2u a, float b);
unsigned int sfVec2u_dot(sfVector2u a, sfVector2u b);
unsigned int sfVec2u_lenSquared(sfVector2u v);
float sfVec2u_len(sfVector2u v);
sfVector2f sfVec2u_norm(sfVector2u v);
sfVector2f sfVec2u_compDiv(sfVector2u a, sfVector2u b);
sfVector2u sfVec2u_compDivFloor(sfVector2u a, sfVector2u b);
sfVector2u sfVec2u_compMul(sfVector2u a, sfVector2u b);
sfVector2u sfVec2u_perpCw(sfVector2u v);
sfVector2u sfVec2u_perpCcw(sfVector2u v);
int   sfVec2u_cmp(sfVector2u a, sfVector2u b);
float sfVec2u_angle(sfVector2u a, sfVector2u b);


#endif // SFML_VECTOR2_H
