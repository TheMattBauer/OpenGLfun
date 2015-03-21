#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

class Camera
{
public:

    Camera(){}

    inline glm::mat4 GetViewProjection() const
    {
        return m_perspective * glm::lookAt(m_eye, m_lookAt, m_vup);
    }

	inline void UpdateViewport(GLsizei width, GLsizei height)
	{
		glViewport(m_viewport[0] * width, (1.0-m_viewport[3]) * height, (m_viewport[2]-m_viewport[0]) * width, (m_viewport[3]-m_viewport[1]) * height);
	};

	inline void SetIsParallel(bool isParallel){m_isParallel= isParallel;};
	inline void SetName(const std::string& name){m_name = name;};
	inline void SetEye(glm::vec3 eye){m_eye = eye;};
	inline void SetLookAt(glm::vec3 lookAt){m_lookAt = lookAt;};
	inline void SetVup(glm::vec3 vup){m_vup = vup;};
	inline void SetPerspective(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
	{
		m_xMax = xMax;
		m_xMin = xMin;
		m_yMax = yMax;
		m_yMin = yMin;
		m_zMax = zMax;
		m_zMin = zMin;

		if(m_isParallel)
			m_perspective = glm::ortho(xMin, xMax, yMin, yMax,  zMin, zMax);
		else
			m_perspective = glm::perspective(90.0f, (xMax - xMin) / (yMax - yMin), zMin, zMax);
	};
	inline void SetViewport(glm::vec4 viewport)
	{
		m_viewport = viewport;
	};

	inline glm::vec4& GetViewport(){return m_viewport;};
	inline glm::vec3& GetEye(){return m_eye;};
	inline glm::vec3& GetLookAt(){return m_lookAt;};
	inline glm::vec3& GetVup(){return m_vup;};
	inline bool& GetIsParallel(){return m_isParallel;};
	inline glm::mat4 GetPerspective(){return m_perspective;};


	inline void ResetPerspective()
	{
		if(m_isParallel)
			m_perspective = glm::ortho(m_xMin, m_xMax, m_yMin, m_yMax,  m_zMin, m_zMax);
		else
			m_perspective = glm::perspective(90.0f, (m_xMax - m_xMin) / (m_yMax - m_yMin), m_zMin, m_zMax);
	};

protected:
private:
	bool m_isParallel;
	std::string m_name;
	glm::vec3 m_eye;
	glm::vec3 m_lookAt;
	glm::vec3 m_vup;
	glm::vec4 m_viewport;

	float m_xMax;
	float m_xMin;
	float m_yMax;
	float m_yMin;
	float m_zMax;
	float m_zMin;

    glm::mat4 m_perspective;
};
