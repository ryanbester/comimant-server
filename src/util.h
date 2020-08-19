/*
 * Comimant Server
 * Copyright (C) 2019 - 2020 Ryan Bester
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#if defined _WIN32 || defined __CYGWIN__
#ifdef BUILDING_DLL
#ifdef __GNUC__
#define SO_PUBLIC __attribute__ ((dllexport))
#else
#define SO_PUBLIC __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define SO_PUBLIC __attribute__ ((dllimport))
#else
#define SO_PUBLIC __declspec(dllimport)
#endif
#endif
#define SO_LOCAL
#else
#if __GNUC__ >= 4
#define SO_PUBLIC __attribute__ ((visibility("default")))
#define SO_LOCAL __attribute__ ((visibility("hidden")))
#else
#define SO_PUBLIC
#define SO_LOCAL
#endif
#endif

#endif //_UTIL_H_
