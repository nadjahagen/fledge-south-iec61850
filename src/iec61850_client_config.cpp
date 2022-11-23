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

#include <arpa/inet.h>
#include <string.h>

#include <algorithm>
#include <regex>

// Fledge headers
#include <logger.h>

const char *const JSON_PROTOCOL_STACK = "protocol_stack";
const char *const JSON_TRANSPORT_LAYER = "transport_layer";
const char *const JSON_APPLICATION_LAYER = "application_layer";
const char *const JSON_EXCHANGED_DATA = "exchanged_data";

const char *const DEFAULT_LOG_MIN_LEVEL = "info";
const char *const DEFAULT_ASSET_NAME = "iec61850";

void IEC61850ClientConfig::importConfig(const ConfigCategory &newConfig)
{
    if (newConfig.itemExists("log min level")) {
        logMinLevel = newConfig.getValue("log min level");
    } else {
        logMinLevel = DEFAULT_LOG_MIN_LEVEL;
    }
    Logger::getLogger()->info("IEC61850ClientConfig: logMinLevel = %s",
                              logMinLevel.c_str());

    assetName = DEFAULT_ASSET_NAME;
    if (newConfig.itemExists("asset")) {
        assetName = newConfig.getValue("asset");
    }
    Logger::getLogger()->info("IEC61850ClientConfig: assetName = %s",
                              assetName.c_str());

    std::string inputProtocolStack;
    std::string inputExchangedData;

    try {
        inputProtocolStack = newConfig.getValue(JSON_PROTOCOL_STACK);
    }
    catch (...) {
        throw ConfigurationException("'Protocol stack' not found");
    }
    importJsonProtocolConfig(inputProtocolStack);

    try {
        inputExchangedData = newConfig.getValue(JSON_EXCHANGED_DATA);
    }
    catch (...) {
        throw ConfigurationException("'Exchanged Data' not found");
    }
    importJsonExchangeConfig(inputExchangedData);
}

void IEC61850ClientConfig::importJsonProtocolConfig(const std::string &protocolConfig)
{
    rapidjson::Document document;

    /** Parse the input JSON std::string */
    if (document.Parse(protocolConfig.c_str()).HasParseError()) {
        throw ConfigurationException("'Protocol stack' parsing error");
    }

    /** The 'protocol_stack' section is mandatory */
    if (!document.IsObject()) {
        throw ConfigurationException("'Protocol stack' empty conf");
    }

    if (!document.HasMember(JSON_PROTOCOL_STACK) || !document[JSON_PROTOCOL_STACK].IsObject()) {
        throw ConfigurationException("'Protocol stack' empty conf");
    }

    const rapidjson::Value& protocolStack = document[JSON_PROTOCOL_STACK];

    /** Check if 'transport_layer' is found */
    if (!protocolStack.HasMember(JSON_TRANSPORT_LAYER) || !protocolStack[JSON_TRANSPORT_LAYER].IsObject()) {
        throw ConfigurationException("'transport layer' configuration is missing");
    }

    const rapidjson::Value &transportLayer = protocolStack[JSON_TRANSPORT_LAYER];

    importJsonTransportLayerConfig(transportLayer);

    /** Check if 'application_layer' is found */
    if (!protocolStack.HasMember(JSON_APPLICATION_LAYER) || !protocolStack[JSON_APPLICATION_LAYER].IsObject()) {
        throw ConfigurationException("'application layer' configuration is missing");
    }

    const rapidjson::Value &applicationLayer = protocolStack[JSON_APPLICATION_LAYER];

    importJsonApplicationLayerConfig(applicationLayer);
}

void IEC61850ClientConfig::importJsonTransportLayerConfig(const rapidjson::Value &transportLayer)
{
    // Preconditions
    if (! transportLayer.HasMember("ied_name")) {
        throw ConfigurationException("the mandatory 'ied_name' not found");
    }
    if (! transportLayer["ied_name"].IsString()) {
        throw ConfigurationException("bad format for the mandatory 'ied_name'");
    }
    if (! transportLayer.HasMember("connections")) {
        throw ConfigurationException("'Transport Layer' parsing error: no 'connections'");
    }
    if (! transportLayer["connections"].IsArray()) {
        throw ConfigurationException("'connections' is not an array -> fail to parse 'Transport Layer'");
    }

    iedName = std::string(transportLayer["ied_name"].GetString());

    const rapidjson::Value& connections = transportLayer["connections"];

    /** Parse each 'connection' JSON structure */
    for (const auto &conn: connections.GetArray()) {
        importJsonConnectionConfig(conn);
    }
}

void IEC61850ClientConfig::importJsonConnectionConfig(const rapidjson::Value &connConfig)
{
    // Preconditions
    if (! connConfig.IsObject()) {
        throw ConfigurationException("'Connection' is not valid");
    }
    if (! connConfig.HasMember("srv_ip")) {
        throw ConfigurationException("the mandatory 'srv_ip' not found");
    }
    if (! connConfig["srv_ip"].IsString()) {
        throw ConfigurationException("bad format for the mandatory 'srv_ip'");
    }
    if (! connConfig.HasMember("port")) {
        throw ConfigurationException("the mandatory 'port' not found");
    }
    if (! connConfig["port"].IsInt()) {
        throw ConfigurationException("bad format for the mandatory 'port'");
    }

    ServerConnectionParameters iedConnectionParam;

    iedConnectionParam.ipAddress = std::string(connConfig["srv_ip"].GetString());

    if ( ! isValidIPAddress(iedConnectionParam.ipAddress)) {
        throw ConfigurationException("not a valid IP address for the mandatory 'srv_ip'");
    }

    iedConnectionParam.mmsPort = connConfig["port"].GetInt();

    if (connConfig.HasMember("osi")) {
        importJsonConnectionOsiConfig(connConfig["osi"], iedConnectionParam);
    }

    logIedConnectionParam(iedConnectionParam);

    ServerDictKey key = buildKey(iedConnectionParam);

    serverConfigDict[key] = iedConnectionParam;
}

void IEC61850ClientConfig::importJsonConnectionOsiConfig(const rapidjson::Value &connOsiConfig,
                                                         ServerConnectionParameters &iedConnectionParam)
{
    // Preconditions
    if (! connOsiConfig.IsObject()) {
        throw ConfigurationException("'OSI' section is not valid");
    }

    OsiParameters *osiParams = &iedConnectionParam.osiParameters;

    // AE qualifiers
    if (connOsiConfig.HasMember("local_ae_qualifier")) {
        if (! connOsiConfig["local_ae_qualifier"].IsInt()) {
            throw ConfigurationException("bad format for 'local_ae_qualifier'");
        }
        osiParams->localAeQualifier = connOsiConfig["local_ae_qualifier"].GetInt();
    }

    if (connOsiConfig.HasMember("remote_ae_qualifier")) {
        if (! connOsiConfig["remote_ae_qualifier"].IsInt()) {
            throw ConfigurationException("bad format for 'remote_ae_qualifier'");
        }
        osiParams->remoteAeQualifier = connOsiConfig["remote_ae_qualifier"].GetInt();
    }

    // AP Title
    if (connOsiConfig.HasMember("local_ap_title")) {
        if (! connOsiConfig["local_ap_title"].IsString()) {
            throw ConfigurationException("bad format for 'local_ap_title'");
        }
        osiParams->localApTitle = connOsiConfig["local_ap_title"].GetString();

        std::replace(osiParams->localApTitle.begin(),
                     osiParams->localApTitle.end(),
                     ',', '.');

        // check 'localApTitle' contains digits and dot only
        std::string strToCheck = osiParams->localApTitle;
        strToCheck.erase(std::remove(strToCheck.begin(), strToCheck.end(), '.'), strToCheck.end());
        if (! std::regex_match(strToCheck, std::regex("[0-9]*"))) {
            throw ConfigurationException("'local_ap_title' is not valid");
        };
    }

    if (connOsiConfig.HasMember("remote_ap_title")) {
        if (! connOsiConfig["remote_ap_title"].IsString()) {
            throw ConfigurationException("bad format for 'remote_ap_title'");
        }
        osiParams->remoteApTitle = connOsiConfig["remote_ap_title"].GetString();

        std::replace(osiParams->remoteApTitle.begin(),
                     osiParams->remoteApTitle.end(),
                     ',', '.');

        // check 'remoteApTitle' contains digits and dot only
        std::string strToCheck = osiParams->remoteApTitle;
        strToCheck.erase(std::remove(strToCheck.begin(), strToCheck.end(), '.'), strToCheck.end());
        if (! std::regex_match(strToCheck, std::regex("[0-9]*"))) {
            throw ConfigurationException("'remote_ap_title' is not valid");
        };
    }

    // Selector
    if (connOsiConfig.HasMember("local_psel")) {
        if (! connOsiConfig["local_psel"].IsString()) {
            throw ConfigurationException("bad format for 'local_psel'");
        }
        std::string inputOsiSelector = connOsiConfig["local_psel"].GetString();
        osiParams->localPSelector.size = parseOsiPSelector(inputOsiSelector, &osiParams->localPSelector);
    }

    if (connOsiConfig.HasMember("local_ssel")) {
        if (! connOsiConfig["local_ssel"].IsString()) {
            throw ConfigurationException("bad format for 'local_ssel'");
        }
        std::string inputOsiSelector = connOsiConfig["local_ssel"].GetString();
        osiParams->localSSelector.size = parseOsiSSelector(inputOsiSelector, &osiParams->localSSelector);
    }

    if (connOsiConfig.HasMember("local_tsel")) {
        if (! connOsiConfig["local_tsel"].IsString()) {
            throw ConfigurationException("bad format for 'local_tsel'");
        }
        std::string inputOsiSelector = connOsiConfig["local_tsel"].GetString();
        osiParams->localTSelector.size = parseOsiTSelector(inputOsiSelector, &osiParams->localTSelector);
    }

    if (connOsiConfig.HasMember("remote_psel")) {
        if (! connOsiConfig["remote_psel"].IsString()) {
            throw ConfigurationException("bad format for 'remote_psel'");
        }
        std::string inputOsiSelector = connOsiConfig["remote_psel"].GetString();
        osiParams->remotePSelector.size = parseOsiPSelector(inputOsiSelector, &osiParams->remotePSelector);
    }

    if (connOsiConfig.HasMember("remote_ssel")) {
        if (! connOsiConfig["remote_ssel"].IsString()) {
            throw ConfigurationException("bad format for 'remote_ssel'");
        }
        std::string inputOsiSelector = connOsiConfig["remote_ssel"].GetString();
        osiParams->remoteSSelector.size = parseOsiSSelector(inputOsiSelector, &osiParams->remoteSSelector);
    }

    if (connOsiConfig.HasMember("remote_tsel")) {
        if (! connOsiConfig["remote_tsel"].IsString()) {
            throw ConfigurationException("bad format for 'remote_tsel'");
        }
        std::string inputOsiSelector = connOsiConfig["remote_tsel"].GetString();
        osiParams->remoteTSelector.size = parseOsiTSelector(inputOsiSelector, &osiParams->remoteTSelector);
    }
    iedConnectionParam.isOsiParametersEnabled = true;
}

OsiSelectorSize
IEC61850ClientConfig::parseOsiPSelector(std::string &inputOsiSelector,
                                        PSelector *pselector)
{
    return parseOsiSelector(inputOsiSelector, pselector->value, 16);
}

OsiSelectorSize
IEC61850ClientConfig::parseOsiSSelector(std::string &inputOsiSelector,
                                        SSelector *sselector)
{
    return parseOsiSelector(inputOsiSelector, sselector->value, 16);
}

OsiSelectorSize
IEC61850ClientConfig::parseOsiTSelector(std::string &inputOsiSelector,
                                        TSelector *tselector)
{
    return parseOsiSelector(inputOsiSelector, tselector->value, 4);
}

OsiSelectorSize
IEC61850ClientConfig::parseOsiSelector(std::string &inputOsiSelector,
                                       uint8_t *selectorValue,
                                       const uint8_t selectorSize)
{
    char * nextToken = strtok(&inputOsiSelector[0], " ,.-");
    unsigned int count = 0;
    while(nullptr != nextToken) {

        if (count >= selectorSize) {
            throw ConfigurationException("bad format for 'OSI Selector' (too many bytes)");
        }

        int base = 10;
        if (0 == strncmp(nextToken, "0x", 2)) {
            base = 16;
        }

        unsigned long ul = 0;
        try {
            ul = std::stoul(nextToken, nullptr, base);
        }
        catch (std::invalid_argument &e) {
            throw ConfigurationException("bad format for 'OSI Selector' (not a byte)");
        }
        catch (std::out_of_range &e) {
            throw ConfigurationException("bad format for 'OSI Selector (exceed an int)'");
        }

        if (ul > 255) {
            throw ConfigurationException("bad format for 'OSI Selector' (exceed a byte)");
        }

        selectorValue[count] = ul;
        count++;
        nextToken = strtok(nullptr, " ,.-");
    }

    return count;
}

void IEC61850ClientConfig::importJsonApplicationLayerConfig(const rapidjson::Value &/*applicationLayer*/) const
{}

void IEC61850ClientConfig::logIedConnectionParam(const ServerConnectionParameters &iedConnectionParam)
{
    Logger::getLogger()->info("Config: Transport Layer:");
    Logger::getLogger()->info("\tIED: IP address:  %s", iedConnectionParam.ipAddress.c_str());
    Logger::getLogger()->info("\tIED: MMS port:    %d", iedConnectionParam.mmsPort);

    if (iedConnectionParam.isOsiParametersEnabled) {
        Logger::getLogger()->info("\tIED: local AP Title: %s", iedConnectionParam.osiParameters.localApTitle.c_str());
        Logger::getLogger()->info("\tIED: local AE qualifier: %d", iedConnectionParam.osiParameters.localAeQualifier);
        Logger::getLogger()->info("\tIED: remote AP Title: %s", iedConnectionParam.osiParameters.remoteApTitle.c_str());
        Logger::getLogger()->info("\tIED: remote AE qualifier: %d", iedConnectionParam.osiParameters.remoteAeQualifier);

        logOsiSelector("local PSelector",
                       iedConnectionParam.osiParameters.localPSelector.size,
                       iedConnectionParam.osiParameters.localPSelector.value);

        logOsiSelector("local SSelector",
                       iedConnectionParam.osiParameters.localSSelector.size,
                       iedConnectionParam.osiParameters.localSSelector.value);

        logOsiSelector("local TSelector",
                       iedConnectionParam.osiParameters.localTSelector.size,
                       iedConnectionParam.osiParameters.localTSelector.value);

        logOsiSelector("remote PSelector",
                       iedConnectionParam.osiParameters.remotePSelector.size,
                       iedConnectionParam.osiParameters.remotePSelector.value);

        logOsiSelector("remote SSelector",
                       iedConnectionParam.osiParameters.remoteSSelector.size,
                       iedConnectionParam.osiParameters.remoteSSelector.value);

        logOsiSelector("remote TSelector",
                       iedConnectionParam.osiParameters.remoteTSelector.size,
                       iedConnectionParam.osiParameters.remoteTSelector.value);
    }
}

void IEC61850ClientConfig::logOsiSelector(const std::string &selectorName,
                                               const int selectorSize,
                                               const uint8_t *selectorValues)
{
    if (0 != selectorSize) {
        Logger::getLogger()->info("\t\tOSI Selector '%s': size= %d",
                                  selectorName.c_str(),
                                  selectorSize);
        for (int i = 0; i < selectorSize; i++) {
            Logger::getLogger()->info("\t\tOSI Selector '%s': [%d]= 0x%02X",
                    selectorName.c_str(),
                    i,
                    selectorValues[i]);
        }
    }
}

void IEC61850ClientConfig::importJsonExchangeConfig(const std::string &exchangeConfig)
{
    rapidjson::Document document;

    /** Parse the input JSON std::string */
    if (document.Parse(exchangeConfig.c_str()).HasParseError()) {
        throw ConfigurationException("'Exchanged data' parsing error");
    }

    /** The 'exchanged_data' section is mandatory */
    if (!document.IsObject()) {
        throw ConfigurationException("'Exchanged data' empty conf");
    }

    if (!document.HasMember(JSON_EXCHANGED_DATA) || !document[JSON_EXCHANGED_DATA].IsObject()) {
        throw ConfigurationException("'Exchanged data' empty conf");
    }

    const rapidjson::Value& exchange = document[JSON_EXCHANGED_DATA];

    /** The 'Logical Device' param is mandatory */
    if (! exchange.HasMember("Logical Device")) {
        throw ConfigurationException("the mandatory 'Logical Device' not found");
    }
    if (! exchange["Logical Device"].IsString()) {
        throw ConfigurationException("bad format for the mandatory 'Logical Device'");
    }

    exchangedData.logicalDeviceName = std::string(exchange["Logical Device"].GetString());

    /** The 'Logical Node' param is mandatory */
    if (! exchange.HasMember("Logical Node")) {
        throw ConfigurationException("the mandatory 'Logical Node' not found");
    }
    if (! exchange["Logical Node"].IsString()) {
        throw ConfigurationException("bad format for the mandatory 'Logical Node'");
    }

    exchangedData.logicalNodeName = std::string(exchange["Logical Node"].GetString());

    /** The 'CDC' param is mandatory */
    if (! exchange.HasMember("CDC")) {
        throw ConfigurationException("the mandatory 'CDC' not found");
    }
    if (! exchange["CDC"].IsString()) {
        throw ConfigurationException("bad format for the mandatory 'CDC'");
    }

    exchangedData.cdc = std::string(exchange["CDC"].GetString());

    /** The 'Data Attribute' param is mandatory */
    if (! exchange.HasMember("Data Attribute")) {
        throw ConfigurationException("the mandatory 'Data Attribute' not found");
    }
    if (! exchange["Data Attribute"].IsString()) {
        throw ConfigurationException("bad format for the mandatory 'Data Attribute'");
    }

    exchangedData.dataAttribute = std::string(exchange["Data Attribute"].GetString());

    /** The 'Functional Constraint' param is mandatory */
    if (! exchange.HasMember("Functional Constraint")) {
        throw ConfigurationException("the mandatory 'Functional Constraint' not found");
    }
    if (! exchange["Functional Constraint"].IsString()) {
        throw ConfigurationException("bad format for the mandatory 'Functional Constraint'");
    }

    exchangedData.fcName = std::string(exchange["Functional Constraint"].GetString());

    exchangedData.daPath = iedName + exchangedData.logicalDeviceName +
                           "/" +
                           exchangedData.logicalNodeName +
                           "." +
                           exchangedData.cdc +
                           "." +
                           exchangedData.dataAttribute;
}

bool IEC61850ClientConfig::isValidIPAddress(const std::string &addrStr)
{
    // see https://stackoverflow.com/questions/318236/how-do-you-validate-that-a-string-is-a-valid-ipv4-address-in-c
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, addrStr.c_str(), &(sa.sin_addr));

    return (result == 1);
}
