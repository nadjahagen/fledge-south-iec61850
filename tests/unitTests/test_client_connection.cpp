#include <gtest/gtest.h>
#include <gmock/gmock.h>

// South_IEC61850_Plugin headers
#include "iec61850_client_config.h"
#include "iec61850_client_connection.h"

// test utilities headers
#include "../common/mms_server_basic_io/mms_server_basic_io.h"

using namespace ::testing;

TEST(IEC61850ClientConnectionTest, openConnection)
{
    // Test Init
    ServerConnectionParameters connParam;
    connParam.ipAddress = "127.0.0.1";
    connParam.mmsPort = 8102;
    MmsServerBasicIO mmsServer(8102);
    mmsServer.start();
    // Test Body
    IEC61850ClientConnection conn(connParam);
    ASSERT_EQ("127.0.0.1", conn.m_connectionParam.ipAddress);
    ASSERT_EQ(8102, conn.m_connectionParam.mmsPort);
    ASSERT_THAT(conn.m_iedConnection, NotNull());
    ASSERT_EQ(true, conn.isConnected());
    ASSERT_EQ(true, conn.isNoError());
    conn.logError();
    // Test Teardown
    mmsServer.stop();
}

TEST(IEC61850ClientConnectionTest, openConnectionWithOsiParams)
{
    // Test Init
    ServerConnectionParameters connParam;

    connParam.osiParameters.localApTitle = "1.3.9999.13";
    connParam.osiParameters.localAeQualifier = 12;
    connParam.osiParameters.remoteApTitle = "1.2.1200.15.3";
    connParam.osiParameters.remoteAeQualifier = 1;
    connParam.osiParameters.localTSelector = {3, {0x00, 0x01, 0x02} };
    connParam.osiParameters.remoteTSelector = {2, {0x00, 0x01} };
    connParam.osiParameters.localSSelector = {5, {0, 1, 2, 3, 4, 5} };
    connParam.osiParameters.remoteSSelector = {2, {0, 1} };
    connParam.osiParameters.localPSelector = {4, {0x12, 0x34, 0x56, 0x78} };
    connParam.osiParameters.remotePSelector = {4, {0x87, 0x65, 0x43, 0x21} };

    connParam.ipAddress = "127.0.0.1";
    connParam.mmsPort = 8102;
    MmsServerBasicIO mmsServer(8102);
    mmsServer.start();

    // Test Body
    IEC61850ClientConnection conn(connParam);
    ASSERT_EQ("127.0.0.1", conn.m_connectionParam.ipAddress);
    ASSERT_EQ(8102, conn.m_connectionParam.mmsPort);
    ASSERT_EQ("1.3.9999.13", conn.m_connectionParam.osiParameters.localApTitle);
    ASSERT_EQ(12, conn.m_connectionParam.osiParameters.localAeQualifier);
    ASSERT_THAT(conn.m_iedConnection, NotNull());
    ASSERT_EQ(true, conn.isConnected());
    ASSERT_EQ(true, conn.isNoError());
    conn.logError();
    // Test Teardown
    mmsServer.stop();
}
