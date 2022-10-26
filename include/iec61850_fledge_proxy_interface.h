#ifndef INCLUDE_IEC61850_FLEDGE_PROXY_INTERFACE_H_
#define INCLUDE_IEC61850_FLEDGE_PROXY_INTERFACE_H_

/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2022, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 */

#include <vector>

// Fledge headers
#include <reading.h>

class Datapoint;

class FledgeProxyInterface
{
    public :

        virtual void ingest(std::vector<Datapoint *>  points) = 0;
        virtual void registerIngest(void *data,
                                    void (*ingest_cb)(void *, Reading)) = 0;

};

#endif  // INCLUDE_IEC61850_FLEDGE_PROXY_INTERFACE_H_
