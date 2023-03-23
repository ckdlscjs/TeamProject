#pragma once
#include "Map.h"
#include "Character.h"
class Object : public Character
{
private:
	std::wstring mtw(std::string str);
	std::string wtm(std::wstring str);
	std::vector<std::wstring> SplitPath(std::wstring fullPath);

public:
	bool Frame() override;
	bool Render() override;
public:
	Object(std::string szFullPath, Transform transform, XMFLOAT3 vAxis0, XMFLOAT3 vAxis1, XMFLOAT3 vAxis2, float fExtentX, float fExtentY, float fExtentZ, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	~Object();
};
