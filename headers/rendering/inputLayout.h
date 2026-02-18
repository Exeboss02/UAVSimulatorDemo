#pragma once

#include "utilities/logger.h"
#include <d3d11.h>
#include <format>
#include <string>
#include <vector>
#include <wrl/client.h>
#include <unordered_map>

class InputLayout {
private:
	std::vector<std::string> semanticNames; // Needed to store the semantic names of the element descriptions
	std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout = Microsoft::WRL::ComPtr<ID3D11InputLayout>(nullptr);

	std::unordered_map<UINT, UINT> slotOffsets;

	bool doNotOverwrite = false;
	size_t currentIndex = 0;

public:
	InputLayout() = default;
	~InputLayout() = default;
	InputLayout(const InputLayout& other) = delete;
	InputLayout& operator=(const InputLayout& other) = delete;
	InputLayout(InputLayout&& other) = delete;
	InputLayout& operator=(InputLayout&& other) = delete;

	void PrepareInputLayout(size_t elementCount);
	void AddInputElement(const std::string& semanticName, DXGI_FORMAT format, UINT inputSlot = 0,
						 D3D11_INPUT_CLASSIFICATION inputSlotClass = D3D11_INPUT_PER_VERTEX_DATA);
	void FinalizeInputLayout(ID3D11Device* device, const void* vsDataPtr, size_t vsDataSize);

	ID3D11InputLayout* GetInputLayout() const;
};