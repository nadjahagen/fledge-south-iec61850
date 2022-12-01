#ifndef INCLUDE_PLUGIN_H_
#define INCLUDE_PLUGIN_H_

/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2022, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mikael Bourhis-Cloarec
 */

// Fledge headers
#include <config_category.h>

/**
 * Default configuration
 */

#define PLUGIN_NAME "iec61850"  // NOSONAR (Fledge API)

// *INDENT-OFF* (disable 'astyle' tool on this section)
static const char *const default_config = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "default" : PLUGIN_NAME,
        "readonly" : "true"
    },

    "log min level" : {
        "description" : "minimum level for the Fledge logger (debug, info)",
        "type" : "string",
        "default" : "info",
        "displayName" : "logger minimum level",
        "order" : "1",
        "mandatory" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "default" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "default" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 102
                        },
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 8102
                        }
                    ]
                },
                "application_layer" : {
                    "read_polling_period_in_ms" : 1000
                }
            }
        })
    },

    "exchanged_data" : {
        "description" : "exchanged data list",
        "type" : "JSON",
        "displayName" : "Exchanged data list",
        "order" : "4",
        "default" : QUOTE({
            "exchanged_data": {
                "name" : "iec61850client",
                "version" : "1.0",
                "datapoints": [
                    {
                        "label":"TS1",
                        "pivot_id": "IDxxxxxx",
                        "pivot_type": "SpsTyp",
                        "pivot_subtypes": [
                            "transient"
                        ],
                        "protocols":[
                           {
                              "name":"iec61850",
                              "address":"simpleIOGenericIO/GGIO1.SPCSO2",
                              "typeid":"SP"
                           }
                        ]
                    },
                    {
                        "label":"TM1",
                        "pivot_id": "IDxxxxxx",
                        "pivot_type": "MVTyp",
                        "protocols":[
                           {
                              "name":"iec61850",
                              "address":"simpleIOGenericIO/GGIO1.AnIn1",
                              "typeid":"MV"
                           }
                        ]
                    }
                ]
            }
        })
    }
});
// *INDENT-ON*

#endif  // INCLUDE_PLUGIN_H_
