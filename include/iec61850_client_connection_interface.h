#ifndef INCLUDE_IEC61850_CLIENT_CONNECTION_INTERFACE_H_
#define INCLUDE_IEC61850_CLIENT_CONNECTION_INTERFACE_H_

/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2022, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mikael Bourhis-Cloarec
 */

#include <vector>

// libiec61850 headers
#include <libiec61850/iec61850_client.h>

// local library
#include "./wrapped_mms.h"

class MmsNameNode;

class IEC61850ClientConnectionInterface
{
    public :

        virtual ~IEC61850ClientConnectionInterface() = default;

        virtual bool isConnected() = 0;
        virtual bool isNoError() const = 0;
        virtual void logError() const = 0;

        virtual std::shared_ptr<WrappedMms> readDO(const std::string &doPath,
                const FunctionalConstraint &functionalConstraint) = 0;

        virtual std::shared_ptr<WrappedMms> readDataset(const std::string &datasetRef) = 0;

        virtual void buildNameTree(const std::string &pathInDatamodel,
                                   const FunctionalConstraint &functionalConstraint,
                                   MmsNameNode *nameTree) = 0;

        virtual std::vector<std::string>
        getDoPathListWithFCFromDataset(const std::string &datasetRef) = 0;
};
#endif  // INCLUDE_IEC61850_CLIENT_CONNECTION_INTERFACE_H_
