/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#define NC_NAME "neocortex"
#define NC_VERSION "1.1"
#define NC_BUILDTIME __DATE__

#ifdef _WIN32
#define NC_WIN32
#define NC_PLATFORM "Windows"
#elif defined(__linux__)
#define NC_LINUX
#define NC_PLATFORM "Linux"
#elif defined(__APPLE__)
#define NC_OSX
#define NC_PLATFORM "OSX"
#else
#error "Platform not detected! Supported platforms are Windows, Linux, and OSX."
#endif
