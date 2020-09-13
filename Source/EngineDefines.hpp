/*
*  EngineDefines.hpp
*  Copyright (C) 2020 by Maxim Stoyanov
*  scapior.github.io
*/

#pragma once

/** \file global.hpp
\brief The file with often user functions or global macros.
The file contains
*/

/*! \def ENGINE_BUILD_SERVER
\brief A macro that be defined when target build is a server.
If you need to build a server define this macro.
If this macro not defined, will be defined ENGINE_BUILD_CLIENT.
*/

/*! \def ENGINE_BUILD_CLIENT
\brief A macro that be defined when target build is a client.
Details.
*/

/*! \fn int open(const char *pathname,int flags)
\brief Opens a file descriptor.
\param pathname The name of the descriptor.
\param flags Opening flags.
*/

/* Build mode */
#ifndef ENGINE_BUILD_SERVER
    #define ENGINE_BUILD_CLIENT
#endif

#if defined(_DEBUG) || !defined(NDEBUF)
    #define ENGINE_DEBUG
#endif

/* Platform and OS macros */

#if defined(_WIN32) || defined(__WIN32__)
    #define ENGINE_OS_WINDOWS
    #if defined(_WIN64) || defined(WIN64)
        #define ENGINE_PLATFORM_x64
        #define ENGINE_OS_WINDOWS_x64
    #else
        #define ENGINE_PLATFORM_x86
        #define ENGINE_OS_WINDOWS_x32
    #endif
#endif

#if defined(__linux__)
    #define ENGINE_OS_LINUX
    #if defined(__x86_64__)
        #define ENGINE_PLATFORM_x64
        #define ENGINE_OS_LINUX_x64
    #elif defined(__i386__)
        #define ENGINE_PLATFORM_x86
        #define ENGINE_OS_LINUX_x32
    #endif
#endif
