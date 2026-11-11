#ifndef ARMORTARGET_H
#define ARMORTARGET_H

#include "TrackingBase.hpp"

class ArmorTarget : public TrackingBase{
public:
    ArmorTarget(const rm::InputFrame& frame,int i) : TrackingBase(frame,i){

    }
    int lostThreshold() const override {
        return 3;
    }
    void target_tackle(double dt_ns) override;
};

#endif