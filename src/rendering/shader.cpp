#include "rendering/shader.h"

Shader::Shader() : type(ShaderType::PIXEL_SHADER)
{
}

Shader::~Shader()
{
	if (this->shader.vertex)
		this->shader.vertex->Release();
	else if (this->shader.hull)
		this->shader.hull->Release();
	else if (this->shader.domain)
		this->shader.domain->Release();
	else if (this->shader.geometry)
		this->shader.geometry->Release();
	else if (this->shader.pixel)
		this->shader.pixel->Release();
	else if (this->shader.compute)
		this->shader.compute->Release();
}

void Shader::Init(ID3D11Device* device, ShaderType shaderType, const char* csoPath)
{
	std::string shaderData;
	std::ifstream reader;
	reader.open(csoPath, std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		throw std::exception("Could not open shader file!");
	}

	reader.seekg(0, std::ios::end);
	reader.seekg(0, std::ios::end);
	std::streampos endPos = reader.tellg();
	if (endPos != std::streampos(-1)) {
		shaderData.reserve(static_cast<size_t>(endPos));
	} else {
		// If tellg failed, clear state and continue without reserving
		reader.clear();
	}
	reader.seekg(0, std::ios::beg);

	shaderData.assign((std::istreambuf_iterator<char>(reader)),
		std::istreambuf_iterator<char>());

	HRESULT hr = 0;

	switch (shaderType)
	{
	case ShaderType::VERTEX_SHADER:
		hr = device->CreateVertexShader(shaderData.c_str(), shaderData.length(), nullptr, &this->shader.vertex);
		break;
	case ShaderType::HULL_SHADER:
		hr = device->CreateHullShader(shaderData.c_str(), shaderData.length(), nullptr, &this->shader.hull);
		break;
	case ShaderType::DOMAIN_SHADER:
		hr = device->CreateDomainShader(shaderData.c_str(), shaderData.length(), nullptr, &this->shader.domain);
		break;
	case ShaderType::GEOMETRY_SHADER:
		hr = device->CreateGeometryShader(shaderData.c_str(), shaderData.length(), nullptr, &this->shader.geometry);
		break;
	case ShaderType::PIXEL_SHADER:
		hr = device->CreatePixelShader(shaderData.c_str(), shaderData.length(), nullptr, &this->shader.pixel);
		break;
	case ShaderType::COMPUTE_SHADER:
		hr = device->CreateComputeShader(shaderData.c_str(), shaderData.length(), nullptr, &this->shader.compute);
		break;
	}

	if (FAILED(hr))
	{
		throw std::exception(std::format("Failed to create shader, HRESULT: 0x{:08X}", static_cast<unsigned long>(hr)).c_str());
	}

	if (shaderType == ShaderType::VERTEX_SHADER) {
		this->shaderByteCode = shaderData;
	}

	shaderData.clear();
	reader.close();

	this->type = shaderType;
}

const void* Shader::GetShaderByteData() const
{
	return this->shaderByteCode.c_str();
}

size_t Shader::GetShaderByteSize() const
{
	return this->shaderByteCode.length();
}

std::string& Shader::GetShaderByteCode()
{
	return this->shaderByteCode;
}

void* Shader::GetShader()
{
	return this->shader.vertex;
}

void Shader::BindShader(ID3D11DeviceContext* context) const
{
	switch (this->type)
	{
	case ShaderType::VERTEX_SHADER:
		context->VSSetShader(this->shader.vertex, nullptr, 0);
		break;
	case ShaderType::HULL_SHADER:
		context->HSSetShader(this->shader.hull, nullptr, 0);
		break;
	case ShaderType::DOMAIN_SHADER:
		context->DSSetShader(this->shader.domain, nullptr, 0);
		break;
	case ShaderType::GEOMETRY_SHADER:
		context->GSSetShader(this->shader.geometry, nullptr, 0);
		break;
	case ShaderType::PIXEL_SHADER:
		context->PSSetShader(this->shader.pixel, nullptr, 0);
		break;
	case ShaderType::COMPUTE_SHADER:
		context->CSSetShader(this->shader.compute, nullptr, 0);
		break;
	}
}