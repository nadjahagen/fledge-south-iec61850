/*
 * Fledge IEC 61850 south plugin.
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Estelle Chigot, Colin Constans, Akli Rahmoun
 */

// C++ standard library headers
#include <string>

// Fledge headers
#include <plugin_api.h>
#include <logger.h>
#include <reading.h>

// local library
#include "./version.h"
#include "./plugin.h"
#include "./iec61850.h"

/**
 * \brief Function pointer type for the 'Reading' object processing.
 */
using INGEST_CB = void (*)(void *, Reading);

/**
 * \brief The 61850 plugin interface
 */
extern "C" {
    static const PLUGIN_INFORMATION info = {
        PLUGIN_NAME,              // Name
        VERSION,                  // Version
        SP_ASYNC,                 // Flags
        PLUGIN_TYPE_SOUTH,        // Type
        "1.0.0",                  // Interface version
        default_config            // Default configuration
    };

    /**
     * \brief Return the information about this plugin
     */
    PLUGIN_INFORMATION *plugin_info()
    {
        Logger::getLogger()->info("61850 Config is %s", info.config);
        return const_cast<PLUGIN_INFORMATION *>(&info); // NOSONAR (Fledge API)
    }

    /**
     * \brief Initialize the plugin with a JSON configuration
     */
    PLUGIN_HANDLE plugin_init(ConfigCategory *config)  // NOSONAR (Fledge API)
    {
        Logger::getLogger()->setMinLevel("info");
        IEC61850 *iec61850;
        Logger::getLogger()->info("Initializing the plugin");

        try {
            iec61850 = new IEC61850();  // NOSONAR (Fledge API)

            if (config) {
                iec61850->setConfig(*config);
            }
        } catch (std::exception &e) {
            Logger::getLogger()->error("%s", e.what());
            throw;
        } catch (...) {
            Logger::getLogger()->error("Error: unknown exception caught");
            throw;
        }

        return (PLUGIN_HANDLE) iec61850;
    }

    /**
     * \brief Start the asynchronous Fledge feeding with MMS readings
     */
    void plugin_start(PLUGIN_HANDLE handle)
    {
        if (!handle) {
            return;
        }

        Logger::getLogger()->info("Starting the plugin");
        auto iec61850 = static_cast<IEC61850 *>(handle);
        Logger::getLogger()->setMinLevel(iec61850->getLogMinLevel());
        iec61850->start();
    }

    /**
     * \brief Register ingest callback
     */
    void plugin_register_ingest(PLUGIN_HANDLE handle, INGEST_CB ingestCallback, void *data)  // NOSONAR (Fledge API)
    {
        if (!handle) {
            throw std::invalid_argument("PLUGIN_HANDLE is null");
        }

        auto iec61850 = static_cast<IEC61850 *>(handle);
        iec61850->registerIngest(data, ingestCallback);
    }

    /**
     * \brief Poll for a plugin reading (not used)
     */
    Reading plugin_poll(PLUGIN_HANDLE)  // NOSONAR (Fledge API)
    {
        throw std::domain_error(
            "IEC_61850 is an async plugin, poll should not be called");
    }

    /**
     * \brief Reconfigure the plugin
     */
    void plugin_reconfigure(PLUGIN_HANDLE *handle, std::string &newConfig)  // NOSONAR (Fledge API)
    {
        if (!handle) {
            Logger::getLogger()->warn("plugin_reconfigure: PLUGIN_HANDLE is null");
            return;
        }

        try {
            ConfigCategory config("new", newConfig);
            auto *iec61850 = static_cast<IEC61850 *>(*handle);
            iec61850->stop();
            iec61850->setConfig(config);
            Logger::getLogger()->setMinLevel(iec61850->getLogMinLevel());
            iec61850->start();
        } catch (std::exception &e) {
            Logger::getLogger()->error("%s", e.what());
            throw;
        } catch (...) {
            Logger::getLogger()->error("Error: unknown exception caught");
            throw;
        }
    }

    /**
     * \brief Shutdown the plugin
     */
    void plugin_shutdown(PLUGIN_HANDLE handle)
    {
        if (!handle) {
            Logger::getLogger()->warn("plugin_shutdown: PLUGIN_HANDLE is null");
            return;
        }

        IEC61850 *iec61850 = nullptr;
        iec61850 = static_cast<IEC61850 *>(handle);

        if (nullptr != iec61850) {
            iec61850->stop();
            delete iec61850;  // NOSONAR (Fledge API)
            iec61850 = nullptr;
        }
    }
}  // end of 'extern "C"'
