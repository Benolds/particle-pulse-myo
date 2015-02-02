//
//  DataCollector.h
//  emptyExample
//
//  Created by Benjamin Reynolds on 1/11/15.
//
//

#ifndef __emptyExample__DataCollector__
#define __emptyExample__DataCollector__

#include <stdio.h>
#include <myo/myo.hpp>

// Classes that inherit from myo::DeviceListener can be used to receive events from Myo devices. DeviceListener
// provides several virtual functions for handling different kinds of events. If you do not override an event, the
// default behavior is to do nothing.
class DataCollector : public myo::DeviceListener {
private:
#pragma mark - Event Listeners
    
    void onPair(myo::Myo* myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion);
    
    // onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
    void onUnpair(myo::Myo* myo, uint64_t timestamp);
    
    // onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
    // as a unit quaternion.
    void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat);
    
    // onPose() is called whenever the Myo detects that the person wearing it has changed their pose, for example,
    // making a fist, or not making a fist anymore.
    void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose);
    
    void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg);
    
    // onArmSync() is called whenever Myo has recognized a Sync Gesture after someone has put it on their
    // arm. This lets Myo know which arm it's on and which way it's facing.
    void onArmSync(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection);
    
    // onArmUnsync() is called whenever Myo has detected that it was moved from a stable position on a person's arm after
    // it recognized the arm. Typically this happens when someone takes Myo off of their arm, but it can also happen
    // when Myo is moved around on the arm.
    void onArmUnsync(myo::Myo* myo, uint64_t timestamp);
    
    // onUnlock() is called whenever Myo has become unlocked, and will start delivering pose events.
    void onUnlock(myo::Myo* myo, uint64_t timestamp);
    
    // onLock() is called whenever Myo has become locked. No pose events will be sent until the Myo is unlocked again.
    void onLock(myo::Myo* myo, uint64_t timestamp);
    
    void onAccelerometerData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3<float> &accel);
    
public:
    // These values are set by onArmSync() and onArmUnsync() above.
    bool onArm;
//    myo::Arm whichArm;
    myo::Arm arm1;
    myo::Arm arm2;
    
    // This is set by onUnlocked() and onLocked() above.
    bool isUnlocked;
    
    // These values are set by onOrientationData() and onPose() above.
//    int roll_w, pitch_w, yaw_w;
    float roll, pitch, yaw;
    float accel_x, accel_y, accel_z;
    myo::Pose currentPose;
    
    //int emgVals[8];
    std::vector<float> emgVals;
    
    DataCollector()
    : onArm(false), isUnlocked(false), /*roll_w(0), pitch_w(0), yaw_w(0),*/ currentPose()
    {
    }
    
    void setupDataCollector();
    
    std::vector<myo::Myo*> knownMyos;
    size_t identifyMyo(myo::Myo* myo);
    
#pragma mark - Print
    
    // There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
    // For this example, the functions overridden above are sufficient.
    
    // We define this function to print the current values that were updated by the on...() functions above.
    void print();
    
#pragma mark - Getters
    
//    int getRoll_W();
//    int getPitch_W();
//    int getYaw_W();
    
    float getRoll(myo::Arm);
    float getPitch(myo::Arm);
    float getYaw(myo::Arm);
    
    float getAccelX(myo::Arm);
    float getAccelY(myo::Arm);
    float getAccelZ(myo::Arm);
    
    std::vector<float> getEmgData();
    
    myo::Arm getArmForInt(int myoNumber);

};

#endif /* defined(__emptyExample__DataCollector__) */
