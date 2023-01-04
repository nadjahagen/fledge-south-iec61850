#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Fledge headers
#include <plugin_api.h>

// Fledge plugin
#include "plugin.h"

// test utilities headers
#include "./SouthIEC61850PluginTestWithIEC61850Server.h"
#include "../common/configuration_examples.h"

using namespace ::testing;

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
    ConfigCategory config("TestDefaultConfig", functional_tests_config_do_reading_mode);
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
