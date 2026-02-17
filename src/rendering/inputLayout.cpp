#include "rendering/inputLayout.h"
#include "string"

void InputLayout::PrepareInputLayout(size_t elementCount) 
{
	this->elements = std::vector<D3D11_INPUT_ELEMENT_DESC>(elementCount);
	this->semanticNames = std::vector <std::string> (elementCount);
	this->doNotOverwrite = true;
}

void InputLayout::AddInputElement(const std::string& semanticName, DXGI_FORMAT format, UINT inputSlot,
								  D3D11_INPUT_CLASSIFICATION inputSlotClass) {
	
	UINT semanticIndex = 0;
	// Some exceptions for matrices which should have the same name
	for (size_t i = 0; i < this->semanticNames.size(); i++) {
		if (this->semanticNames[i] == semanticName) {
			if (semanticName != "WORLD_MATRIX" && semanticName != "INVERSED_TRANSPOSED_WORLD_MATRIX") {
				throw std::exception("Failed to add InputElement: Semantic already exist.");
			} else {
				semanticIndex++;
			}
		}
	}

	if (!this->slotOffsets.contains(inputSlot)) {
		this->slotOffsets.emplace(inputSlot, 0);
	}

	D3D11_INPUT_ELEMENT_DESC input = {semanticName.c_str(),	 semanticIndex,	 format, inputSlot,
									  (UINT) this->slotOffsets[inputSlot],
									  inputSlotClass,
									  inputSlotClass == D3D11_INPUT_PER_VERTEX_DATA ? (UINT) 0 : (UINT) 1};
	if (this->doNotOverwrite) {
		this->elements[this->currentIndex] = input;
	} else {
		this->elements.push_back(input);
	}

	size_t size = 0;
	switch (format) {
	case DXGI_FORMAT_R32G32B32_FLOAT:
		size = 12;
		break;
	case DXGI_FORMAT_R32G32_FLOAT:
		size = 8;
		break;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		size = 16;
		break;
	default:
		Logger::Error("Unknown input");
		throw std::exception("Input layout fatal error.");
	}

	this->slotOffsets[inputSlot] += size;

	if (this->doNotOverwrite) {
		this->semanticNames[this->currentIndex] = semanticName;
	} else {
		this->semanticNames.push_back(semanticName);
	}

	this->currentIndex++;
}

void InputLayout::FinalizeInputLayout(ID3D11Device* device, const void* vsDataPtr, size_t vsDataSize)
{
	for (size_t i = 0; i < this->elements.size(); i++)
	{
		this->elements[i].SemanticName = this->semanticNames[i].c_str();
	}

	if (vsDataPtr == nullptr) {
		throw std::exception("Failed to create input layout: vsDataPtr is null");
	}

	HRESULT hr = device->CreateInputLayout(this->elements.data(), this->elements.size(), vsDataPtr, vsDataSize, this->inputLayout.GetAddressOf());

	if (FAILED(hr)) {
		throw std::exception(std::format("Failed to create input layout, HRESULT: 0x{:08X}", static_cast<unsigned long>(hr)).c_str());
	}
}

ID3D11InputLayout* InputLayout::GetInputLayout() const
{
	return this->inputLayout.Get();
}