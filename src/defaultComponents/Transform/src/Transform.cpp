#include "Transform/Transform.h"

void Canella::Transform::start()
{
	calculateRotationQuaternion(glm::vec3(1, 0, 0), 0.0f);
}

void Canella::Transform::update(float timeStep)
{
}

void Canella::Transform::awake()
{
}

Canella::Transform::Transform(std::string id ) :Component(id)
{
	setComponentType(Canella::COMPONENT_TYPE::TRANSFORM);
}

glm::mat4& Canella::Transform::getModelMatrix()
{
	return model;
}

glm::vec3& Canella::Transform::getPosition()
{
	return position;
}

glm::vec3& Canella::Transform::getScale()
{
	return scale;
}

glm::vec3 Canella::Transform::getRotation()
{
	return  rotation;
}

void Canella::Transform::setPosition(glm::vec3 p)
{
	position = p;

}
void Canella::Transform::setRotation(glm::vec3 r)
{
	rotation = r;
}

void Canella::Transform::setRotation(float x, float y, float z)
{
	rotation = glm::vec3(x, y, z);

}

void Canella::Transform::rotateMix(glm::quat q1, glm::quat q2, float dt)
{
	glm::quat q = glm::mix(q1, q2, dt);

	rotMatrix = glm::toMat4(q);

}

void Canella::Transform::setPosition(float x,float y , float z)
{
	position = glm::vec3(x,y,z);
}

void Canella::Transform::setScale(glm::vec3 p)
{
	scale = p;
}

glm::quat Canella::Transform::calculateRotationQuaternion(glm::vec3 axis, float angle)
{
	glm::quat quat = glm::quat(0.0, position.x, position.y, position.z);

	glm::vec3 p = glm::vec3(glm::sin(angle / 2.0f)) * glm::normalize(axis);

	glm::quat q2 = glm::quat(glm::cos(angle / 2.0f), p.x, p.y, p.z);

	glm::quat t = q2 * quat * glm::inverse(q2);

	rotQuaternion = t;

	return t;
}


void Canella::Transform::increasePos(float x, float y, float z)
{
	position += glm::vec3(x, y, z);
}

void Canella::Transform::increasePos(glm::vec3 p)
{
	position += p;
}

void Canella::Transform::updateModelMatrix(Transform& parent)
{/*
	glm::quat internalQuat = glm::quat(0.0, position.x, position.y, position.z);
	glm::quat rotationQuaternion = glm::quat(glm::normalize(rotation));
	glm::quat r = rotationQuaternion * internalQuat * glm::inverse(rotationQuaternion);
	rotMatrix = glm::mat4_cast(r);*/

	rotMatrix = glm::toMat4(glm::normalize(rotQuaternion));

	glm::mat4 identity(1.0f);
	identity = glm::translate(identity, position);
	identity *= rotMatrix;
	identity = glm::scale(identity, scale);

	glm::mat4 parentModel = parent.getModelMatrix();

	model =  parentModel* identity;

}

void Canella::Transform::updateModelMatrix()
{
	glm::mat4 identity(1.0f);
	rotMatrix = glm::toMat4(glm::normalize(rotQuaternion));

	identity = glm::translate(identity, position);
	identity = glm::scale(identity, scale);
	identity *= rotMatrix;


	model = identity;
	
}

void Canella::Transform::rotate(glm::vec3 axis, float angle)
{
	axis = glm::normalize(axis);
	glm::quat quat = glm::quat(0.0,position.x,position.y,position.z);
	glm::vec3 p = glm::vec3(glm::sin(glm::radians(angle / 2.))) * (axis);
	glm::quat q2 =glm::normalize(glm::quat(glm::cos(glm::radians(angle / 2.0)), p.x, p.y, p.z));
	glm::quat t =  q2 * quat * glm::conjugate(q2);
	rotQuaternion *= (t);

}

// void Canella::Transform::buildUi()
// {
// 	ImGui::InputFloat3("Position", glm::value_ptr(this->position));
// 	ImGui::InputFloat3("Rotation", glm::value_ptr(this->rotation));
// 	ImGui::InputFloat3("Scale", glm::value_ptr(this->scale));
// }

void Canella::Transform::shouldUpdateRotations(bool b)
{
	shouldUpdateQuaternion = b;;
}
void Canella::Transform::loadState(nlohmann::json& config){

};

void Canella::Transform::saveState(nlohmann::json& config) {

};
std::shared_ptr<Canella::Component> Canella::Transform::create(const nlohmann::json& config){
	std::shared_ptr<Canella::Component> m =	std::make_shared<Canella::Transform>(config["Id"]);
	return m;

};