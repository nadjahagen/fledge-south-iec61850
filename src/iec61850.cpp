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

#include "./iec61850.h"

IEC61850::IEC61850()
    : ClientGatewayInterface(),
      FledgeProxyInterface(),
      m_client(nullptr)
{
    m_config = std::make_shared<IEC61850ClientConfig>();
}

IEC61850::~IEC61850()
{
}

void IEC61850::setConfig(const ConfigCategory &config)
{
    if (m_config) {
        m_config->importConfig(config);
    } else {
        Logger::getLogger()->warn("IEC61850ClientConfig object is null");
    }
}

std::string IEC61850::getLogMinLevel()
{
    if (m_config) {
        return (m_config->logMinLevel);
    } else {
        return ("info");
    }
}

void IEC61850::start()
{
    Logger::getLogger()->info("Plugin started");

    /* Creating the client for Fledge */
    if ( ! m_client) {
        m_client = std::make_unique<IEC61850Client>(this, m_config);
    } else {
        Logger::getLogger()->info("IEC61850: client is already created");
    }

    m_client->start();
}



void IEC61850::stop()
{
    if (m_client) {
        m_client->stop();
    }
}

void IEC61850::ingest(std::vector<Datapoint *> points)
{
    std::unique_lock<std::mutex> ingestGuard(m_ingestMutex);

    if (m_ingest_callback) {
        /* Creating the name of the type of data */
        std::string asset = points[0]->getName();
        /* Callback function used after receiving data */
        (*m_ingest_callback)(m_data, Reading(asset, points));
    }
}
