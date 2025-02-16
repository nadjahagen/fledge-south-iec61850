/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2022, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mikael Bourhis-Cloarec
 */

#include "./iec61850_client_connection.h"
#include "./iec61850_client_config.h"

// libiec61850 headers
#include <libiec61850/iec61850_common.h>

IEC61850ClientConnection::IEC61850ClientConnection(
    const ServerConnectionParameters &connParam)
    : m_connectionParam(connParam)
{
    Logger::getLogger()->debug("IEC61850ClientConn: constructor");
    m_iedConnection = IedConnection_create();
    open();
}

IEC61850ClientConnection::~IEC61850ClientConnection()
{
    Logger::getLogger()->debug("IEC61850ClientConn: destructor");
    close();
    IedConnection_destroy(m_iedConnection);
}

bool IEC61850ClientConnection::isNoError() const
{
    return (m_networkStack_error == IED_ERROR_OK);
}

bool IEC61850ClientConnection::isConnected()
{
    std::unique_lock<std::mutex> connectionGuard(m_iedConnectionMutex);
    return (IedConnection_getState(m_iedConnection) == IED_STATE_CONNECTED);
}

void IEC61850ClientConnection::open()
{
    Logger::getLogger()->debug("IEC61850ClientConn: open");
    std::unique_lock<std::mutex> connectionGuard(m_iedConnectionMutex);
    Logger::getLogger()->info("Open connection with: ");
    IEC61850ClientConfig::logIedConnectionParam(m_connectionParam);

    // Set OSI parameters
    if (m_connectionParam.isOsiParametersEnabled) {
        setOsiConnectionParameters();
    }

    IedConnection_connect(m_iedConnection,
                          &m_networkStack_error,
                          m_connectionParam.ipAddress.c_str(),
                          m_connectionParam.mmsPort);
}

void IEC61850ClientConnection::setOsiConnectionParameters()
{
    MmsConnection mmsConnection = IedConnection_getMmsConnection(m_iedConnection);
    IsoConnectionParameters libiecIsoParams = MmsConnection_getIsoConnectionParameters(mmsConnection);
    const OsiParameters &osiParams = m_connectionParam.osiParameters;

    // set Remote 'AP Title' and 'AE Qualifier'
    if (! osiParams.remoteApTitle.empty()) {
        IsoConnectionParameters_setRemoteApTitle(libiecIsoParams,
                osiParams.remoteApTitle.c_str(),
                osiParams.remoteAeQualifier);
    }

    // set Local 'AP Title' and 'AE Qualifier'
    if (! osiParams.localApTitle.empty()) {
        IsoConnectionParameters_setLocalApTitle(libiecIsoParams,
                                                osiParams.localApTitle.c_str(),
                                                osiParams.localAeQualifier);
    }

    /* change parameters for presentation, session and transport layers */
    IsoConnectionParameters_setRemoteAddresses(libiecIsoParams,
            osiParams.remotePSelector,
            osiParams.remoteSSelector,
            osiParams.localTSelector);
    IsoConnectionParameters_setLocalAddresses(libiecIsoParams,
            osiParams.localPSelector,
            osiParams.localSSelector,
            osiParams.remoteTSelector);
}

void IEC61850ClientConnection::close()
{
    Logger::getLogger()->debug("IEC61850ClientConn: close");
    std::unique_lock<std::mutex> connectionGuard(m_iedConnectionMutex);
    IedConnection_close(m_iedConnection);
}

std::shared_ptr<WrappedMms>
IEC61850ClientConnection::readDO(const std::string &doPath,
                                 const FunctionalConstraint &functionalConstraint)
{
    // Preconditions
    if (! isConnected()) {
        return nullptr;
    }

    auto wrapped_mms = std::make_shared<WrappedMms>();
    std::unique_lock<std::mutex> connectionGuard(m_iedConnectionMutex);
    wrapped_mms->setMmsValue(IedConnection_readObject(m_iedConnection,
                             &m_networkStack_error,
                             doPath.c_str(),
                             functionalConstraint));
    return wrapped_mms;
}

std::shared_ptr<WrappedMms>
IEC61850ClientConnection::readDataset(const std::string &datasetRef)
{
    // Preconditions
    if (! isConnected()) {
        return nullptr;
    }

    auto wrapped_mms = std::make_shared<WrappedMms>();
    std::unique_lock<std::mutex> connectionGuard(m_iedConnectionMutex);

    ClientDataSet readDataset = IedConnection_readDataSetValues(m_iedConnection,
                                                                &m_networkStack_error,
                                                                datasetRef.c_str(),
                                                                nullptr);

    /** Keep only the MmsValue, not the full ClientDataSet structure */
    wrapped_mms->setMmsValue(MmsValue_clone(ClientDataSet_getValues(readDataset)));

    ClientDataSet_destroy(readDataset);
    return wrapped_mms;
}

void
IEC61850ClientConnection::buildNameTree(const std::string &pathInDatamodel,
                                        const FunctionalConstraint &functionalConstraint,
                                        MmsNameNode *nameTree)
{
    // Preconditions
    if (! nameTree) {
        return;
    }
    if (! isConnected()) {
        return;
    }

    LinkedList dataAttributes = nullptr;
    dataAttributes = getDataDirectory(pathInDatamodel, functionalConstraint);

    if (dataAttributes != nullptr) {
        LinkedList dataAttribute = LinkedList_getNext(dataAttributes);

        while (dataAttribute != nullptr) {
            std::string daName(static_cast<char*>(dataAttribute->data));

            auto newNameNode = std::make_shared<MmsNameNode>();
            newNameNode->mmsName = daName;

            buildNameTree(pathInDatamodel + "." + daName,
                          functionalConstraint,
                          newNameNode.get());

            nameTree->children.push_back(std::move(newNameNode));
            dataAttribute = LinkedList_getNext(dataAttribute);
        }

        LinkedList_destroy(dataAttributes);
    }
}

LinkedList
IEC61850ClientConnection::getDataDirectory(const std::string &pathInDatamodel,
                                           const FunctionalConstraint &functionalConstraint)
{
    std::unique_lock<std::mutex> connectionGuard(m_iedConnectionMutex);
    LinkedList dataAttributes = nullptr;

    dataAttributes = IedConnection_getDataDirectoryByFC(m_iedConnection,
                                                        &m_networkStack_error,
                                                        pathInDatamodel.c_str(),
                                                        functionalConstraint);

    return dataAttributes;
}

std::vector<std::string>
IEC61850ClientConnection::getDoPathListWithFCFromDataset(const std::string &datasetRef)
{
    std::vector<std::string> doPathList;

    // Preconditions
    if (! isConnected()) {
        return doPathList;
    }

    LinkedList dataSetMembers = nullptr;
    dataSetMembers = getDataSetDirectory(datasetRef);

    if (dataSetMembers != nullptr) {
        LinkedList dataSetMemberRef = LinkedList_getNext(dataSetMembers);

        while (dataSetMemberRef != nullptr) {
            doPathList.emplace_back(static_cast<char*>(dataSetMemberRef->data));

            dataSetMemberRef = LinkedList_getNext(dataSetMemberRef);
        }
    }

    return doPathList;
}

LinkedList
IEC61850ClientConnection::getDataSetDirectory(const std::string &datasetRef)
{
    std::unique_lock<std::mutex> connectionGuard(m_iedConnectionMutex);
    LinkedList dataSetMembers = nullptr;
    dataSetMembers = IedConnection_getDataSetDirectory(m_iedConnection,
                                                       &m_networkStack_error,
                                                       datasetRef.c_str(),
                                                       nullptr);

    return dataSetMembers;
}
