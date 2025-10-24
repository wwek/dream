/*
--------------------------------------------------------------------------------
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
Boston, MA  02110-1301, USA.
--------------------------------------------------------------------------------
*/

// Copyright (c) 2021 John Seamons, ZL4VO/KF6VO

#pragma once

#include "sanitizer.h"


#define DRM_MAX_RX 4


#ifdef USE_ASAN
    #warning SHMEM_DISABLE_ALL due to USE_ASAN
    #define SHMEM_DISABLE_ALL
#endif

#ifdef NO_SHMEM
    #warning SHMEM_DISABLE_ALL due to NO_SHMEM
    #define SHMEM_DISABLE_ALL
#endif

//#define SHMEM_DISABLE_ALL
#ifdef SHMEM_DISABLE_ALL
    #if !defined(USE_ASAN) && !defined(NO_SHMEM)
        #warning dont forget to remove SHMEM_DISABLE_ALL
    #endif
    #define DRM_SHMEM_DISABLE
    #define RX_SHMEM_DISABLE
    #define WSPR_SHMEM_DISABLE
    #define SPI_SHMEM_DISABLE
    #define WF_SHMEM_DISABLE
#endif

#define SHMEM_CONFIG_H_INCLUDED
