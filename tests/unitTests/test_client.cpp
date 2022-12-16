#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Fledge headers
#include <config_category.h>

#include "iec61850_client.h"
#include "iec61850_client_config.h"
#include "mock_iec61850_client_connection.h"

using namespace ::testing;

class IEC61850ClientTest_injectMockConnection_Test
{
    public:
        static void injectMockConnection(IEC61850Client &client,
                                         IEC61850ClientConnectionInterface *mockConnection)
        {
            while (client.m_connection) {
                // wait the previous 'connection' object is destroyed
            }

            client.m_connection = std::unique_ptr<IEC61850ClientConnectionInterface>(mockConnection);
        }
};


TEST(IEC61850ClientTest, createOneConnection)
{
    ServerConnectionParameters connParam;
    ExchangedData exchangedData;
    ExchangedDatasets exchangedDatasets;
    ApplicationParameters applicationParams;
    IEC61850Client client(nullptr,
                          connParam,
                          exchangedData,
                          exchangedDatasets,
                          applicationParams);
    client.createConnection();
    ASSERT_THAT(client.m_connection, NotNull());
}


TEST(IEC61850ClientTest, reuseCreatedConnection)
{
    ServerConnectionParameters connParam;
    ExchangedData exchangedData;
    ExchangedDatasets exchangedDatasets;
    ApplicationParameters applicationParams;
    IEC61850Client client(nullptr,
                          connParam,
                          exchangedData,
                          exchangedDatasets,
                          applicationParams);
    // Create connection
    client.createConnection();
    IEC61850ClientConnectionInterface *connection_pointer = client.m_connection.get();
    // Reuse the already created connection
    client.createConnection();
    ASSERT_EQ(client.m_connection.get(), connection_pointer);
}

TEST(IEC61850ClientTest, destroyConnection)
{
    ServerConnectionParameters connParam;
    ExchangedData exchangedData;
    ExchangedDatasets exchangedDatasets;
    ApplicationParameters applicationParams;
    IEC61850Client client(nullptr,
                          connParam,
                          exchangedData,
                          exchangedDatasets,
                          applicationParams);
    client.createConnection();
    client.destroyConnection();
    ASSERT_THAT(client.m_connection, IsNull());
}

TEST(IEC61850ClientTest, destroyNullConnection)
{
    ServerConnectionParameters connParam;
    ExchangedData exchangedData;
    ExchangedDatasets exchangedDatasets;
    ApplicationParameters applicationParams;
    IEC61850Client client(nullptr,
                          connParam,
                          exchangedData,
                          exchangedDatasets,
                          applicationParams);
    client.createConnection();
    client.destroyConnection();
    ASSERT_NO_THROW(client.destroyConnection());
    ASSERT_THAT(client.m_connection, IsNull());
}

TEST(IEC61850ClientTest, initializeConnectionInOneTry)
{
    // Configuration of the Mock objects
    MockIEC61850ClientConnection mockConnectedConnection;
    EXPECT_CALL(mockConnectedConnection, isConnected())
    .Times(3)
    .WillRepeatedly(Return(true));
    // End of configuration of the Mock objects
    // Test Init
    ServerConnectionParameters connParam;
    ExchangedData exchangedData;
    ExchangedDatasets exchangedDatasets;
    ApplicationParameters applicationParams;
    IEC61850Client client(nullptr,
                          connParam,
                          exchangedData,
                          exchangedDatasets,
                          applicationParams);
    client.createConnection(); // create a Foo connection before MockConnection injection
    // Test Body
    std::thread initializeConnectionThread(&IEC61850Client::initializeConnection,
                                           &client);
    IEC61850ClientTest_injectMockConnection_Test::injectMockConnection(client, &mockConnectedConnection);
    ASSERT_THAT(client.m_connection, NotNull());
    ASSERT_EQ(true, client.m_connection->isConnected());
    // Test teardown
    initializeConnectionThread.join();
    // Do not delete the MockConnection object, through the unique_ptr
    client.m_connection.release();
}

TEST(IEC61850ClientTest, initializeConnectionFailed)
{
    // Test Init
    ServerConnectionParameters connParam;
    ExchangedData exchangedData;
    ExchangedDatasets exchangedDatasets;
    ApplicationParameters applicationParams;
    IEC61850Client client(nullptr,
                          connParam,
                          exchangedData,
                          exchangedDatasets,
                          applicationParams);
    // Test Body
    ASSERT_THAT(client.m_connection, IsNull());
    std::thread initializeConnectionThread(&IEC61850Client::initializeConnection,
                                           &client);
    sleep(3);
    client.m_stopOrder = true;
    ASSERT_THAT(client.m_connection, NotNull());
    ASSERT_EQ(false, client.m_connection->isConnected());
    // Test teardown
    initializeConnectionThread.join();
}

TEST(IEC61850ClientTest, startAndStop)
{
    // Configuration of the Mock objects
    auto empty_mms = std::make_shared<WrappedMms>();
    empty_mms->setMmsValue(nullptr);
    MockIEC61850ClientConnection mockConnectedConnection;
    EXPECT_CALL(mockConnectedConnection, isConnected())
    .Times(5)
    .WillRepeatedly(Return(true));
    EXPECT_CALL(mockConnectedConnection, readDO(_, _))
    .Times(2)
    .WillRepeatedly(Return(empty_mms));
    EXPECT_CALL(mockConnectedConnection, isNoError())
    .WillRepeatedly(Return(true));
    // End of configuration of the Mock objects
    // Test Init
    ServerConnectionParameters connParam;
    ExchangedData exchangedData;
    ExchangedDatasets exchangedDatasets;
    DatapointConfig dpConfig;
    dpConfig.dataPath = "Foo.DoPath";
    exchangedData.push_back(dpConfig);
    ApplicationParameters applicationParams;
    IEC61850Client client(nullptr,
                          connParam,
                          exchangedData,
                          exchangedDatasets,
                          applicationParams);
    // Test Body
    ASSERT_THAT(client.m_connection, IsNull());
    ASSERT_EQ(false, client.m_stopOrder);
    ASSERT_EQ(false, client.m_backgroundLaunchThread.joinable());
    client.createConnection(); // create a Foo connection before MockConnection injection
    std::thread startClientThread(&IEC61850Client::start,
                                  &client);
    IEC61850ClientTest_injectMockConnection_Test::injectMockConnection(client, &mockConnectedConnection);
    sleep(3);
    ASSERT_THAT(client.m_connection, NotNull());
    ASSERT_EQ(true, client.m_connection->isConnected());
    ASSERT_EQ(false, client.m_stopOrder);
    // Do not delete the MockConnection object, through the unique_ptr
    client.m_connection.release();
    client.stop();
    ASSERT_EQ(true, client.m_stopOrder);
    ASSERT_THAT(client.m_connection, IsNull());
    // Test teardown
    startClientThread.join();
}

TEST(IEC61850ClientTest, buildIntegerDatapoint)
{
    ServerConnectionParameters connParam;
    ApplicationParameters applicationParams;
    DatapointConfig dpConfig;
    dpConfig.mmsNameTree.mmsName = "my_int";

    MmsValue *mmsValue = MmsValue_newInteger(32);
    MmsValue_setInt32(mmsValue, -31416);
    auto wrappedMms = std::make_shared<WrappedMms>();
    wrappedMms->setMmsValue(mmsValue);

    auto dp = IEC61850Client::convertMmsToDatapoint(wrappedMms->getMmsValue(), dpConfig);

    ASSERT_EQ(dp->getName(), "my_int");
    ASSERT_EQ(dp->getData().getTypeStr(), "INTEGER");
    ASSERT_EQ(dp->getData().toInt(), -31416);
}

TEST(IEC61850ClientTest, buildUnsignedIntegerDatapoint)
{
    ServerConnectionParameters connParam;
    ApplicationParameters applicationParams;
    DatapointConfig dpConfig;
    dpConfig.mmsNameTree.mmsName = "my_uint";

    MmsValue *mmsValue = MmsValue_newUnsigned(32);
    MmsValue_setUint32(mmsValue, -31416);
    auto wrappedMms = std::make_shared<WrappedMms>();
    wrappedMms->setMmsValue(mmsValue);

    auto dp = IEC61850Client::convertMmsToDatapoint(wrappedMms->getMmsValue(), dpConfig);

    ASSERT_EQ(dp->getName(), "my_uint");
    ASSERT_EQ(dp->getData().getTypeStr(), "INTEGER");
    ASSERT_EQ(dp->getData().toInt(), 4294935880);
}

TEST(IEC61850ClientTest, buildBoolDatapoint)
{
    ServerConnectionParameters connParam;
    ApplicationParameters applicationParams;
    DatapointConfig dpConfig;
    dpConfig.mmsNameTree.mmsName = "stVal";

    MmsValue *mmsValue = MmsValue_newBoolean(false);
    auto wrappedMms = std::make_shared<WrappedMms>();
    wrappedMms->setMmsValue(mmsValue);

    auto dp = IEC61850Client::convertMmsToDatapoint(wrappedMms->getMmsValue(), dpConfig);

    ASSERT_EQ(dp->getName(), "do_value");
    ASSERT_EQ(dp->getData().getTypeStr(), "INTEGER");
    ASSERT_EQ(dp->getData().toInt(), 0);
}

TEST(IEC61850ClientTest, buildFloatDatapoint)
{
    ServerConnectionParameters connParam;
    ApplicationParameters applicationParams;
    DatapointConfig dpConfig;
    dpConfig.mmsNameTree.mmsName = "stVal";

    MmsValue *mmsValue = MmsValue_newFloat(-3.1416);
    auto wrappedMms = std::make_shared<WrappedMms>();
    wrappedMms->setMmsValue(mmsValue);

    auto dp = IEC61850Client::convertMmsToDatapoint(wrappedMms->getMmsValue(), dpConfig);

    ASSERT_EQ(dp->getName(), "do_value");
    ASSERT_EQ(dp->getData().getTypeStr(), "FLOAT");
    ASSERT_LT(abs(-3.1416 - dp->getData().toDouble()), 10e-7);
}

TEST(IEC61850ClientTest, buildDoubleDatapoint)
{
    ServerConnectionParameters connParam;
    ApplicationParameters applicationParams;
    DatapointConfig dpConfig;
    dpConfig.mmsNameTree.mmsName = "stVal";

    MmsValue *mmsValue = MmsValue_newDouble(-3.1416e-7);
    auto wrappedMms = std::make_shared<WrappedMms>();
    wrappedMms->setMmsValue(mmsValue);

    auto dp = IEC61850Client::convertMmsToDatapoint(wrappedMms->getMmsValue(), dpConfig);

    ASSERT_EQ(dp->getName(), "do_value");
    ASSERT_EQ(dp->getData().getTypeStr(), "FLOAT");
    ASSERT_LT(abs(-3.1416e-7 - dp->getData().toDouble()), 10e-15);
}

TEST(IEC61850ClientTest, buildTimestampDatapoint)
{
    ServerConnectionParameters connParam;
    ApplicationParameters applicationParams;
    DatapointConfig dpConfig;
    dpConfig.mmsNameTree.mmsName = "t";

    MmsValue *mmsValue = MmsValue_newUtcTime(1670316432);
    auto wrappedMms = std::make_shared<WrappedMms>();
    wrappedMms->setMmsValue(mmsValue);

    auto dp = IEC61850Client::convertMmsToDatapoint(wrappedMms->getMmsValue(), dpConfig);

    ASSERT_EQ(dp->getName(), "do_ts");
    ASSERT_EQ(dp->getData().getTypeStr(), "INTEGER");
    ASSERT_EQ(dp->getData().toInt(), 1670316432);
}

TEST(IEC61850ClientTest, buildBitStringDatapoint)
{
    ServerConnectionParameters connParam;
    ApplicationParameters applicationParams;
    DatapointConfig dpConfig;
    dpConfig.mmsNameTree.mmsName = "q";

    MmsValue *mmsValue = MmsValue_newBitString(27);
    MmsValue_setBitStringFromInteger(mmsValue, 1026); // "0b10000000010"
    auto wrappedMms = std::make_shared<WrappedMms>();
    wrappedMms->setMmsValue(mmsValue);

    auto dp = IEC61850Client::convertMmsToDatapoint(wrappedMms->getMmsValue(), dpConfig);

    ASSERT_EQ(dp->getName(), "do_quality");
    ASSERT_EQ(dp->getData().getTypeStr(), "STRING");
    ASSERT_EQ(dp->getData().toStringValue(), "010000000010000000000000000");
}

TEST(IEC61850ClientTest, buildVisibleStringDatapoint)
{
    ServerConnectionParameters connParam;
    ApplicationParameters applicationParams;
    DatapointConfig dpConfig;
    dpConfig.mmsNameTree.mmsName = "str";

    MmsValue *mmsValue = MmsValue_newVisibleString("fooStr");
    auto wrappedMms = std::make_shared<WrappedMms>();
    wrappedMms->setMmsValue(mmsValue);

    auto dp = IEC61850Client::convertMmsToDatapoint(wrappedMms->getMmsValue(), dpConfig);

    ASSERT_EQ(dp->getName(), "str");
    ASSERT_EQ(dp->getData().getTypeStr(), "STRING");
    ASSERT_EQ(dp->getData().toStringValue(), "fooStr");
}

TEST(IEC61850ClientTest, buildComplexDatapoint)
{
    ServerConnectionParameters connParam;
    ApplicationParameters applicationParams;
    DatapointConfig dpConfig;
    dpConfig.datapointType = "my_cdc";
    dpConfig.mmsNameTree.mmsName = "complexDp";

    auto node1 = std::make_shared<MmsNameNode>();
    node1->mmsName = "1";
    dpConfig.mmsNameTree.children.push_back(node1);

    auto node2 = std::make_shared<MmsNameNode>();
    node2->mmsName = "2";
    dpConfig.mmsNameTree.children.push_back(node2);

    auto node3 = std::make_shared<MmsNameNode>();
    node3->mmsName = "3";
    dpConfig.mmsNameTree.children.push_back(node3);

    MmsValue *mmsValueArray = MmsValue_createEmptyArray(3);

    MmsValue *mmsValue1 = MmsValue_newVisibleString("first");
    MmsValue *mmsValue2 = MmsValue_newVisibleString("second");
    MmsValue *mmsValue3 = MmsValue_newVisibleString("third");

    MmsValue_setElement(mmsValueArray, 0, mmsValue1);
    MmsValue_setElement(mmsValueArray, 1, mmsValue2);
    MmsValue_setElement(mmsValueArray, 2, mmsValue3);

    auto wrappedMms = std::make_shared<WrappedMms>();
    wrappedMms->setMmsValue(mmsValueArray);

    auto dp = IEC61850Client::convertMmsToDatapoint(wrappedMms->getMmsValue(), dpConfig);

    ASSERT_EQ(dp->getName(), "complexDp");
    ASSERT_EQ(dp->getData().getTypeStr(), "DP_DICT");
    ASSERT_EQ(dp->getData().getDpVec()->size(), 4);
    ASSERT_EQ(dp->getData().getDpVec()->at(0)->getName(), "do_type");
    ASSERT_EQ(dp->getData().getDpVec()->at(0)->getData().getTypeStr(), "STRING");
    ASSERT_EQ(dp->getData().getDpVec()->at(0)->getData().toStringValue(), "my_cdc");

    ASSERT_EQ(dp->getData().getDpVec()->at(1)->getName(), "1");
    ASSERT_EQ(dp->getData().getDpVec()->at(1)->getData().getTypeStr(), "STRING");
    ASSERT_EQ(dp->getData().getDpVec()->at(1)->getData().toStringValue(), "first");

    ASSERT_EQ(dp->getData().getDpVec()->at(2)->getName(), "2");
    ASSERT_EQ(dp->getData().getDpVec()->at(2)->getData().getTypeStr(), "STRING");
    ASSERT_EQ(dp->getData().getDpVec()->at(2)->getData().toStringValue(), "second");

    ASSERT_EQ(dp->getData().getDpVec()->at(3)->getName(), "3");
    ASSERT_EQ(dp->getData().getDpVec()->at(3)->getData().getTypeStr(), "STRING");
    ASSERT_EQ(dp->getData().getDpVec()->at(3)->getData().toStringValue(), "third");
}

TEST(IEC61850ClientTest, buildComplexMxDatapoint)
{
    ServerConnectionParameters connParam;
    ApplicationParameters applicationParams;
    DatapointConfig dpConfig;
    dpConfig.datapointType = "my_cdc";
    dpConfig.mmsNameTree.mmsName = "complexDp";

    auto subnode1 = std::make_shared<MmsNameNode>();
    subnode1->mmsName = "float or int";

    auto node1 = std::make_shared<MmsNameNode>();
    node1->mmsName = "mag";
    node1->children.push_back(subnode1);
    dpConfig.mmsNameTree.children.push_back(node1);

    auto node2 = std::make_shared<MmsNameNode>();
    node2->mmsName = "2";
    dpConfig.mmsNameTree.children.push_back(node2);

    auto node3 = std::make_shared<MmsNameNode>();
    node3->mmsName = "3";
    dpConfig.mmsNameTree.children.push_back(node3);

    MmsValue *mmsValueArray = MmsValue_createEmptyArray(3);

    MmsValue *mmsValue1 = MmsValue_createEmptyArray(1);
    MmsValue *mmsSubValue1 = MmsValue_newVisibleString("first");
    MmsValue_setElement(mmsValue1, 0, mmsSubValue1);

    MmsValue *mmsValue2 = MmsValue_newVisibleString("second");
    MmsValue *mmsValue3 = MmsValue_newVisibleString("third");

    MmsValue_setElement(mmsValueArray, 0, mmsValue1);
    MmsValue_setElement(mmsValueArray, 1, mmsValue2);
    MmsValue_setElement(mmsValueArray, 2, mmsValue3);

    auto wrappedMms = std::make_shared<WrappedMms>();
    wrappedMms->setMmsValue(mmsValueArray);

    auto dp = IEC61850Client::convertMmsToDatapoint(wrappedMms->getMmsValue(), dpConfig);

    ASSERT_EQ(dp->getName(), "complexDp");
    ASSERT_EQ(dp->getData().getTypeStr(), "DP_DICT");
    ASSERT_EQ(dp->getData().getDpVec()->size(), 4);
    ASSERT_EQ(dp->getData().getDpVec()->at(0)->getName(), "do_type");
    ASSERT_EQ(dp->getData().getDpVec()->at(0)->getData().getTypeStr(), "STRING");
    ASSERT_EQ(dp->getData().getDpVec()->at(0)->getData().toStringValue(), "my_cdc");

    ASSERT_EQ(dp->getData().getDpVec()->at(1)->getName(), "mag.float or int");
    ASSERT_EQ(dp->getData().getDpVec()->at(1)->getData().getTypeStr(), "STRING");
    ASSERT_EQ(dp->getData().getDpVec()->at(1)->getData().toStringValue(), "first");

    ASSERT_EQ(dp->getData().getDpVec()->at(2)->getName(), "2");
    ASSERT_EQ(dp->getData().getDpVec()->at(2)->getData().getTypeStr(), "STRING");
    ASSERT_EQ(dp->getData().getDpVec()->at(2)->getData().toStringValue(), "second");

    ASSERT_EQ(dp->getData().getDpVec()->at(3)->getName(), "3");
    ASSERT_EQ(dp->getData().getDpVec()->at(3)->getData().getTypeStr(), "STRING");
    ASSERT_EQ(dp->getData().getDpVec()->at(3)->getData().toStringValue(), "third");
}

TEST(IEC61850ClientTest, buildComplexDatapointWithErroneousStructure)
{
    ServerConnectionParameters connParam;
    ApplicationParameters applicationParams;
    DatapointConfig dpConfig;
    dpConfig.datapointType = "my_cdc";
    dpConfig.mmsNameTree.mmsName = "complexDp";

    auto subnode1 = std::make_shared<MmsNameNode>();
    subnode1->mmsName = "float or int";

    auto node1 = std::make_shared<MmsNameNode>();
    node1->mmsName = "mag";
    node1->children.push_back(subnode1);
    dpConfig.mmsNameTree.children.push_back(node1);

    auto node2 = std::make_shared<MmsNameNode>();
    node2->mmsName = "2";
    dpConfig.mmsNameTree.children.push_back(node2);

    auto node3 = std::make_shared<MmsNameNode>();
    node3->mmsName = "3";
    dpConfig.mmsNameTree.children.push_back(node3);

    MmsValue *mmsValueArray = MmsValue_createEmptyArray(2);

    MmsValue *mmsValue1 = MmsValue_createEmptyArray(1);
    MmsValue *mmsSubValue1 = MmsValue_newVisibleString("first");
    MmsValue_setElement(mmsValue1, 0, mmsSubValue1);

    MmsValue *mmsValue3 = MmsValue_newVisibleString("third");

    MmsValue_setElement(mmsValueArray, 0, mmsValue1);
    MmsValue_setElement(mmsValueArray, 1, mmsValue3);

    auto wrappedMms = std::make_shared<WrappedMms>();
    wrappedMms->setMmsValue(mmsValueArray);

    try {
        auto dp = IEC61850Client::convertMmsToDatapoint(wrappedMms->getMmsValue(), dpConfig);
        FAIL();
    } catch (MmsParsingException e) {
        ASSERT_STREQ(e.what(), "MMS Parsing exception: MMS structure does not match");
    } catch (...) {
        FAIL();
    }
}
