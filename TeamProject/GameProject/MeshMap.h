#pragma once
#include "Map.h"

struct face_info
{
	UINT     vertexArray[3];
	XMVECTOR vNormal;
};
struct vertex_info
{
	std::vector<UINT> faceIndexArray;
	XMVECTOR vNormal;
};

namespace MAPLOAD
{
	void OpenFile(std::wstring szFullPath);
	float	Lerp(float fStart, float fEnd, float fTangent)
	{
		return fStart - (fStart * fTangent) + (fEnd * fTangent);
	}
};

class MeshMap
{
public:
	std::vector<PTNC>& GetListVertex();
	std::vector<DWORD>& GetListIndex();
	float	GetHeightmap(int row, int col);
	float	GetHeight(float fPosX, float fPosZ);
	void	GenerateVertexNormal();																		//�� ���ؽ������� �����븻����
	XMVECTOR ComputeFaceNormal(UINT i0, UINT i1, UINT i2);
	void	ComputeVertexNormal(UINT iVertex);
	friend std::ostream& operator<<(std::ostream& os, const MeshMap* pMap);

public:
	MeshMap();
	MeshMap(UINT iWidth, UINT iHeight, float fCellDistance);
	virtual ~MeshMap();
public:
	DWORD m_dwNumRows;
	DWORD m_dwNumColumns;
	DWORD m_dwFace;

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11InputLayout* m_pVertexInputLayout;

	ID3D11Buffer* m_pIndexBuffer;

	std::vector<PTNC> m_ListVertex;	//��������
	std::vector<DWORD> m_ListIndex; //�ε�������
	std::vector<float> m_fHeightList;	//���̸��ǳ�������
	std::vector<vertex_info> m_ListVertexInfo;	//���������븻
	std::vector<face_info> m_ListFaceNormal;	//ǥ������븻
	float m_fCellDistance = 1.0f;
	friend class FNode;
	friend class FQuadTree;
};

