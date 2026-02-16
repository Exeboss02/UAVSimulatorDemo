#pragma once

#include "utilities/logger.h"
#include <d3d11.h>
#include <format>
#include <string>
#include <vector>
#include <wrl/client.h>

class InputLayout {
private:
	std::vector<std::string> semanticNames; // Needed to store the semantic names of the element descriptions
	std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout = Microsoft::WRL::ComPtr<ID3D11InputLayout>(nullptr);

	size_t next_pos = 0;

public:
	InputLayout() = default;
	~InputLayout() = default;
	InputLayout(const InputLayout& other) = delete;
	InputLayout& operator=(const InputLayout& other) = delete;
	InputLayout(InputLayout&& other) = delete;
	InputLayout& operator=(InputLayout&& other) = delete;

	void AddInputElement(const std::string& semanticName, DXGI_FORMAT format);
	void FinalizeInputLayout(ID3D11Device* device, const void* vsDataPtr, size_t vsDataSize);

	ID3D11InputLayout* GetInputLayout() const;
};