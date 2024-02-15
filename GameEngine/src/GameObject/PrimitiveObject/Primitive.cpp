#include "Primitive.h"
#include "../../Constants.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


using namespace std;

// Assimp 사용하여 모델 로드한 경우엔 쓰지말것.
void Primitive::CalculateTangents(
) {
    for (size_t i = 0; i < indices.size(); i += 3) {
        Vertex& v0 = vertices[indices[i]];
        Vertex& v1 = vertices[indices[i + 1]];
        Vertex& v2 = vertices[indices[i + 2]];

        // Tangent 구하는 공식.
        // see here : https://learnopengl.com/Advanced-Lighting/Normal-Mapping
		glm::vec3 edge1 = v1.position - v0.position;
		glm::vec3 edge2 = v2.position - v0.position;

		float deltaU1 = v1.texcoord.x - v0.texcoord.x;
		float deltaV1 = v1.texcoord.y - v0.texcoord.y;
		float deltaU2 = v2.texcoord.x - v0.texcoord.x;
		float deltaV2 = v2.texcoord.y - v0.texcoord.y;

		float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

		glm::vec3 tangent;
		tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
		tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
		tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

        // vertex가 공유되는 경우 평균으로 처리한다.
		v0.tangentModel += tangent;
		v1.tangentModel += tangent;
		v2.tangentModel += tangent;
	}

    for (size_t i = 0; i < vertices.size(); i++) {
		vertices[i].tangentModel = glm::normalize(vertices[i].tangentModel);
	}
}

Primitive::~Primitive() {
	vertices.clear();
	indices.clear();
	for (auto& texture : textures) {
		glDeleteTextures(1, &texture.id);
	}

	mesh.reset();
}

void Primitive::PutModelUniform(const char* shaderProgramName) {
	auto shader = Shader::getInstance();

	glm::mat4 model = glm::mat4(1.0f);

	/*
	* 이미 움직인 물체를 피킹해서 회전시킬경우 짐벌락이 발생할 수 있기 때문에 쿼터니언으로 처리.
	*/
	glm::vec3 rotationRadians = glm::radians(rotation);
	glm::quat quaternion = glm::quat(rotationRadians);
	glm::mat4 rotationMatrix = glm::toMat4(quaternion);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

	// 연산자 순서 주의. 뒤에서 부터 연산된다.
	// translationMatrix가 rotationMatrix보다 먼저 연산될 경우 회전이동이 된다. 이에 주의할것
	model = translationMatrix * rotationMatrix * scaleMatrix;

	shader->setMat4(shaderProgramName, "model", model);

	glm::mat4 invTranspose = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	shader->setMat4(shaderProgramName,"invTranspose", invTranspose);

}

