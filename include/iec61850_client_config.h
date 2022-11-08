#ifndef INCLUDE_IEC61850_CLIENT_CONFIG_H_
#define INCLUDE_IEC61850_CLIENT_CONFIG_H_

/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2022, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mikael Bourhis-Cloarec
 */

#include <string>

// Fledge headers
#include <config_category.h>


struct ConnectionParameters {
    std::string ipAddress;
    uint16_t    mmsPort;
};


class IEC61850ClientConfig
{
    public :

        std::string logMinLevel;
        std::string assetName;

        ConnectionParameters connectionParam;

        std::string iedModel;
        std::string logicalDeviceName;
        std::string logicalNodeName;
        std::string cdc;
        std::string dataAttribute;
        std::string fcName;

        std::string daPath;

        void importConfig(const ConfigCategory &newConfig);
};

#endif  // INCLUDE_IEC61850_CLIENT_CONFIG_H_
