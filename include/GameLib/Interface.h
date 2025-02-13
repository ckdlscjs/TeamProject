#pragma once
//#include "CSprite.h"
#include "BaseObject.h"
#include "Input.h"


enum	E_UIState
{
	UI_NORMAL,
	UI_HOVER,
	UI_PUSH,
	UI_SELECT,
	UI_DIS,
	UI_MAXSTATE
};

class InterfaceWork;
class Interface : public BaseObject
{
public:
	virtual bool Frame() override;
	virtual bool Render() override;
	virtual bool Release() override;
	virtual bool SetAttribute(TVector3 vPos, TRectangle rc);
	virtual bool SetAttribute(TVector3 vPos = { 0, 0, 0 }, TVector3 vScale = { 1, 1, 1 }, TColor color = { 1, 1, 1, 1 });
	virtual void SetNormalizeDesc();
	virtual bool SetDrawList(TRectangle rcScaleXY, TRectangle rcScaleUV);
	virtual void ToNDC();
	virtual void AddChild(Interface* pUI)
	{
		m_pChildList.push_back(pUI);
	}
	int		GetMaxFrame()
	{
		return m_pTexList.size();
	}
	void	SetFrame(int index)
	{
		if (index < 0 || index >= m_pTexList.size())
			return;

		m_pTexture = m_pTexList[index];
	}
	void SetAllAlpha(float fAlpha)
	{
		for (int i = 0; i < m_VertexList.size(); i++)
		{
			m_VertexList[i].c.w = fAlpha;
		}
		for (int iChild = 0; iChild < m_pChildList.size(); iChild++)
		{
			m_pChildList[iChild]->SetAllAlpha(fAlpha);
		}
	}

public:
	E_UIState	m_CurrentState;
public:
	std::list<InterfaceWork*> m_pWorkList;
	std::vector<Interface*> m_rcDrawList;
	std::vector<Interface*> m_pChildList;
	std::vector<Texture*>	m_pTexList;
	TVector2 m_NormalizeDesc;
public:
	bool	m_isUsing = true;
};

class InterfaceBillboard : public Interface
{
public:
	virtual void SetNormalizeDesc() override;
	virtual void CreateBillboardMatrix();
	virtual bool Render() override;
	virtual void ToNDC() override;
};

struct DamageFont
{
	int m_iNum;
	float m_u0;
	float m_v0;
	float m_u1;
	float m_v1;
	DamageFont(int iNum, float u0, float v0, float u1, float v1)
	{
		m_iNum = iNum;
		m_u0 = u0;
		m_v0 = v0;
		m_u1 = u1;
		m_v1 = v1;
	}
};

class InterfaceDamage : public InterfaceBillboard
{
public:
	virtual bool Frame() override;
	virtual bool Render() override;
public:
	void SetDamageList(std::vector<DamageFont>* pDamageList);
	std::vector<DamageFont>* m_pDamageList;
};

class InterfaceMinimap : public Interface
{
public:
	//virtual bool Frame() override;
	//virtual bool Render() override;
	virtual void SetNormalizeDesc() override;
	virtual void ToNDC() override;

public:
	void SetMapDesc(float fWidth, float fHeight)
	{
		m_MapDesc.x = fWidth;
		m_MapDesc.y = fHeight;
	}
	TVector2 m_MapDesc;
};
class InterfaceWork
{
public:
	virtual bool Frame(Interface* pInter) { return true; }
	virtual bool Render(Interface* pInter) { return true; }
public:
	bool	m_isDone = false;
};

class InterfaceLifeTime : public InterfaceWork
{
public:
	bool Frame(Interface* pInter)
	{
		m_fLifeTime -= g_fSecondPerFrame;
		if (m_fLifeTime <= 0)
		{
			pInter->m_isUsing = false;
		}
		return true;
	}
public:
	InterfaceLifeTime(float fLifeTime)
	{
		m_fLifeTime = fLifeTime;
	}
public:
	float	m_fLifeTime;
};

class InterfaceAnim : public InterfaceWork
{
public:
	bool Frame(Interface* pInter)
	{
		m_fTimer += g_fSecondPerFrame;
		if (m_fTimer >= m_fStep)
		{
			m_fTimer -= m_fStep;
			m_iFrame++;
			if (m_iFrame >= pInter->GetMaxFrame())
				m_iFrame = 0;
		}
		pInter->SetFrame(m_iFrame);
		return true;
	}
public:
	InterfaceAnim(float fStep)
	{
		m_fStep = fStep;
	}
public:
	int		m_iFrame = 0;
	float	m_fTimer = 0.0f;
	float	m_fStep;
};

class InterfaceFade : public InterfaceWork
{
public:
	bool Frame(Interface* pInter)
	{
		m_fAlpha += m_iSwitch * g_fSecondPerFrame * 2.5f;
		if (m_fAlpha >= 2.5f)
		{
			m_iSwitch = -1;
			m_fAlpha = 1.2f;
		}
		if (m_fAlpha <= 0.0f)
		{
			m_isDone = true;
		}

		for (int i = 0; i < pInter->m_VertexList.size(); i++)
		{
			pInter->m_VertexList[i].c.w = m_fAlpha;
		}
		return true;
	}

public:
	bool	m_isFade = false;
	int		m_iSwitch = 1;
	float	m_fAlpha = 0.0f;
};

class InterfaceFadeIn : public InterfaceWork
{
public:
	bool	Frame(Interface* pInter)
	{
		m_fCurrent += g_fSecondPerFrame;
		m_fAlpha = m_fCurrent / m_fFadeTime;
		if (m_fCurrent >= m_fFadeTime)
		{
			m_isDone = true;
			m_fAlpha = 1.0f;
		}

		for (int i = 0; i < pInter->m_VertexList.size(); i++)
		{
			pInter->m_VertexList[i].c.w = m_fAlpha;
		}
		return true;
	}
public:
	InterfaceFadeIn(float fFadeTime = 1.0f)
	{
		m_fFadeTime = fFadeTime;
		m_fCurrent = 0.0f;
	}
public:
	float m_fAlpha = 0.0f;
	float m_fFadeTime = 1.0f;
	float m_fCurrent = 0.0f;
};

class InterfaceFadeOut : public InterfaceWork
{
public:
	bool	Frame(Interface* pInter)
	{
		m_fRemain -= g_fSecondPerFrame;
		m_fAlpha = m_fRemain / m_fFadeTime;
		if (m_fRemain <= 0.0f)
		{
			m_isDone = true;
			m_fAlpha = 0.0f;
		}

		for (int i = 0; i < pInter->m_VertexList.size(); i++)
		{
			pInter->m_VertexList[i].c.w = m_fAlpha;
		}
		return true;
	}
public:
	InterfaceFadeOut(float fFadeTime = 1.0f)
	{
		m_fFadeTime = fFadeTime;
		m_fRemain = fFadeTime;
	}
public:
	float m_fAlpha = 1.0f;
	float m_fFadeTime;
	float m_fRemain;
};

class InterfaceFadeInOut : public InterfaceWork
{
public:
	bool	Frame(Interface* pInter)
	{
		m_fCurrent += g_fSecondPerFrame;
		if(m_fCurrent < m_fFadeInTime)
			m_fAlpha = m_fCurrent / m_fFadeInTime;
		if (m_fCurrent > m_fDuraiton - m_fFadeOutTime)
			m_fAlpha = 1.0f - (m_fFadeOutTime - (m_fDuraiton - m_fCurrent)) / m_fFadeOutTime;
		if (m_fCurrent >= m_fDuraiton)
		{
			m_isDone = true;
			m_fAlpha = 0.0f;
		}
		for (int i = 0; i < pInter->m_VertexList.size(); i++)
		{
			pInter->m_VertexList[i].c.w = m_fAlpha;
		}
		return true;
	}
public:
	InterfaceFadeInOut(float fDuration, float fFadeIn = 0.25f, float fFadeOut = 0.5f)
	{
		m_fDuraiton = fDuration;
		m_fCurrent = 0.0f;
		m_fFadeInTime = fFadeIn;
		m_fFadeOutTime = fFadeOut;
	}
public:
	float m_fAlpha = 0.0f;
	float m_fDuraiton;
	float m_fCurrent = 0.0f;
	float m_fFadeInTime = 0.5f;
	float m_fFadeOutTime = 0.5f;
};

class InterfaceLoopFade : public InterfaceWork
{
public:
	bool Frame(Interface* pInter)
	{
		m_fAlpha += m_iSwitch * g_fSecondPerFrame * m_fFadeSpeed;
		if (m_fAlpha >= 1.0f)
		{
			m_iSwitch = -1;
		}
		if (m_fAlpha <= 0.0f)
		{
			m_iSwitch = 1;
		}

		for (int i = 0; i < pInter->m_VertexList.size(); i++)
		{
			pInter->m_VertexList[i].c.w = m_fAlpha;
		}
		return true;
	}
public:
	InterfaceLoopFade(float fFadeSpeed)
	{
		m_fFadeSpeed = fFadeSpeed;
	}

public:
	int		m_iSwitch = 1;
	float	m_fAlpha = 0.0f;
	float	m_fFadeSpeed = 0.0f;
};

class InterfaceClick : public InterfaceWork
{
public:
	bool Frame(Interface* pInter)
	{
		POINT ptMouse = I_Input.m_ptPos;
		pInter->m_vScale = TVector3(m_fScale);

		TVector3 pos = pInter->m_vPos;
		long width = pInter->m_pTexture->m_Desc.Width;
		long height = pInter->m_pTexture->m_Desc.Height;

		TRectangle rect = { (long)pos.x, (long)pos.y, width, height };
		if (rect.IntersectPoint(ptMouse))
		{
			pInter->m_CurrentState = UI_HOVER;
			pInter->m_vScale = TVector3(m_fScale * 1.1f);
			if (I_Input.GetKey(VK_LBUTTON) == KEY_PUSH ||
				I_Input.GetKey(VK_LBUTTON) == KEY_HOLD)
			{
				pInter->m_CurrentState = UI_PUSH;
				pInter->m_vScale = TVector3(m_fScale * 0.9f);
			}
			if (I_Input.GetKey(VK_LBUTTON) == KEY_UP)
			{
				pInter->m_CurrentState = UI_SELECT;
			}
		}
		else
		{
			pInter->m_CurrentState = UI_NORMAL;
		}
		pInter->SetFrame(pInter->m_CurrentState);
		return true;
	}

public:
	InterfaceClick(float fScale)
	{
		m_fScale = fScale;
	}
public:
	float	m_fScale;
};

class InterfaceSetGage : public InterfaceWork
{
public:
	bool	Frame(Interface* pInter)
	{
		m_fRemain -= g_fSecondPerFrame;
		if (m_fRemain <= 0.0f)
		{
			pInter->m_VertexList[1].t.x = m_fGage;
			pInter->m_VertexList[3].t.x = m_fGage;
			m_isDone = true;
		}
		float t = 1.0f - m_fRemain / m_fDuration;
		float diff = m_fGage - pInter->m_VertexList[1].t.x;

		pInter->m_VertexList[1].t.x += diff * t;
		pInter->m_VertexList[3].t.x = pInter->m_VertexList[1].t.x;

		/*	std::string remainValue = "---";
			remainValue += std::to_string(m_fRemain);
			remainValue += "---";
			remainValue += std::to_string(pInter->m_VertexList[1].t.x);
			remainValue += "\n";
			OutputDebugStringA(remainValue.c_str());*/
		return true;
	}
public:
	InterfaceSetGage(float fGage, float fDuration = 2.5f) : m_fGage(fGage), m_fDuration(fDuration), m_fRemain(fDuration)
	{

	}
public:
	float m_fGage;
	float m_fDuration;
	float m_fRemain;
};

class InterfaceDamageFloating : public InterfaceWork
{
public:
	bool	Frame(Interface* pInter) override
	{
		m_fRemain -= g_fSecondPerFrame;
		m_fAlpha = m_fRemain / m_fDuration;
		if (m_fRemain <= 0.0f)
		{
			m_isDone = true;
		}
		for (int i = 0; i < pInter->m_VertexList.size(); i++)
		{
			pInter->m_VertexList[i].c.w = m_fAlpha;
		}
		return true;
	}

	bool	Render(Interface* pInter) override
	{
		pInter->m_VertexList[0].p.x = -m_fDamageSize / 2.0f;
		pInter->m_VertexList[0].p.y = m_Damage[0].m_v1 / 2.0f + (m_fFloatLength * (1.0f - m_fRemain / m_fDuration));
		for (int idx = 0; idx < m_Damage.size(); idx++)
		{
			float width = m_Damage[idx].m_u1;
			float height = m_Damage[idx].m_v1;

			pInter->m_VertexList[1].p.x = pInter->m_VertexList[0].p.x + width;
			pInter->m_VertexList[1].p.y = pInter->m_VertexList[0].p.y;

			pInter->m_VertexList[2].p.x = pInter->m_VertexList[0].p.x;
			pInter->m_VertexList[2].p.y = pInter->m_VertexList[0].p.y - height;

			pInter->m_VertexList[3].p.x = pInter->m_VertexList[1].p.x;
			pInter->m_VertexList[3].p.y = pInter->m_VertexList[2].p.y;

			float descWidth = (float)pInter->m_pTexture->m_Desc.Width;
			float descHeight = (float)pInter->m_pTexture->m_Desc.Height;

			pInter->m_VertexList[0].t.x = m_Damage[idx].m_u0 / descWidth;
			pInter->m_VertexList[0].t.y = m_Damage[idx].m_v0 / descHeight;

			pInter->m_VertexList[1].t.x = (m_Damage[idx].m_u0 + m_Damage[idx].m_u1) / descWidth;
			pInter->m_VertexList[1].t.y = pInter->m_VertexList[0].t.y;

			pInter->m_VertexList[2].t.x = pInter->m_VertexList[0].t.x;
			pInter->m_VertexList[2].t.y = (m_Damage[idx].m_v0 + m_Damage[idx].m_v1) / descHeight;

			pInter->m_VertexList[3].t.x = pInter->m_VertexList[1].t.x;
			pInter->m_VertexList[3].t.y = pInter->m_VertexList[2].t.y;

			pInter->UpdateVertexBuffer();
			pInter->UpdateConstantBuffer();
			pInter->Interface::Render();
			pInter->m_VertexList[0].p.x = pInter->m_VertexList[1].p.x;
		}

		return true;
	}
public:
	InterfaceDamageFloating(UINT iDamage, Interface* pInter, float fDuration = 2.5f, float fFloatLength = 5.0f) : m_fDuration(fDuration), m_fRemain(fDuration), m_fFloatLength(fFloatLength)
	{
		m_iDamage = iDamage;
		m_pInter = pInter;
		std::string szDamage = std::to_string(iDamage);
		for (int idx = 0; idx < szDamage.size(); idx++)
		{
			m_Damage.push_back(dynamic_cast<InterfaceDamage*>(pInter)->m_pDamageList->at(szDamage[idx] - 48));
			m_fDamageSize += m_Damage[idx].m_u1;
		}
	}
public:
	std::vector<DamageFont> m_Damage;
	UINT m_iDamage;
	Interface* m_pInter;
	float m_fDamageSize;
	float m_fDuration;
	float m_fRemain;
	float m_fFloatLength;
	float m_fAlpha;
};

class InterfaceFadeClockwise : public InterfaceWork
{
public:
	bool Frame(Interface* pInter)
	{
		m_fCurrent += g_fSecondPerFrame;
		if (m_fCurrent >= m_fTime)
		{
			m_isDone = true;
		}
		pInter->m_cbData.fTimer = 360.0f * m_fCurrent / m_fTime;
		/*OutputDebugStringA("------------------");
		OutputDebugStringA(std::to_string(m_fCurrent).c_str());
		OutputDebugStringA(" ");
		OutputDebugStringA(std::to_string(pInter->m_cbData.fTimer).c_str());
		OutputDebugStringA("------------------");
		OutputDebugStringA("\n");*/
		return true;
	}

public:
	InterfaceFadeClockwise(float fTime) : m_fTime(fTime)
	{
		m_fCurrent = 0.0f;
	}
	InterfaceFadeClockwise(float fTime, float fRemain) : m_fTime(fTime)
	{
		m_fCurrent = fTime - fRemain;
	}
public:
	float m_fTime;
	float m_fCurrent;
};

