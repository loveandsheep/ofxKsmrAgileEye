//
//  ofxKsmrAgileEye.h
//  agileEyeTest
//
//  Created by Ovis aries on 2015/02/02.
//
//

#ifndef __agileEyeTest__ofxKsmrAgileEye__
#define __agileEyeTest__ofxKsmrAgileEye__

#include "ofMain.h"

class singleArm{
public:

	singleArm(int r, int w, int d,int iD){
		radius = r;
		width = w;
		depth = d;
		id_ = iD;
		genArm();
	}

	~singleArm(){
	}

	void genArm();
	void draw();

	int radius;
	int width;
	int depth;
	int id_;

	ofNode nodeA;
	ofNode nodeB;

	vector <ofVec3f> verts;

};

class armUnit{
public:

	armUnit(){

		float max_outline = 80.0;
		float arm_width = 10.0;
		float arm_spacing = 3.2;

		arm[0] = new singleArm(max_outline - arm_width/2,
							   arm_width,arm_width,0);

		arm[1] = new singleArm(max_outline - arm_width*1.5 - arm_spacing,
							   arm_width,arm_width,0);
		arm[2] = new singleArm(max_outline - arm_width*2.5 - arm_spacing*2,
							   arm_width,arm_width,0);

		arm[1]->nodeA.setPosition(-(arm_width + arm_spacing), 0.0, 0.0);
		arm[1]->nodeA.pan(90);
		arm[1]->nodeA.tilt(-90);
		arm[1]->nodeA.setParent(arm[0]->nodeB);

		arm[2]->nodeA.pan(180);
		arm[2]->nodeA.roll(-90);
		arm[2]->nodeA.setPosition(-(arm_width + arm_spacing), 0.0, 0.0);
		arm[2]->nodeA.setParent(arm[1]->nodeB);

		motorNode.setParent(arm[0]->nodeA);
		motorNode.setPosition(0, -30, 0);

		rootPanSmth = 0.0;
	}

	void panRoot(float degree){
		rootPan = degree;
		rootNode()->pan(degree);
	}

	void setID(int i){
		arm[0]->id_ = i;
		arm[1]->id_ = i;
		arm[2]->id_ = i;

		if (i == 0) c.setHsb(0.0, 0.7, 1.0);
		if (i == 1) c.setHsb(0.2, 0.7, 1.0);
		if (i == 2) c.setHsb(0.4, 0.7, 1.0);
	}

	void draw(){

		rootPanSmth += (rootPan - rootPanSmth) / 12.0;

		ofPushStyle();
		ofSetColor(c);
		arm[0]->draw();
		arm[1]->draw();
		arm[2]->draw();
		ofSetLineWidth(5.0);
		ofLine(arm[0]->nodeA.getGlobalPosition(),
			   motorNode.getGlobalPosition());

		motorNode.draw();
		ofPushMatrix();
		ofMultMatrix(motorNode.getGlobalTransformMatrix());
		ofRotateX(90);

		ofSetColor(c);
		ofNoFill();
		ofCircle(0, 0, 60);
		ofFill();
		ofLine(-60,0,60,0);
		ofLine(0,-60,0,60);
		ofPopMatrix();

		ofSetLineWidth(1.0);

		ofPushMatrix();
		ofTranslate(motorNode.getGlobalPosition());
		ofTranslate(-60,10);
		ofSetColor(c);
		ofDrawBitmapString("Angle :" + ofToString(rootPan) + ":(" + ofToString((rootPan/1.8)) + "Step)",
						   0,0);
		ofPopMatrix();
		ofPopStyle();
	}

	void backOrientation(){
		for (int i = 0;i < 3;i++)
			arm[i]->nodeA.setOrientation(ori[i]);
	}

	void registerOrientation(){
		ori[0] = arm[0]->nodeA.getOrientationQuat();
		ori[1] = arm[1]->nodeA.getOrientationQuat();
		ori[2] = arm[2]->nodeA.getOrientationQuat();
	}

	float rootPan;
	float rootPanSmth;
	ofFloatColor c;
	ofNode *rootNode(){return &arm[0]->nodeA;};
	ofNode *tipNode(){return &arm[2]->nodeA;};
	ofNode *linkNode(){return &arm[2]->nodeB;};

	ofQuaternion ori[3];
	singleArm* arm[3];
	ofNode motorNode;
};

class eyeUnit{
public:

	void setup(armUnit* au1,armUnit* au2,armUnit *au3,
			   ofVec3f defaultTarg){

		center.setPosition(ofVec3f(-15, -15, 15));
		center.lookAt(defaultTarg);
		centerBaseMat = center.getGlobalTransformMatrix();

		armUnit *unt[3] = {au1,au2,au3};

		for (int i = 0;i < 3;i++){

			linker[i].setParent(center);

			linker[i].setPosition(0,0,0);
			linker[i].setGlobalOrientation(unt[i]->linkNode()->getGlobalOrientation());
			linker[i].setGlobalPosition(unt[i]->linkNode()->getGlobalPosition());
		}
	}

	void draw(){
		center.draw();
		linker[0].draw();
		linker[1].draw();
		linker[2].draw();

		for (int i = 0;i < 3;i++)
		ofLine(center.getGlobalPosition(),
			   linker[i].getGlobalPosition());
	}

	ofMatrix4x4 centerBaseMat;
	ofNode center;
	ofNode linker[3];

};

class ofxKsmrAgileEye{
public:
	void setup();
	void update(const ofVec3f & pos);
	void draw();

	float getAngleDist(float *dst_W, int index, float rootAngle, float tipAngle);

	eyeUnit eye;
	armUnit arm[3];

	ofNode targetNode;
	ofVec3f targSmooth;

	vector<ofVec3f> trail;
};

#endif /* defined(__agileEyeTest__ofxKsmrAgileEye__) */
