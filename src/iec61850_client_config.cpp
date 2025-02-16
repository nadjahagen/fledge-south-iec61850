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
const char *const JSON_EXCHANGED_DATASETS = "exchanged_datasets";
const char *const JSON_CONNECTIONS = "connections";
const char *const JSON_DATAPOINTS = "datapoints";
const char *const JSON_DATASETS = "datasets";
const char *const JSON_DATA_OBJECTS = "data_objects";
const char *const JSON_PROTOCOLS = "protocols";

const char *const DEFAULT_LOG_MIN_LEVEL = "info";
const char *const DEFAULT_ASSET_NAME = "iec61850";

const char *const IEC61850_PROTOCOL_NAME = "iec61850";

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

    try {
        inputProtocolStack = newConfig.getValue(JSON_PROTOCOL_STACK);
    } catch (...) {
        throw ConfigurationException("'Protocol stack' not found");
    }

    importJsonProtocolConfig(inputProtocolStack);

    if (newConfig.itemExists(JSON_EXCHANGED_DATA)) {
        importJsonExchangedDataConfig(newConfig.getValue(JSON_EXCHANGED_DATA));
    } else {
        Logger::getLogger()->info("IEC61850ClientConfig: No ExchangedData section");
    }

    if (newConfig.itemExists(JSON_EXCHANGED_DATASETS)) {
        importJsonExchangedDatasetsConfig(newConfig.getValue(JSON_EXCHANGED_DATASETS));
    } else {
        Logger::getLogger()->info("IEC61850ClientConfig: No ExchangedDatasets section");
    }
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

    const rapidjson::Value &protocolStack = document[JSON_PROTOCOL_STACK];

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

    if (! transportLayer.HasMember(JSON_CONNECTIONS)) {
        throw ConfigurationException("'Transport Layer' parsing error: no 'connections'");
    }

    if (! transportLayer[JSON_CONNECTIONS].IsArray()) {
        throw ConfigurationException("'connections' is not an array -> fail to parse 'Transport Layer'");
    }

    iedName = std::string(transportLayer["ied_name"].GetString());

    /** Parse each 'connection' JSON structure */
    for (const auto &conn : transportLayer[JSON_CONNECTIONS].GetArray()) {
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
        ServerConnectionParameters &iedConnectionParam) const
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
        }
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
        }
    }

    // Selector
    importJsonConnectionOsiSelectors(connOsiConfig, osiParams);
    iedConnectionParam.isOsiParametersEnabled = true;
}

void IEC61850ClientConfig::importJsonConnectionOsiSelectors(const rapidjson::Value &connOsiConfig,
                                                           OsiParameters *osiParams) const
{
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
    char *tokenContext = nullptr;
    const char *nextToken = strtok_r(&inputOsiSelector[0], " ,.-", &tokenContext);
    uint8_t count = 0;

    while (nullptr != nextToken) {
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
        } catch (std::invalid_argument &) {
            throw ConfigurationException("bad format for 'OSI Selector' (not a byte)");
        } catch (std::out_of_range &) {
            throw ConfigurationException("bad format for 'OSI Selector (exceed an int)'");
        }

        if (ul > 255) {
            throw ConfigurationException("bad format for 'OSI Selector' (exceed a byte)");
        }

        selectorValue[count] = static_cast<uint8_t>(ul);
        count++;
        nextToken = strtok_r(nullptr, " ,.-", &tokenContext);
    }

    return count;
}

void IEC61850ClientConfig::importJsonApplicationLayerConfig(const rapidjson::Value &applicationLayer)
{
    if (applicationLayer.HasMember("reading_period")) {
        if (! applicationLayer["reading_period"].IsInt()) {
            throw ConfigurationException("bad format for 'reading_period'");
        }

        applicationParams.readPollingPeriodInMs = applicationLayer["reading_period"].GetInt();
    }

    if (applicationLayer.HasMember("read_mode")) {
        if (! applicationLayer["read_mode"].IsString()) {
            throw ConfigurationException("bad format for 'read_mode'");
        }

        std::string inputReadMode = applicationLayer["read_mode"].GetString();
        if (inputReadMode.compare("dataset") == 0) {
            applicationParams.readMode = ReadMode::DATASET_READING;
        } else {
            applicationParams.readMode = ReadMode::DO_READING;
        }
    }
}

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

void IEC61850ClientConfig::importJsonExchangedDataConfig(const std::string &exchangedDataConfig)
{
    rapidjson::Document document;

    /** Parse the input JSON std::string. */
    if (document.Parse(exchangedDataConfig.c_str()).HasParseError()) {
        throw ConfigurationException("'Exchanged data' parsing error");
    }

    /** The 'exchanged_data' section is mandatory: throw exception if not found. */
    if (!document.IsObject()) {
        throw ConfigurationException("'Exchanged data' empty conf");
    }

    if (!document.HasMember(JSON_EXCHANGED_DATA) || !document[JSON_EXCHANGED_DATA].IsObject()) {
        throw ConfigurationException("'Exchanged data' empty conf");
    }

    const rapidjson::Value &jsonExchangedData = document[JSON_EXCHANGED_DATA];

    if (! jsonExchangedData.HasMember(JSON_DATAPOINTS)) {
        throw ConfigurationException("'ExchangedData' parsing error: no 'datapoints'");
    }

    if (! jsonExchangedData[JSON_DATAPOINTS].IsArray()) {
        throw ConfigurationException("'datapoints' is not an array -> fail to parse 'ExchangedData'");
    }

    /** Parse each 'datapoint' JSON structure. */
    for (const auto &jsonDatapointConfig : jsonExchangedData[JSON_DATAPOINTS].GetArray()) {
        importJsonDatapointConfig(jsonDatapointConfig);
    }
    logExchangedData(exchangedData);
}

void IEC61850ClientConfig::importJsonExchangedDatasetsConfig(const std::string &exchangedDatasetsConfig)
{
    rapidjson::Document document;

    /** Parse the input JSON std::string. */
    if (document.Parse(exchangedDatasetsConfig.c_str()).HasParseError()) {
        throw ConfigurationException("'Exchanged datasets' parsing error");
    }

    /** The 'exchanged_datasets' section is mandatory: throw exception if not found. */
    if (!document.IsObject()) {
        throw ConfigurationException("'Exchanged datasets' empty conf");
    }

    if (!document.HasMember(JSON_EXCHANGED_DATASETS) || !document[JSON_EXCHANGED_DATASETS].IsObject()) {
        throw ConfigurationException("'Exchanged datasets' empty conf");
    }

    const rapidjson::Value &jsonExchangedDatasets = document[JSON_EXCHANGED_DATASETS];

    if (! jsonExchangedDatasets.HasMember(JSON_DATASETS)) {
        throw ConfigurationException("'ExchangedDatasets' parsing error: no 'datasets'");
    }

    if (! jsonExchangedDatasets[JSON_DATASETS].IsArray()) {
        throw ConfigurationException("'datasets' is not an array -> fail to parse 'ExchangedDatasets'");
    }

    /** Parse each 'dataset' JSON structure. */
    for (const auto &jsonDatasetConfig : jsonExchangedDatasets[JSON_DATASETS].GetArray()) {
        importJsonDatasetConfig(jsonDatasetConfig);
    }
}

void IEC61850ClientConfig::importJsonDatapointConfig(const rapidjson::Value &jsonDatapointConfig)
{
    // Preconditions
    if (! jsonDatapointConfig.IsObject()) {
        throw ConfigurationException("'datapoint' is not valid");
    }

    if (! jsonDatapointConfig.HasMember("label")) {
        throw ConfigurationException("the mandatory 'label' not found");
    }
    if (! jsonDatapointConfig["label"].IsString()) {
        throw ConfigurationException("bad format for the mandatory 'label'");
    }

    DatapointConfig newDatapointConfig;
    newDatapointConfig.label = std::string(jsonDatapointConfig["label"].GetString());

    if (! jsonDatapointConfig.HasMember(JSON_PROTOCOLS)) {
        throw ConfigurationException("'datapoints' parsing error: no 'protocols'");
    }

    if (! jsonDatapointConfig[JSON_PROTOCOLS].IsArray()) {
        throw ConfigurationException("'protocols' is not an array -> fail to parse 'datapoints'");
    }

    /** Parse the IEC61850 'protocol' JSON structure of Datapoint configuration. */
    for (const auto &protocol : jsonDatapointConfig[JSON_PROTOCOLS].GetArray()) {
        importJsonDatapointProtocolConfig(protocol, newDatapointConfig);
    }

    exchangedData.push_back(newDatapointConfig);
}

void IEC61850ClientConfig::importJsonDatasetConfig(const rapidjson::Value &jsonDatasetConfig)
{
    // Preconditions
    if (! jsonDatasetConfig.IsObject()) {
        throw ConfigurationException("'dataset' is not valid");
    }
    if (! jsonDatasetConfig.HasMember("dataset_ref")) {
        throw ConfigurationException("the mandatory 'dataset_ref' not found");
    }
    if (! jsonDatasetConfig["dataset_ref"].IsString()) {
        throw ConfigurationException("bad format for the mandatory 'dataset_ref'");
    }
    // end of preconditions

    auto datasetRef = std::string(jsonDatasetConfig["dataset_ref"].GetString());
    if (datasetRef.empty()) {
        throw ConfigurationException("the mandatory 'dataset_ref' is empty");
    }

    std::vector<DatapointConfig> selectedDataObjectList;

    if ( (jsonDatasetConfig.HasMember(JSON_DATA_OBJECTS)) &&
         (jsonDatasetConfig[JSON_DATA_OBJECTS].IsArray())) {
        /** Parse each 'data_objects' JSON structure */
        for (const auto &jsonDataObject : jsonDatasetConfig[JSON_DATA_OBJECTS].GetArray()) {

            DatapointConfig dpConfig;

            if (jsonDataObject.HasMember("label")) {
                dpConfig.label = std::string(jsonDataObject["label"].GetString());
            } else {
                throw ConfigurationException("'label', in 'data_object' of 'dataset' is missing");
            }

            if (jsonDataObject.HasMember("doName")) {
                dpConfig.dataPath = std::string(jsonDataObject["doName"].GetString());
            } else {
                throw ConfigurationException("'do_name', in 'data_object' of 'dataset' is missing");
            }

            if (jsonDataObject.HasMember("typeid")) {
                setDatapointType(jsonDataObject, dpConfig);
            } else {
                throw ConfigurationException("'typeid', in 'data_object' of 'dataset' is missing");
            }

            selectedDataObjectList.push_back(dpConfig);
        }
    }

    selectedDOInExchangedDatasets[datasetRef] = selectedDataObjectList;
}

void IEC61850ClientConfig::importJsonDatapointProtocolConfig(const rapidjson::Value &datapointProtocolConfig,
                                                             DatapointConfig &datapointConfig) const
{
    // Preconditions
    if (! datapointProtocolConfig.IsObject()) {
        throw ConfigurationException("'protocol' is not valid");
    }

    if (! datapointProtocolConfig.HasMember("name")) {
        throw ConfigurationException("the mandatory 'name' not found");
    }
    if (! datapointProtocolConfig["name"].IsString()) {
        throw ConfigurationException("bad format for the mandatory 'name'");
    }

    if (! datapointProtocolConfig.HasMember("typeid")) {
        throw ConfigurationException("the mandatory 'typeid' not found");
    }
    if (! datapointProtocolConfig["typeid"].IsString()) {
        throw ConfigurationException("bad format for the mandatory 'typeid'");
    }

    if (! datapointProtocolConfig.HasMember("address")) {
        throw ConfigurationException("the mandatory 'address' not found");
    }
    if (! datapointProtocolConfig["address"].IsString()) {
        throw ConfigurationException("bad format for the mandatory 'address'");
    }

    if (std::string(datapointProtocolConfig["name"].GetString()) != std::string(IEC61850_PROTOCOL_NAME) ) {
        Logger::getLogger()->debug("Config: ignore the protocol '%s'", datapointProtocolConfig["name"].GetString());
        return;
    }

    datapointConfig.dataPath = datapointProtocolConfig["address"].GetString();

    setDatapointType(datapointProtocolConfig, datapointConfig);
}

void IEC61850ClientConfig::setDatapointType(const rapidjson::Value &jsonConfig,
                                            DatapointConfig &dpConfigToComplete)
{
    std::string strTypeId = jsonConfig["typeid"].GetString();
    if (strTypeId.compare("SPS") == 0) {
        dpConfigToComplete.datapointType = "SPS";
        dpConfigToComplete.datapointTypeId = DatapointTypeId::SPS_DATAPOINT_TYPE;
        dpConfigToComplete.functionalConstraint = FunctionalConstraint_fromString("ST");
    } else if (strTypeId.compare("MV") == 0) {
        dpConfigToComplete.datapointType = "MV";
        dpConfigToComplete.datapointTypeId = DatapointTypeId::MV_DATAPOINT_TYPE;
        dpConfigToComplete.functionalConstraint = FunctionalConstraint_fromString("MX");
    } else {
        Logger::getLogger()->error("Config: datapoint typeid '%s' not supported yet", strTypeId.c_str());
    }
}

bool IEC61850ClientConfig::isValidIPAddress(const std::string &addrStr)
{
    // see https://stackoverflow.com/questions/318236/how-do-you-validate-that-a-string-is-a-valid-ipv4-address-in-c
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, addrStr.c_str(), &(sa.sin_addr));
    return (result == 1);
}

void IEC61850ClientConfig::logExchangedData(const ExchangedData &exchangedData)
{
    Logger::getLogger()->info("Config: Exchanged Data:");

    for (const auto &dpConfig : exchangedData) {
        Logger::getLogger()->info("\tDatapoint: ==== new datapoint ===");
        Logger::getLogger()->info("\tDatapoint: label: %s", dpConfig.label.c_str());
        Logger::getLogger()->info("\tDatapoint: type: %d", dpConfig.datapointTypeId);
        Logger::getLogger()->info("\tDatapoint: dataPath: %s", dpConfig.dataPath.c_str());
        Logger::getLogger()->info("\tDatapoint: FC: %s", FunctionalConstraint_toString(dpConfig.functionalConstraint));

        if (dpConfig.mmsNameTree != nullptr) {
            logMmsNameTree(dpConfig.mmsNameTree.get());
        }
    }
}

void IEC61850ClientConfig::logExchangedDatasets(const ExchangedDatasets &exchangedDatasets)
{
    Logger::getLogger()->info("Config: Exchanged Datasets:");

    for (const auto &datasetDictEntry : exchangedDatasets) {
        const ExchangedData &l_exchangedData = datasetDictEntry.second;

        Logger::getLogger()->info("Config: Dataset: ==== new dataset ===");
        Logger::getLogger()->info("\tDataset: ref: %s", datasetDictEntry.first.c_str());
        logExchangedData(l_exchangedData);
    }
}

void IEC61850ClientConfig::logMmsNameTree(const MmsNameNode *mmsNameNode, uint8_t currentDepth)
{
    // preconditions
    if (mmsNameNode == nullptr) {
        return;
    }

    std::string padding;
    for (uint8_t i = 0; i < currentDepth; i++) {padding.append("\t");}

    Logger::getLogger()->info("\tDatapoint: %sMmsName: %s", padding.c_str(), mmsNameNode->mmsName.c_str());

    for (const auto &child : mmsNameNode->children) {
        if (child) {
            logMmsNameTree(child.get(), currentDepth + 1);
        }
    }
}
