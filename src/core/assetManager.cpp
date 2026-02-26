#include "core/assetManager.h"
#include "core/filepathHolder.h"
#include "gameObjects/mesh.h"
#include "utilities/logger.h"
#include <WICTextureLoader.h>
#include <algorithm>
#include <cctype>
#include <filesystem>

void AssetManager::InitializeSoundBank(std::string pathToSoundFolder) { this->soundBank.Initialize(pathToSoundFolder); }

void AssetManager::AddSoundClipStandardFolder(std::string filename, std::string id) {
	this->soundBank.AddSoundClipStandardFolder(filename, id);
}

void AssetManager::AddSoundClip(std::string path, std::string id) { this->soundBank.AddSoundClip(path, id); }

void AssetManager::SetDevicePointer(ID3D11Device* device) { this->d3d11Device = device; }

bool AssetManager::GetMaterial(std::string identifier) {
	if (this->materials.find(identifier) != materials.end()) {
		return true;
	} else {
		if (!this->LoadNewGltf(getCleanPath(identifier))) {
			return false;
		}
	}
	return true;
}

bool AssetManager::GetMesh(std::string identifier) {
	if (this->meshes.find(identifier) != meshes.end()) {
		return true;
	} else {
		if (!this->LoadNewGltf(getCleanPath(identifier))) {
			return false;
		}
	}
	return true;
}

bool AssetManager::GetTexture(std::string identifier) {
	// Quick hit: exact key
	if (this->textures.find(identifier) != textures.end()) {
		return true;
	}

	// Try to resolve identifier to an existing stored path (normalization)
	std::string resolved = this->ResolveTexturePath(identifier);
	if (!resolved.empty() && this->textures.find(resolved) != this->textures.end()) {
		return true;
	}

	namespace fs = std::filesystem;
	fs::path idPath(identifier);
	std::string ext = idPath.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
	bool looksLikeImage =
		(ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".dds" || ext == ".tga" || ext == ".bmp");

	if (looksLikeImage) {
		// Directly try to load loose image files instead of attempting glTF parsing
		return this->LoadTextureFromFile(identifier);
	} else {
		if (!this->LoadNewGltf(getCleanPath(identifier))) {
			// Fallback: maybe it's a loose image without extension or unusual path
			if (!this->LoadTextureFromFile(identifier)) {
				return false;
			}
		}
	}
	return true;
}

bool AssetManager::LoadNewGltf(std::string identifier) {
	MeshLoadData meshLoadData;

	bool objectLoaded = this->objectLoader.LoadGltf(identifier, meshLoadData, this->d3d11Device);
	if (!objectLoaded) {
		return false;
	}
	for (std::shared_ptr<Mesh>& data : meshLoadData.meshes) {
		this->meshes.emplace(data->GetIdentifier(), std::move(data));
	}
	for (std::shared_ptr<GenericMaterial>& data : meshLoadData.materials) {
		this->materials.emplace(data->GetIdentifier(), std::move(data));
	}
	for (std::shared_ptr<Texture>& data : meshLoadData.textures) {
		this->textures.emplace(data->GetIdentifier(), std::move(data));
	}
	for (MeshObjData& data : meshLoadData.meshData) {
		this->meshObjDataSets.emplace(data.GetMeshIdentifier(), std::move(data));
	}

	return true;
}

bool AssetManager::LoadTextureFromFile(std::string identifier) {
	namespace fs = std::filesystem;
	if (identifier.empty()) return false;

	if (this->textures.find(identifier) != this->textures.end()) return true;

	fs::path p(identifier);
	if (!fs::exists(p)) {
		fs::path cwdTry = fs::current_path() / identifier;
		if (fs::exists(cwdTry)) {
			p = cwdTry;
		} else {
			fs::path exeTry = FilepathHolder::GetExeDirectory() / identifier;
			if (fs::exists(exeTry)) {
				p = exeTry;
			} else {
				std::string idStr = identifier;
				size_t pos = idStr.find("assets/");
				if (pos == std::string::npos) pos = idStr.find("assets\\");
				fs::path assetsTry;
				if (pos != std::string::npos) {
					std::string rel = idStr.substr(pos + std::string("assets/").length());
					assetsTry = FilepathHolder::GetAssetsDirectory() / rel;
				} else {
					assetsTry = FilepathHolder::GetAssetsDirectory() / identifier;
				}

				if (fs::exists(assetsTry)) {
					p = assetsTry;
				} else {
					Logger::Warn("AssetManager: Texture file not found:", identifier);
					return false;
				}
			}
		}
	}
	// Normalize to absolute path and use that as the canonical key
	p = fs::absolute(p);
	std::string key = p.string();

	std::wstring wp = p.wstring();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	HRESULT hr = DirectX::CreateWICTextureFromFile(this->d3d11Device, wp.c_str(), nullptr, srv.GetAddressOf());
	if (FAILED(hr)) {
		Logger::Error("AssetManager: Failed to create texture SRV from file:", p.string());
		return false;
	}
	auto tex = std::make_shared<Texture>(srv, key);
	this->textures.emplace(identifier, tex);
	Logger::Log("AssetManager: Loaded texture:", key);
	return true;
}

MeshObjData AssetManager::GetMeshObjData(std::string identifier) {
	if (this->meshObjDataSets.find(identifier) != meshObjDataSets.end()) {
		return meshObjDataSets.at(identifier);
	} else {
		Logger::Log(getCleanPath(identifier));
		Logger::Log(identifier);
		Logger::Log(this->LoadNewGltf(getCleanPath(identifier)));
		return meshObjDataSets.at(identifier);
	}
}

std::string AssetManager::GetPathToSoundFolder() { return this->soundBank.GetPathToSoundFolder(); }

SoundClip* AssetManager::GetSoundClip(std::string id) { return this->soundBank.GetSoundClip(id); }

AssetManager& AssetManager::GetInstance() {
	static AssetManager instance;
	return instance;
}

std::string AssetManager::getCleanPath(std::string pathToFix) {
	int point = pathToFix.find(":");
	if (point != std::string::npos) {
		return pathToFix.substr(0, point);
	} else {
		return pathToFix;
	}
}

void AssetManager::CreateDefaultAssets() {
	// Shaders

	auto vertexShader = std::shared_ptr<Shader>(new Shader());
	vertexShader->Init(this->d3d11Device, ShaderType::VERTEX_SHADER, "VSTest.cso");
	AddShader("VSStandard", vertexShader);

	auto pixelShaderLit = std::shared_ptr<Shader>(new Shader());
	pixelShaderLit->Init(this->d3d11Device, ShaderType::PIXEL_SHADER, "PSLit.cso");
	AddShader("PSLit", pixelShaderLit);

	auto pixelShaderUnlit = std::shared_ptr<Shader>(new Shader());
	pixelShaderUnlit->Init(this->d3d11Device, ShaderType::PIXEL_SHADER, "PSUnlit.cso");
	AddShader("PSUnlit", pixelShaderUnlit);

	auto skyboxVertexShader = std::shared_ptr<Shader>(new Shader());
	skyboxVertexShader->Init(this->d3d11Device, ShaderType::VERTEX_SHADER, "vsSkybox.cso");
	AddShader("VSSkybox", skyboxVertexShader);

	auto skyboxPixelShader = std::shared_ptr<Shader>(new Shader());
	skyboxPixelShader->Init(this->d3d11Device, ShaderType::PIXEL_SHADER, "psSkybox.cso");
	AddShader("PSSkybox", skyboxPixelShader);

	// Materials

	auto defaultMat = std::make_shared<GenericMaterial>(this->d3d11Device);
	AddMaterial("defaultLitMaterial", defaultMat);

	auto defaultUnlitMat = std::make_shared<UnlitMaterial>(this->d3d11Device);
	defaultUnlitMat->unlitShader = pixelShaderUnlit;
	AddMaterial("defaultUnlitMaterial", defaultUnlitMat);

	auto wireframeMaterial = std::make_shared<UnlitMaterial>(this->d3d11Device);
	wireframeMaterial->unlitShader = pixelShaderUnlit;
	wireframeMaterial->wireframe = true;
	AddMaterial("wireframeWhite", wireframeMaterial);
}

void AssetManager::AddShader(std::string identifier, std::shared_ptr<Shader> shader) {
	this->shaders.emplace(identifier, shader);
}

void AssetManager::AddMaterial(std::string identifier, std::shared_ptr<BaseMaterial> material) {
	material->SetIdentifier(identifier);
	this->materials.emplace(identifier, std::move(material));
}

std::weak_ptr<Texture> AssetManager::GetTextureWeakPtr(std::string identifier) {
	if (identifier != "") {
		if (this->textures.find(identifier) != this->textures.end()) return this->textures.at(identifier);
		std::string resolved = this->ResolveTexturePath(identifier);
		if (!resolved.empty() && this->textures.find(resolved) != this->textures.end())
			return this->textures.at(resolved);
	}
	return std::weak_ptr<Texture>();
}

std::string AssetManager::ResolveTexturePath(const std::string& identifier) {
	namespace fs = std::filesystem;
	if (identifier.empty()) return std::string();

	fs::path p(identifier);
	if (fs::exists(p)) return fs::absolute(p).string();

	fs::path cwdTry = fs::current_path() / identifier;
	if (fs::exists(cwdTry)) return fs::absolute(cwdTry).string();

	fs::path exeTry = FilepathHolder::GetExeDirectory() / identifier;
	if (fs::exists(exeTry)) return fs::absolute(exeTry).string();

	std::string idStr = identifier;
	size_t pos = idStr.find("assets/");
	if (pos == std::string::npos) pos = idStr.find("assets\\");
	fs::path assetsTry;
	if (pos != std::string::npos) {
		std::string rel = idStr.substr(pos + std::string("assets/").length());
		assetsTry = FilepathHolder::GetAssetsDirectory() / rel;
	} else {
		assetsTry = FilepathHolder::GetAssetsDirectory() / identifier;
	}

	if (fs::exists(assetsTry)) return fs::absolute(assetsTry).string();

	return std::string();
}

void AssetManager::PreloadTexturesInFolder(const std::filesystem::path& path, bool recursive) {
	namespace fs = std::filesystem;
	try {
		fs::path p(FilepathHolder::GetAssetsDirectory() / path);
		if (!fs::exists(p)) {
			Logger::Warn("AssetManager: Preload path not found:", path);
			return;
		}

		if (recursive) {
			for (auto& entry : fs::recursive_directory_iterator(p)) {
				if (!entry.is_regular_file()) continue;
				std::string ext = entry.path().extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
				if (ext == ".png") {
					this->GetTexture(std::filesystem::relative(entry.path(), path).string());
				}
			}
		} else {
			for (auto& entry : fs::directory_iterator(p)) {
				if (!entry.is_regular_file()) continue;
				std::string ext = entry.path().extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
				if (ext == ".png") {
					this->GetTexture(std::filesystem::relative(entry.path(), path).string());
				}
			}
		}
	} catch (const std::exception& e) {
		Logger::Warn("AssetManager: Preload failed:", e.what());
	}
}
