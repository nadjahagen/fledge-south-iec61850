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


TEST_F(SouthIEC61850PluginTestWithIEC61850Server, readDatasets)
{
    ConfigCategory config("TestDefaultConfig", functional_tests_config_dataset_reading_mode);
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

    ASSERT_GE(ingestCallCount, 14);
    ASSERT_LE(ingestCallCount, 28);

    // 1st reading
    ASSERT_EQ("AnIn1", storedReadings.at(0)->getAssetName());

    ASSERT_TRUE(hasObject(*(storedReadings.at(0)), "AnIn1"));
    Datapoint *dp = getObject(*(storedReadings.at(0)), "AnIn1");
    ASSERT_THAT(dp, NotNull());

    ASSERT_TRUE(hasChild(*dp, "do_type"));
    ASSERT_EQ("", getStrValue(getChild(*dp, "do_type")));

    ASSERT_TRUE(hasChild(*dp, "do_value"));
    ASSERT_LE(getDoubleValue(getChild(*dp, "do_value")), 1.0);
    ASSERT_GE(getDoubleValue(getChild(*dp, "do_value")), -1.0);
    ASSERT_TRUE(hasChild(*dp, "do_quality"));
    ASSERT_TRUE(hasChild(*dp, "do_ts"));
    ASSERT_LE(getIntValue(getChild(*dp, "do_ts")), 2147483648);
    ASSERT_GE(getIntValue(getChild(*dp, "do_ts")), 1670509743);

    // 2nd reading
    ASSERT_EQ("AnIn2", storedReadings.at(1)->getAssetName());
    ASSERT_TRUE(hasObject(*(storedReadings.at(1)), "AnIn2"));

    // 3rd reading
    ASSERT_EQ("AnIn3", storedReadings.at(2)->getAssetName());
    ASSERT_TRUE(hasObject(*(storedReadings.at(2)), "AnIn3"));

    // 4th reading
    ASSERT_EQ("AnIn4", storedReadings.at(3)->getAssetName());
    ASSERT_TRUE(hasObject(*(storedReadings.at(3)), "AnIn4"));

    // 5th reading
    ASSERT_EQ("TS1", storedReadings.at(4)->getAssetName());

    ASSERT_TRUE(hasObject(*(storedReadings.at(4)), "TS1"));
    dp = getObject(*(storedReadings.at(4)), "TS1");
    ASSERT_THAT(dp, NotNull());

    ASSERT_TRUE(hasChild(*dp, "do_type"));
    ASSERT_EQ("SPS", getStrValue(getChild(*dp, "do_type")));

    ASSERT_TRUE(hasChild(*dp, "do_value"));
    ASSERT_TRUE(hasChild(*dp, "do_quality"));
    ASSERT_LE(getStrValue(getChild(*dp, "do_quality")), "0000000000000");
    ASSERT_TRUE(hasChild(*dp, "do_ts"));

    // 6th reading
    ASSERT_EQ("TS2", storedReadings.at(5)->getAssetName());

    ASSERT_TRUE(hasObject(*(storedReadings.at(5)), "TS2"));
    dp = getObject(*(storedReadings.at(5)), "TS2");
    ASSERT_THAT(dp, NotNull());

    ASSERT_TRUE(hasChild(*dp, "do_type"));
    ASSERT_EQ("SPS", getStrValue(getChild(*dp, "do_type")));

    ASSERT_TRUE(hasChild(*dp, "do_value"));
    ASSERT_TRUE(hasChild(*dp, "do_quality"));
    ASSERT_LE(getStrValue(getChild(*dp, "do_quality")), "0000000000000");
    ASSERT_TRUE(hasChild(*dp, "do_ts"));

    // 7th reading
    ASSERT_EQ("SPSSO4", storedReadings.at(6)->getAssetName());

    ASSERT_TRUE(hasObject(*(storedReadings.at(6)), "SPSSO4"));
    dp = getObject(*(storedReadings.at(6)), "SPSSO4");
    ASSERT_THAT(dp, NotNull());

    ASSERT_TRUE(hasChild(*dp, "do_type"));
    ASSERT_EQ("", getStrValue(getChild(*dp, "do_type")));

    ASSERT_TRUE(hasChild(*dp, "do_value"));
    ASSERT_TRUE(hasChild(*dp, "do_quality"));
    ASSERT_LE(getStrValue(getChild(*dp, "do_quality")), "0000000000000");
    ASSERT_TRUE(hasChild(*dp, "do_ts"));
}

