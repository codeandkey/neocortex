#pragma once

#include <iostream>
#include <mutex>

#include "util.h"
#include "platform.h"

#ifdef PINE_WIN32
#include <Windows.h>
#elif defined PINE_LINUX || defined PINE_OSX
#include <unistd.h>
#endif

namespace pine {
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

		void set_color(ColorMode mode);
		void set_level(int level);

		int get_level();
		ColorMode get_color();

		/* Log mutex access */
		extern std::recursive_mutex log_mutex;

		template <int level = DEFAULT_LEVEL, typename ... Args>
		void write(std::string message, Args ... args) {
			std::lock_guard<std::recursive_mutex> lock(log_mutex);

			if (get_level() < level) {
				return;
			}

			/* Test if color supported */
			bool color_supported = true;

#if defined PINE_LINUX || defined PINE_OSX
			color_supported &= isatty(stderr);
#endif

			/* Write color reset */
			if (color_supported) {
#if defined PINE_WIN32
				SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), 15);
#elif defined PINE_LINUX || defined PINE_OSX
				fprintf(stderr, "\e[0;39m");
#endif
			}

			/* Write timestamp tag */
			fprintf(stderr, "%s ", pine::util::timestring().c_str());

			if (color_supported) {
#if defined PINE_WIN32
				static const int colors[] = { 12, 14, 15, 11 };
				SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), colors[level]);
#elif defined PINE_LINUX || defined PINE_OSX
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

			fprintf(stderr, "%s\xb3 ", level_strings[level]);

			/* Write content */
			fprintf(stderr, "%s", pine::util::format(message, args...).c_str());

			/* Write color reset */
			if (color_supported) {
#if defined PINE_WIN32
				SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), 15);
#elif defined PINE_LINUX || defined PINE_OSX
				fprintf(stderr, "\e[0;39m");
#endif
			}
		}
	}
}

/* Log convienience macros */
#define pine_error pine::log::write<pine::log::ERR>
#define pine_warn pine::log::write<pine::log::WARNING>
#define pine_info pine::log::write<pine::log::INFO>
#define pine_debug pine::log::write<pine::log::DEBUG>