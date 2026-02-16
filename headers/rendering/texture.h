#pragma once

#include <d3d11.h>
#include <wrl.h>
class Texture {
public:
	Texture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv, std::string identifier)
		: srv(srv), identifier(std::move(identifier)) {}
	Texture(ID3D11ShaderResourceView* srv, std::string identifier) : srv(srv), identifier(std::move(identifier)) {}

	ID3D11ShaderResourceView* GetSrv() { return this->srv.Get(); }

	std::string GetIdentifier() { return this->identifier; }

private:
	std::string identifier;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
};