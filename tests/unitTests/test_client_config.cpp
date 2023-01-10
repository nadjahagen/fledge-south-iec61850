#include <string>

#include <gtest/gtest.h>

#include <rapidjson/document.h>

// Fledge headers
#include <config_category.h>
#include <plugin_api.h>

#include "plugin.h"
#include "iec61850_client_config.h"

#include "../common/configuration_examples.h"

TEST(IEC61850ClientConfigTest, importDefaultConfig)
{
    ConfigCategory config("TestDefaultConfig", default_config);
    config.setItemsValueFromDefault();
    IEC61850ClientConfig clientConfig;
    ASSERT_NO_THROW(clientConfig.importConfig(config));
    ASSERT_EQ(clientConfig.logMinLevel, "info");
    ASSERT_EQ(clientConfig.assetName, "iec61850");
    ASSERT_EQ(clientConfig.iedName, "simpleIO");
    ASSERT_EQ(clientConfig.serverConfigDict.size(), 2);
    ASSERT_EQ(clientConfig.serverConfigDict["0.0.0.0_102"].ipAddress, "0.0.0.0");
    ASSERT_EQ(clientConfig.serverConfigDict["0.0.0.0_102"].mmsPort, 102);
    ASSERT_EQ(clientConfig.serverConfigDict["0.0.0.0_8102"].ipAddress, "0.0.0.0");
    ASSERT_EQ(clientConfig.serverConfigDict["0.0.0.0_8102"].mmsPort, 8102);
}

TEST(IEC61850ClientConfigTest, importConfigWithoutLogLevel)
{
    ConfigCategory config("Config", configWithoutLogLevel);
    IEC61850ClientConfig clientConfig;
    ASSERT_NO_THROW(clientConfig.importConfig(config));
    ASSERT_EQ(clientConfig.logMinLevel, "info");
}

TEST(IEC61850ClientConfigTest, importConfigMissingProtocolStack)
{
    ConfigCategory config("Config", configMissingProtocolStack);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'Protocol stack' not found");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackWithParsingError)
{
    ConfigCategory config("Config", configProtocolStackWithParsingError);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'Protocol stack' parsing error");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackEmptyConf)
{
    ConfigCategory config("Config", configProtocolStackEmptyConf);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'Protocol stack' empty conf");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackMissingTransport)
{
    ConfigCategory config("Config", configProtocolStackMissingTransport);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'transport layer' configuration is missing");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackMissingApplication)
{
    ConfigCategory config("Config", configProtocolStackMissingApplication);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'application layer' configuration is missing");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackMissingIedName)
{
    ConfigCategory config("Config", configProtocolStackMissingIedName);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: the mandatory 'ied_name' not found");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackIedNameBadFormat)
{
    ConfigCategory config("Config", configProtocolStackIedNameBadFormat);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for the mandatory 'ied_name'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackMissingConnection)
{
    ConfigCategory config("Config", configProtocolStackMissingConnection);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'Transport Layer' parsing error: no 'connections'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackConnectionBadFormat)
{
    ConfigCategory config("Config", configProtocolStackConnectionBadFormat);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'connections' is not an array -> fail to parse 'Transport Layer'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackMissingSrvIp)
{
    ConfigCategory config("Config", configProtocolStackMissingSrvIp);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: the mandatory 'srv_ip' not found");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackgSrvIpBadFormat)
{
    ConfigCategory config("Config", configProtocolStackSrvIpBadFormat);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for the mandatory 'srv_ip'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackMissingPort)
{
    ConfigCategory config("Config", configProtocolStackMissingPort);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: the mandatory 'port' not found");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackPortBadFormat)
{
    ConfigCategory config("Config", configProtocolStackPortBadFormat);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for the mandatory 'port'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigProtocolStackInvalidIPAddress)
{
    ConfigCategory config("Config", configProtocolStackInvalidIPAddress);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: not a valid IP address for the mandatory 'srv_ip'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importValidOsiConnectionParameters)
{
    ConfigCategory config("Config", configValidOsiConnectionParameters);
    IEC61850ClientConfig clientConfig;

    ASSERT_NO_THROW(clientConfig.importConfig(config));
    ASSERT_EQ(clientConfig.serverConfigDict["0.0.0.0_102"].isOsiParametersEnabled, true);
    ASSERT_EQ(clientConfig.serverConfigDict["0.0.0.0_8102"].isOsiParametersEnabled, false);

    OsiParameters *osiParameters = &clientConfig.serverConfigDict["0.0.0.0_102"].osiParameters;

    ASSERT_STREQ(osiParameters->localApTitle.c_str(), "1.3.9999.13");
    ASSERT_EQ(osiParameters->localAeQualifier, 12);
    ASSERT_STREQ(osiParameters->remoteApTitle.c_str(), "1.2.1200.15.3");
    ASSERT_EQ(osiParameters->remoteAeQualifier, 1);
    ASSERT_EQ(osiParameters->localTSelector.size, 3);
    ASSERT_EQ(osiParameters->localTSelector.value[0], 0);
    ASSERT_EQ(osiParameters->localTSelector.value[1], 1);
    ASSERT_EQ(osiParameters->localTSelector.value[2], 2);
    ASSERT_EQ(osiParameters->remoteTSelector.size, 2);
    ASSERT_EQ(osiParameters->remoteTSelector.value[0], 0);
    ASSERT_EQ(osiParameters->remoteTSelector.value[1], 1);
    ASSERT_EQ(osiParameters->localSSelector.size, 5);
    ASSERT_EQ(osiParameters->localSSelector.value[0], 0);
    ASSERT_EQ(osiParameters->localSSelector.value[1], 1);
    ASSERT_EQ(osiParameters->localSSelector.value[2], 2);
    ASSERT_EQ(osiParameters->localSSelector.value[3], 3);
    ASSERT_EQ(osiParameters->localSSelector.value[4], 4);
    ASSERT_EQ(osiParameters->remoteSSelector.size, 2);
    ASSERT_EQ(osiParameters->remoteSSelector.value[0], 0);
    ASSERT_EQ(osiParameters->remoteSSelector.value[1], 1);
    ASSERT_EQ(osiParameters->localPSelector.size, 4);
    ASSERT_EQ(osiParameters->localPSelector.value[0], 0x12);
    ASSERT_EQ(osiParameters->localPSelector.value[1], 0x34);
    ASSERT_EQ(osiParameters->localPSelector.value[2], 0x56);
    ASSERT_EQ(osiParameters->localPSelector.value[3], 0x78);
    ASSERT_EQ(osiParameters->remotePSelector.size, 4);
    ASSERT_EQ(osiParameters->remotePSelector.value[0], 0x87);
    ASSERT_EQ(osiParameters->remotePSelector.value[1], 0x65);
    ASSERT_EQ(osiParameters->remotePSelector.value[2], 0x43);
    ASSERT_EQ(osiParameters->remotePSelector.value[3], 0x21);
}

TEST(IEC61850ClientConfigTest, importConfigOsiConnectionBadLocalAe)
{
    ConfigCategory config("Config", configOsiConnectionBadLocalAe);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for 'local_ae_qualifier'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigOsiConnectionBadRemoteAe)
{
    ConfigCategory config("Config", configOsiConnectionBadRemoteAe);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for 'remote_ae_qualifier'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigOsiConnectionBadLocalApTitle)
{
    ConfigCategory config("Config", configOsiConnectionBadLocalAp);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for 'local_ap_title'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigOsiConnectionBadRemoteApTitle)
{
    ConfigCategory config("Config", configOsiConnectionBadRemoteAp);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for 'remote_ap_title'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigOsiConnectionInvalidLocalApTitle)
{
    ConfigCategory config("Config", configOsiConnectionInvalidLocalAp);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'local_ap_title' is not valid");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigOsiConnectionInvalidRemoteApTitle)
{
    ConfigCategory config("Config", configOsiConnectionInvalidRemoteAp);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'remote_ap_title' is not valid");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigOsiConnectionSelectorTooLong)
{
    ConfigCategory config("Config", configOsiConnectionSelectorTooLong);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for 'OSI Selector' (too many bytes)");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigOsiConnectionSelectorNotAByte1)
{
    ConfigCategory config("Config", configOsiConnectionSelectorNotAByte1);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for 'OSI Selector' (not a byte)");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importConfigOsiConnectionSelectorNotAByte2)
{
    ConfigCategory config("Config", configOsiConnectionSelectorNotAByte2);
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importConfig(config);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for 'OSI Selector' (exceed a byte)");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importValidExchangedData)
{
    IEC61850ClientConfig clientConfig;

    ASSERT_NO_THROW(clientConfig.importJsonExchangedDataConfig(validExchangedData));

    ASSERT_EQ(clientConfig.exchangedData.size(), 2);
    ASSERT_EQ(clientConfig.exchangedData[1].label, "TM1");
    ASSERT_EQ(clientConfig.exchangedData[1].datapointType, "MV");
    ASSERT_EQ(clientConfig.exchangedData[1].datapointTypeId, DatapointTypeId::MV_DATAPOINT_TYPE);
    ASSERT_EQ(clientConfig.exchangedData[1].dataPath, "simpleIOGenericIO/GGIO1.AnIn1");
    ASSERT_EQ(clientConfig.exchangedData[1].functionalConstraint, IEC61850_FC_MX);
//    ASSERT_EQ(clientConfig.exchangedData[1].mmsNameTree->mmsName, "TM1");
//    ASSERT_EQ(clientConfig.exchangedData[1].mmsNameTree->children.size(), 3);
//    ASSERT_EQ(clientConfig.exchangedData[1].mmsNameTree->children[0]->mmsName, "mag");
//    ASSERT_EQ(clientConfig.exchangedData[1].mmsNameTree->children[1]->mmsName, "q");
//    ASSERT_EQ(clientConfig.exchangedData[1].mmsNameTree->children[2]->mmsName, "t");

    ASSERT_EQ(clientConfig.exchangedData[0].label, "TS1");
    ASSERT_EQ(clientConfig.exchangedData[0].datapointType, "SPS");
    ASSERT_EQ(clientConfig.exchangedData[0].datapointTypeId, DatapointTypeId::SPS_DATAPOINT_TYPE);
    ASSERT_EQ(clientConfig.exchangedData[0].dataPath, "simpleIOGenericIO/GGIO1.Ind1");
    ASSERT_EQ(clientConfig.exchangedData[0].functionalConstraint, IEC61850_FC_ST);
//    ASSERT_EQ(clientConfig.exchangedData[0].mmsNameTree->mmsName, "TS1");
//    ASSERT_EQ(clientConfig.exchangedData[0].mmsNameTree->children.size(), 3);
//    ASSERT_EQ(clientConfig.exchangedData[0].mmsNameTree->children[0]->mmsName, "stVal");
//    ASSERT_EQ(clientConfig.exchangedData[0].mmsNameTree->children[1]->mmsName, "q");
//    ASSERT_EQ(clientConfig.exchangedData[0].mmsNameTree->children[2]->mmsName, "t");
}

TEST(IEC61850ClientConfigTest, importExchangedDataWithParsingError)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(exchangedDataWithParsingError);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'Exchanged data' parsing error");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importExchangedDataWithMissingSection)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(missingExchangedDataSection);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'Exchanged data' empty conf");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importExchangedDataWithMissingDatapointSection)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(missingDatapointSection);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'ExchangedData' parsing error: no 'datapoints'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importExchangedDataWithDatapointNotArray)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(datapointNotArray);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'datapoints' is not an array -> fail to parse 'ExchangedData'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importDatapointWithMissingLabel)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(datapointWithMissingLabel);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: the mandatory 'label' not found");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importDatapointWithBadFormatLabel)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(datapointWithBadFormatLabel);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for the mandatory 'label'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importDatapointWithMissingProtocol)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(datapointWithMissingProtocol);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'datapoints' parsing error: no 'protocols'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importDatapointWithProtocolNotArray)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(datapointWithProtocolNotArray);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: 'protocols' is not an array -> fail to parse 'datapoints'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importDatapointWithMissingMandatoryName)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(datapointWithMissingMandatoryName);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: the mandatory 'name' not found");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importDatapointWithNameBadFormat)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(datapointWithNameBadFormat);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for the mandatory 'name'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importDatapointWithMissingMandatoryTypeId)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(datapointWithMissingMandatoryTypeId);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: the mandatory 'typeid' not found");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importDatapointWithTypeIdBadFormat)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(datapointWithTypeIdBadFormat);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for the mandatory 'typeid'");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importDatapointWithMissingMandatoryAddress)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(datapointWithMissingMandatoryAddress);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: the mandatory 'address' not found");
    } catch (...) {
        FAIL();
    }
}

TEST(IEC61850ClientConfigTest, importDatapointWithAddressBadFormat)
{
    IEC61850ClientConfig clientConfig;

    try {
        clientConfig.importJsonExchangedDataConfig(datapointWithAddressBadFormat);
        FAIL();
    } catch (ConfigurationException e) {
        ASSERT_STREQ(e.what(), "Configuration exception: bad format for the mandatory 'address'");
    } catch (...) {
        FAIL();
    }
}


TEST(IEC61850ClientConfigTest, importValidExchangedDataWithIgnoredProtocols)
{
    IEC61850ClientConfig clientConfig;

    ASSERT_NO_THROW(clientConfig.importJsonExchangedDataConfig(validExchangedDataWithIgnoredProtocols));

    ASSERT_EQ(clientConfig.exchangedData.size(), 1);
    ASSERT_EQ(clientConfig.exchangedData[0].label, "TS1");
    ASSERT_EQ(clientConfig.exchangedData[0].datapointType, "SPS");
    ASSERT_EQ(clientConfig.exchangedData[0].datapointTypeId, DatapointTypeId::SPS_DATAPOINT_TYPE);
    ASSERT_EQ(clientConfig.exchangedData[0].dataPath, "path for iec61850 model");
}

