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

#ifndef _PLUGINS_H_
#define _PLUGINS_H_

#include <limits.h>

/**
 * Plugin details structure.
 */
typedef struct plugin_details {
    char *name;
    char *version;
    char *description;
    char *author;
    char *website;
    char *target_comimant_version;
    char *min_comimant_version;
    // Other parameters
} plugin_details_t;

typedef int(*event_callback)(void *);

/**
 * Plugin event structure.
 */
typedef struct plugin_event {
    char *name;
    event_callback callback;
    int priority;
} plugin_event_t;

// TODO: This will be modified to a faster approach. Linked lists are inefficient for iteration whenever an event is fired.
typedef struct plugin_event_interface {
    struct plugin_event *event;

    struct plugin_event_interface *prev;
    struct plugin_event_interface *next;
} plugin_event_interface_t;

/**
 * Plugin structure.
 */
typedef struct plugin {
    struct plugin_details *details; /**< The plugin details */
    struct plugin_event_interface *events; /**< Events array. Expands as more events are added */

    int
    (*on_activate)(void);

    int
    (*on_deactivate)(void);
} plugin_t;

/**
 * Plugins structure.
 */
typedef struct internal_plugin {
    char file_name[PATH_MAX];
    void *dl_handle;
    int valid_plugin;
    int activated;
    struct plugin *plugin;

    struct internal_plugin *prev;
    struct internal_plugin *next;
} internal_plugin_t;

/**
 * Add a plugin to the plugins list.
 * @param plugin
 */
void
add_plugin(internal_plugin_t *plugin);

void
get_plugins();

/**
 * Free the plugins list from memory.
 */
void
free_plugins();

/**
 * Load plugins found in the plugins directory into the plugins list.
 * @param plugins_dir
 * @return 0 on success or -1 on error.
 */
int
load_plugins_dir(const char *plugins_dir);

/**
 * Loads all plugins in the plugins list, and sets the DL handle. valid_plugin will be set to 0 if the plugin is invalid,
 * and will be ignored for the rest of the program's execution.
 * @return 0 on success or -1 on error.
 */
int
load_plugins();

/**
 * Registers all plugins in the plugins list, that have a not-null DL handle, and are marked as valid.
 * @return 0 on success or -1 on error.
 */
int
register_plugins();

/**
 * Activates all plugins in the plugins list, that are marked as valid.
 * @return 0 on success or -1 on error.
 */
int
activate_plugins();

/**
 * Activates all plugins in the plugins list, that are marked as activated. This can be used to free memory.
 * @return 0 on success or -1 on error.
 */
int
deactivate_plugins();

/**
 * Initialises the plugin details structure with default values.
 * @param plugin The plugin.
 */
void
init_plugin_details(plugin_t *plugin);

/**
 * Calls the callback registered for the event for all plugins.
 * @param name The event name.
 */
void
call_event(char *name, void *param);


#endif //_PLUGINS_H_
