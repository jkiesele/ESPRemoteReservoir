#pragma once

#include <Arduino.h>

#include "TCPMessenger.h"
#include "DataFormats.h"

class RemoteReservoir {
public:
    enum class Status : uint8_t {
        Unknown = 0,
        Fresh,
        Stale
    };

    struct Config {
        uint8_t channelID = 0;
        uint32_t staleTimeoutMs = 120000;

        bool checkSourceMAC = false;
        tcpmsg::MACAddress expectedSourceMAC;
    };

    explicit RemoteReservoir(const Config& cfg);

    bool consume(const tcpmsg::TCPMessenger::ReceivedMessage& rx);
    void loop();

    Status status() const;
    uint32_t lastSeenMs() const;

    const tcpmsg::formats::ReservoirInfo& info() const;
    const Config& config() const;

    void setChannelID(uint8_t chanId) { cfg_.channelID = chanId; }

    const IPAddress& lastSeenReservoirIp() const { return lastSeenReservoirIp_; }

private:
    Config cfg_;
    tcpmsg::formats::ReservoirInfo info_{};
    Status status_ = Status::Unknown;
    uint32_t lastSeenMs_ = 0;
    IPAddress lastSeenReservoirIp_;
};