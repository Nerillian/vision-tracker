#include "TargetManager/SpinningTarget.hpp"
#include "data_types.hpp"
#include <iostream>

void SpinningTarget::target_tackle(double dt_ns){
    if(l_obs.id == 0){
        t_data.predicted_x = t_data.x;
        t_data.predicted_y = t_data.y;
        t_data.predicted_angle = t_data.angle;
    }else if(l_obs.id != 0 && l_obs.status != rm::TargetStatus::LOST){
        const double dt_obs = (t_data.ts_ns - l_obs.last_ts_ns) / 1e9;
        if(dt_obs <= 0.0){
            return;
        }
        t_data.velocity_x = (t_data.x - l_obs.last_x)/dt_obs;
        t_data.velocity_y = (t_data.y - l_obs.last_y)/dt_obs;
        t_data.angular_velocity = (t_data.angle - l_obs.last_angle)/dt_obs;
        t_data.predicted_x = t_data.x + t_data.velocity_x*dt_ns;
        t_data.predicted_y = t_data.y + t_data.velocity_y*dt_ns;
        t_data.predicted_angle = t_data.angle + t_data.angular_velocity*dt_ns;
    }else if(l_obs.id != 0 && l_obs.status == rm::TargetStatus::LOST){
        t_data.predicted_x = t_data.x;
        t_data.predicted_y = t_data.y;
        t_data.predicted_angle = t_data.angle;
    }
}