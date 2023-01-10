#ifndef INCLUDE_MOCK_IEC61850_CLIENT_CONNECTION_H_
#define INCLUDE_MOCK_IEC61850_CLIENT_CONNECTION_H_

/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2022, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mikael Bourhis-Cloarec
 */

#include "gmock/gmock.h"

// local library
#include "./iec61850_client_connection_interface.h"

class MockIEC61850ClientConnection : public IEC61850ClientConnectionInterface
{
    public:
        MOCK_METHOD(bool, isConnected, (), (override));
        MOCK_METHOD(bool, isNoError, (), (const, override));
        MOCK_METHOD(void, logError, (), (const, override));
        MOCK_METHOD(std::shared_ptr<WrappedMms>,
                    readDO, (const std::string &doPath,
                             const FunctionalConstraint &functionalConstraint), (override));

        MOCK_METHOD(std::shared_ptr<WrappedMms>,
                    readDataset, (const std::string &datasetRef), (override));

        MOCK_METHOD(void,
                    buildNameTree, (const std::string &pathInDatamodel,
                                    const FunctionalConstraint &functionalConstraint,
                                    MmsNameNode *nameTree), (override));

        MOCK_METHOD(std::vector<std::string>,
                    getDoPathListWithFCFromDataset,
                    (const std::string &datasetRef), (override));
};
#endif  // INCLUDE_MOCK_IEC61850_CLIENT_CONNECTION_H_
