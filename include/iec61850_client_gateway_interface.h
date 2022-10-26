#ifndef INCLUDE_IEC61850_CLIENT_GATEWAY_INTERFACE_H_
#define INCLUDE_IEC61850_CLIENT_GATEWAY_INTERFACE_H_

/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2022, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 */

class ClientGatewayInterface
{
    public :

        virtual void start() = 0;
        virtual void stop() = 0;
};

#endif  // INCLUDE_IEC61850_CLIENT_GATEWAY_INTERFACE_H_
