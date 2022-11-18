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
                            "port" : 102,
                            "osi" : {
                                "local_ap_title": "1,3,9999,13",
                                "local_ae_qualifier": 12,
                                "remote_ap_title": "1,2,1200,15,3",
                                "remote_ae_qualifier": 1,
                                "local_psel": "0x12, 0x34, 0x56, 0x78",
                                "local_ssel": "0, 1, 2, 3, 4",
                                "local_tsel": "0x00,0x01, 0x02",
                                "remote_psel": "0x87, 0x65, 0x43, 0x21",
                                "remote_ssel": "0, 1",
                                "remote_tsel": "0x00, 0x01"
                            }
                        },
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 8102
                        }
                    ]
                },
                "application_layer" : {
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
                "name" : "iec104client",
                "version" : "1.0",
                "Logical Device": "GenericIO",
                "Logical Node": "GGIO1",
                "CDC" : "AnIn1",
                "Data Attribute": "mag.f",
                "Functional Constraint": "MX"
            }
        })
    }
});
// *INDENT-ON*

#endif  // INCLUDE_PLUGIN_H_
