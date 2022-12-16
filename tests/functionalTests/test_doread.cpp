#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Fledge headers
#include <plugin_api.h>
#include <reading.h>

// Fledge plugin
#include "plugin.h"

// test utilities headers
#include "../common/configuration_examples.h"
#include "../common/mms_server_basic_io/mms_server_basic_io.h"

using namespace ::testing;

class SouthIEC61850PluginTestWithIEC61850Server : public testing::Test
{
    protected:
        MmsServerBasicIO *m_mmsServer{nullptr};

        void cleanReceivedData() {
            ingestCallCount = 0;

            for(auto reading: storedReadings) {
                delete(reading);
                reading = nullptr;
            }
            storedReadings.clear();
        };

        void SetUp() {
            if (!m_mmsServer) {
                m_mmsServer = new MmsServerBasicIO(8102);
                m_mmsServer->start();
            }
            cleanReceivedData();
        }

        void TearDown() {
            if (m_mmsServer) {
                m_mmsServer->stop();
                delete m_mmsServer;
                m_mmsServer = nullptr;
            }
            cleanReceivedData();
        }

        static bool hasChild(Datapoint& dp, std::string childLabel)
        {
            DatapointValue& dpv = dp.getData();

            auto dps = dpv.getDpVec();

            for (auto sdp : *dps) {
                if (sdp->getName() == childLabel) {
                    return true;
                }
            }

            return false;
        }

        static Datapoint* getChild(Datapoint& dp, std::string childLabel)
        {
            DatapointValue& dpv = dp.getData();

            auto dps = dpv.getDpVec();

            for (Datapoint* childDp : *dps) {
                if (childDp->getName() == childLabel) {
                    return childDp;
                }
            }

            return nullptr;
        }

        static int64_t getIntValue(Datapoint* dp)
        {
            DatapointValue dpValue = dp->getData();
            return dpValue.toInt();
        }

        static int64_t getDoubleValue(Datapoint* dp)
        {
            DatapointValue dpValue = dp->getData();
            return dpValue.toDouble();
        }

        static std::string getStrValue(Datapoint* dp)
        {
            return dp->getData().toStringValue();
        }

        static bool hasObject(Reading& reading, std::string label)
        {
            std::vector<Datapoint*> dataPoints = reading.getReadingData();

            for (const auto &dp : dataPoints) {
                std::cout << "get name : " << dp->getName() << std::endl;
                if (dp->getName() == label) {
                    return true;
                }
            }

            return false;
        }

        static Datapoint* getObject(Reading& reading, std::string label)
        {
            std::vector<Datapoint*> dataPoints = reading.getReadingData();

            for (const auto &dp : dataPoints) {
                if (dp->getName() == label) {
                    return dp;
                }
            }

            return nullptr;
        }

        static void ingestCallback(void* parameter, Reading reading)
        {
            ingestCallCount++;
            storedReadings.push_back(new Reading(reading));

            std::string readingInJson = storedReadings.back()->toJSON();
            std::cout << "Last Reading:" << readingInJson << std::endl;
        };

        static int ingestCallCount;
        static std::vector<Reading*> storedReadings;
};

int SouthIEC61850PluginTestWithIEC61850Server::ingestCallCount;
std::vector<Reading*> SouthIEC61850PluginTestWithIEC61850Server::storedReadings;

using INGEST_CB = void (*)(void *, Reading);

extern "C"
{
    PLUGIN_INFORMATION *plugin_info();
    PLUGIN_HANDLE plugin_init(ConfigCategory *config);
    void plugin_register_ingest(PLUGIN_HANDLE handle, INGEST_CB ingestCallback,
                                void *data);
    void plugin_reconfigure(PLUGIN_HANDLE handle, std::string &newConfig);
    void plugin_shutdown(PLUGIN_HANDLE handle);
    void plugin_start(PLUGIN_HANDLE handle);
};

TEST_F(SouthIEC61850PluginTestWithIEC61850Server, readDO)
{
    ConfigCategory config("TestDefaultConfig", default_config);
    config.setItemsValueFromDefault();

    PLUGIN_HANDLE handle = nullptr;
    handle = plugin_init(&config);

    ASSERT_NO_THROW(
           plugin_register_ingest((PLUGIN_HANDLE)handle,
                                   SouthIEC61850PluginTestWithIEC61850Server::ingestCallback,
                                   NULL)
    );

    ASSERT_NO_THROW(plugin_start((PLUGIN_HANDLE)handle));

    sleep(2);
    plugin_shutdown((PLUGIN_HANDLE)handle);

    ASSERT_GE(ingestCallCount, 4);
    ASSERT_LE(ingestCallCount, 8);

    Reading *firstReading = storedReadings.at(0);
    Reading *secondReading = storedReadings.at(1);

    ASSERT_EQ("TS1", firstReading->getAssetName());
    ASSERT_EQ("TM1", secondReading->getAssetName());

    ASSERT_TRUE(hasObject(*firstReading, "TS1"));
    Datapoint *dp = getObject(*firstReading, "TS1");
    ASSERT_THAT(dp, NotNull());

    ASSERT_TRUE(hasChild(*dp, "do_type"));
    ASSERT_EQ("SPS", getStrValue(getChild(*dp, "do_type")));

    ASSERT_TRUE(hasChild(*dp, "do_value"));
    ASSERT_TRUE(hasChild(*dp, "do_quality"));
    ASSERT_TRUE(hasChild(*dp, "do_ts"));


    ASSERT_TRUE(hasObject(*secondReading, "TM1"));
    dp = getObject(*secondReading, "TM1");
    ASSERT_THAT(dp, NotNull());

    ASSERT_TRUE(hasChild(*dp, "do_type"));
    ASSERT_EQ("MV", getStrValue(getChild(*dp, "do_type")));

    ASSERT_TRUE(hasChild(*dp, "do_value"));
    ASSERT_LE(getDoubleValue(getChild(*dp, "do_value")), 1.0);
    ASSERT_GE(getDoubleValue(getChild(*dp, "do_value")), -1.0);

    ASSERT_TRUE(hasChild(*dp, "do_quality"));

    ASSERT_TRUE(hasChild(*dp, "do_ts"));
    ASSERT_LE(getIntValue(getChild(*dp, "do_ts")), 2147483648);
    ASSERT_GE(getIntValue(getChild(*dp, "do_ts")), 1670509743);
}
