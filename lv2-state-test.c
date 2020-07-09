/*
    Copyright (C) 2020 falkTX
    This work is free. You can redistribute it and/or modify it under the
    terms of the Do What The Fuck You Want To Public License, Version 2,
    as published by Sam Hocevar. See the COPYING file for more details.
*/

#include <lv2.h>
#include <lv2/atom/atom.h>
#include <lv2/log/logger.h>
#include <lv2/state/state.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef DO_NOT_HAVE_LV2_STATE_FREE_PATH
// forwards compatibility with old lv2 headers
#define LV2_STATE__freePath LV2_STATE_PREFIX "freePath"
typedef void* LV2_State_Free_Path_Handle;
typedef struct {
    LV2_State_Free_Path_Handle handle;
    void (*free_path)(LV2_State_Free_Path_Handle handle, char* path);
} LV2_State_Free_Path;
#endif

typedef struct {
    bool activated, deactivated, saved, restored;
    LV2_Log_Logger logger;
    const LV2_Atom_Sequence* seqIn;
    LV2_Atom_Sequence* seqOut;
    const LV2_State_Free_Path* freePath;
    const LV2_State_Make_Path* makePath;
    const LV2_State_Map_Path* mapPath;
} StateTest;

static void lv2_free_path(const LV2_State_Free_Path* freePath, char* path)
{
    if (freePath != NULL)
        freePath->free_path(freePath->handle, path);
    else
        free(path);
}

static LV2_Handle instantiate(const LV2_Descriptor* const descriptor,
                              const double sample_rate,
                              const char* const bundle_path,
                              const LV2_Feature* const* const features)
{
    const LV2_State_Free_Path* freePath = NULL;
    const LV2_State_Make_Path* makePath = NULL;
    const LV2_State_Map_Path* mapPath = NULL;
    const LV2_Log_Log* log = NULL;
    const LV2_URID_Map* uridMap = NULL;

    for (int i=0; features[i] != NULL; ++i)
    {
        /**/ if (strcmp(features[i]->URI, LV2_STATE__freePath) == 0)
            freePath = features[i]->data;
        else if (strcmp(features[i]->URI, LV2_STATE__makePath) == 0)
            makePath = features[i]->data;
        else if (strcmp(features[i]->URI, LV2_STATE__mapPath) == 0)
            mapPath = features[i]->data;
        else if (strcmp(features[i]->URI, LV2_LOG__log) == 0)
            log = features[i]->data;
        else if (strcmp(features[i]->URI, LV2_URID__map) == 0)
            uridMap = features[i]->data;
    }

    StateTest* instance = calloc(1, sizeof(StateTest));

    instance->freePath = freePath;
    instance->makePath = makePath;
    instance->mapPath = mapPath;

    lv2_log_logger_init(&instance->logger, uridMap, log);

    if (makePath == NULL)
    {
        lv2_log_note(&instance->logger, "state-test init, host does not have makePath\n");
        return instance;
    }

    char* projectPath;

    // test getting initial dir
    projectPath = makePath->path(makePath->handle, ".");

    if (projectPath != NULL)
    {
        const int status = access(projectPath, F_OK);
        lv2_log_note(&instance->logger, "state-test init, host has makePath and initial dir is: '%s' (access = %i)\n",
                     projectPath, status);

        lv2_free_path(freePath, projectPath);
    }
    else
    {
        lv2_log_note(&instance->logger, "state-test init, host has makePath but failed to get initial path\n");
    }

    // test creating single subdirs
    projectPath = makePath->path(makePath->handle, "single-subdir/");

    if (projectPath != NULL)
    {
        const int status = access(projectPath, F_OK);
        lv2_log_note(&instance->logger, "state-test init, single-subdir creation resulted in '%s' (access = %i)\n",
                     projectPath, status);

        lv2_free_path(freePath, projectPath);
    }
    else
    {
        lv2_log_note(&instance->logger, "state-test init, single-subdir creation failed\n");
    }

    // test creating multiple subdirs
    projectPath = makePath->path(makePath->handle, "subdir1/subdir2/subdir3/");

    if (projectPath != NULL)
    {
        const int status = access(projectPath, F_OK);
        lv2_log_note(&instance->logger, "state-test init, multi-subdir creation resulted in '%s' (access = %i)\n",
                     projectPath, status);

        lv2_free_path(freePath, projectPath);
    }
    else
    {
        lv2_log_note(&instance->logger, "state-test init, multi-subdir creation failed\n");
    }

    return instance;

    // unused
    (void)descriptor;
    (void)sample_rate;
    (void)bundle_path;
}

#define instancePtr ((StateTest*)instance)

static void connect_port(LV2_Handle instance, uint32_t port, void* data_location)
{
    switch (port)
    {
    case 0:
        instancePtr->seqIn = data_location;
        break;
    case 1:
        instancePtr->seqOut = data_location;
        break;
    }
}

static void activate(LV2_Handle instance)
{
    instancePtr->activated = true;
}

static void run(LV2_Handle instance, uint32_t sample_count)
{
    if (instancePtr->deactivated)
    {
        instancePtr->deactivated = false;
        lv2_log_note(&instancePtr->logger, "plugin was deactivated\n");
    }

    if (instancePtr->activated)
    {
        instancePtr->activated = false;
        lv2_log_note(&instancePtr->logger, "plugin was activated\n");
    }

    if (instancePtr->restored)
    {
        instancePtr->restored = false;
        lv2_log_note(&instancePtr->logger, "plugin state was restored\n");
    }

    if (instancePtr->saved)
    {
        instancePtr->saved = false;
        lv2_log_note(&instancePtr->logger, "plugin state was saved\n");
    }

    return;

    // unused
    (void)sample_count;
}

static void deactivate(LV2_Handle instance)
{
    instancePtr->deactivated = true;
}

static void cleanup(LV2_Handle instance)
{
    free(instance);
}

static LV2_State_Status save(LV2_Handle instance,
                             LV2_State_Store_Function store,
                             LV2_State_Handle handle,
                             uint32_t flags,
                             const LV2_Feature* const* features)
{
    const LV2_State_Free_Path* freePath = instancePtr->freePath;
    const LV2_State_Make_Path* makePath = instancePtr->makePath;

    char* const projectPath = makePath->path(makePath->handle, ".");

    if (projectPath != NULL)
    {
        lv2_log_note(&instancePtr->logger, "state-test save ok, path is: '%s'\n", projectPath);

        lv2_free_path(freePath, projectPath);
    }
    else
    {
        lv2_log_note(&instancePtr->logger, "state-test save ok, but failed to get initial path\n");
    }

    instancePtr->saved = true;
    return LV2_STATE_SUCCESS;

    // TODO
    (void)store;
    (void)handle;
    (void)flags;
    (void)features;
}

static LV2_State_Status restore(LV2_Handle instance,
                                LV2_State_Retrieve_Function retrieve,
                                LV2_State_Handle handle,
                                uint32_t flags,
                                const LV2_Feature* const* features)
{
    const LV2_State_Free_Path* freePath = instancePtr->freePath;
    const LV2_State_Make_Path* makePath = instancePtr->makePath;

    char* const projectPath = makePath->path(makePath->handle, ".");

    if (projectPath != NULL)
    {
        lv2_log_note(&instancePtr->logger, "state-test restore ok, path is: '%s'\n", projectPath);

        if (freePath != NULL)
            freePath->free_path(freePath->handle, projectPath);
        else
            free(projectPath);
    }
    else
    {
        lv2_log_note(&instancePtr->logger, "state-test restore ok, but failed to get initial path\n");
    }

    instancePtr->restored = true;
    return LV2_STATE_SUCCESS;

    // TODO
    (void)retrieve;
    (void)handle;
    (void)flags;
    (void)features;
}

static const void* extension_data(const char* const uri)
{
    static const LV2_State_Interface iface = {
        .save = save,
        .restore = restore
    };

    if (strcmp(uri, LV2_STATE__interface) == 0)
        return &iface;

    return NULL;
}

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(const uint32_t index)
{
    static const LV2_Descriptor desc = {
        .URI = "https://git.kx.studio/falkTX/lv2-state-test",
        .instantiate = instantiate,
        .connect_port = connect_port,
        .activate = activate,
        .run = run,
        .deactivate = deactivate,
        .cleanup = cleanup,
        .extension_data = extension_data
    };

    return index == 0 ? &desc : NULL;
}
