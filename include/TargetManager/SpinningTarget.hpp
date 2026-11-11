#ifndef SPINNINGTARGET_H
#define SPINNINGTARGET_H

#include "TrackingBase.hpp"

class SpinningTarget : public TrackingBase{
public:
    SpinningTarget(const rm::InputFrame& frame,int i) : TrackingBase(frame,i){

    }
    int lostThreshold() const override {
        return 5;
    }
    void target_tackle(double dt_ns) override;
};

#endif