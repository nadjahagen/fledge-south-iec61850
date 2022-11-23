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
#include <map>

// Fledge headers
#include <config_category.h>

// libiec61850 headers
#include <libiec61850/libiec61850_common_api.h>
#include <libiec61850/iso_connection_parameters.h>

#include <rapidjson/document.h>

/**
 *  Lower layer parameters (below the MMS layer) for connection with server
 */
struct OsiParameters {
    std::string localApTitle{""};
    int localAeQualifier{0};
    std::string remoteApTitle{""};
    int remoteAeQualifier{0};
    TSelector localTSelector;
    TSelector remoteTSelector;
    SSelector localSSelector;
    SSelector remoteSSelector;
    PSelector localPSelector;
    PSelector remotePSelector;
};

/**
 *  Parameters for creating a connection with 1 IEC61850 server
 */
struct ServerConnectionParameters {
    std::string ipAddress;
    int mmsPort;
    bool isOsiParametersEnabled{false};
    OsiParameters osiParameters;
};

/**
 *  Parameters about the data to transfer to Fledge
 */
struct ExchangedData {
    std::string logicalDeviceName = "LD_NOT_DEFINED";
    std::string logicalNodeName = "LN_NOT_DEFINED";
    std::string cdc = "CDC_NOT_DEFINED";
    std::string dataAttribute = "DA_NOT_DEFINED";
    std::string fcName = "FC_NOT_DEFINED";

    std::string daPath = "NOT_DEFINED";
};

using OsiSelectorSize = uint8_t;
using ServerDictKey = std::string;
using ServerConfigDict = std::map<ServerDictKey, ServerConnectionParameters, std::less<>>;



/** \class ConfigurationException
 *  \brief a error in the input configuration has been detected
 */
class ConfigurationException: public std::logic_error
{
    public:
        explicit ConfigurationException(std::string const &msg):
            std::logic_error("Configuration exception: " + msg) {}
};


/** \class IEC61850ClientConfig
 *  \brief All the configuration for the South plugin
 *
 *  The input configuration is parsed and stored
 *  in data structures
 */
class IEC61850ClientConfig
{
    public :

        std::string logMinLevel;
        std::string assetName;
        std::string iedName;

        ServerConfigDict serverConfigDict;

        // Data model section
        ExchangedData exchangedData;

        void importConfig(const ConfigCategory &newConfig);

        inline static std::string buildKey(const ServerConnectionParameters &serverConn)
        {
            return (serverConn.ipAddress + "_" +
                    std::to_string(serverConn.mmsPort));
        }

        static void logIedConnectionParam(const ServerConnectionParameters &iedConnectionParam);
        static void logOsiSelector(const std::string &selectorName,
                                   const int selectorSize,
                                   const uint8_t *selectorValues);

    private:
        void importJsonProtocolConfig(const std::string &protocolConfig);
        void importJsonTransportLayerConfig(const rapidjson::Value &transportLayer);
        void importJsonConnectionConfig(const rapidjson::Value &connConfig);
        void importJsonConnectionOsiConfig(const rapidjson::Value &connOsiConfig,
                                           ServerConnectionParameters &iedConnectionParam) const;
        void importJsonConnectionOsiSelectors(const rapidjson::Value &connOsiConfig,
                                              OsiParameters *osiParams) const;
        void importJsonApplicationLayerConfig(const rapidjson::Value &transportLayer) const;
        void importJsonExchangeConfig(const std::string &exchangeConfig);

        static OsiSelectorSize parseOsiPSelector(std::string &inputOsiSelector, PSelector *pselector);
        static OsiSelectorSize parseOsiTSelector(std::string &inputOsiSelector, TSelector *tselector);
        static OsiSelectorSize parseOsiSSelector(std::string &inputOsiSelector, SSelector *sselector);
        static OsiSelectorSize parseOsiSelector(std::string &inputOsiSelector,
                                                uint8_t *selectorValue,
                                                const uint8_t selectorSize);

        static bool isValidIPAddress(const std::string &addrStr);
};

#endif  // INCLUDE_IEC61850_CLIENT_CONFIG_H_
