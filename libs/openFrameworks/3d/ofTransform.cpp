
#include "ofTransform.h"
#include "ofMath.h"
#include "ofLog.h"
#include "of3dGraphics.h"

ofTransform::ofTransform()
	:parent(nullptr)
	, legacyCustomDrawOverrided(true)
{
	setPosition(ofVec3f(0, 0, 0));
	setOrientation(ofVec3f(0, 0, 0));
	setScale(1);
}

//----------------------------------------
void ofTransform::setParent(ofTransform& parent, bool bMaintainGlobalTransform) {
	if (bMaintainGlobalTransform) {
		ofMatrix4x4 postParentGlobalTransform = getGlobalTransformMatrix() * parent.getGlobalTransformMatrix().getInverse();
		this->parent = &parent;
		setTransformMatrix(postParentGlobalTransform);
	}
	else {
		this->parent = &parent;
	}
}

//----------------------------------------
void ofTransform::clearParent(bool bMaintainGlobalTransform) {
	if (bMaintainGlobalTransform) {
		ofMatrix4x4 globalTransform(getGlobalTransformMatrix());
		this->parent = nullptr;
		setTransformMatrix(globalTransform);
	}
	else {
		this->parent = nullptr;
	}
}

//----------------------------------------
ofTransform* ofTransform::getParent() const {
	return parent;
}

//----------------------------------------
void ofTransform::setTransformMatrix(const ofMatrix4x4 &m44) {
	localTransformMatrix = m44;

	ofQuaternion so;
	localTransformMatrix.decompose(position, orientation, scale, so);
	updateAxis();

	onPositionChanged();
	onOrientationChanged();
	onScaleChanged();
}

//----------------------------------------
void ofTransform::setPosition(float px, float py, float pz) {
	setPosition(ofVec3f(px, py, pz));
}

//----------------------------------------
void ofTransform::setPosition(const ofVec3f& p) {
	position = p;
	localTransformMatrix.setTranslation(position);
	onPositionChanged();
}

//----------------------------------------
void ofTransform::setGlobalPosition(float px, float py, float pz) {
	setGlobalPosition(ofVec3f(px, py, pz));
}

//----------------------------------------
void ofTransform::setGlobalPosition(const ofVec3f& p) {
	if (parent == nullptr) {
		setPosition(p);
	}
	else {
		setPosition(p * ofMatrix4x4::getInverseOf(parent->getGlobalTransformMatrix()));
	}
}

//----------------------------------------
ofVec3f ofTransform::getPosition() const {
	return position;
}

//----------------------------------------
float ofTransform::getX() const {
	return position.x;
}

//----------------------------------------
float ofTransform::getY() const {
	return position.y;
}

//----------------------------------------
float ofTransform::getZ() const {
	return position.z;
}

//----------------------------------------
void ofTransform::setOrientation(const ofQuaternion& q) {
	orientation = q;
	createMatrix();
	onOrientationChanged();
}

//----------------------------------------
void ofTransform::setOrientation(const ofVec3f& eulerAngles) {
	setOrientation(ofQuaternion(eulerAngles.x, ofVec3f(1, 0, 0), eulerAngles.z, ofVec3f(0, 0, 1), eulerAngles.y, ofVec3f(0, 1, 0)));
}

//----------------------------------------
void ofTransform::setGlobalOrientation(const ofQuaternion& q) {
	if (parent == nullptr) {
		setOrientation(q);
	}
	else {
		ofMatrix4x4 invParent(ofMatrix4x4::getInverseOf(parent->getGlobalTransformMatrix()));
		ofMatrix4x4 m44(ofMatrix4x4(q) * invParent);
		setOrientation(m44.getRotate());
	}
}

//----------------------------------------
ofQuaternion ofTransform::getOrientationQuat() const {
	return orientation;
}

//----------------------------------------
ofVec3f ofTransform::getOrientationEuler() const {
	return orientation.getEuler();
}

//----------------------------------------
void ofTransform::setScale(float s) {
	setScale(s, s, s);
}

//----------------------------------------
void ofTransform::setScale(float sx, float sy, float sz) {
	setScale(ofVec3f(sx, sy, sz));
}

//----------------------------------------
void ofTransform::setScale(const ofVec3f& s) {
	this->scale = s;
	createMatrix();
	onScaleChanged();
}

//----------------------------------------
ofVec3f ofTransform::getScale() const {
	return scale;
}

//----------------------------------------
void ofTransform::move(float x, float y, float z) {
	move(ofVec3f(x, y, z));
}

//----------------------------------------
void ofTransform::move(const ofVec3f& offset) {
	position += offset;
	localTransformMatrix.setTranslation(position);
	onPositionChanged();
}

//----------------------------------------
void ofTransform::truck(float amount) {
	move(getXAxis() * amount);
}

//----------------------------------------
void ofTransform::boom(float amount) {
	move(getYAxis() * amount);
}

//----------------------------------------
void ofTransform::dolly(float amount) {
	move(getZAxis() * amount);
}

//----------------------------------------
void ofTransform::tilt(float degrees) {
	rotate(degrees, getXAxis());
}

//----------------------------------------
void ofTransform::pan(float degrees) {
	rotate(degrees, getYAxis());
}

//----------------------------------------
void ofTransform::roll(float degrees) {
	rotate(degrees, getZAxis());
}

//----------------------------------------
void ofTransform::rotate(const ofQuaternion& q) {
	orientation *= q;
	createMatrix();
	onOrientationChanged();
}

//----------------------------------------
void ofTransform::rotate(float degrees, const ofVec3f& v) {
	rotate(ofQuaternion(degrees, v));
}

//----------------------------------------
void ofTransform::rotate(float degrees, float vx, float vy, float vz) {
	rotate(ofQuaternion(degrees, ofVec3f(vx, vy, vz)));
}

//----------------------------------------
void ofTransform::rotateAround(const ofQuaternion& q, const ofVec3f& point) {
	//	ofLogVerbose("ofTransform") << "rotateAround(const ofQuaternion& q, const ofVec3f& point) not implemented yet";
	//	ofMatrix4x4 m = getLocalTransformMatrix();
	//	m.setTranslation(point);
	//	m.rotate(q);

	setGlobalPosition((getGlobalPosition() - point)* q + point);

	onOrientationChanged();
	onPositionChanged();
}

//----------------------------------------
void ofTransform::rotateAround(float degrees, const ofVec3f& axis, const ofVec3f& point) {
	rotateAround(ofQuaternion(degrees, axis), point);
}

//----------------------------------------
void ofTransform::lookAt(const ofVec3f& lookAtPosition, ofVec3f upVector) {
	if (parent) upVector = upVector * ofMatrix4x4::getInverseOf(parent->getGlobalTransformMatrix());
	ofVec3f zaxis = (getGlobalPosition() - lookAtPosition).getNormalized();
	if (zaxis.length() > 0) {
		ofVec3f xaxis = upVector.getCrossed(zaxis).getNormalized();
		ofVec3f yaxis = zaxis.getCrossed(xaxis);

		ofMatrix4x4 m;
		m._mat[0].set(xaxis.x, xaxis.y, xaxis.z, 0);
		m._mat[1].set(yaxis.x, yaxis.y, yaxis.z, 0);
		m._mat[2].set(zaxis.x, zaxis.y, zaxis.z, 0);

		setGlobalOrientation(m.getRotate());
	}
}

//----------------------------------------
void ofTransform::lookAt(const ofTransform& lookAtNode, const ofVec3f& upVector) {
	lookAt(lookAtNode.getGlobalPosition(), upVector);
}

//----------------------------------------
void ofTransform::updateAxis() {
	if (scale[0]>0) axis[0] = getLocalTransformMatrix().getRowAsVec3f(0) / scale[0];
	if (scale[1]>0) axis[1] = getLocalTransformMatrix().getRowAsVec3f(1) / scale[1];
	if (scale[2]>0) axis[2] = getLocalTransformMatrix().getRowAsVec3f(2) / scale[2];
}

//----------------------------------------
ofVec3f ofTransform::getXAxis() const {
	return axis[0];
}

//----------------------------------------
ofVec3f ofTransform::getYAxis() const {
	return axis[1];
}

//----------------------------------------
ofVec3f ofTransform::getZAxis() const {
	return axis[2];
}

//----------------------------------------
ofVec3f ofTransform::getSideDir() const {
	return getXAxis();
}

//----------------------------------------
ofVec3f ofTransform::getLookAtDir() const {
	return -getZAxis();
}

//----------------------------------------
ofVec3f ofTransform::getUpDir() const {
	return getYAxis();
}

//----------------------------------------
float ofTransform::getPitch() const {
	return getOrientationEuler().x;
}

//----------------------------------------
float ofTransform::getHeading() const {
	return getOrientationEuler().y;
}

//----------------------------------------
float ofTransform::getRoll() const {
	return getOrientationEuler().z;
}

//----------------------------------------
const ofMatrix4x4& ofTransform::getLocalTransformMatrix() const {
	return localTransformMatrix;
}

//----------------------------------------
ofMatrix4x4 ofTransform::getGlobalTransformMatrix() const {
	/*
	if(bMatrixDirty)
	{
	if(parent) globalTransformMatrix = getLocalTransformMatrix() * parent->getGlobalTransformMatrix();
	else globalTransformMatrix = getLocalTransformMatrix;
	bMatrixDirty = false;
	}
	return globalTransformMatrix;
	*/


	if (parent) return getLocalTransformMatrix() * parent->getGlobalTransformMatrix();
	else return getLocalTransformMatrix();
}

//----------------------------------------
ofVec3f ofTransform::getGlobalPosition() const {
	return getGlobalTransformMatrix().getTranslation();
}

//----------------------------------------
ofQuaternion ofTransform::getGlobalOrientation() const {
	return getGlobalTransformMatrix().getRotate();
}

//----------------------------------------
ofVec3f ofTransform::getGlobalScale() const {
	if (parent) return getScale()*parent->getGlobalScale();
	else return getScale();
}

//----------------------------------------
void ofTransform::orbit(float longitude, float latitude, float radius, const ofVec3f& centerPoint) {
	ofMatrix4x4 m;

	// find position
	ofVec3f p(0, 0, radius);
	p.rotate(ofClamp(latitude, -89, 89), ofVec3f(1, 0, 0));
	p.rotate(longitude, ofVec3f(0, 1, 0));
	p += centerPoint;
	setPosition(p);

	lookAt(centerPoint);//, v - centerPoint);
}

//----------------------------------------
void ofTransform::orbit(float longitude, float latitude, float radius, ofTransform& centerNode) {
	orbit(longitude, latitude, radius, centerNode.getGlobalPosition());
}

//----------------------------------------
void ofTransform::resetTransform() {
	setPosition(ofVec3f());
	setOrientation(ofVec3f());
}

//----------------------------------------
void ofTransform::draw()  const {
	ofNode n;
	n.setGlobalPosition(this->getGlobalPosition());
	n.setGlobalOrientation(this->getGlobalOrientation());
	n.draw();
}

//----------------------------------------
void ofTransform::customDraw(const ofBaseRenderer * renderer) const {
	const_cast<ofTransform*>(this)->customDraw();
	if (!legacyCustomDrawOverrided) {
		renderer->drawBox(10);
		renderer->draw(ofMesh::axis(20), OF_MESH_FILL);
	}
}

//----------------------------------------
void ofTransform::customDraw() {
	legacyCustomDrawOverrided = false;
}

//----------------------------------------
void ofTransform::transformGL(ofBaseRenderer * renderer) const {
	if (renderer == nullptr) {
		renderer = ofGetCurrentRenderer().get();
	}
	renderer->pushMatrix();
	renderer->multMatrix(getGlobalTransformMatrix());
}

//----------------------------------------
void ofTransform::restoreTransformGL(ofBaseRenderer * renderer) const {
	if (renderer == nullptr) {
		renderer = ofGetCurrentRenderer().get();
	}
	renderer->popMatrix();
}

//----------------------------------------
void ofTransform::createMatrix() {
	//if(isMatrixDirty) {
	//	isMatrixDirty = false;
	localTransformMatrix.makeScaleMatrix(scale);
	localTransformMatrix.rotate(orientation);
	localTransformMatrix.setTranslation(position);

	updateAxis();
}


