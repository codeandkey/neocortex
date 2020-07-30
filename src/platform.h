/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#define PINE_NAME "pine"
#define PINE_VERSION "0.1"
#define PINE_BUILDTIME __DATE__

#ifdef _WIN32
#define PINE_WIN32
#define PINE_PLATFORM "Windows"
#elif defined(__linux__)
#define PINE_LINUX
#define PINE_PLATFORM "Linux"
#elif defined(__APPLE__)
#define PINE_OSX
#define PINE_PLATFORM "OSX"
#else
#error "Platform not detected! Supported platforms are Windows, Linux, and OSX."
#endif

#ifndef NDEBUG
#define PINE_DEBUG
#endif