#include "RemoteReservoir.h"
#include "LoggingBase.h"

RemoteReservoir::RemoteReservoir(const Config& cfg)
    : cfg_(cfg) {}

bool RemoteReservoir::consume(const tcpmsg::TCPMessenger::ReceivedMessage& rx) {
    if (rx.type != tcpmsg::formats::ReservoirInfo::TYPE_ID) {
        return false;
    }

    if (rx.chanId != cfg_.channelID) {
        return false;
    }

    if (cfg_.checkSourceMAC && rx.srcMac != cfg_.expectedSourceMAC) {
        return false;
    }

    tcpmsg::formats::ReservoirInfo decoded;
    if (!decoded.fromBuffer(rx.payload.data(), static_cast<uint16_t>(rx.payload.size()))) {
        gLogger->println("RemoteReservoir: Failed to decode ReservoirInfo message");
        return false;
    }

    info_ = decoded;
    status_ = Status::Fresh;
    lastSeenReservoirIp_ = rx.peerIp;
    lastSeenMs_ = millis();
    return true;
}

void RemoteReservoir::loop() {
    if (status_ != Status::Fresh) {
        return;
    }

    const uint32_t nowMs = millis();
    if ((nowMs - lastSeenMs_) >= cfg_.staleTimeoutMs) {
        status_ = Status::Stale;
    }
}

RemoteReservoir::Status RemoteReservoir::status() const {
    return status_;
}

uint32_t RemoteReservoir::lastSeenMs() const {
    return lastSeenMs_;
}

const tcpmsg::formats::ReservoirInfo& RemoteReservoir::info() const {
    return info_;
}

const RemoteReservoir::Config& RemoteReservoir::config() const {
    return cfg_;
}