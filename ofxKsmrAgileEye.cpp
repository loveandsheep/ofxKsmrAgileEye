//
//  ofxKsmrAgileEye.cpp
//  agileEyeTest
//
//  Created by Ovis aries on 2015/02/02.
//
//

#include "ofxKsmrAgileEye.h"


void ofxKsmrAgileEye::setup(){

	for (int i = 0;i < 3;i++) arm[i].setID(i);

	arm[0].rootNode()->setPosition(80.0, 0.0, 0.0);
	arm[1].rootNode()->setPosition(0.0, 80.0, 0.0);
	arm[2].rootNode()->setPosition(0.0, 0.0, -80.0);

	arm[0].rootNode()->roll(90);
	arm[0].rootNode()->pan(270);

	arm[1].rootNode()->roll(180);

	arm[2].rootNode()->tilt(90);
	arm[2].rootNode()->pan(-90);

	arm[0].tipNode()->pan(45);
	arm[1].tipNode()->pan(45);
	arm[2].tipNode()->pan(45);

	for (int i = 0;i < 3;i++) arm[i].registerOrientation();

	targetNode.setGlobalPosition(-100, -100, 100);
	eye.setup(&arm[0], &arm[1], &arm[2],targetNode.getGlobalPosition());
}

void ofxKsmrAgileEye::update(const ofVec3f & pos){

	eye.center.setTransformMatrix(eye.centerBaseMat);

	ofVec3f tc = ofVec3f(-200,-200,200);
	ofVec3f ps = pos;
	float dg = 45;
	ps.x = pos.x * cos(ofDegToRad(dg)) - pos.z * sin(ofDegToRad(dg));
	ps.z = pos.x * sin(ofDegToRad(dg)) + pos.z * cos(ofDegToRad(dg));

	targetNode.setGlobalPosition(tc + ps);

	trail.push_back(tc + ps);
	while (trail.size() > 500) trail.erase(trail.begin());

	targSmooth += (targetNode.getGlobalPosition() - targSmooth) / 4.0;
	eye.center.lookAt(targetNode);


	for (int i = 0;i < 3;i++){

		float minD = 100000000, minQ = 100000000;

		float ja = -45,jb = 45;
		float oa = -45,ob = 45;

		float jFin = 0;
		float oFin = 0;

		for (int p = 0;p < 20;p++){

			minD = 1000000000;
			minQ = 1000000000;
			float qFin = 0,rFin = 0;

			for (int q = 0;q < 2;q++){
				for (int r = 0;r < 2;r++){

					float jad = ja + (q == 0 ? 1 : 3) * (jb - ja) / 4;
					float oad = oa + (r == 0 ? 1 : 3) * (ob - oa) / 4;

					float dstW[4];
					float dstV = getAngleDist(dstW, i, jad, oad);

					if (dstV < minD){
						minD = dstV;
						minQ = dstW[0];

						qFin = q;
						rFin = r;
	
					}
				}

			}

			float jad = ja + (qFin == 0 ? 1 : 3) * (jb - ja) / 4;
			float oad = oa + (rFin == 0 ? 1 : 3) * (ob - oa) / 4;
			float jd = (jb - ja) / 4;
			ja = jad - jd;
			jb = jad + jd;

			float od = (ob - oa) / 4;
			oa = oad - od;
			ob = oad + od;

		}

		jFin = ofLerp(ja, jb, 0.5);
		oFin = ofLerp(oa, ob, 0.5);

		arm[i].backOrientation();
		arm[i].panRoot(jFin);
		arm[i].tipNode()->pan(oFin);
	}

}

float ofxKsmrAgileEye::getAngleDist(float *dst_W, int index, float rootAngle, float tipAngle){
	int i = index;
	float j = rootAngle;
	float o = tipAngle;

	ofNode *trg = &eye.linker[i];
	ofNode *lnk = arm[i].linkNode();

	arm[i].backOrientation();
	arm[i].rootNode()->pan(j);
	arm[i].tipNode()->pan(o);

	ofQuaternion dstQ = lnk->getGlobalOrientation().inverse() *
	trg->getGlobalOrientation();
	dstQ.getRotate(dst_W[0], dst_W[1], dst_W[2], dst_W[3]);

	return trg->getGlobalPosition().distanceSquared(lnk->getGlobalPosition());
}

void ofxKsmrAgileEye::draw(){
	ofPushMatrix();
	ofRotateZ(180);
	ofRotateY(-135);

	ofEnableDepthTest();
	
	ofPushMatrix();
	ofTranslate(0, 100);
	ofRotateZ(90);
	ofSetColor(60);
	ofDrawGridPlane(300);
	ofPopMatrix();

	ofSetColor(255);


	if (!ofGetKeyPressed('1')) arm[0].draw();
	if (!ofGetKeyPressed('2')) arm[1].draw();
	if (!ofGetKeyPressed('3')) arm[2].draw();

	targetNode.draw();

	ofPushMatrix();
	ofTranslate(-200, -200, 200);
	ofRotateY(-45);
	ofLine(-10, 0, 10, 0);
	ofLine(0, -10, 0, 10);
	ofPopMatrix();
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &trail[0]);
	glDrawArrays(GL_LINE_STRIP, 0, trail.size());
	glDisableClientState(GL_VERTEX_ARRAY);

	ofSetColor(255);

	ofSetLineWidth(3.0);
	eye.draw();
	ofLine(targetNode.getGlobalPosition(),
		   eye.center.getGlobalPosition());
	ofSetLineWidth(1.0);

	ofPushMatrix();
	ofMultMatrix(eye.center.getGlobalTransformMatrix());
	ofPopMatrix();

	ofDisableDepthTest();

	ofPopMatrix();
}







/* ================================================ */

void singleArm::genArm(){
	nodeA.setPosition(0,0,0);
	nodeB.setParent(nodeA);
	nodeB.setPosition(radius,radius,0);

	int reso = 5;

	vector<ofVec3f> sampleVert[4];
	for (int i = 0;i < 90 + reso;i += reso){

		for (int j = 0;j < 4;j++){

			float r,d;
			ofVec3f v;

			if (j == 0) r = radius - width/2;
			if (j == 1) r = radius + width/2;
			if (j == 2) r = radius + width/2;
			if (j == 3) r = radius - width/2;

			if (j == 0) d =  depth / 2;
			if (j == 1) d =  depth / 2;
			if (j == 2) d = -depth / 2;
			if (j == 3) d = -depth / 2;

			v.set(cos(ofDegToRad(i - 90)) * r,
				  sin(ofDegToRad(i - 90)) * r + radius, d);

			sampleVert[j].push_back(v);

		}

	}

	verts.clear();
	for (int i = 0;i < 4;i++){
		for (int j = 0;j < sampleVert[i].size() - 1;j++){
			int n = (i + 1) % 4;
			int k = (j + 1);
			verts.push_back(sampleVert[i][j]);
			verts.push_back(sampleVert[n][j]);
			verts.push_back(sampleVert[i][k]);

			verts.push_back(sampleVert[n][j]);
			verts.push_back(sampleVert[i][k]);
			verts.push_back(sampleVert[n][k]);
		}
	}
}

void singleArm::draw(){

	nodeA.draw();
	nodeB.draw();

	ofPushMatrix();
	ofMultMatrix(nodeA.getGlobalTransformMatrix());

	ofPushStyle();
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &verts[0]);

	ofSetColor(255);
	ofFloatColor c;
	if (id_ == 0) c.setHsb(0.0, 0.7, 1.0);
	if (id_ == 1) c.setHsb(0.2, 0.7, 1.0);
	if (id_ == 2) c.setHsb(0.4, 0.7, 1.0);
	ofSetColor(c);

	glDrawArrays(GL_TRIANGLES, 0, verts.size());

	ofSetColor(100);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, verts.size());
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisableClientState(GL_VERTEX_ARRAY);
	ofPopStyle();
	ofPopMatrix();
	
}