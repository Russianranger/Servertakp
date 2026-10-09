#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace BotFollow {
// Server path movement is integer_speed * .4 * 1.45 world units/second.
// Native player reports in the captured straight runs use ~20 velocity ticks/sec.
constexpr float WorldSpeedPerInteger = .58f;
constexpr float ClientVelocityTicks = 20.f;
inline float PacketVelocity(int integer_speed) { return integer_speed*WorldSpeedPerInteger/ClientVelocityTicks; }
// Position reports can be a second apart even while velocity stays nonzero.
// Measure over actual position changes, not over every server AI tick.
struct Motion {
    bool initialized=false, has_velocity=false;
    float x=0,y=0,speed=0,dir_x=0,dir_y=0;
    uint64_t sampled=0;
    void Update(uint64_t now,float nx,float ny,float dx,float dy) {
        const float length=std::hypot(dx,dy);
        const bool velocity=std::isfinite(length)&&length>.001f;
        if(!initialized || now<sampled) {
            initialized=true;x=nx;y=ny;sampled=now;speed=0;has_velocity=false;
        }
        const float distance=std::hypot(nx-x,ny-y);
        if(distance>.1f) {
            const auto elapsed=now-sampled;
            const float measured=elapsed?distance*1000.f/elapsed:0;
            if(elapsed>=100 && elapsed<=1500 && measured<200.f && has_velocity)
                speed=speed>0?.5f*(speed+measured):measured;
            else speed=0;
            x=nx;y=ny;sampled=now;
        }
        if(velocity && !has_velocity) {sampled=now;speed=0;}
        has_velocity=velocity;
        if(!velocity) {speed=0;dir_x=dir_y=0;return;}
        dir_x=dx/length;dir_y=dy/length;
        // Initial estimate only, calibrated by subsequent real position reports.
        if(speed<=0) speed=std::min(100.f,length*20.f);
    }
    bool Moving(uint64_t now) const {return has_velocity && now>=sampled && now-sampled<=1500;}
    float Lead(uint64_t now) const {return Moving(now)?speed*std::min(1.1f,(now-sampled)/1000.f):0;}
    float X(uint64_t now) const {return x+dir_x*Lead(now);}
    float Y(uint64_t now) const {return y+dir_y*Lead(now);}
};
// Extend the navigation endpoint beyond the current formation slot while moving.
// Pace is based on the unextended slot, so this does not request a speed boost.
constexpr float SteeringSeconds = .35f;
struct PaceFilter {
    bool initialized=false;
    int value=0;
    uint64_t changed=0;
    int Update(uint64_t now,int target) {
        if(!initialized || target<=0 || now<changed) {initialized=true;value=target;changed=now;return value;}
        if(now-changed>=200) {value+=std::max(-8,std::min(8,target-value));changed=now;}
        return value;
    }
    void Reset() {initialized=false;}
};
inline bool ShouldMove(float gap,float desired,bool owner_moving,bool already_moving) {
    if(owner_moving) return gap>std::max(3.f,desired-1.f);
    return gap>std::max(3.f,desired+(already_moving?-.5f:1.f));
}
struct Point { float x,y; };
// End the route outside the owner's personal space, even between AI updates.
// A small moving allowance prevents reaching the endpoint every update.
inline Point FollowTarget(float bx,float by,float ox,float oy,float desired,bool moving,float speed=0.f) {
    const float gap=std::hypot(ox-bx,oy-by);
    const float spacing=std::max(3.f,desired-(moving?std::max(2.f,speed*SteeringSeconds):0.f));
    if(gap<=spacing) return {bx,by};
    return {ox+(bx-ox)*spacing/gap,oy+(by-oy)*spacing/gap};
}
// Physical movement uses integer speed * .58 world units per second.
inline int Pace(int maximum,float owner_speed,float error,bool owner_moving, bool keep_pace=false) {
    if(maximum<=0) return maximum;
        const float gain=keep_pace?1.f:2.f;
    float units=owner_moving?owner_speed+gain*error:gain*std::max(0.f,error);
    if(keep_pace && owner_moving && owner_speed>0 && std::isfinite(owner_speed)) {
        // Bounded catch-up only: do not grant a permanent run-speed stat bonus.
        maximum=std::max(maximum,static_cast<int>(std::ceil(std::min(200.f,owner_speed+12.f)/.58f)));
    }
    // Animation has coarse integer steps, but physical speed need not share
    // those steps. Eight-unit rounding forced steady following to alternate
    // between 27.84 and 32.48 units/sec for a ~29-unit/sec player.
    int desired=std::max(8,static_cast<int>(std::round(units/.58f)));
    return std::min(maximum,desired);
}
}
