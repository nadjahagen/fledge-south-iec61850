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
#include <vector>
#include <map>
#include <memory>

// Fledge headers
#include <config_category.h>

// libiec61850 headers
#include <libiec61850/libiec61850_common_api.h>
#include <libiec61850/iec61850_common.h>
#include <libiec61850/iso_connection_parameters.h>

#include <rapidjson/document.h>

// For white box unit tests
#include <gtest/gtest_prod.h>

constexpr unsigned int DEFAULT_READ_POLLING_PERIOD_IN_MS = 1000;

/**
 *  \brief Lower layer parameters (below the MMS layer) for connection with server
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
 *  \brief Parameters for creating a connection with 1 IEC61850 server
 */
struct ServerConnectionParameters {
    std::string ipAddress;
    int mmsPort{0};
    bool isOsiParametersEnabled{false};
    OsiParameters osiParameters;
};


/**
 *  \brief Defined types of Datapoint
 */
enum class DatapointTypeId {
   MV_DATAPOINT_TYPE = 0,
   SPS_DATAPOINT_TYPE = 1,
   UNKNOWN_DATAPOINT_TYPE = -1
};

/**
 *  \brief Node for the MMS name tree (name for each element of a complex MMS)
 */
struct MmsNameNode {
    std::string mmsName;
    std::vector<std::shared_ptr<const MmsNameNode>> children;
};

/**
 *  \brief Mode of the reading activity: list of DO or list of Dataset
 */
enum class ReadMode {
    DO_READING = 0,
    DATASET_READING
};

/**
 *  \brief Application parameters about the IEC61850 client
 */
struct ApplicationParameters {
    unsigned int readPollingPeriodInMs = DEFAULT_READ_POLLING_PERIOD_IN_MS;  /** Default polling period: 1 second */
    ReadMode readMode = ReadMode::DO_READING;  /** Default reading mode: DO, not dataset */
};

using OsiSelectorSize = uint8_t;
using ServerDictKey = std::string;
using ServerConfigDict = std::map<ServerDictKey, ServerConnectionParameters, std::less<>>;
using DatapointLabel = std::string;
using DatapointTypeStr = std::string;
using DataPath = std::string;
using DatasetRef = std::string;

/**
 *  \brief Parameters about the data to transfer to Fledge
 */
struct DatapointConfig {
    DatapointLabel label;  /**< name for the Datapoint */
    DatapointTypeStr datapointType; /**< to define the structure of the datapoint */
    DatapointTypeId datapointTypeId = DatapointTypeId::UNKNOWN_DATAPOINT_TYPE;
    DataPath dataPath = "NOT_DEFINED";  /**< Object path in the IEC61850 data mode */
    FunctionalConstraint functionalConstraint = IEC61850_FC_NONE;
    std::shared_ptr<MmsNameNode> mmsNameTree = nullptr;  /**< name of each subelement of the MMS and datapoint */
};

/**
 *  \brief Collection of Datapoint configuration, extracted from the input JSON configuration
 */
using ExchangedData = std::vector<DatapointConfig>;
using ExchangedDatasets = std::map<DatasetRef, ExchangedData, std::less<>>;

/** \class ConfigurationException
 *  \brief Error in the input configuration
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

        ApplicationParameters applicationParams;

        // Data model section
        ExchangedData exchangedData;
        ExchangedDatasets selectedDOInExchangedDatasets;

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
        static void logExchangedData(const ExchangedData &exchangedData);
        static void logExchangedDatasets(const ExchangedDatasets &exchangedDatasets);
        static void logMmsNameTree(const MmsNameNode *mmsNameNode, uint8_t currentDepth = 0);

    private:
        void importJsonProtocolConfig(const std::string &protocolConfig);
        void importJsonTransportLayerConfig(const rapidjson::Value &transportLayer);
        void importJsonConnectionConfig(const rapidjson::Value &connConfig);
        void importJsonConnectionOsiConfig(const rapidjson::Value &connOsiConfig,
                                           ServerConnectionParameters &iedConnectionParam) const;
        void importJsonConnectionOsiSelectors(const rapidjson::Value &connOsiConfig,
                                              OsiParameters *osiParams) const;
        void importJsonApplicationLayerConfig(const rapidjson::Value &applicationLayer);
        void importJsonExchangedDataConfig(const std::string &exchangedDataConfig);
        void importJsonExchangedDatasetsConfig(const std::string &exchangedDatasetsConfig);
        void importJsonDatapointConfig(const rapidjson::Value &jsonDatapointConfig);
        void importJsonDatasetConfig(const rapidjson::Value &jsonDatasetConfig);
        void importJsonDatapointProtocolConfig(const rapidjson::Value &datapointProtocolConfig,
                                               DatapointConfig &datapointConfig) const;
        static void setDatapointType(const rapidjson::Value &jsonConfig,
                                     DatapointConfig &dpConfigToComplete);

        static OsiSelectorSize parseOsiPSelector(std::string &inputOsiSelector, PSelector *pselector);
        static OsiSelectorSize parseOsiTSelector(std::string &inputOsiSelector, TSelector *tselector);
        static OsiSelectorSize parseOsiSSelector(std::string &inputOsiSelector, SSelector *sselector);
        static OsiSelectorSize parseOsiSelector(std::string &inputOsiSelector,
                                                uint8_t *selectorValue,
                                                const uint8_t selectorSize);

        static bool isValidIPAddress(const std::string &addrStr);

        // Section: see the class as a white box for unit tests
        FRIEND_TEST(IEC61850ClientConfigTest, importValidExchangedData);
        FRIEND_TEST(IEC61850ClientConfigTest, importExchangedDataWithParsingError);
        FRIEND_TEST(IEC61850ClientConfigTest, importExchangedDataWithMissingSection);
        FRIEND_TEST(IEC61850ClientConfigTest, importExchangedDataWithMissingDatapointSection);
        FRIEND_TEST(IEC61850ClientConfigTest, importExchangedDataWithDatapointNotArray);
        FRIEND_TEST(IEC61850ClientConfigTest, importDatapointWithMissingLabel);
        FRIEND_TEST(IEC61850ClientConfigTest, importDatapointWithBadFormatLabel);
        FRIEND_TEST(IEC61850ClientConfigTest, importDatapointWithSameLabel);
        FRIEND_TEST(IEC61850ClientConfigTest, importDatapointWithMissingProtocol);
        FRIEND_TEST(IEC61850ClientConfigTest, importDatapointWithProtocolNotArray);
        FRIEND_TEST(IEC61850ClientConfigTest, importDatapointWithMissingMandatoryName);
        FRIEND_TEST(IEC61850ClientConfigTest, importDatapointWithNameBadFormat);
        FRIEND_TEST(IEC61850ClientConfigTest, importDatapointWithMissingMandatoryTypeId);
        FRIEND_TEST(IEC61850ClientConfigTest, importDatapointWithTypeIdBadFormat);
        FRIEND_TEST(IEC61850ClientConfigTest, importDatapointWithMissingMandatoryAddress);
        FRIEND_TEST(IEC61850ClientConfigTest, importDatapointWithAddressBadFormat);
        FRIEND_TEST(IEC61850ClientConfigTest, importValidExchangedDataWithIgnoredProtocols);
};

#endif  // INCLUDE_IEC61850_CLIENT_CONFIG_H_
