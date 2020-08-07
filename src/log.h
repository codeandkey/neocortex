/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <iostream>
#include <mutex>

#include "util.h"
#include "platform.h"

#ifdef NEOCORTEX_WIN32
#include <Windows.h>
#elif defined NEOCORTEX_LINUX || defined NEOCORTEX_OSX
#include <unistd.h>
#endif

namespace neocortex {
	namespace log {
		enum class ColorMode {
			ALWAYS,
			IF_TTY,
			NEVER,
		};

		constexpr int ERR = 0;
		constexpr int WARNING = 1;
		constexpr int INFO = 2;
		constexpr int DEBUG = 3;
		constexpr int DEFAULT_LEVEL = INFO;

		constexpr ColorMode DEFAULT_COLORMODE = ColorMode::IF_TTY;

		/**
		 * Set the log color mode.
		 *
		 * @param mode Color mode to set.
		 */
		void set_color(ColorMode mode);

		/**
		 * Set the log verbosity level.
		 *
		 * @param level Maximum level to show.
		 */
		void set_level(int level);

		/**
		 * Gets the current log verbosity level.
		 *
		 * @return Log verbosity level.
		 */
		int get_level();

		/**
		 * Gets the current log color mode.
		 *
		 * @return Current color mode.
		 */
		ColorMode get_color();

		/* Log mutex access */
		extern std::recursive_mutex log_mutex;

		/**
		 * Writes a log message to the output if it should be displayed.
		 * The log format and arguments should be formatted for printf().
		 *
		 * @param message Log format.
		 * @param args Arguments to log.
		 */
		template <int level = DEFAULT_LEVEL, typename ... Args>
		void write(std::string message, Args ... args) {
			std::lock_guard<std::recursive_mutex> lock(log_mutex);

			if (get_level() < level) {
				return;
			}

			/* Test if color supported */
			bool color_supported = true;

#if defined NEOCORTEX_LINUX || defined NEOCORTEX_OSX
			color_supported &= isatty(fileno(stderr));
#endif

			/* Write color reset */
			if (color_supported) {
#if defined NEOCORTEX_WIN32
				SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), 15);
#elif defined NEOCORTEX_LINUX || defined NEOCORTEX_OSX
				fprintf(stderr, "\e[0;39m");
#endif
			}

			/* Write timestamp tag */
			fprintf(stderr, "%s ", neocortex::util::timestring().c_str());

			if (color_supported) {
#if defined NEOCORTEX_WIN32
				static const int colors[] = { 12, 14, 15, 11 };
				SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), colors[level]);
#elif defined NEOCORTEX_LINUX || defined NEOCORTEX_OSX
				static const char* colors[] = {
					"\e[0;31m",
					"\e[0;33m",
					"\e[0;37m",
					"\e[0;36m",
				};

				fprintf(stderr, colors[level]);
#endif
			}

			/* Write level tag */
			static const char* level_strings[] = {
				"ERROR   ",
				"WARNING ",
				"INFO    ",
				"DEBUG   ",
			};

			fprintf(stderr, "%s > ", level_strings[level]);

			/* Write content */
			std::string content = neocortex::util::format(message, args...);
			fprintf(stderr, "%s", content.c_str());

			/* Write color reset */
			if (color_supported) {
#if defined NEOCORTEX_WIN32
				SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), 15);
#elif defined NEOCORTEX_LINUX || defined NEOCORTEX_OSX
				fprintf(stderr, "\e[0;39m");
#endif
			}
		}
	}
}

/* Log convienience macros */
#define neocortex_error neocortex::log::write<neocortex::log::ERR>
#define neocortex_warn neocortex::log::write<neocortex::log::WARNING>
#define neocortex_info neocortex::log::write<neocortex::log::INFO>
#define neocortex_debug neocortex::log::write<neocortex::log::DEBUG>
