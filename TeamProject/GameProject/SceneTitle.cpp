#include "SceneTitle.h"
#include "Input.h"

void SceneTitle::DataLoad()
{

}

E_SCENE     SceneTitle::NextScene()
{
	if (I_Input.GetKey(VK_SPACE) == KEY_UP)
	{
		return E_SCENE::S_INGAME;
	}
	return S_TITLE;
}

bool    SceneTitle::Init()
{
	/*I_Input.SwitchShowMouse(true);

	m_pObj = new TestObj;
	m_pInteract = new InteractiveObject();

	m_pInteract->SetSwitch(&m_pObj->m_isB);*/

	m_pInter = new Interface();
	m_pInter->Create(m_pd3dDevice, m_pImmediateContext, L"../../data/shader/Ui.txt", L"../../data/UI/scene_title.dds");
	m_pInter->SetAttribute(TVector3(0, 0, 0));

	Shader* temp;
	I_Shader.VSLoad(L"../../data/shader/DefaultParticle.hlsl", L"VS", &temp);
	I_Shader.VSLoad(L"../../data/shader/DefaultParticle.hlsl", L"RibbonVS", &temp);

	I_Shader.PSLoad(L"../../data/shader/DefaultParticle.hlsl", L"PS", &temp);
	I_Shader.PSLoad(L"../../data/shader/DefaultParticle.hlsl", L"Distortion", &temp);
	I_Shader.PSLoad(L"../../data/shader/DefaultParticle.hlsl", L"COLOR_PS", &temp);

	I_Shader.GSLoad(L"../../data/shader/DefaultParticle.hlsl", L"RibbonGS", &temp);

	Texture* tTex;
	I_Tex.Load(L"../../data/effectdata/smoke_tex.png", &tTex);
	return true;
}

bool    SceneTitle::Frame()
{
	/*if (I_Input.GetKey(VK_F3) == KEY_PUSH)
		I_Input.SwitchShowMouse(!I_Input.GetShowMouse());
	if (I_Input.GetKey('Q') == KEY_PUSH)
	{
		m_pInteract->Switching();
	}*/

	m_pInter->Frame();
	/*POINT pt = I_Input.m_ptPos;
	ScreenToClient(g_hWnd, &pt);
	m_pInter->SetAttribute(TVector3(pt.x, pt.y, 0), TVector3(1, 1, 1), TColor(1, 1, 1, 1));*/

	return true;
}

bool    SceneTitle::Render()
{
	
	return true;
}

bool SceneTitle::PostRender()
{
	m_pInter->Render();
	return true;
}

bool    SceneTitle::Release()
{
	if (m_pInter)
	{
		m_pInter->Release();
		delete m_pInter;
		m_pInter = nullptr;
	}

	/*delete m_pObj;
	delete m_pInteract;*/
	return true;
}