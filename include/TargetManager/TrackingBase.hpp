#ifndef TRACKINGBASE_H
#define TRACKINGBASE_H

#include "data_types.hpp"
#include <vector>

struct last_Observation{
    int id;
    double last_angle;
    double last_x;
    double last_y;
    double predicted_x;
    double predicted_y;
    double velocity_x;
    double velocity_y;
    double angular_velocity;
    double predicted_angle;
    int missed_count;
    rm::TargetType type;
    rm::TargetStatus status;
    std::int64_t last_ts_ns;
};

struct target_data{
    int id;                  
    rm::TargetType type;         
    rm::TargetStatus status;     
    double x;                
    double y;                
    double velocity_x;       
    double velocity_y;       
    double predicted_x;      
    double predicted_y;      
    double angle;            
    double angular_velocity; 
    double predicted_angle;  
    int missed_count;
    std::int64_t ts_ns;       
};

class TrackingBase{
protected:
    last_Observation l_obs;
    target_data t_data;
public:
    TrackingBase(const rm::InputFrame& frame,int i) : l_obs{0,0,0,0,0,0,0,0,0,0,0,frame.observations[i].type,rm::TargetStatus::LOST,0},t_data{frame.observations[i].id,frame.observations[i].type,rm::TargetStatus::TRACK,frame.observations[i].x,frame.observations[i].y,0,0,0,0,frame.observations[i].angle,0,0,0,frame.timestamp_ns}{}
    virtual ~TrackingBase() = default;
    int id();
    rm::TargetStatus status();
    rm::TargetType type();
    virtual void target_tackle(double dt_ns) = 0;
    void removedata(const rm::InputFrame& frame,int i);
    void writedata(const rm::InputFrame& frame,int i);
    static void tackle_lost(TrackingBase* t,double dt_ns);
    virtual int lostThreshold() const = 0;
    rm::TargetSnapshot writesnapshot();
};

#endif
