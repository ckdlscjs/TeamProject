#include "TrailEffect.h"
#include "DXState.h"

void	InterpolateCatmullRom(const std::vector<TVector3>& controlPos, std::vector<TVector3>* CatmullRomList, size_t controlSize, int numSamples)
{
	for (int i = 0; i < controlSize - 1; i++) {
		for (int j = 0; j < numSamples; j++) {
			float t = static_cast<float>(j) / numSamples;
			TVector3 p;
			D3DXVec3CatmullRom(&p,
				&controlPos[i > 0 ? i - 1 : i],
				&controlPos[i],
				&controlPos[i + 1],
				&controlPos[i < controlSize - 2 ? i + 2 : i + 1], t);
			CatmullRomList->push_back(p);
		}
	}
}

bool	TrailEffect::Create(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext, std::wstring shaderPath, std::wstring texPath,
	std::wstring VSname, std::wstring PSname)
{
	//m_VertexList.resize(100);
	int controlSize = 100;
	for (int i = 0; i < controlSize; i++)
	{
		Vertex v;
		float x = i;
		v.p = { x, 2, 0 };
		v.c = { 1, 1, 1, 0.0 };
		v.t = { 1, 1 };
		v.n = { 0, 0, 0 };

		m_VertexList.push_back(v);

		v.p = { x, 0, 0 };
		v.t = { 0, 0 };
		m_VertexList.push_back(v);
	}

	for (int i = 0; i < m_iTrailNum; i++)
	{
		m_vHighControlPosList[i].resize(controlSize);
		m_vLowControlPosList[i].resize(controlSize);
		m_VertexCatmullRomList[i].resize(990 * 2);
	}

	m_VertexList.resize(990 * 2);

	//m_IndexList.resize((100 - 2) * 3);
	long dwTriCnt = m_VertexList.size() - 2;
	for (UINT iTri = 0; iTri < dwTriCnt; iTri += 2)
	{
		m_IndexList.push_back(iTri + 3);
		m_IndexList.push_back(iTri + 1);
		m_IndexList.push_back(iTri + 0);

		m_IndexList.push_back(iTri + 2);
		m_IndexList.push_back(iTri + 3);
		m_IndexList.push_back(iTri + 0);
	}

	BaseObject::Create(pd3dDevice, pContext, shaderPath, texPath, VSname, PSname);

	return true;
}

bool TrailEffect::Frame()
{
	//
	// high ���� low�� ���� normal�� �ϸ� right 
	// �� ��ġ�� ����� �߽� 
	//  �� ��ġ�� �Ÿ��� �Ź� ��� ���ϴ°� ������
	// counter�� �Ź� ���� �����ؾ߰ڳ� 
	// m_iPos�� �״�� ����ϴµ� m_iPos�� ������� ������� VertexList�� ����� ��� 
	// �װ� ������� Index ���ڸ� ���� �� ����
	// 
	// �ݸ��� �浹
	// x-axis�� ���� ĳ������ ������ �����ؼ� + / - / 0�� ���� 
	// +-�� ��� 
	// 
	// �׳� T_BOX�� ������ Plane 6���� ����� ����� ����غ��� ������
	// 
	//
	//m_VertexList = std::move(m_AttackTrail[m_iCurTrail]);
	//m_VertexList = m_AttackTrail[m_iCurTrail];
	m_VertexList = m_VertexCatmullRomList[m_iCurTrail];
	int vertexListSize = m_VertexCatmullRomList[m_iCurTrail].size();
	for (int i = 0; i < vertexListSize; i++)
	{
		m_VertexCatmullRomList[m_iCurTrail][i].c.w -= g_fSecondPerFrame * 2;
	}

	return BaseObject::Frame();
}

bool TrailEffect::PreRender()
{
	return BaseObject::PreRender();
}

bool TrailEffect::Render()
{
	PreRender();
	PostRender();

	return true;
}

bool TrailEffect::PostRender()
{
	m_pImmediateContext->RSSetState(DXState::g_pNonCullRSSolid);
	if (m_pIndexBuffer == nullptr)
		m_pImmediateContext->Draw(m_VertexList.size(), 0);
	else
	{
		//int iTriCnt = (m_iPos - 4);
		int iTriCnt = min((m_iPos * 2 * 10), m_iCatmullRomSize[m_iCurTrail]) - 4;
		if (iTriCnt > 1)
			m_pImmediateContext->DrawIndexed(iTriCnt * 3, 0, 0);
	}
	m_pImmediateContext->RSSetState(DXState::g_pDefaultRSSolid);
	return true;
}

#include "Player.h"
void	TrailEffect::AddTrailPos(TVector3 low, TVector3 high)
{
	if (m_iPos == m_VertexList.size())
		return;

	TMatrix matInv = Player::GetInstance().m_matWorld.Invert();
	Vertex v;
	//v.p = low;
	D3DXVec3TransformCoord(&v.p, &low, &matInv);
	v.n = TVector3(0, 0, 0);
	v.c = TVector4(1, 1, 1, 1);
	v.t = TVector2(1, 1);

	m_vLowControlPosList[m_iCurTrail][m_iPos] = v.p;

	//v.p = high;
	D3DXVec3TransformCoord(&v.p, &high, &matInv);
	m_vHighControlPosList[m_iCurTrail][m_iPos] = v.p;
	m_iPos++;

	m_vLowCatmullRomList[m_iCurTrail].clear();
	m_vHighCatmullRomList[m_iCurTrail].clear();
	InterpolateCatmullRom(m_vLowControlPosList[m_iCurTrail], &m_vLowCatmullRomList[m_iCurTrail], m_iPos, 10);
	InterpolateCatmullRom(m_vHighControlPosList[m_iCurTrail], &m_vHighCatmullRomList[m_iCurTrail], m_iPos, 10);
	
	// �ο� ���� �� ���� ����� ���缭 
	// MixList�� ���� ���� ȣ�ηη� ������ ��
	//m_MixVertex[m_iCurTrail].clear();
	size_t minSize = min(m_vLowCatmullRomList[m_iCurTrail].size(), m_vHighCatmullRomList[m_iCurTrail].size());
	m_iCatmullRomSize[m_iCurTrail] = minSize * 2;

	int start = max(((m_iPos - 2) * 10), 0);
	for (int i = start; i < minSize; i++)
	{
		float delta = i / (float)minSize;
		//delta = i / (float)450;

		v.c = TVector4(1, 1, 1, 1);
		v.t = TVector2(delta, 1);
		v.p = m_vLowCatmullRomList[m_iCurTrail][i];
		m_VertexCatmullRomList[m_iCurTrail][i * 2 + 0] = v;

		v.p = m_vHighCatmullRomList[m_iCurTrail][i];
		v.t = TVector2(delta, 0);
		m_VertexCatmullRomList[m_iCurTrail][i * 2 + 1] = v;
	}
}
