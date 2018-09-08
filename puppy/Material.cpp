#include "Material.h"

namespace puppy
{
	Material::Material(ShaderType p_shaderType)
	{
		m_shader = ShaderManager::getShaderProgram(p_shaderType);
	}

	Material::~Material()
	{
		delete m_shader;
		delete m_tex;
	}
	
	void Material::apply()
	{
		if (m_shader != nullptr)
		{
			ShaderManager::applyShader(m_shader);
		}
		
		if (m_tex != nullptr)
		{
			m_tex->apply();
		}

		//apply memorized uniforms

	}

	void Material::setTexture(const char* p_pathToTex)
	{
		if (m_tex != nullptr)
		{
			delete m_tex;
		}
		m_tex = new Texture(p_pathToTex);
	}

	int Material::getUniformPlace(const std::string& p_name)
	{
		return m_shader->getUniformPlace(p_name);
	}

	void Material::setUniform(const std::string& p_name, const glm::mat4& p_mat4)
	{
		GLint place = m_shader->getUniformPlace(p_name);
		glUniformMatrix4fv(place, 1, GL_FALSE, glm::value_ptr(p_mat4));
	}

	void Material::memorizeUniform(const std::string& p_name, const glm::mat4& p_mat4)
	{

	}

	void Material::memorizeUniform(const std::string& p_name, const glm::mat3& p_mat3)
	{

	}

	void Material::memorizeUniform(const std::string& p_name, const glm::vec4& p_vec4)
	{

	}
	
	void Material::memorizeUniform(const std::string& p_name, const glm::vec3& p_vec3)
	{

	}

	void Material::memorizeUniform(const std::string& p_name, const GLfloat& p_float)
	{

	}

	void Material::memorizeUniform(const std::string& p_name, const GLint& p_int)
	{

	}
}