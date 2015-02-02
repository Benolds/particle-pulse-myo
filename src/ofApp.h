#pragma once

#include "ofMain.h"
#include "Particle.h"
#include <myo/myo.hpp>
#include "DataCollector.h"
#include "ofxTonic.h"

using namespace Tonic;

class ofApp : public ofBaseApp{
public:

#pragma mark - Default openFrameworks Methods
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

private:

#pragma mark - Particle Pulse Custom Variables
    ofVec2f mousePos;
    std::vector<Particle *>particles;
    void wrapOnScreenBounds(Particle* p);
    void countNeighbors(Particle* p, float threshold);
    ofVec2f getWindowCenter();
    ofVec2f getMouseToCenter();
//        ofVec2f getPerpendicularVector(ofVec2f startVec);
//        ofVec2f addNoiseToVec(ofVec2f baseVec, float dMult, float dAdd);
    void spawnRandomParticles(int numToSpawn);
    void spawnVolumeBasedParticles();
    void mergeIfNeeded(Particle* p, float mergeThreshold);

    //audio
    ofSoundStream soundStream;
    void audioReceived(float *input, int bufferSize, int nChannels);
//        void audioOut(float *output, int bufferSize, int nChannels);

    float rawVolume;
    float volumePercent;
    std::vector<float> lastInput;
    std::vector<float> last2Input;

    ofSoundPlayer soundPlayer;

    float neighborThresholdAdjustment;

    int counter;
    std::vector<float>volumeHistory;

    float maxVol;
    float scaleFactor;

    void drawSnowflakeHistogram(float baseHeight);
    void drawHistogram(float baseHeight, bool leftToRight, bool bottomToTop);

    float volumeCalibration;
    
#pragma mark - Myo Custom Methods
    
    void setupMyo();
    void setupAudio();
    void updateMyo();
    void drawMyo();
    
#pragma mark - Myo Custom Variables
    
    myo::Hub* hub;
    //    myo::Myo* myo;
    std::vector<myo::Myo*> knownMyos;
    
    DataCollector collector;
    
    int roll_w, pitch_w, yaw_w;
    float accel_x, accel_y, accel_z;
    float d_accel_x, d_accel_y, d_accel_z;
    int timeCounter;
    float roll, pitch, yaw;
    std::vector<float> emgVals;
    std::vector<float> accelXValues;
    std::vector<float> accelYValues;
    std::vector<float> accelZValues;
    
    ofColor backgroundColor;
    ofColor bgColor;
    
    void triggerVisuals();
    void triggerAudio();
    
#pragma mark - ofxTonic Custom Variables & Methods
    
    ofxTonicSynth synth;
//    int scaleDegree;
    void trigger();
//    void setScaleDegreeBasedOnMouseX();

};
