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

#ifndef _EVENTS_H_
#define _EVENTS_H_

#include "plugins.h"

// TODO: These functions will be modified when a new event storage system is in place.

/**
 * Registers an event for a plugin.
 * @param plugin The plugin.
 * @param name The event name.
 * @param callback The callback.
 */
void
add_event(plugin_t *plugin, char *name, event_callback callback);

/**
 * Calls the callback registered for an event.
 * @param plugin The plugin.
 * @param name The event name.
 */
void
plugin_call_event(plugin_t *plugin, char *name, void *param);

#endif //_EVENTS_H_
