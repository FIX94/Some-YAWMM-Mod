// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// Copyright (C) 2012	damysteryman


#ifndef _IOSPATCH_H
#define _IOSPATCH_H

#ifdef __cplusplus
extern "C" {
#endif
/* __cplusplus */

#include <gccore.h>

#define AHBPROT_DISABLED ((*(vu32*)0xcd800064 == 0xFFFFFFFF) ? 1 : 0)

u32 IOSPATCH_AHBPROT();
u32 IOSPATCH_Apply();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _IOSPATCH_H */
