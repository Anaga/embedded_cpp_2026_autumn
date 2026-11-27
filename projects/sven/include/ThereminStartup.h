#pragma once

namespace sounder {

// One startup request per boot, owned by the main task. Readiness may change
// during an attempt; retry until starting succeeds or a manual command cancels.
class ThereminStartup {
public:
    void cancel() { pending_ = false; }

    template <typename Start>
    void update(bool ready, Start start) {
        if (pending_ && ready && start()) pending_ = false;
    }

private:
    bool pending_ = true;
};

} // namespace sounder
