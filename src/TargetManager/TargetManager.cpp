#include "TargetManager/TargetManager.hpp"
#include "TargetManager/TrackingBase.hpp"
#include "TargetManager/ArmorTarget.hpp"
#include "TargetManager/SpinningTarget.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include "data_types.hpp"

bool TargetManager::has_last_frame(const rm::InputFrame& frame){
    if(frame.frame_id==0){
        return false;
    }
    return true;
}

bool TargetManager::isframeidok(const rm::InputFrame& frame){
    if(frame.frame_id > last_frame_.last_frame_id){
        return true;
    }
    return false;
}

bool TargetManager::isnsok(const rm::InputFrame& frame){
    if(frame.timestamp_ns > last_frame_.last_timestamp_ns){
        return true;
    }
    return false;
}

bool TargetManager::isidok(const rm::InputFrame& frame){
    for (std::size_t i = 0; i < frame.observations.size(); ++i){
        if (frame.observations[i].id < 0){
            return false;
        }
    }
    return true;
}

bool TargetManager::isidokk(const rm::InputFrame& frame){
    int n=0;
    if(frame.observations.size() > 1){
        for(std::size_t i=0;i<frame.observations.size();i++){
            std::size_t j = i+1;
            while(j<frame.observations.size()){
                if(frame.observations[i].id == frame.observations[j].id){
                    n=-100;
                }else if(frame.observations[i].id != frame.observations[j].id){
                    n++;
                }
                j++;
            }
        }
        if(n > 0){
            return true;
        }else{
            return false;
        }
    }
    return true;
}

bool TargetManager::istypeok(const rm::InputFrame& frame){
    for(std::size_t i=0;i<frame.observations.size();i++){
        for(auto& t : targets_){
            if(t->id() == frame.observations[i].id){
                if(t->type() != frame.observations[i].type){
                    return false;
                }
            }
        }
    }
    return true;
}

bool TargetManager::validate(const rm::InputFrame& frame){
    if(TargetManager::has_last_frame(frame)){
        if(TargetManager::isframeidok(frame)            &&
           TargetManager::isnsok(frame)                 &&
           TargetManager::isidok(frame)                 &&
           TargetManager::isidokk(frame)                &&
           TargetManager::istypeok(frame)                 )
           {
            return true;
           }else{
            return false;
           }
    }else if(!TargetManager::has_last_frame(frame)){
        if(TargetManager::isidok(frame)                 &&
           TargetManager::isidokk(frame)                &&
           TargetManager::istypeok(frame)                 )
           {
            return true;
           }else{
            return false;
           }
    }
    return false;
}

TrackingBase* TargetManager::findtarget(int id){
    for(auto& t : targets_){
        if(t->id() == id){
            return t.get();
        }
    }
    return nullptr;
}

TargetManager::~TargetManager() = default;

std::unique_ptr<TrackingBase> TargetManager::createTarget(const rm::InputFrame& frame,int i){
    switch (frame.observations[i].type) {
        case rm::TargetType::ARMOR:    return std::make_unique<ArmorTarget>(frame, i);
        case rm::TargetType::SPINNING: return std::make_unique<SpinningTarget>(frame, i);
    }
    return nullptr;
}

void TargetManager::fillResult(rm::Result& result){
    for(auto& t : targets_){
            auto s = t->writesnapshot();
            result.targets.push_back(s);
            if(s.status == rm::TargetStatus::TRACK){
                result.tracking_target_count++;
            }else{
                result.lost_target_count++;
            }
            if(s.type == rm::TargetType::ARMOR){
                result.armor_target_count++;
            }else{
                result.spinning_target_count++;
            }
        }
    std::sort(result.targets.begin(), result.targets.end(),[](const rm::TargetSnapshot& a, const rm::TargetSnapshot& b){
            if(a.status != b.status){
                return a.status == rm::TargetStatus::TRACK;
            }
            const double da = a.x*a.x + a.y*a.y;
            const double db = b.x*b.x + b.y*b.y;
            if(da < db){
                return true;
            }
            if(db < da){
                return false;
            }
            return a.id < b.id;
    });
}

rm::Result TargetManager::onFrame(const rm::InputFrame& frame){
    tackled.clear();
    rm::Result result{};
    result.frame_id = frame.frame_id;
    double dt_ns = (frame.timestamp_ns - last_frame_.last_timestamp_ns)/1e9;
    if(TargetManager::validate(frame)){
        result.accepted = true;
        last_frame_.last_frame_id = frame.frame_id;
        last_frame_.last_timestamp_ns = frame.timestamp_ns;
        for(std::size_t i=0;i<frame.observations.size();i++){
            if(TrackingBase* t = findtarget(frame.observations[i].id)){
                if(t->status() == rm::TargetStatus::LOST){
                    t->writedata(frame,i);
                    t->target_tackle(dt_ns);
                    tackled.push_back(frame.observations[i].id);
                }else if(t->status() == rm::TargetStatus::TRACK){
                    t->removedata(frame,i);
                    t->target_tackle(dt_ns);
                    tackled.push_back(frame.observations[i].id);
                }
            }else if(auto t = createTarget(frame,i)){
                t->writedata(frame,i);
                t->target_tackle(dt_ns);
                targets_.push_back(std::move(t));
                tackled.push_back(frame.observations[i].id);
            }
        }
        for (auto& t : targets_) {
            bool observed = false;
            for (int id : tackled){
                if (t->id() == id){
                    observed = true;
                    break;
                }
            }
            if (!observed){
                TrackingBase::tackle_lost(t.get(), dt_ns);
            }
        }
        fillResult(result);
        return result;
    }
    result.accepted = false;
    fillResult(result);
    return result;
}