#ifndef INCLUDE_IEC61850_CLIENT_CONNECTION_H_
#define INCLUDE_IEC61850_CLIENT_CONNECTION_H_

/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2022, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mikael Bourhis-Cloarec
 */

#include <memory>
#include <mutex>   // NOLINT

// libiec61850 headers
#include <libiec61850/iec61850_client.h>

// Fledge headers
#include <logger.h>

// local library
#include "./iec61850_client_config.h"

class IEC61850Client;


/** \class Mms
 *  \brief Encapsulate an MmsValue pointer
 *
 *  Encapsulate an MmsValue pointer for automatically deleting the
 *  allocated memory at the end of the life cycle of this object
 */
class Mms
{
    public:
        Mms();
        ~Mms();
        void setMmsValue(MmsValue *mmsValue);
        const MmsValue *getMmsValue() const;
        bool isNull();

    private:
        MmsValue *m_mmsValue;
};

/** \class IEC61850ClientConnection
 *  \brief Create and use a IEC61850 connection with a IED
 *
 *  Wrap a libiec61850 'IedConnection' object for
 *  opening connection with a distant IED,
 *  sending 'operation' or 'reading' order,
 *  receiving asynchronous reports
 */
class IEC61850ClientConnection
{
    public :

        explicit
        IEC61850ClientConnection(const ConnectionParameters &connParam);

        ~IEC61850ClientConnection();

        bool isConnected();
        bool isNoError();
        void logError();

        std::shared_ptr<Mms> readMms(const std::string &daPath,
                                     const std::string &fcName);

    private:
        void open();
        void close();

        ConnectionParameters m_connectionParam;
        std::mutex m_iedConnectionMutex; // libiec61850 thread safe?: protect the IedConnection

        // libiec61850 objects
        IedConnection       m_iedConnection = nullptr;
        IedClientError      m_networkStack_error = IED_ERROR_OK;
};

#endif  // INCLUDE_IEC61850_CLIENT_CONNECTION_H_
