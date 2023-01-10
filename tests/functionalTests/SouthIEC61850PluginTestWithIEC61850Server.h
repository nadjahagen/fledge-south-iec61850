#ifndef SOUTHIEC61850PLUGINTESTWITHIEC61850SERVER_H_
#define SOUTHIEC61850PLUGINTESTWITHIEC61850SERVER_H_

#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Fledge headers
#include <plugin_api.h>
#include <reading.h>

// test utilities headers
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


using INGEST_CB = void (*)(void *, Reading);

#endif  // SOUTHIEC61850PLUGINTESTWITHIEC61850SERVER_H_
