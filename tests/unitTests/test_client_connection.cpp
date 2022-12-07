#include <gtest/gtest.h>
#include <gmock/gmock.h>

// South_IEC61850_Plugin headers
#include "iec61850_client_config.h"
#include "iec61850_client_connection.h"

// test utilities headers
#include "../common/mms_server_basic_io/mms_server_basic_io.h"

using namespace ::testing;

class IEC61850ClientConnectionTestWithIEC61850Server : public testing::Test
{
    protected:
        MmsServerBasicIO *m_mmsServer{nullptr};

        void SetUp() {
            if (!m_mmsServer) {
                m_mmsServer = new MmsServerBasicIO(8102);
                m_mmsServer->start();
            }
        }

        void TearDown() {
            if (m_mmsServer) {
                m_mmsServer->stop();
                delete m_mmsServer;
                m_mmsServer = nullptr;
            }
        }
};

TEST_F(IEC61850ClientConnectionTestWithIEC61850Server, openConnection)
{
    // Test Init
    ServerConnectionParameters connParam;
    connParam.ipAddress = "127.0.0.1";
    connParam.mmsPort = 8102;
    // Test Body
    IEC61850ClientConnection conn(connParam);
    ASSERT_EQ("127.0.0.1", conn.m_connectionParam.ipAddress);
    ASSERT_EQ(8102, conn.m_connectionParam.mmsPort);
    ASSERT_THAT(conn.m_iedConnection, NotNull());
    ASSERT_EQ(true, conn.isConnected());
    ASSERT_EQ(true, conn.isNoError());
    conn.logError();
}

TEST_F(IEC61850ClientConnectionTestWithIEC61850Server, openConnectionWithOsiParams)
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
    connParam.isOsiParametersEnabled = true;

    connParam.ipAddress = "127.0.0.1";
    connParam.mmsPort = 8102;

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
}

TEST_F(IEC61850ClientConnectionTestWithIEC61850Server, readSingleValidMms)
{
    // Test Init
    ServerConnectionParameters connParam;
    connParam.ipAddress = "127.0.0.1";
    connParam.mmsPort = 8102;
    // Test Body
    IEC61850ClientConnection conn(connParam);
    ASSERT_EQ("127.0.0.1", conn.m_connectionParam.ipAddress);
    ASSERT_EQ(8102, conn.m_connectionParam.mmsPort);
    ASSERT_THAT(conn.m_iedConnection, NotNull());
    ASSERT_EQ(true, conn.isConnected());
    ASSERT_EQ(true, conn.isNoError());


    auto wrappedMms = conn.readSingleMms("simpleIOGenericIO/GGIO1.AnIn1",
                                         FunctionalConstraint_fromString("MX"));

    auto mmsValue = wrappedMms->getMmsValue();
    ASSERT_THAT(mmsValue, NotNull());
    ASSERT_EQ(3, MmsValue_getArraySize(mmsValue));

    auto mmsValueTimestamp = MmsValue_getElement(mmsValue, 2);
    ASSERT_EQ(MmsValue_getType(mmsValueTimestamp), MMS_UTC_TIME);

    uint32_t timestamp = MmsValue_toUnixTimestamp(mmsValueTimestamp);
    ASSERT_EQ(16, timestamp / 100000000);

    auto mmsValueFloat = MmsValue_getElement(MmsValue_getElement(mmsValue, 0), 0);
    ASSERT_EQ(MmsValue_getType(mmsValueFloat), MMS_FLOAT);

    float floatValue = MmsValue_toFloat(mmsValueFloat);
    ASSERT_LT(floatValue, 1.0);
    ASSERT_GT(floatValue, -1.0);
}

TEST_F(IEC61850ClientConnectionTestWithIEC61850Server, readSingleMmsButNotConnected)
{
    // Test Init
    ServerConnectionParameters connParam;
    connParam.ipAddress = "127.0.0.1";
    connParam.mmsPort = 8102;
    // Test Body
    IEC61850ClientConnection conn(connParam);
    ASSERT_EQ("127.0.0.1", conn.m_connectionParam.ipAddress);
    ASSERT_EQ(8102, conn.m_connectionParam.mmsPort);
    ASSERT_THAT(conn.m_iedConnection, NotNull());
    ASSERT_EQ(true, conn.isConnected());
    ASSERT_EQ(true, conn.isNoError());

    // shutdown the server
    m_mmsServer->stop();

    auto wrappedMms = conn.readSingleMms("simpleIOGenericIO/GGIO1.AnIn1",
                                         FunctionalConstraint_fromString("MX"));

    ASSERT_THAT(wrappedMms, IsNull());
    ASSERT_EQ(false, conn.isConnected());
    ASSERT_EQ(true, conn.isNoError());
    conn.logError();
}

TEST_F(IEC61850ClientConnectionTestWithIEC61850Server, readBadSingleMms)
{
    // Test Init
    ServerConnectionParameters connParam;
    connParam.ipAddress = "127.0.0.1";
    connParam.mmsPort = 8102;
    // Test Body
    IEC61850ClientConnection conn(connParam);
    ASSERT_EQ("127.0.0.1", conn.m_connectionParam.ipAddress);
    ASSERT_EQ(8102, conn.m_connectionParam.mmsPort);
    ASSERT_THAT(conn.m_iedConnection, NotNull());
    ASSERT_EQ(true, conn.isConnected());
    ASSERT_EQ(true, conn.isNoError());

    auto wrappedMms = conn.readSingleMms("simpleIOGenericIO/foo_doesnt_exist",
                                         FunctionalConstraint_fromString("MX"));

    ASSERT_THAT(wrappedMms->getMmsValue(), NotNull());
    ASSERT_EQ(MmsValue_getType(const_cast<MmsValue*>(wrappedMms->getMmsValue())), MMS_DATA_ACCESS_ERROR);
    ASSERT_EQ(true, conn.isConnected());
    ASSERT_EQ(true, conn.isNoError());
    conn.logError();
}

