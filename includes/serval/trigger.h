/*
 Copyright (C) 2015 Serval Project Inc.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __SERVAL_DNA__TRIGGER_H
#define __SERVAL_DNA__TRIGGER_H

#include "section.h"

/* Macros for triggers
 */

#define DECLARE_TRIGGER(TRIG, ...) \
    typedef void TRIGGER_FUNC_##TRIG (__VA_ARGS__); \
    DECLARE_SECTION(TRIGGER_FUNC_##TRIG *, tr_##TRIG)

// Don't include the trailing ";" in the macro, the caller has to supply it.
// Otherwise etags(1) gets confused and omits any function definition that
// immediately follows a DEFINE_TRIGGER() line.
#define DEFINE_TRIGGER(TRIG, FUNC) \
    TRIGGER_FUNC_##TRIG * __trigger_##FUNC IN_SECTION(tr_##TRIG) = FUNC

#define CALL_TRIGGER(TRIG, ...) \
    do { \
        TRIGGER_FUNC_##TRIG **__trig; \
        for (__trig = SECTION_START(tr_##TRIG); __trig < SECTION_END(tr_##TRIG); ++__trig) \
            (**__trig)(__VA_ARGS__); \
    } while (0)

#endif // __SERVAL_DNA__TRIGGER_H
