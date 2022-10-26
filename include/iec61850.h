#ifndef INCLUDE_IEC61850_H_
#define INCLUDE_IEC61850_H_

/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Estelle Chigot, Lucas Barret
 */

#include <string>
#include <memory>
#include <mutex>   // NOLINT

// Fledge headers
#include <reading.h>
#include <logger.h>

// local library
#include "./iec61850_client_gateway_interface.h"
#include "./iec61850_fledge_proxy_interface.h"
#include "./iec61850_client.h"
#include "./iec61850_client_config.h"

class IEC61850: public ClientGatewayInterface, public FledgeProxyInterface
{
    public:
        IEC61850();
        virtual ~IEC61850();

        /** Unavailable copy constructor */
        IEC61850(const IEC61850 &) = delete;
        /** Unavailable copy assignment operator */
        IEC61850 &operator = (const IEC61850 &) = delete;
        /** Unavailable move constructor */
        IEC61850(IEC61850 &&) = delete;
        /** Unavailable move assignment operator */
        IEC61850 &operator = (IEC61850 &&) = delete;

        void setConfig(const ConfigCategory &config);
        std::string getLogMinLevel();

        void start() override;
        void stop() override;

        void ingest(std::vector<Datapoint *>  points) override;
        void registerIngest(void *data, void (*ingest_cb)(void *, Reading)) override
        {
            m_ingest_callback = ingest_cb;
            m_data = data;
        }


    private:

        void                (*m_ingest_callback)(void *, Reading) {}; // NOLINT
        void                *m_data = nullptr;
        std::mutex          m_ingestMutex;

        std::unique_ptr<IEC61850Client> m_client;

        std::shared_ptr<IEC61850ClientConfig> m_config;
};
#endif  // INCLUDE_IEC61850_H_
