#pragma once

#include <d3d11.h>

#include <fstream>
#include <iostream>
#include <string>

#include <wrl/client.h>

#include <format>

enum class ShaderType { VERTEX_SHADER, HULL_SHADER, DOMAIN_SHADER, GEOMETRY_SHADER, PIXEL_SHADER, COMPUTE_SHADER };

class Shader {
private:
	ShaderType type;

	// ComPtr:s didn't work well in a union, maybe someone should check out if there's a different approach
	union {
		ID3D11VertexShader* vertex = nullptr;
		ID3D11HullShader* hull;
		ID3D11DomainShader* domain;
		ID3D11GeometryShader* geometry;
		ID3D11PixelShader* pixel;
		ID3D11ComputeShader* compute;
	} shader;

	Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob = nullptr;

	std::string shaderByteCode;

public:
	Shader();
	~Shader();
	Shader(const Shader& other) = delete;
	Shader& operator=(const Shader& other) = delete;
	Shader(Shader&& other) = delete;
	Shader& operator=(Shader&& other) = delete;

	void Init(ID3D11Device* device, ShaderType shaderType, const char* csoPath);

	const void* GetShaderByteData() const;
	size_t GetShaderByteSize() const;
	std::string& GetShaderByteCode();

	void* GetShader();

	void BindShader(ID3D11DeviceContext* context) const;
};