/*
    Copyright (C) 2020 falkTX
    This work is free. You can redistribute it and/or modify it under the
    terms of the Do What The Fuck You Want To Public License, Version 2,
    as published by Sam Hocevar. See the COPYING file for more details.
*/

#include <lv2.h>
#include <lv2/state/state.h>

#include <stdlib.h>

static LV2_Handle instantiate(const LV2_Descriptor* const descriptor,
                              const double sample_rate,
                              const char* const bundle_path,
                              const LV2_Feature* const* const features)
{
    return NULL;
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data_location)
{
}

static void activate(LV2_Handle instance)
{
}

static void run(LV2_Handle instance, uint32_t sample_count)
{
}

static void deactivate(LV2_Handle instance)
{
}

static void cleanup(LV2_Handle instance)
{
    free(instance);
}

static const void* extension_data(const char* const uri)
{
    return NULL;
}

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(const uint32_t index)
{
    static const LV2_Descriptor desc = {
        .URI = "",
        .connect_port = connect_port,
        .activate = activate,
        .run = run,
        .deactivate = deactivate,
        .cleanup = cleanup,
        .extension_data = extension_data
    };

    return index == 0 ? &desc : NULL;
}
