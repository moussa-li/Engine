#pragma once

#include <string>

#include "libexport.h"

class DLLAPI Texture
{
private:
	unsigned int m_RendererId;
	std::string m_FilePath;
    std::string m_Type;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;
public:
	Texture(const std::string& path, std::string type);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unibind() const;

	inline int GetWidth() const { return m_Width; }
	inline std::string GetFilePath() const { return m_FilePath; }
	inline std::string GetType() const { return m_Type; }
	inline int GetHeight() const { return m_Height; }
};
