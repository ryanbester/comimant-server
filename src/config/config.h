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

/**
 * @file
 * @brief Config.h
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <limits.h>
#include "config_util.h"

/**
 * Configuration section.
 */
typedef struct config_section {
    const char *name; /**< The section name */

    int
    (*load_options)(config_file_t *config_file_t); /**< Pointer to the load options function */
    struct config_section *prev; /**< Pointer to previous section in list */
    struct config_section *next; /**< Pointer to next section in list */
} config_section_t;

//config_section_t *config_sections;

/**
 * Default PID file path
 */
#define DEFAULT_PID_FILE "/var/run/comimant.pid"

/**
 * Reads the raw data from the configuration file. The @ref config_file_t::data "data" field of the configuration file structure will be set
 * accordingly.
 * @param[in,out] config The configuration file structure.
 * @return 0 on success or -1 on error.
 */
int
read_config(config_file_t *config);

/**
 * Parses the raw data from the configuration file. The @ref config_file_t::data "data" field of the configuration file structure must be set.
 * The @ref config_file_t::config_json "config_json" field will contain the parsed JSON object.
 * @param[in,out] config The configuration file structure.
 * @return 0 on success or -1 on error.
 */
int
parse_config(config_file_t *config);

/**
 * Load config sections into array.
 */
void
init_default_config_sections();

/**
 * Load config sections added by plugins.
 */
void
init_plugin_config_sections();

/**
 * Adds a configuration section to the linked list.
 * @param section[in] The configuration section.
 */
void
add_config_section(config_section_t *section);

/**
 * Adds a configuration section to the linked list.
 * @param name The name of the section.
 * @param load_options[in] Callback function to load the config options.
 */
void
add_config_section_with_name(const char *name, int(*load_options)(config_file_t *config_file));

/**
 * Loads the config for each section. init_default_config_sections() should be called before this function.
 * @param config_file[in] The @ref config_file_t structure.
 * @return 0 on success or -1 on error.
 */
int
load_config_sections(config_file_t *config_file);


/**
 * Gets the PID file path from the configuration file.
 * @param[in] config The configuration object.
 * @param[out] buffer A buffer to store the file path. This must have capacity for the path.
 * @return 0 on success or -1 on error.
 */
int
get_pid_file(config_file_t *config, char *buffer);

/**
 * Frees the allocated memory.
 * @param[in] config The configuration object.
 */
void
free_config(config_file_t *config);

#endif //_CONFIG_H_
