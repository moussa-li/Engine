#pragma once
#include <string>
#include <unordered_map>


#include <eigen3/Eigen/Eigen>

#include "libexport.h"


struct ShaderProgramSource
{
    std::string VertexSource = "";
    std::string FragmentSource = "";
    std::string ComputeSource = "";
};

class DLLAPI Shader
{
private:
	std::string m_FilePath;
	unsigned int m_RendererId;
	std::unordered_map<std::string, int> m_UniformLocationCache;
public:
    Shader(){}
	Shader(const std::string& filepath);
	~Shader();

	void Bind() const;
	void Unbind() const;

	void SetUniform1i(const std::string& name, int v0);
	void SetUniform1f(const std::string& name, float v0);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform3f(const std::string& name, Eigen::Vector3f v);
	void SetUniformMat4f(const std::string& name, const Eigen::Matrix4f& matrix);
	void SetUniformMat3f(const std::string& name, const Eigen::Matrix3f& matrix);
private:
	int GetUniformLocation(const std::string& name);
	//bool CompileShader();
	ShaderProgramSource ParseShader(const std::string& filepath);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	int CreateShader(ShaderProgramSource source);

};
