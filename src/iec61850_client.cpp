/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Estelle Chigot, Lucas Barret
 *
 * Contributor: Colin Constans
 */

#include "./iec61850_client.h"

// C++ headers
#include <memory>
#include <vector>

// libiec61850 headers
#include <libiec61850/mms_value.h>

// local library
#include "./iec61850.h"
#include "./iec61850_client_connection.h"
#include "./wrapped_mms.h"

constexpr const uint32_t RECONNECTION_FREQUENCY_IN_HERTZ = 1;
constexpr const uint32_t SECOND_IN_MILLISEC = 1000;

/** Name mapping between the DO attributes and the Reading attributes */
const std::map<std::string, std::string, std::less<>> DO_READING_MAPPING = {
    {"cdc", "do_type"},
    {"stVal", "do_value"},
    {"mag.f", "do_value"},
    {"q", "do_quality"},
    {"t", "do_ts"}
};

IEC61850Client::IEC61850Client(IEC61850 *iec61850,
                               const ServerConnectionParameters &connectionParam,
                               const ExchangedData &exchangedData,
                               const ApplicationParameters &applicationParams)
    : m_connectionParam(connectionParam),
      m_exchangedData(exchangedData),
      m_applicationParams(applicationParams),
      m_iec61850(iec61850)
{
    m_clientId = IEC61850ClientConfig::buildKey(m_connectionParam);
    Logger::getLogger()->debug("IEC61850Client: constructor %s",
                               m_clientId.c_str());
}

IEC61850Client::~IEC61850Client()
{
    Logger::getLogger()->debug("IEC61850Client: destructor %s",
                               m_clientId.c_str());
    stop();  // ensure a correct shutdown, if 'stop' order was missing
}

void IEC61850Client::start()
{
    m_backgroundLaunchThread = std::thread(&IEC61850Client::launch, this);
}

void IEC61850Client::stop()
{
    m_stopOrder = true;

    if (m_backgroundLaunchThread.joinable()) {
        m_backgroundLaunchThread.join();
    }

    // Stop the MMS reading thread
    stopMmsReading();
    destroyConnection();
}

void IEC61850Client::launch()
{
    initializeConnection();
    /** Make subscriptions */
    // TODO
    /** Start application loop */
    startMmsReading();
}

void IEC61850Client::initializeConnection()
{
    do {
        Logger::getLogger()->debug("IEC61850Client: init connection (%s)",
                                   m_clientId.c_str());
        destroyConnection();
        createConnection();

        if (! m_connection->isConnected()) {
            Logger::getLogger()->warn("IEC61850Client: failed to connect with %s",
                                      m_clientId.c_str());
        }

        // Wait connection establishment
        std::chrono::milliseconds timespan(SECOND_IN_MILLISEC / RECONNECTION_FREQUENCY_IN_HERTZ);
        std::this_thread::sleep_for(timespan);
    } while ( (! m_stopOrder) &&
              (! m_connection->isConnected()));
}

void IEC61850Client::createConnection()
{
    // Preconditions
    if (m_connection) {
        Logger::getLogger()->info("IEC61850Client: connection is already created (%s)",
                                  m_clientId.c_str());
        return;
    }

    Logger::getLogger()->info("IEC61850Client: create connection with %s",
                              m_clientId.c_str());
    m_connection = std::make_unique<IEC61850ClientConnection>(m_connectionParam);
}

void IEC61850Client::destroyConnection()
{
    if (! m_connection) {
        Logger::getLogger()->info("IEC61850Client: connection does not exist (%s)",
                                  m_clientId.c_str());
        return;
    }

    Logger::getLogger()->debug("IEC61850Client: destroy connection (%s)",
                               m_clientId.c_str());
    m_connection.reset(nullptr);
}


template <typename T>
Datapoint *IEC61850Client::createDatapoint(const std::string &dataName,
        T primitiveTypeValue)
{
    DatapointValue value = DatapointValue(primitiveTypeValue);
    /**
     *  Dynamic allocation with raw pointer: Fledge core will deallocate it.
     *  See fledge/C/common/reading.cpp, the 'destructor' method.
     **/
    return new Datapoint(dataName, value);  // NOSONAR
}

Datapoint *IEC61850Client::createComplexDatapoint(const std::string &dataName,
                                                  std::vector<Datapoint*> *&values)
{
    auto value = DatapointValue(values, true);
    /**
     *  Dynamic allocation with raw pointer: Fledge core will deallocate it.
     *  See fledge/C/common/reading.cpp, the 'destructor' method.
     **/
    return new Datapoint(dataName, value);  // NOSONAR
}


void IEC61850Client::sendData(Datapoint *datapoint)
{
    // Preconditions
    if (nullptr == m_iec61850) {
        Logger::getLogger()->warn("IEC61850Client: abort 'sendData' (receiver is null)");
        // datapoint is now useless
        delete datapoint;
        return;
    }

    if (nullptr == datapoint) {
        Logger::getLogger()->warn("IEC61850Client: abort 'sendData' (datapoint is empty)");
        return;
    }

    std::vector<Datapoint *> points(0);
    points.push_back(datapoint);
    m_iec61850->ingest(points, datapoint->getName());
}

Datapoint *IEC61850Client::convertMmsToDatapoint(std::shared_ptr<WrappedMms> wrappedMms,
                                                 const DatapointConfig &datapointConfig)
{
    // Precondition
    if (nullptr == wrappedMms) {
        return nullptr;
    }

    if (nullptr == wrappedMms->getMmsValue()) {
        return nullptr;
    }

    Datapoint *datapoint = buildDatapointFromMms(wrappedMms->getMmsValue(),
                                                 &datapointConfig.mmsNameTree,
                                                 datapointConfig.dataPath);

    insertTypeInDatapoint(datapoint, datapointConfig.datapointType);

    return datapoint;
}

void IEC61850Client::insertTypeInDatapoint(Datapoint *datapoint,
                                           const std::string &doType)
{
    // Precondition
    if (!datapoint) {
        return;
    }

    DatapointValue &dpv = datapoint->getData();

    if (dpv.getType() == DatapointValue::T_DP_DICT) {
        dpv.getDpVec()->insert(dpv.getDpVec()->begin(),
                               createDatapoint("do_type", doType));
    }
}

Datapoint *IEC61850Client::buildDatapointFromMms(const MmsValue *mmsValue,
                                                 const MmsNameNode *mmsNameNode,
                                                 const DataPath &dataPath)
{
    // Preconditions
    if (nullptr == mmsValue) {
        throw MmsParsingException("the input MmsValue is null");
    }
    if (nullptr == mmsNameNode) {
        throw MmsParsingException("the input MmsNameNode is null");
    }

    std::string mmsName = mmsNameNode->mmsName;
    Datapoint *datapoint = nullptr;

    switch (MmsValue_getType(mmsValue)) {
        case MMS_STRUCTURE:
        case MMS_ARRAY: {
            uint32_t arraySize = MmsValue_getArraySize(mmsValue);
            if (arraySize != mmsNameNode->children.size()) {
                throw MmsParsingException("MMS structure does not match");
            }

            if (mmsName.compare("mag") == 0) {
                // keep only the 1st child, and concatenate the names
                datapoint = buildDatapointFromMms(MmsValue_getElement(mmsValue, 0),
                                                  mmsNameNode->children[0].get(),
                                                  dataPath);
                datapoint->setName(mmsName + "." + datapoint->getName());
            } else {
                /**
                 *  Dynamic allocation with raw pointer: Fledge core will deallocate it.
                 *  See fledge/C/common/datapoint.cpp, the 'deleteNestedDPV()' method.
                 **/
                auto *dpArray = new std::vector<Datapoint *>;  // NOSONAR
                for (uint32_t index = 0; index < arraySize; ++index) {
                    Datapoint *dpChild = nullptr;
                    dpChild = buildDatapointFromMms(MmsValue_getElement(mmsValue, index),
                                                    mmsNameNode->children[index].get(),
                                                    dataPath);
                    dpArray->push_back(dpChild);
                }
                datapoint = createComplexDatapoint(mmsName, dpArray);
            }

            break;
        }

        case MMS_BOOLEAN: {
            bool boolValue = MmsValue_getBoolean(mmsValue);
            datapoint = createDatapoint(mmsName,
                                        static_cast<int64_t>(boolValue ? 1 : 0));
            break;
        }

        case MMS_FLOAT:
            datapoint = createDatapoint(mmsName,
                                        MmsValue_toFloat(mmsValue));
            break;

        case MMS_UNSIGNED:
            datapoint = createDatapoint(mmsName,
                                        static_cast<int64_t>(MmsValue_toUint32(mmsValue)));
            break;
        case MMS_INTEGER:
            datapoint = createDatapoint(mmsName,
                                        static_cast<int64_t>(MmsValue_toInt32(mmsValue)));
            break;

        case MMS_UTC_TIME:
            datapoint = createDatapoint(mmsName,
                                        static_cast<int64_t>(MmsValue_toUnixTimestamp(mmsValue)));
            break;

        case MMS_VISIBLE_STRING:
            // TODO fix 'MmsValue_toString()' signature in libiec61850
            // TODO use: createDatapoint("MMS_VISIBLE_STRING", MmsValue_toString(mmsValue));
            datapoint = createDatapoint(mmsName,
                                        MmsValue_toString(const_cast<MmsValue *>(mmsValue)));
            break;

        case MMS_BIT_STRING: {
            const uint8_t maxSize = 32;
            char buffer[maxSize];  // NOSONAR
            memset(buffer, '\0', maxSize);

            std::string strval = MmsValue_printToBuffer(mmsValue, buffer, maxSize);
            datapoint = createDatapoint(mmsName, strval);
            break;
        }

        case MMS_DATA_ACCESS_ERROR:
            Logger::getLogger()->error("MMS access error (num %d), failed to access to: %s",
                                       MmsValue_getDataAccessError(mmsValue),
                                       dataPath.c_str());
            break;

        default :
            throw MmsParsingException(std::string("Unsupported MMS data type: ") +
                                      std::string(MmsValue_getTypeString(const_cast<MmsValue*>(mmsValue))));
    }

    if (datapoint) {
        // Rename the datapoint i.e. the Reading attributes
        if (DO_READING_MAPPING.find(datapoint->getName()) != DO_READING_MAPPING.end()) {
            Logger::getLogger()->debug("Datapoint creation: name mapping %s -> %s",
                        datapoint->getName().c_str(),
                        DO_READING_MAPPING.at(datapoint->getName()).c_str());
            datapoint->setName(DO_READING_MAPPING.at(datapoint->getName()));
        }
    }

    return datapoint;
}

// MMS reading section

void IEC61850Client::startMmsReading()
{
    m_isMmsReadingActivated = true;
    m_mmsReadingThread = std::thread(&IEC61850Client::readMmsLoop, this);
}

void IEC61850Client::stopMmsReading()
{
    // Preconditions
    if (false == m_isMmsReadingActivated) {
        return;
    }

    m_isMmsReadingActivated = false;
    m_mmsReadingThread.join();
}

void IEC61850Client::readMmsLoop()
{
    // Preconditions
    if (! m_connection) {
        Logger::getLogger()->warn("IEC61850Client: Connection object is null");
    }

    unsigned int pollingPeriodInMs = m_applicationParams.readPollingPeriodInMs;
    if (pollingPeriodInMs == 0) {
        // Force to 1 second
        pollingPeriodInMs = 1000;
    }

    try {
        while (m_isMmsReadingActivated) {
            readAndExportMms();
            std::chrono::milliseconds timespan(pollingPeriodInMs);
            std::this_thread::sleep_for(timespan);
        }
    } catch (std::exception &e) {
        Logger::getLogger()->error("%s", e.what());
    } catch (...) {
        Logger::getLogger()->error("Error: unknown exception caught");
    }
}

void IEC61850Client::readAndExportMms()
{
    // Preconditions
    if (! m_connection->isConnected()) {
        initializeConnection();
        return;
    }

    if (! m_connection->isNoError()) {
        m_connection->logError();
        return;
    }

    /* read the desired MMS from server */

    switch (m_applicationParams.readMode) {
        case ReadMode::DATASET_READING:
            // implementation next
            break;

        case ReadMode::DO_READING:
        {
            for (const auto &it : m_exchangedData) {
                const DatapointConfig &dpConfig = it.second;
                std::shared_ptr<WrappedMms> wrapped_mms;
                wrapped_mms = m_connection->readDO(dpConfig.dataPath,
                                                   dpConfig.functionalConstraint);

                sendData(convertMmsToDatapoint(wrapped_mms, dpConfig));
            }
            break;
        }
        default:
            Logger::getLogger()->error("Read MMS: unknown reading mode: %u",
                        m_applicationParams.readMode);
            break;
    }
}
