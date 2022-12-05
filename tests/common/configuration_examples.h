#ifndef CONFIGURATION_EXAMPLES_H_
#define CONFIGURATION_EXAMPLES_H_

#include <string>
#include <plugin_api.h>

// *INDENT-OFF* (disable 'astyle' tool on this section)
const std::string configForReconfiguration = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "log min level" : {
        "description" : "minimum level for the Fledge logger (debug, info)",
        "type" : "string",
        "value" : "info",
        "displayName" : "logger minimum level",
        "order" : "1",
        "mandatory" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "value" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip": "0.0.0.0",
                            "port" : 102
                        },
                        {
                            "srv_ip": "0.0.0.0",
                            "port" : 8102
                        }
                    ]
                },
                "application_layer" : {
                }
            }
        })
    }
});

const std::string configWithoutLogLevel = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "value" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip": "0.0.0.0",
                            "port" : 102
                        },
                        {
                            "srv_ip": "0.0.0.0",
                            "port" : 8102
                        }
                    ]
                },
                "application_layer" : {
                }
            }
        })
    }
});

const std::string configMissingProtocolStack = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    }
});

const std::string configProtocolStackWithParsingError = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3"
    }
});

const std::string configProtocolStackEmptyConf = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
        })
    }
});

const std::string configProtocolStackMissingTransport = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0"
            }
        })
    }
});

const std::string configProtocolStackMissingApplication = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip": "0.0.0.0",
                            "port" : 102
                        }
                    ]
                }
            }
        })
    }
});

const std::string configProtocolStackMissingIedName = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "connections" : [
                        {
                            "srv_ip": "0.0.0.0",
                            "port" : 102
                        }
                    ]
                }
            }
        })
    }
});

const std::string configProtocolStackIedNameBadFormat = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : 8102,
                    "connections" : [
                        {
                            "srv_ip": "0.0.0.0",
                            "port" : 102
                        }
                    ]
                }
            }
        })
    }
});

const std::string configProtocolStackMissingConnection = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO"
                }
            }
        })
    }
});

const std::string configProtocolStackConnectionBadFormat = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : {
                            "srv_ip": "0.0.0.0",
                            "port" : 102
                    }
                }
            }
        })
    }
});

const std::string configProtocolStackMissingSrvIp = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "port" : 102
                        }
                    ]
                }
            }
        })
    }
});

const std::string configProtocolStackSrvIpBadFormat = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : 102
                        }
                    ]
                }
            }
        })
    }
});

const std::string configProtocolStackMissingPort = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0"
                        }
                    ]
                }
            }
        })
    }
});

const std::string configProtocolStackPortBadFormat = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : "8102"
                        }
                    ]
                }
            }
        })
    }
});


const std::string configProtocolStackInvalidIPAddress = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "192.168.1.345",
                            "port" : 8102
                        }
                    ]
                }
            }
        })
    }
});

const std::string configValidOsiConnectionParameters = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "value" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 102,
                            "osi" : {
                                "local_ap_title": "1,3,9999.13",
                                "local_ae_qualifier": 12,
                                "remote_ap_title": "1,2,1200,15,3",
                                "remote_ae_qualifier": 1,
                                "local_psel": "0x12, 0x34, 0x56, 0x78",
                                "local_ssel": "0, 1, 2, 3, 4",
                                "local_tsel": "0x00,0x01, 0x02",
                                "remote_psel": "0x87, 0x65, 0x43, 0x21",
                                "remote_ssel": "0, 1",
                                "remote_tsel": "0x00, 0x01"
                            }
                        },
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 8102
                        }
                    ]
                },
                "application_layer" : {
                }
            }
        })
    }
});

const std::string configOsiConnectionBadLocalAe = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "value" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 102,
                            "osi" : {
                                "local_ae_qualifier": "12"
                            }
                        }
                    ]
                },
                "application_layer" : {
                }
            }
        })
    }
});

const std::string configOsiConnectionBadRemoteAe = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "value" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 102,
                            "osi" : {
                                "remote_ae_qualifier": "12"
                            }
                        }
                    ]
                },
                "application_layer" : {
                }
            }
        })
    }
});

const std::string configOsiConnectionBadLocalAp = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "value" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 102,
                            "osi" : {
                                "local_ap_title": 12
                            }
                        }
                    ]
                },
                "application_layer" : {
                }
            }
        })
    }
});

const std::string configOsiConnectionBadRemoteAp = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "value" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 102,
                            "osi" : {
                                "remote_ap_title": 12.5
                            }
                        }
                    ]
                },
                "application_layer" : {
                }
            }
        })
    }
});

const std::string configOsiConnectionInvalidLocalAp = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "value" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 102,
                            "osi" : {
                                "local_ap_title": "1,Err,1200,15,3"
                            }
                        }
                    ]
                },
                "application_layer" : {
                }
            }
        })
    }
});

const std::string configOsiConnectionInvalidRemoteAp = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "value" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 102,
                            "osi" : {
                                "remote_ap_title": "1, 2,1200,15,3"
                            }
                        }
                    ]
                },
                "application_layer" : {
                }
            }
        })
    }
});

const std::string configOsiConnectionSelectorTooLong = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "value" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 102,
                            "osi" : {
                                "remote_tsel": "0,1,2,3,4,5,6,7,8,9,0,1,2,3,4"
                            }
                        }
                    ]
                },
                "application_layer" : {
                }
            }
        })
    }
});

const std::string configOsiConnectionSelectorNotAByte1 = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "value" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 102,
                            "osi" : {
                                "remote_ssel": "0,1,2,Three,4,5,6,7,8"
                            }
                        }
                    ]
                },
                "application_layer" : {
                }
            }
        })
    }
});

const std::string configOsiConnectionSelectorNotAByte2 = QUOTE({
    "plugin" : {
        "description" : "iec61850 south plugin",
        "type" : "string",
        "value" : "iec61850",
        "readonly" : "true"
    },

    "asset" : {
        "description" : "Asset name",
        "type" : "string",
        "value" : "iec61850",
        "displayName" : "Asset Name",
        "order" : "2",
        "mandatory" : "true"
    },

    "protocol_stack" : {
        "description" : "protocol stack parameters",
        "type" : "JSON",
        "displayName" : "Protocol stack parameters",
        "order" : "3",
        "value" : QUOTE({
            "protocol_stack" : {
                "name" : "iec61850client",
                "version" : "1.0",
                "transport_layer" : {
                    "ied_name" : "simpleIO",
                    "connections" : [
                        {
                            "srv_ip" : "0.0.0.0",
                            "port" : 102,
                            "osi" : {
                                "remote_ssel": "0,1,2,345,7,8,9"
                            }
                        }
                    ]
                },
                "application_layer" : {
                }
            }
        })
    }
});
// *INDENT-ON*
//
#endif
