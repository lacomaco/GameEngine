#include "Shader.h"

std::string Shader::readShaderSource(const char* filePath)
{
	std::ifstream shaderFile;
    /*
    * failbit <- I/O 작업중 논리적 실패 발생
    * badbit <- 읽기 쓰기 오류로 인한 실패
    */
	shaderFile.exceptions(
		std::ifstream::failbit | std::ifstream::badbit
	);
    try {
        shaderFile.open(filePath);
        std::stringstream shaderStream;
        // 파일 내용을 스트림에 읽어옵니다.
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        // 스트림을 std::string으로 변환합니다.
        return shaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        return "";
    }
}

GLuint Shader::compileShader(const char* shaderCode, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderCode, NULL);
    glCompileShader(shader);

    // 컴파일 오류 확인
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

GLuint Shader::getVertexShader(const char* filePath) {
    auto vertexShader = shaderMap.find(filePath);

    if (vertexShader != shaderMap.end()) {
        std::string vertexShaderSource = readShaderSource(filePath);
        GLuint vertexShaderID = compileShader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
        shaderMap[filePath] = vertexShaderID;
        return vertexShaderID;
    }

    return vertexShader->second;
}

GLuint Shader::getFragmentShader(const char* filePath) {
    auto fragmentShader = shaderMap.find(filePath);

	if (fragmentShader != shaderMap.end()) {
		std::string fragmentShaderSource = readShaderSource(filePath);
		GLuint fragmentShaderID = compileShader(fragmentShaderSource.c_str(), GL_FRAGMENT_SHADER);
		shaderMap[filePath] = fragmentShaderID;
		return fragmentShaderID;
	}

	return fragmentShader->second;
}
