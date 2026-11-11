#ifndef TARGETMANAGER_H
#define TARGETMANAGER_H

#include "data_types.hpp"
#include <vector>
#include <memory>

class TrackingBase;

struct last_frame{
    int last_frame_id;
    std::int64_t last_timestamp_ns;
};

class TargetManager{
private:
    last_frame last_frame_{0,0};
    std::vector<std::unique_ptr<TrackingBase>> targets_;
    std::vector<int> tackled;
public:
    ~TargetManager();
    bool has_last_frame(const rm::InputFrame &frame);
    bool isframeidok(const rm::InputFrame& frame);
    bool isnsok(const rm::InputFrame& frame);
    bool isidok(const rm::InputFrame& frame);
    bool isidokk(const rm::InputFrame& frame);
    bool istypeok(const rm::InputFrame& frame);
    bool validate(const rm::InputFrame& frame);
    TrackingBase* findtarget(int id);
    rm::Result onFrame(const rm::InputFrame& frame);
    std::unique_ptr<TrackingBase> createTarget(const rm::InputFrame& frame,int i);
    void fillResult(rm::Result& result);
};

#endif