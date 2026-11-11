#include "TargetManager/TrackingBase.hpp"
#include <iostream>
#include "data_types.hpp"

int TrackingBase::id(){
    return t_data.id;
}

void TrackingBase::removedata(const rm::InputFrame& frame,int i){
    l_obs.id = t_data.id;
    l_obs.last_angle = t_data.angle;
    l_obs.last_x = t_data.x;
    l_obs.last_y = t_data.y;
    l_obs.last_ts_ns = t_data.ts_ns;
    l_obs.predicted_x = t_data.predicted_x;
    l_obs.predicted_y = t_data.predicted_y;
    l_obs.velocity_x = t_data.velocity_x;
    l_obs.velocity_y = t_data.velocity_y;
    l_obs.angular_velocity = t_data.angular_velocity;
    l_obs.predicted_angle = t_data.predicted_angle;
    l_obs.missed_count = t_data.missed_count;
    l_obs.type = t_data.type;
    l_obs.status = t_data.status;
    t_data = {};
    t_data.ts_ns = frame.timestamp_ns;
    t_data.status = rm::TargetStatus::TRACK;
    t_data.missed_count = 0;
    t_data.id = frame.observations[i].id;
    t_data.x = frame.observations[i].x;
    t_data.y = frame.observations[i].y;
    t_data.angle = frame.observations[i].angle;
    t_data.type = frame.observations[i].type;
}

void TrackingBase::writedata(const rm::InputFrame& frame,int i){
    t_data.id = frame.observations[i].id;
    t_data.x = frame.observations[i].x;
    t_data.y = frame.observations[i].y;
    t_data.angle = frame.observations[i].angle;
    t_data.type = frame.observations[i].type;
    t_data.ts_ns = frame.timestamp_ns;
    t_data.status = rm::TargetStatus::TRACK;
    t_data.missed_count = 0;
    t_data.velocity_x = 0.0;
    t_data.velocity_y = 0.0;
    t_data.angular_velocity = 0.0;
    t_data.predicted_x = t_data.x;
    t_data.predicted_y = t_data.y;
    t_data.predicted_angle = t_data.angle;
}

void TrackingBase::tackle_lost(TrackingBase* t,double dt_ns){
    if(t->t_data.status == rm::TargetStatus::LOST){
        return;
    }
    t->t_data.missed_count ++;
    if(t->t_data.missed_count >= t->lostThreshold()){
        t->l_obs={t->t_data.id,0,0,0,0,0,0,0,0,0,0,t->t_data.type,rm::TargetStatus::LOST,0};
        t->t_data={t->t_data.id,t->t_data.type,rm::TargetStatus::LOST,0,0,0,0,0,0,0,0,0,0,0};
        t->t_data.missed_count = t->lostThreshold();
        return;
    }
    t->t_data.predicted_x += t->t_data.velocity_x*dt_ns;
    t->t_data.predicted_y += t->t_data.velocity_y*dt_ns;
    if (t->t_data.type == rm::TargetType::SPINNING) {
        t->t_data.angle += t->t_data.angular_velocity * dt_ns;
        t->t_data.predicted_angle = t->t_data.angle + t->t_data.angular_velocity * dt_ns;
    }
}

rm::TargetStatus TrackingBase::status(){
    return t_data.status;
}

rm::TargetType TrackingBase::type(){
    return t_data.type;
}

rm::TargetSnapshot TrackingBase::writesnapshot(){
    rm::TargetSnapshot snapshot;
    snapshot={t_data.id,t_data.type,t_data.status,t_data.x,t_data.y,t_data.velocity_x,t_data.velocity_y,t_data.predicted_x,t_data.predicted_y,t_data.angle,t_data.angular_velocity,t_data.predicted_angle,t_data.missed_count};
    return snapshot;
}