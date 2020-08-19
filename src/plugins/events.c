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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "events.h"

void
add_event(plugin_t *plugin, char *name, event_callback callback)
{
    plugin_event_t *event = malloc(sizeof(plugin_event_t));
    memset(event, 0, sizeof(*event));
    event->name = name;
    event->callback = callback;

    if (NULL == plugin->events) {
        plugin_event_interface_t *events = malloc(sizeof(plugin_event_interface_t));
        memset(events, 0, sizeof(*events));
        events->event = event;
        events->next = events;
        events->prev = events;
        plugin->events = events;
    } else {
        plugin_event_interface_t *last_event = plugin->events->prev;
        plugin_event_interface_t *new_event = malloc(sizeof(plugin_event_interface_t));
        memset(new_event, 0, sizeof(*new_event));
        new_event->event = event;
        last_event->next = new_event;
        last_event->next->prev = last_event;
        last_event->next->next = plugin->events;
        plugin->events->prev = new_event;
    }
}

void
plugin_call_event(plugin_t *plugin, char *name, void *param)
{
    plugin_event_interface_t *event;
    event = plugin->events;

    if (plugin->events != NULL) {
        do {
            if (NULL == event->event) {
                event = event->next;
                continue;
            }

            if (0 == strcmp(event->event->name, name)) {
                if (NULL != event->event->callback) {
                    event->event->callback(param);
                    break;
                }
            }

            event = event->next;
        } while (event != plugin->events);
    }
}
