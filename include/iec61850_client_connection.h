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

#include <mutex>   // NOLINT

// Fledge headers
#include <logger.h>

// local library
#include "./iec61850_client_connection_interface.h"
#include "./iec61850_client_config.h"

// For white box unit tests
#include <gtest/gtest_prod.h>

class IEC61850Client;

/** \class IEC61850ClientConnection
 *  \brief Create and use a IEC61850 connection with a IED
 *
 *  Wrap a libiec61850 'IedConnection' object for
 *  opening connection with a distant IED,
 *  sending 'operation' or 'reading' order,
 *  receiving asynchronous reports
 */
class IEC61850ClientConnection: public IEC61850ClientConnectionInterface
{
    public :

        explicit
        IEC61850ClientConnection(const ServerConnectionParameters &connParam);

        ~IEC61850ClientConnection() override;

        /** Disable default constructor */
        IEC61850ClientConnection() = delete;

        /** Disable copy constructor */
        IEC61850ClientConnection(const IEC61850ClientConnection &) = delete;
        /** Disable copy assignment operator */
        IEC61850ClientConnection &operator = (const IEC61850ClientConnection &) = delete;
        /** Disable move constructor */
        IEC61850ClientConnection(IEC61850ClientConnection &&) = delete;
        /** Disable move assignment operator */
        IEC61850ClientConnection &operator = (IEC61850ClientConnection &&) = delete;

        bool isConnected() override;
        bool isNoError() const override;
        void logError() const override;

        /**
         * \brief Read an object (DO: Data Object) of the Server data model
         *
         * Reentrant function, thread safe
         */
        std::shared_ptr<WrappedMms> readDO(const std::string &doPath,
                                           const FunctionalConstraint &functionalConstraint) override;

        /**
         * \brief Read a dataset of the Server data model
         *
         * Reentrant function, thread safe
         */
        std::shared_ptr<WrappedMms> readDataset(const std::string &datasetRef) override;

        void buildNameTree(const std::string &pathInDatamodel,
                           const FunctionalConstraint &functionalConstraint,
                           MmsNameNode *nameTree) override;

        std::vector<std::string>
        getDoPathListWithFCFromDataset(const std::string &datasetRef) override;

    private:
        /** \brief Open a connection with an IEC61850 server */
        void open();

        /** \brief Close the connection with an IEC61850 server */
        void close();

        void setOsiConnectionParameters();

        LinkedList getDataDirectory(const std::string &pathInDatamodel,
                                    const FunctionalConstraint &functionalConstraint);

        LinkedList getDataSetDirectory(const std::string &datasetRef);

        ServerConnectionParameters m_connectionParam;
        std::mutex m_iedConnectionMutex;  /**< Protect the libiec61850 'IedConnection' resource */

        // libiec61850 objects
        IedConnection       m_iedConnection = nullptr;
        IedClientError      m_networkStack_error = IED_ERROR_OK;
        AcseAuthenticationParameter m_acseAuthentParams{nullptr};

        // Section: see the class as a white box for unit tests
        FRIEND_TEST(IEC61850ClientConnectionTestWithIEC61850Server, openConnection);
        FRIEND_TEST(IEC61850ClientConnectionTestWithIEC61850Server, openConnectionWithOsiParams);
        FRIEND_TEST(IEC61850ClientConnectionTestWithIEC61850Server, readDOValidMms);
        FRIEND_TEST(IEC61850ClientConnectionTestWithIEC61850Server, readDOButNotConnected);
        FRIEND_TEST(IEC61850ClientConnectionTestWithIEC61850Server, readBadSingleMms);
};

#endif  // INCLUDE_IEC61850_CLIENT_CONNECTION_H_
