#pragma once

#include <set>
#include "Light.h"
#include "std.h"

namespace SSB
{
	// �̹� ������Ʈ������ 1���� Light �� ����Ѵ� ����
	class LightManager : public Singleton<LightManager>
	{
	private:
		//std::set<SSB::Light*> m_Lights;
		SSB::Light* m_MainLight = nullptr;

	public:
		~LightManager();

	public:
		//void RegisterLight(SSB::Light* light);
		void SetLight(SSB::Light* light);

	public:
		std::vector<Light*> GetLightList();

	public:
		void Init();
		void Frame();
		//void PreRender();
		void Render();
		void Release();
	};
}
#define I_Light LightManager::GetInstance()
