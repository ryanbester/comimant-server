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
#include <string.h>
#include <errno.h>

#include "config.h"
#include "../../include/cjson/cJSON.h"

#include "config_log.h"
#include "config_plugins.h"
#include "config_ssl.h"
#include "config_listen.h"

static config_section_t *config_sections = NULL;

int
read_config(config_file_t *config)
{
    FILE *fp = fopen(config->file_name, "r");

    if (NULL == fp) {
        fprintf(stderr, "[Config] Failed to open config file: %s: %s\n", config->file_name, strerror(errno));
        return 0;
    }

    // Get file size
    fseek(fp, 0L, SEEK_END);
    size_t file_size = ftell(fp);
    rewind(fp);

    config->data = calloc(1, file_size + 1);
    if (!config->data) {
        fclose(fp);
        fprintf(stderr, "[Config] Memory allocation failed\n");
        return -1;
    }

    if (1 != fread(config->data, file_size, 1, fp)) {
        fclose(fp);
        free(config->data);
        fprintf(stderr, "[Config] Failed to read config file: %s\n", config->file_name);
        return -1;
    }

    return 0;
}

int
parse_config(config_file_t *config)
{
    if (NULL == config->data) {
        return -1;
    }

    config->config_json = cJSON_Parse(config->data);
    if (NULL == config->config_json) {
        const char *err_ptr = cJSON_GetErrorPtr();
        if (NULL != err_ptr) {
            fprintf(stderr, "[Config] Malformed JSON before: %.*s\n", 25, err_ptr);
        }

        cJSON_Delete(config->config_json);
        return -1;
    }

    return 0;
}

void
init_default_config_sections()
{
    add_config_section_with_name("log", load_log_options);
    add_config_section_with_name("plugins", load_plugins_options);
    add_config_section_with_name("ssl", load_ssl_options);
    add_config_section_with_name("listen", load_listen_options);
}

void
init_plugin_config_sections()
{
    // TODO: Call function on plugin to register config sections.
}

void
add_config_section(config_section_t *section)
{
    if (NULL == config_sections) {
        config_sections = malloc(sizeof(config_section_t));
        *config_sections = *section;
        config_sections->next = config_sections;
        config_sections->prev = config_sections;
    } else {
        config_section_t *last_section = config_sections->prev;
        config_section_t *new_section = malloc(sizeof(config_section_t));
        *new_section = *section;
        last_section->next = new_section;
        last_section->next->prev = last_section;
        last_section->next->next = config_sections;
        config_sections->prev = new_section;
    }
}

void
add_config_section_with_name(const char *name, int(*load_options)(config_file_t *))
{
    config_section_t section;
    memset(&section, 0, sizeof(section));
    section.name = name;
    section.load_options = load_options;

    add_config_section(&section);
}

int
load_config_sections(config_file_t *config_file)
{
    config_section_t *section;
    section = config_sections;

    if (config_sections != NULL) {
        do {
            if (NULL != section->load_options) {
                section->load_options(config_file);
            }
            section = section->next;
        } while (section != config_sections);
    }

    return 0;
}

int
get_pid_file(config_file_t *config, char *pid_file)
{
    if (NULL == config->config_json) {
        return -1;
    }

    // Initiate temporary variables
    const cJSON *pid_file_obj = NULL;

    pid_file_obj = cJSON_GetObjectItemCaseSensitive(config->config_json, "pid_file");
    if (NULL != pid_file_obj) {
        if (-1 == check_config_option(pid_file_obj, STRING, "pid_file")) {
            return -1;
        }
        strcpy(pid_file, pid_file_obj->valuestring);
    }

    return 0;
}

void
free_config(config_file_t *config)
{
    free(config->data);
    cJSON_Delete(config->config_json);
}
