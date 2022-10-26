/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2022, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mikael Bourhis-Cloarec
 */

#include "./iec61850_client_config.h"

constexpr const uint16_t DEFAULT_MMS_PORT = 8102;
const char *const DEFAULT_IED_IP_ADDRESS = "127.0.0.1";

void IEC61850ClientConfig::importConfig(const ConfigCategory &newConfig)
{
    if (newConfig.itemExists("asset")) {
        assetName = newConfig.getValue("asset");
    } else {
        assetName = "iec61850";
    }

    // Set the IP of the 61850 server
    if (newConfig.itemExists("ip")) {
        connectionParam.ipAddress = newConfig.getValue("ip");
    } else {
        connectionParam.ipAddress = DEFAULT_IED_IP_ADDRESS;
    }

    // Set the MMS port of the 61850 server
    if (newConfig.itemExists("port")) {
        connectionParam.mmsPort = static_cast<uint16_t>(stoi(newConfig.getValue("port")));
    } else {
        connectionParam.mmsPort = DEFAULT_MMS_PORT;
    }

    // Set the model of the IED
    if (newConfig.itemExists("IED Model")) {
        iedModel = newConfig.getValue("IED Model");
    }

    // Set the name of the logical device
    if (newConfig.itemExists("Logical Device")) {
        logicalDeviceName = newConfig.getValue("Logical Device");
    }

    // Set the name of the logical node
    if (newConfig.itemExists("Logical Node")) {
        logicalNodeName = newConfig.getValue("Logical Node");
    }

    if (newConfig.itemExists("CDC")) {
        cdc = newConfig.getValue("CDC");
    }

    // Data Attribute: DA
    if (newConfig.itemExists("Data Attribute")) {
        dataAttribute = newConfig.getValue("Data Attribute");
    }

    if (newConfig.itemExists("Functional Constraint")) {
        fcName = newConfig.getValue("Functional Constraint");
    }

    daPath = iedModel + logicalDeviceName +
             "/" +
             logicalNodeName +
             "." +
             cdc +
             "." +
             dataAttribute;
}
