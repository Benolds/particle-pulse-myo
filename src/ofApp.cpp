#include "ofApp.h"
#include "Constants.h"
#include "Utils.h"

#pragma mark - Setup
//--------------------------------------------------------------
void ofApp::setup(){
    srand (time(NULL));
    
//    this->spawnRandomParticles(100);
    
    // set up the audio receiver
    lastInput.resize(bbInputSize); // volume levels from prev timestep
    last2Input.resize(bbInputSize); // volume levels from 2 timesteps ago
    soundStream.setup(this, 0, 1, 44100, 512, 4);
    
    neighborThresholdAdjustment = -50.0f;
    volumeCalibration = 0.5f;
    
    // for timekeeping purposes to know if volumeHistory is filled or not (also used for histogram drawing)
    counter = 0;
    
    backgroundColor = bbDefaultBgColor;
    
    this->setupMyo();
    this->setupAudio();
}

void ofApp::setupMyo(){
    // We catch any exceptions that might occur below -- see the catch statement for more details.
    try {
        
        // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
        // publishing your application. The Hub provides access to one or more Myos.
        //        myo::Hub hub("com.example.hello-myo");
        hub = new myo::Hub("com.example.hello-myo");
        //        hub = myo::Hub("com.example.hello-myo");
        
        std::cout << "Attempting to find a Myo..." << std::endl;
        
        // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
        // immediately.
        // waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
        // if that fails, the function will return a null pointer.
        myo::Myo* myo1 = hub->waitForMyo(10000);
        
        // If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
        if (!myo1) {
            throw std::runtime_error("Unable to find a Myo!");
        }
        
        // We've found a Myo.
        std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
        
        myo1->setStreamEmg(myo::Myo::streamEmgEnabled);
        knownMyos.push_back(myo1);
        
        ///////////////
        
        std::cout << "Attempting to find a Myo..." << std::endl;
        
        // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
        // immediately.
        // waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
        // if that fails, the function will return a null pointer.
        myo::Myo* myo2 = hub->waitForMyo(10000);
        
        // If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
        if (!myo2) {
            throw std::runtime_error("Unable to find a Myo!");
        }
        
        // We've found a Myo.
        std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
        
        myo2->setStreamEmg(myo::Myo::streamEmgEnabled);
        knownMyos.push_back(myo2);
        
        collector.knownMyos = knownMyos;
        
        ///////////////
        
        
        // Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
        
        collector.setupDataCollector();
        
        // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
        // Hub::run() to send events to all registered device listeners.
        hub->addListener(&collector);
        
        // If a standard exception occurred, we print out its message and exit.
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
}

void ofApp::setupAudio(){
    ofSoundStreamSetup(2, 0, this, 44100, 256, 4);
    ControlGenerator midiNote = synth.addParameter("midiNumber");
    
    //    // convert a midi note to a frequency (plugging that parameter into another object)
    ControlGenerator noteFreq =  ControlMidiToFreq().input(midiNote);
    
    // create a named parameter on the synth which we can set at runtime
    //    ControlGenerator noteFreq = synth.addParameter("noteFreq");
    ControlGenerator roll = synth.addParameter("roll");
    ControlGenerator pitch = synth.addParameter("pitch");
    ControlGenerator yaw = synth.addParameter("yaw");
    
    // convert a midi note to a frequency (plugging that parameter into another object)
    //    ControlGenerator noteFreq =  ControlMidiToFreq().input(midiNote);
    
    // Here's the actual noise-making object
    Generator tone = SineWave().freq( noteFreq );
    
    //    Generator tone = SineWave().freq(440);
    //+ 400 );// * SineWave().freq(50);
    
    // It's just a steady tone until we modulate the amplitude with an envelope
    ControlGenerator envelopeTrigger = synth.addParameter("trigger");
    
    ADSR env = ADSR()
    .attack(0.003)
    .decay( 1.0 )
    .sustain(0)
    .release(.118)
    .doesSustain(false)
    .trigger(envelopeTrigger);
    
    //    StereoDelay delay = StereoDelay(3.0f,3.0f)
    //    .delayTimeLeft( 0.5 + SineWave().freq(0.2) * 0.01)
    //    .delayTimeRight(0.55 + SineWave().freq(0.23) * 0.01)
    //    .feedback(0.3)
    //    .dryLevel(0.8)
    //    .wetLevel(0.2);
    
    Generator toneMix = 0.1 * SquareWave() + 0.15 * SawtoothWave() + 0.27 * SineWave();
    
    //    Generator filterFreq = (SineWave().freq(0.01) + 1) * 200 + 225;
    
    //    LPF24 filter = LPF24().Q(2).cutoff( filterFreq );
    
    Generator output = (tone * toneMix * env); //(( tone * env ) >> filter >> delay) * 0.3;
    
    synth.setOutputGen(output);
}

#pragma mark - Audio
void ofApp::audioReceived(float *input, int bufferSize, int nChannels)
{
    float sum = 0.0f;
    for (int i = 0; i < bufferSize * nChannels; i++) {
        sum += input[i];
//        cout << "input[" << i << "] = " << input[i] << "\n";
        last2Input[i] = lastInput[i];
        lastInput[i] = input[i];
    }
    
    float avg = float(sum) / float(bufferSize);
    
    //cout << input[0] << "\n";
//    cout << avg << "\n";
    
    rawVolume = avg * volumeCalibration;
}

#pragma mark - Update loop & particle behavior

//--------------------------------------------------------------
void ofApp::update()
{
    this->updateMyo();
    
    // keep track of volume history
    
    volumePercent = MIN(1.0f, MAX(0.0f, (log2(abs(bbInputSize/10.0f * rawVolume *1028.0f)))));
    float volumePercentUnbounded = MAX(0.0f, (log2(abs(bbInputSize/10.0f * rawVolume * 1028.0f))));
    
    if (counter < bbVolumeHistoryLength) {
        counter++;
    } else {
        volumeHistory.erase(volumeHistory.begin());
    }
    volumeHistory.push_back(volumePercentUnbounded);
    
    // calculate max volume value //TODO: speedup. also speedup neighbor counting
    maxVol = 0;
    scaleFactor = 1.0f;
    for(int i = 0; i < counter; i++) {
        if (volumeHistory[i] > maxVol) {
            maxVol = volumeHistory[i];
        }
    }
    if (maxVol > 1.0f) {
        scaleFactor = 1.0f / float(maxVol);
    } else {
        scaleFactor = 1.0f;
    }
    
    volumePercent = volumePercent * scaleFactor;
    
    // spawn particles based on current volume level
    
    this->spawnVolumeBasedParticles();
    
    // add rotational velocity to particles based on mouse position
    
    //    cout << "vol % = " << volumePercent << "\n";
    
    //    ofVec2f mouseVec = getDistToCenter(mouseVec);
    ofVec2f mouseVec = getMouseToCenter();
    //    mouseVec = this->getPerpendicularVector(mouseVec);
    
    if ((mouseVec.x > ofGetWindowWidth()*0.5f) ||
        (mouseVec.x < ofGetWindowWidth()*-0.5f) ||
        (mouseVec.y > ofGetWindowHeight()*0.5f) ||
        (mouseVec.y < ofGetWindowHeight()*-0.5f)) {
        mouseVec = ofVec2f(0.0f, 0.0f);
    }
    
    float mouseRotationSpeed = 0.25f * mouseVec.length() / (0.5 * ofGetWindowHeight());
    
    if (mousePos.x < ofGetWindowWidth()*0.5) {
        mouseRotationSpeed *= -1.0f;
    }
    
    mouseRotationSpeed = -1 * roll;
    
    // update particles and apply inter-particle forces and size adjustments
    
    for(Particle* p : particles){
        //pre-update1
        
        p->setVolumeScale(bbInputSize/10.0f * rawVolume);
        p->neighborThresholdAdjustment = neighborThresholdAdjustment;
        
        ofVec2f particleToCenter = ofVec2f(p->pos - ofVec2f(ofGetWindowWidth()*0.5f, ofGetWindowHeight()*0.5f));
        ofVec2f particleVel = getPerpendicularVector(mouseRotationSpeed * addNoiseToVec(particleToCenter, 0.2f, 5.0f) * 0.05f);
        
        p->vel = particleVel * MAX(0.2, MIN(1.5, particleToCenter.length()/(ofGetWindowWidth()/2))); //this->addNoiseToVec(mouseVec, 0.2f, 5.0f) * 0.05f;
        
        //update1
        p->update();
        if ( ofRandom(p->lifetime + p->accel.length()) < 1.0f) { //p->lifetime <= 60 &&
            //p->accel.length() < 0.05f) {
            //            cout << p->accel.length() << "\n";
            p->flagForRemoval = true;
        }
        
        //post-update1
        this->wrapOnScreenBounds(p);
    }
    
    for(Particle* p : particles){
        this->mergeIfNeeded(p, bbMergeThreshold/*+(neighborThresholdAdjustment*(1.0/150.0f))*/);
    }
    
    for(Particle* p : particles){
        if (p->flagForRemoval) {
            particles.erase(std::remove(particles.begin(), particles.end(), p), particles.end());
        }
    }
    
    // second loop
    for(Particle* p : particles){
        //pre-update2
        
        float volumeAdjusted = sqrt(lerpVal(volumePercentUnbounded, volumePercent, 0.5f));
        
        this->countNeighbors(p, (bbNeighborThreshold+neighborThresholdAdjustment) * (1.0f + volumeAdjusted));
        
        //update2
        p->postUpdate();
    }
    
    if (ofRandom(100) < 10) {
        spawnRandomParticles(1);
    }
}

void ofApp::mergeIfNeeded(Particle *p, float mergeThreshold)
{
    if (p->flagForRemoval) { return; }
    
    for(Particle* n : particles){
        if (n != p && !n->flagForRemoval) {
            ofVec2f dPos = n->pos - p->pos;
            if (dPos.lengthSquared() < mergeThreshold*mergeThreshold) {
                if (n->lifetime >= p->lifetime) {
                    p->flagForRemoval = true;
                    n->lifetime += p->lifetime * 0.5f;
                    //n->baseRadius += p->baseRadius * 0.5f;
                }
            }
        }
    }
}

void ofApp::spawnRandomParticles(int numToSpawn)
{
    for (int i = 0; i < numToSpawn; i++) {
        Particle* p = new Particle(10.0f, bbBlueColor, ofVec2f(ofRandomWidth(),ofRandomHeight()), bbZeroVec, bbZeroVec, bbDefaultLifetime);
        particles.push_back(p);
    }
}

void ofApp::spawnVolumeBasedParticles()
{
    if (volumePercent > 0.0f) {
        spawnRandomParticles( floor(volumePercent * 5.0f * ofRandom(1)) );
    }
}

void ofApp::countNeighbors(Particle *p, float threshold)
{
    p->clearNeighbors();
    
    for(Particle* n : particles){
        if (n != p) {
            ofVec2f dPos = n->pos - p->pos;
            if (dPos.lengthSquared() < threshold*threshold) {
                p->addNeighbor(n);
            }
        }
    }
}

void ofApp::wrapOnScreenBounds(Particle *p)
{
    if (p->pos.x > ofGetWindowWidth()) {
        p->pos.x = 0.0f;
    } else if (p->pos.x < 0) {
        p->pos.x = ofGetWindowWidth();
    }
    
    if (p->pos.y > ofGetWindowHeight()) {
        p->pos.y = 0.0f;
    } else if (p->pos.y < 0) {
        p->pos.y = ofGetWindowHeight();
    }    
}

void ofApp::updateMyo() {
    
    timeCounter++;
    if (timeCounter > ofGetWindowWidth() - 200) {
        timeCounter = 0;
        accelXValues.clear();
        accelYValues.clear();
        accelZValues.clear();
    }
    
    try {
        // In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
        // In this case, we wish to update our display 20 times a second, so we run for 1000/20 milliseconds.
        hub->run(1000/20);
        // After processing events, we call the print() member function we defined above to print out the values we've
        // obtained from any events that have occurred.
        
        //collector.print();
        
        roll = collector.getRoll(myo::armLeft);
        pitch = collector.getPitch(myo::armLeft);
        yaw = collector.getYaw(myo::armLeft);
        
        //        printf("roll: %f\n", roll);
        //        printf("pitch: %f\n", pitch);
        //        printf("yaw: %f\n", yaw);
        
        d_accel_x = collector.getAccelX(myo::armRight) - accel_x;
        d_accel_y = collector.getAccelY(myo::armRight) - accel_y;
        d_accel_z = collector.getAccelZ(myo::armRight) - accel_z;
        
        float dlimit = 1.1f;
        if (abs(d_accel_x) > dlimit || abs(d_accel_y) > dlimit || abs(d_accel_z) > dlimit )
        {
            //            printf("\ndx: %f\n", d_accel_x);
            //            printf("dy: %f\n", d_accel_y);
            //            printf("dz: %f\n", d_accel_z);
            
            //            this->trigger();
        }
        
        accel_x = collector.getAccelX(myo::armRight);
        accel_y = collector.getAccelY(myo::armRight);
        accel_z = collector.getAccelZ(myo::armRight);
        
        accelXValues.push_back(accel_x);
        accelYValues.push_back(accel_y);
        accelZValues.push_back(accel_z);
        
        float limit = 1.1f;
        if (abs(accel_x) > limit || abs(accel_y) > limit || abs(accel_z) > limit )
        {
            //            printf("\nx: %f\n", accel_x);
            //            printf("y: %f\n", accel_y);
            //            printf("z: %f\n", accel_z);
        }
        
        
        if (abs(accel_x) > 0.5) {
            if (d_accel_x < -0.25f) {
                this->trigger();
            }
        }
        
        
        
        emgVals = collector.getEmgData();
        
        //if (emgVals.size() > 0) {
        //printf("len = %i", emgVals.size());
        
        //        printf("emg0: %f\n", emgVals[0]);
        //        printf("emg1: %f\n", emgVals[1]);
        //        printf("emg2: %f\n", emgVals[2]);
        //}
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
}

//--------------------------------------------------------------
void ofApp::trigger(){
    
    this->triggerVisuals();
    this->triggerAudio();
}

void ofApp::triggerVisuals()
{
    this->spawnRandomParticles(50);
    
    backgroundColor = ofColor(backgroundColor.r + ofRandom(-10, 10), backgroundColor.g + ofRandom(-10, 10), backgroundColor.b + ofRandom(-10, 10));
}

void ofApp::triggerAudio()
{
    //    static int twoOctavePentatonicScale[10] = {0, 2, 4, 7, 9, 12, 14, 16, 19, 21};
    //    int degreeToTrigger = floor(ofClamp(scaleDegree, 0, 9));
    //    cout << "degreeToTrigger = " << degreeToTrigger << endl;
    //
    //    // set a parameter that we created when we defined the synth
    //    synth.setParameter("midiNumber", 44 + (int)(roll * 10)); //44 + twoOctavePentatonicScale[degreeToTrigger]);
    //
    //    // simply setting the value of a parameter causes that parameter to send a "trigger" message to any
    //    // using them as triggers
    //    synth.setParameter("trigger", 1);
    
    static int twoOctavePentatonicScale[10] = {0, 2, 4, 7, 9, 12, 14, 16, 19, 21};
    int degreeToTrigger = floor(ofClamp((roll + PI/2)*2, 0, 9));
    cout << "degreeToTrigger = " << degreeToTrigger << endl;
    cout << "roll = " << roll << endl;
    
    cout << "Trigger!" << endl;
    
    // set a parameter that we created when we defined the synth
    synth.setParameter("midiNumber", 44 + (int)(roll * 10)); //44 + twoOctavePentatonicScale[degreeToTrigger]);
    
    // simply setting the value of a parameter causes that parameter to send a "trigger" message to any
    // using them as triggers
    synth.setParameter("trigger", 1);
}

#pragma mark - Drawing

//--------------------------------------------------------------
void ofApp::draw(){
    
//    float scaledVolume = 50.0f * float(bbInputSize) / 10.0f * rawVolume;
    
//    int r = lerpVal(ofGetBackground().r,scaledVolume,0.2f);
//    int g = lerpVal(ofGetBackground().g,scaledVolume,0.2f);
//    int b = lerpVal(ofGetBackground().b,scaledVolume,0.2f);
//    ofBackground(r, g, b);
    
    
    
    bgColor = ofColor(255 * (roll + PI)/(2*PI), 255 * (pitch + PI)/(2*PI), 255 * (yaw + PI)/(2*PI)); //bbDefaultBgColor;
    
    cout << roll << pitch << yaw << bgColor << endl;
    
    ofBackground(bgColor); //backgroundColor);
    
//    ofSetColor(113,110,161,100);
    
    int effectsRed = MIN(255, bgColor.r + 50);
    int effectsGreen = MIN(255, bgColor.g + 50);
    int effectsBlue = MIN(255, bgColor.b + 50);
    
    ofColor effectsColor = ofColor(effectsRed, effectsGreen, effectsBlue);
    
    ofSetColor(effectsColor); //(bgColor.r + 50, bgColor.g + 50, bgColor.b + 50); //113,110,161,100);
    
    this->drawSnowflakeHistogram(50.0f);
    this->drawSnowflakeHistogram(200.0f);
    
//    this->drawHistogram(50.0f, false, true);
//    this->drawHistogram(200.0f, false, true);
//    this->drawHistogram(50.0f, true, false);
//    this->drawHistogram(200.0f, true, false);
    
//    ofSetColor(113,110,161,100);
//    for(int i = 0; i < bbInputSize; i++) {
//        float xPos = i / float(bbInputSize) * ofGetWindowWidth();
//        
//        float lerpedVolume = lerpVal(last2Input[i], lastInput[i], 0.5f);
//        float volHeight = bbInputSize * 5.0f * lerpedVolume;
//        
//        //float volHeight = MIN(ofGetWindowHeight()*0.1f, bbInputSize/10.0f * lerpedVolume * ofGetWindowHeight() * 0.5f);
//        
////        cout << volHeight << "\n";
//        
////        ofRect(xPos, ofGetWindowHeight(), ofGetWindowWidth()/float(bbInputSize), volHeight);
//    }
    
    for(Particle* p : particles){
        p->draw(effectsColor);
    }
    
    
    this->drawMyo();
    
}

void ofApp::drawHistogram(float baseHeight, bool leftToRight, bool bottomToTop)
{
    for(int i = 0; i < counter; i++) {
        //ofRect(xPos, ofGetWindowHeight(), ofGetWindowWidth()/float(bbInputSize), volHeight);
        
        float xPos = i / float(bbVolumeHistoryLength) * ofGetWindowWidth();
        if (!leftToRight) {
            xPos = ofGetWindowWidth() - (i / float(bbVolumeHistoryLength) * ofGetWindowWidth());
        }
        
        float yPos = ofGetWindowHeight();
        if (!bottomToTop) {
            yPos = 0;
        }
        float width = ofGetWindowWidth()/float(bbVolumeHistoryLength);
        
        float height = baseHeight * volumeHistory[i] * (0.9f + 0.5f * volumePercent);
        if (!bottomToTop) {
            height *= -1;
        }
        
        ofRect(xPos, yPos, width, -1*height*scaleFactor);
    }
}

void ofApp::drawSnowflakeHistogram(float baseHeight)
{
    for(int i = 0; i < counter; i++) {
        float height = baseHeight * volumeHistory[i]; // * (0.9f + 0.5f * volumePercent);
        float radialX = height * cos(i * 2 * PI / bbVolumeHistoryLength);
        float radialY = height * sin(i * 2 * PI / bbVolumeHistoryLength);
        ofLine(ofGetWindowWidth()/2, ofGetWindowHeight()/2, ofGetWindowWidth()/2 + radialX, ofGetWindowHeight()/2 + radialY);
    }
}

//--------------------------------------------------------------
void ofApp::drawMyo() {
    
    ofSetColor(ofColor::white);
    ofCircle(30, 30, accel_x * 10);
    ofCircle(60, 30, accel_y * 10);
    ofCircle(90, 30, accel_z * 10);
    
    ofCircle(130, 30, roll * 10);
    ofCircle(160, 30, pitch * 10);
    ofCircle(190, 30, yaw * 10);
    
    float amplitude = 10.0f;
    float margin = 100.0f;
    float axisMargin = 100.0f;
    
    ofSetColor(ofColor::darkGrey);
    ofLine(0, 0*axisMargin + margin, ofGetWindowWidth(), 0*axisMargin + margin);
    ofLine(0, 1*axisMargin + margin, ofGetWindowWidth(), 1*axisMargin + margin);
    ofLine(0, 2*axisMargin + margin, ofGetWindowWidth(), 2*axisMargin + margin);
    ofSetColor(ofColor::white);
    
    for (int i = 0; i < timeCounter-1; i++) {
        
        //        cout << accelXValues[i] << endl;
        
        if (abs(accelXValues[i]) > 0.5) {
            ofSetColor(ofColor::red);
            if (abs(accelXValues[i+1]) - abs(accelXValues[i]) < -0.25f) {
                ofSetColor(ofColor::yellowGreen);
            }
            
        } else {
            ofSetColor(ofColor::white);
        }
        
        // plot x accel values
        ofLine(margin + i, 0*axisMargin + margin + amplitude * accelXValues[i], margin + i+1, 0*axisMargin + margin + amplitude * accelXValues[i+1]);
        
        ofSetColor(ofColor::white);
        
        // plot y accel values
        ofLine(margin + i, 1*axisMargin + margin + amplitude * accelYValues[i], margin + i+1, 1*axisMargin + margin + amplitude * accelYValues[i+1]);
        
        // plot z accel values
        ofLine(margin + i, 2*axisMargin + margin + amplitude * accelZValues[i], margin + i+1, 2*axisMargin + margin + amplitude * accelZValues[i+1]);
    }
    
}


#pragma mark - Utils

ofVec2f ofApp::getWindowCenter()
{
    return ofVec2f(ofGetWindowWidth()*0.5f, ofGetWindowHeight()*0.5f);
}

ofVec2f ofApp::getMouseToCenter()
{
    ofVec2f windowCenter = this->getWindowCenter();
    return ofVec2f(mousePos.x - windowCenter.x, mousePos.y - windowCenter.y);
}

#pragma mark - Input

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    //reset to default parameters
    if (key == OF_KEY_TAB) {
        neighborThresholdAdjustment = -50.0f;
        volumeCalibration = 0.5f;
    }
    
    //shift spawns 100 random particles
    if (key == OF_KEY_SHIFT) {
        this->spawnRandomParticles(100);
    }
    
    // up and down adjust the neighbor threshold distance
    if (key == OF_KEY_UP) {
        neighborThresholdAdjustment = MIN(neighborThresholdAdjustment + 10.0f, 500.0f);
    } else if (key == OF_KEY_DOWN) {
        neighborThresholdAdjustment = MAX(neighborThresholdAdjustment - 10.0f, -140.0f);
    }
    cout << "neighborThresholdAdjustment = " << neighborThresholdAdjustment << "\n";
    
    // left and right adjust the volume calibration level
    if (key == OF_KEY_LEFT) {
        volumeCalibration = MAX(0.005f, volumeCalibration * 0.9f);
    } else if(key == OF_KEY_RIGHT) {
        volumeCalibration = MIN(2.0f, volumeCalibration + 0.05f);
    }
    
    cout << "volumeCalibration = " << volumeCalibration << "\n";
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
    
    mousePos.x = x;
    mousePos.y = y;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
//    ofLine(mousePos.x, mousePos.y, x, y);
    Particle* p = new Particle(bbDefaultRadius, bbBlueColor, ofVec2f(x,y), bbZeroVec, bbZeroVec, bbDefaultLifetime);
    particles.push_back(p);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    Particle* p = new Particle(bbDefaultRadius, bbBlueColor, ofVec2f(x,y), bbZeroVec, bbZeroVec, bbDefaultLifetime);
    particles.push_back(p);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}