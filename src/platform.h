/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#define NEOCORTEX_NAME "neocortex"
#define NEOCORTEX_VERSION "1.0"
#define NEOCORTEX_BUILDTIME __DATE__

#ifdef _WIN32
#define NEOCORTEX_WIN32
#define NEOCORTEX_PLATFORM "Windows"
#include <Windows.h>
#elif defined(__linux__)
#define NEOCORTEX_LINUX
#define NEOCORTEX_PLATFORM "Linux"
#elif defined(__APPLE__)
#define NEOCORTEX_OSX
#define NEOCORTEX_PLATFORM "OSX"
#else
#error "Platform not detected! Supported platforms are Windows, Linux, and OSX."
#endif

#ifndef NDEBUG
#define NEOCORTEX_DEBUG
#define NEOCORTEX_DEBUG_STR "DEBUG"
#else
#define NEOCORTEX_DEBUG_STR ""
#endif
