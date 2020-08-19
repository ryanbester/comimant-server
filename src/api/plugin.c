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

#include "plugin.h"
#include "../plugins/plugins.h"
#include "../plugins/events.h"

SO_PUBLIC void
set_plugin_details(plugin_t *plugin, plugin_details_t *plugin_details)
{
    // Copy to stop dangling pointers
    plugin_details_t *new_details = malloc(sizeof(plugin_details_t));
    *new_details = *plugin_details;
    plugin->details = new_details;
}

SO_PUBLIC void
register_event(plugin_t *plugin, char *name, event_callback callback)
{
    add_event(plugin, name, callback);
}
