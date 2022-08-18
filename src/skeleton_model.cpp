#define GLM_ENABLE_EXPERIMENTAL
// glm
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>


// project
#include "cgra/cgra_geometry.hpp"
#include "skeleton_model.hpp"

//IO
#include <iostream>

using namespace std;
using namespace glm;
using namespace cgra;

void skeleton_model::draw(const mat4 &view, const mat4 &proj) {
	// set up the shader for every draw call
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));

	// if the skeleton is not empty, then draw
	if (!skel.bones.empty()) {
		if (isAnimation) {
		drawAnimation(view, 0);
		} else{
			drawBone(view, 0);
		}
	}
}


void skeleton_model::poseBone(int boneid, vec3 direction) {
	skeleton_bone currentBone = skel.bones[boneid];
	skel.bones[boneid].direction = normalize(direction);
}

string skeleton_model::boneName(int boneid) {
	return skel.bones[boneid].name;
}

void skeleton_model::nextPose(){
	if(poseId + 1 < animation.poses.size()){
	poseId++;
	}
	cout << "pose Id: " << poseId << endl;
}


void skeleton_model::printPoses() {
	cout<<"Print bone directions ..."<<endl;
	for (size_t i = 0; i < skel.bones.size(); i++){
		cout<< skel.bones[i].name;
		cout<<"	:	X: " << skel.bones[i].direction.x <<"	Y: " << skel.bones[i].direction.y <<"	Z: " << skel.bones[i].direction.z << endl;
	}
	cout<<"... Completed printing bone directions"<<endl;	
}

void skeleton_model::drawAnimation(const mat4 &parentTransform, int boneid) {
	vec3 boneCol = vec3(0.5f,0.5f,0.5f);
	vec3 jointCol = vec3(0,1,1);
	vec3 XAxisCol = vec3(1,0,0);
	vec3 YAxisCol = vec3(0,1,0);
	vec3 ZAxisCol = vec3(0,0,1);
	float lengthScalar = 10;

	mat4 modelview = (parentTransform);// * 1.0f;
	skeleton_bone currentBone = skel.bones[boneid];
	skeleton_bone_pose currentSkelPose =   animation.poses.at(poseId).boneTransforms.at(boneid);

	mat4 transformation = mat4(1);
	transformation = rotate(transformation, currentBone.basis.z, vec3(0,0,1));
	transformation = rotate(transformation, currentBone.basis.y, vec3(0,1,0));
	transformation = rotate(transformation, currentBone.basis.x, vec3(1,0,0));


	mat4 inverseTransfomation = inverse(transformation);
	modelview *= transformation;
	modelview = translate(modelview, currentSkelPose.translation);
	modelview = rotate(modelview, currentSkelPose.rotation.z, vec3(0,0,1));
	modelview = rotate(modelview, currentSkelPose.rotation.y, vec3(0,1,0));
	modelview = rotate(modelview, currentSkelPose.rotation.x, vec3(1,0,0));
	modelview *= inverseTransfomation;

	drawBone(modelview, boneid);

	mat4 View = translate(modelview, currentBone.length*currentBone.direction);
	if(currentBone.children.size() != 0) {
		for (int childID : currentBone.children) {
			//cout<<childID;
			drawAnimation(View, childID);
		}
	}
}


void skeleton_model::drawBone(const mat4 &parentTransform, int boneid) {
	// TODO
	vec3 boneCol = vec3(0.5f,0.5f,0.5f);
	vec3 jointCol = vec3(0,1,1);
	vec3 XAxisCol = vec3(1,0,0);
	vec3 YAxisCol = vec3(0,1,0);
	vec3 ZAxisCol = vec3(0,0,1);
	float lengthScalar = 10;

	mat4 modelview = (parentTransform);// * 1.0f;
	skeleton_bone currentBone = skel.bones[boneid];

	mat4 View = translate(modelview, currentBone.length*currentBone.direction);
	if (!isAnimation) {
	if(currentBone.children.size() != 0) {
		for (int childID : currentBone.children) {
			//cout<<childID;
			drawBone(View, childID);
		}
	}
	}

	mat4 jointTrans = scale(parentTransform, vec3(0.02));

	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(jointTrans));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(jointCol));

	drawSphere();

	mat4 transformation = mat4(1.0f);
	transformation *= orientation(currentBone.direction,vec3(0,0,1));
	transformation = scale(transformation, vec3(0.01,0.01,currentBone.length));
	modelview *= transformation;
	
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(boneCol));
	drawCylinder();
	drawArrow(parentTransform, boneid,1);
	drawArrow(parentTransform, boneid,2);
	drawArrow(parentTransform, boneid,3);
}

void skeleton_model::drawArrow(const mat4 &parentTransform, int boneid, int direction){
	skeleton_bone currentBone = skel.bones[boneid];
	vec3 arrowCol = vec3(1,0,0);
	mat4  arrowTransformation = parentTransform;

		arrowTransformation = rotate(arrowTransformation, currentBone.basis.z, vec3(0,0,1));
		arrowTransformation = rotate(arrowTransformation, currentBone.basis.y, vec3(0,1,0));
		arrowTransformation = rotate(arrowTransformation, currentBone.basis.x, vec3(1,0,0));
		
	//X Axis
	if (direction == 1){//X Axis
		arrowTransformation = rotate(arrowTransformation, radians(90.0f), vec3(0,1,0));
	}
	if (direction == 2){ //Y Axis
		arrowTransformation = rotate(arrowTransformation, radians(-90.0f), vec3(1,0,0));
		arrowCol = vec3(0,1,0);
	}
	if (direction == 3){ //Z Axis
		arrowCol = vec3(0,0,1);

	}

	arrowTransformation = scale(arrowTransformation, vec3(0.005,0.005,0.06));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(arrowTransformation));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(arrowCol));
	drawCylinder();

	arrowTransformation = scale(arrowTransformation, vec3(1,1,.5));
	arrowTransformation = translate(arrowTransformation,vec3(0,0,2.0));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(arrowTransformation));
	drawCone();
}