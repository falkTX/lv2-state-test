/*
    Copyright (C) 2020 falkTX
    This work is free. You can redistribute it and/or modify it under the
    terms of the Do What The Fuck You Want To Public License, Version 2,
    as published by Sam Hocevar. See the COPYING file for more details.
*/

#include <lv2.h>
#include <lv2/atom/atom.h>
#include <lv2/state/state.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef HAVE_LV2_STATE_FREE_PATH
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
    const LV2_Atom_Sequence* seqIn;
    LV2_Atom_Sequence* seqOut;
    const LV2_State_Free_Path* freePath;
    const LV2_State_Make_Path* makePath;
    const LV2_State_Map_Path* mapPath;
} StateTest;

static LV2_Handle instantiate(const LV2_Descriptor* const descriptor,
                              const double sample_rate,
                              const char* const bundle_path,
                              const LV2_Feature* const* const features)
{
    const LV2_State_Free_Path* freePath = NULL;
    const LV2_State_Make_Path* makePath = NULL;
    const LV2_State_Map_Path* mapPath = NULL;

    for (int i=0; features[i] != NULL; ++i)
    {
        /**/ if (strcmp(features[i]->URI, LV2_STATE__freePath) == 0)
            freePath = features[i]->data;
        else if (strcmp(features[i]->URI, LV2_STATE__makePath) == 0)
            makePath = features[i]->data;
        else if (strcmp(features[i]->URI, LV2_STATE__mapPath) == 0)
            mapPath = features[i]->data;
    }

    if (freePath == NULL || makePath == NULL || mapPath == NULL)
        return NULL;

    StateTest* instance = calloc(1, sizeof(StateTest));

    instance->freePath = freePath;
    instance->makePath = makePath;
    instance->mapPath = mapPath;

    {
        char* const projectPath = makePath->path(makePath->handle, ".");

        if (projectPath != NULL)
        {
            printf("state-test init ok, initial path is: '%s'\n", projectPath);
            freePath->free_path(freePath->handle, projectPath);
        }
        else
        {
            printf("state-test init ok, but failed to get initial path\n");
        }
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
        // TODO something
        instancePtr->deactivated = false;
    }

    if (instancePtr->activated)
    {
        // TODO something
        instancePtr->activated = false;
    }

    if (instancePtr->restored)
    {
        // TODO something
        instancePtr->restored = false;
    }

    if (instancePtr->saved)
    {
        // TODO something
        instancePtr->saved = false;
    }
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
