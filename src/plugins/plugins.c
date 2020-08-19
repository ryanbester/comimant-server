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
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>

#include "plugins.h"
#include "events.h"

static internal_plugin_t *plugins = NULL;

void
add_plugin(internal_plugin_t *plugin)
{
    if (NULL == plugins) {
        plugins = malloc(sizeof(internal_plugin_t));
        *plugins = *plugin;
        plugins->next = plugins;
        plugins->prev = plugins;
    } else {
        internal_plugin_t *last_plugin = plugins->prev;
        internal_plugin_t *new_plugin = malloc(sizeof(internal_plugin_t));
        *new_plugin = *plugin;
        last_plugin->next = new_plugin;
        last_plugin->next->prev = last_plugin;
        last_plugin->next->next = plugins;
        plugins->prev = new_plugin;
    }
}

void
get_plugins()
{
    internal_plugin_t *plugin;
    plugin = plugins;

    if (plugins != NULL) {
        do {
            printf("%s\n", plugin->file_name);
            plugin = plugin->next;
        } while (plugin != plugins);
    }
}

void
free_plugins()
{
    internal_plugin_t *plugin;
    plugin = plugins;

    if (plugins != NULL) {
        do {
            if (NULL == plugin->dl_handle) {
                plugin = plugin->next;
                continue;
            }

            dlclose(plugin->dl_handle);

            plugin = plugin->next;
        } while (plugin != plugins);
    }
}

int
load_plugins_dir(const char *plugins_dir)
{
    printf("Loading plugins in directory: %s\n", plugins_dir);

    struct dirent **namelist;
    int n;

    n = scandir(plugins_dir, &namelist, NULL, alphasort);
    if (-1 == n) {
        fprintf(stderr, "Error reading plugins directory.\n");
        return -1;
    }

    while (n--) {
        if (namelist[n]->d_type == DT_REG || namelist[n] == DT_UNKNOWN) {
            internal_plugin_t plugin;
            memset(&plugin, 0, sizeof(plugin));
            strcpy(plugin.file_name, plugins_dir);
            strcat(plugin.file_name, "/");
            strcat(plugin.file_name, namelist[n]->d_name);
            add_plugin(&plugin);

            free(namelist[n]);
        }
    }
    free(namelist);
    return 0;
}

int
load_plugins()
{
    internal_plugin_t *plugin;
    plugin = plugins;

    if (plugins != NULL) {
        do {
            plugin->dl_handle = dlopen(plugin->file_name, RTLD_NOW);
            if (!plugin->dl_handle) {
                fprintf(stderr, "Error loading plugin: %s: %s\n", plugin->file_name, dlerror());
                plugin->valid_plugin = 0;
                plugin = plugin->next;
                continue;
            }

            dlerror(); // Clear errors

            plugin->valid_plugin = 1;
            plugin = plugin->next;
        } while (plugin != plugins);
    }

    return 0;
}

int
register_plugins()
{
    internal_plugin_t *internal_plugin;
    internal_plugin = plugins;

    if (plugins != NULL) {
        do {
            if (NULL == internal_plugin->dl_handle || 0 == internal_plugin->valid_plugin) {
                internal_plugin = internal_plugin->next;
                continue;
            }

            int
            (*plugin_main)(plugin_t *);
            plugin_main = (int (*)(plugin_t *)) dlsym(internal_plugin->dl_handle, "plugin_main");

            char *error = dlerror();
            if (NULL != error) {
                internal_plugin->valid_plugin = 0;
                fprintf(stderr, "Error loading plugin: %s: plugin does not have plugin_main function.\n",
                        internal_plugin->file_name);
            }

            // Create plugin object
            plugin_t *plugin = malloc(sizeof(plugin_t));
            memset(plugin, 0, sizeof(*plugin));

            internal_plugin->plugin = plugin;
            init_plugin_details(plugin);

            if (-1 == (*plugin_main)(plugin)) {
                internal_plugin->valid_plugin = 0;
                fprintf(stderr, "Registering plugin: %s returned -1.", internal_plugin->file_name);
            }

            internal_plugin->valid_plugin = 1;
            internal_plugin = internal_plugin->next;
        } while (internal_plugin != plugins);
    }

    return 0;
}

int
activate_plugins()
{
    internal_plugin_t *internal_plugin;
    internal_plugin = plugins;

    if (plugins != NULL) {
        do {
            if (0 == internal_plugin->valid_plugin) {
                internal_plugin->activated = 0;
                internal_plugin = internal_plugin->next;
                continue;
            }

            if (NULL != internal_plugin->plugin->on_activate) {
                if (-1 == internal_plugin->plugin->on_activate()) {
                    internal_plugin->activated = 0;
                    fprintf(stderr, "Error activating plugin: %s\n", internal_plugin->plugin->details->name);
                    internal_plugin = internal_plugin->next;
                    continue;
                }
            }

            internal_plugin->activated = 1;
            printf("Activated plugin: %s\n", internal_plugin->plugin->details->name);

            internal_plugin = internal_plugin->next;
        } while (internal_plugin != plugins);
    }

    return 0;
}

int
deactivate_plugins()
{
    internal_plugin_t *internal_plugin;
    internal_plugin = plugins;

    if (plugins != NULL) {
        do {
            if (0 == internal_plugin->activated) {
                if (NULL != internal_plugin->dl_handle) {
                    dlclose(internal_plugin->dl_handle);
                }
                internal_plugin = internal_plugin->next;
                continue;
            }

            if (NULL != internal_plugin->plugin->on_deactivate) {
                if (-1 == internal_plugin->plugin->on_deactivate()) {
                    internal_plugin->activated = 0;
                    fprintf(stderr, "Error deactivating plugin: %s\n", internal_plugin->plugin->details->name);

                    dlclose(internal_plugin->dl_handle);
                    internal_plugin = internal_plugin->next;
                    continue;
                }
            }

            internal_plugin->activated = 0;
            printf("Deactivated plugin: %s\n", internal_plugin->plugin->details->name);

            dlclose(internal_plugin->dl_handle);

            internal_plugin = internal_plugin->next;
        } while (internal_plugin != plugins);
    }

    return 0;
}

void
init_plugin_details(plugin_t *plugin)
{
    // Set default values
}

// TODO: Implement event priorities.
void
call_event(char *name, void *param)
{
    internal_plugin_t *internal_plugin;
    internal_plugin = plugins;

    if (plugins != NULL) {
        do {
            if (0 == internal_plugin->activated) {
                internal_plugin = internal_plugin->next;
                continue;
            }

            if (NULL != internal_plugin->plugin) {
                plugin_call_event(internal_plugin->plugin, name, param);
            }

            internal_plugin = internal_plugin->next;
        } while (internal_plugin != plugins);
    }
}
