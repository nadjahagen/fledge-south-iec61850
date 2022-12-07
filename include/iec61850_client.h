#ifndef INCLUDE_IEC61850_CLIENT_H_
#define INCLUDE_IEC61850_CLIENT_H_

/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Estelle Chigot, Lucas Barret
 */

#include <memory>
#include <thread>  // NOLINT
#include <mutex>   // NOLINT
#include <atomic>

// Fledge headers
#include <logger.h>
#include <reading.h>


// local library
#include "./iec61850_client_config.h"
#include "./iec61850_client_connection_interface.h"

// For white box unit tests
#include <gtest/gtest_prod.h>

class IEC61850;
class WrappedMms;

/** \class MmsParsingException
 *  \brief an error during the parsing of MMS has been detected
 */
class MmsParsingException: public std::logic_error
{
    public:
        explicit MmsParsingException(std::string const &msg):
            std::logic_error("MMS Parsing exception: " + msg) {}
};


/** \class IEC61850Client
 *  \brief Read from and write to a IED
 *
 *  Handle IEC61850 data objects
 *  for reading data
 *  or sending order and operation
 */
class IEC61850Client
{
    public :

        explicit IEC61850Client(IEC61850 *iec61850,
                                const ServerConnectionParameters &connectionParam,
                                const ExchangedDataDict &exchangedDataDict,
                                const ApplicationParameters &applicationParams);

        ~IEC61850Client();

        /** Disable default constructor */
        IEC61850Client() = delete;

        IEC61850Client(const IEC61850Client &) = default;
        IEC61850Client &operator = (const IEC61850Client &) = default;
        IEC61850Client(IEC61850Client &&) = default;
        IEC61850Client &operator = (IEC61850Client &&) = default;

        /**
         * Open the connection with the IED
         */
        void start();

        /**
         * Close the connection with the IED
         */
        void stop();

    private:
        std::string m_clientId;

        // Section: Configuration
        const ServerConnectionParameters &m_connectionParam;
        const ExchangedDataDict &m_exchangedDataDict;
        const ApplicationParameters &m_applicationParams;

        // Section: Data formatting for the plugin
        IEC61850 *m_iec61850;

        /**
         * Create the Datapoint object that will be ingest by Fledge
         * (dynamic allocation, deallocation by Fledge core).
         * Reentrant function, thread safe
         */
        template <typename T>
        static Datapoint *createDatapoint(const std::string &dataName,
                                          T primitiveTypeValue);

        static Datapoint *createComplexDatapoint(const std::string &dataName,
                                                 std::vector<Datapoint*> *&values);

        static Datapoint *buildDatapointFromMms(const MmsValue *mmsValue,
                                                const MmsNameNode *mmsNameNode,
                                                const DataPath &dataPath);

        static void insertTypeInDatapoint(Datapoint *datapoint,
                                          const std::string &doType);

        /**
         * Convert the MMS into Datapoint
         * by extracting the MMS content and creating a new Datapoint
         * Reentrant function, thread safe
         */
        static Datapoint *convertMmsToDatapoint(std::shared_ptr<WrappedMms> wrappedMms,
                                                const ExchangedData &exchangedData);

        /**
         * Send a datapoint to Fledge core
         * Reentrant function, thread safe
         */
        void sendData(Datapoint *datapoint);

        void readAndExportMms();

        // Section: Client initialization with connection creation
        void launch();
        void initializeConnection();
        void createConnection();
        void destroyConnection();
        std::thread m_backgroundLaunchThread;
        std::atomic<bool> m_stopOrder{false};
        std::unique_ptr<IEC61850ClientConnectionInterface> m_connection;

        // Section: MMS reading (DO and Dataset)
        void startMmsReading();
        void stopMmsReading();
        void readMmsLoop();
        std::atomic<bool> m_isMmsReadingActivated{false};
        std::thread m_mmsReadingThread;

        // Section: see the class as a white box for unit tests
        FRIEND_TEST(IEC61850ClientTest, createOneConnection);
        FRIEND_TEST(IEC61850ClientTest, reuseCreatedConnection);
        FRIEND_TEST(IEC61850ClientTest, destroyConnection);
        FRIEND_TEST(IEC61850ClientTest, destroyNullConnection);
        FRIEND_TEST(IEC61850ClientTest, injectMockConnection);
        FRIEND_TEST(IEC61850ClientTest, initializeConnectionInOneTry);
        FRIEND_TEST(IEC61850ClientTest, initializeConnectionFailed);
        FRIEND_TEST(IEC61850ClientTest, startAndStop);
        FRIEND_TEST(IEC61850ClientTest, buildIntegerDatapoint);
        FRIEND_TEST(IEC61850ClientTest, buildUnsignedIntegerDatapoint);
        FRIEND_TEST(IEC61850ClientTest, buildBoolDatapoint);
        FRIEND_TEST(IEC61850ClientTest, buildFloatDatapoint);
        FRIEND_TEST(IEC61850ClientTest, buildDoubleDatapoint);
        FRIEND_TEST(IEC61850ClientTest, buildTimestampDatapoint);
        FRIEND_TEST(IEC61850ClientTest, buildBitStringDatapoint);
        FRIEND_TEST(IEC61850ClientTest, buildVisibleStringDatapoint);
        FRIEND_TEST(IEC61850ClientTest, buildComplexDatapoint);
        FRIEND_TEST(IEC61850ClientTest, buildComplexMxDatapoint);
        FRIEND_TEST(IEC61850ClientTest, buildComplexDatapointWithErroneousStructure);
};

#endif  // INCLUDE_IEC61850_CLIENT_H_
