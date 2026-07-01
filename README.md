# 🎮 LoLShinSouls 핵심 기능 및 소스코드 상세 분석

---

## 1. 쿼드트리 기반 씬 파일 로딩 및 랜드스케이프 지형/텍스처 복치
* **파일명**: `TeamProject/GameProject/FQuadTree.cpp` (`OpenMap`)
* **기능 개요**: 맵 에디터에서 직렬화 출력해둔 씬 설정 텍스트 파일을 런타임 클라이언트 시작 시 파싱하여, 지형 메쉬 및 다중 스플래팅 텍스처 자원들을 `TextureMgr`을 통해 동적으로 로딩 및 바인딩 완료함.
* **코드 상세 분석**:
  - `std::ifstream` 스트림을 열어 씬 파일(`szFullPath`)을 텍스트 줄 단위로 정독함.
  - 마스크 데이터 및 베이스 지형 텍스처 필드명(`m_pTexture`)을 파싱하면 텍스처 매니저(`I_Tex.Load`)를 가동해 GPU 텍스처 자원으로 로드함.
  - 스플래팅 디테일 텍스처 목록(`m_ListTextureSplatting`)이 감지되면 쉼표(`,`) 구분자 파싱을 기동하여 각각의 스플래팅 소스 경로를 고속 분리한 후 일괄 로드하여 랜드스케이프 텍스처 벡터(`pTexList`)에 적재 연동함.

```cpp
// FQuadTree.cpp: 직렬화 씬 파일 로드를 통한 랜드스케이프 및 텍스처 자원 복원
FQuadTree* OpenMap(std::wstring szFullPath, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext)
{
	Texture* pTexture = nullptr;
	std::vector<Texture*> pTexList;
	Transform mapTransform = {};
	UINT iMaxDepth = 0;
	std::wstring szVSPath;
	std::wstring szPSPath;
	std::vector<Cinema> CinemaList;
	std::vector<InstanceData> InstanceList;
	MeshMap* pMapMesh = new MeshMap();
	pMapMesh->SetDevice(pd3dDevice, pContext);
	std::vector<std::pair<std::string, Transform>> spawnList;
	std::map<std::wstring, T_BOX> triggerList;
	std::unordered_set<Object*> allObjectList;
	std::unordered_set<Object*> allInstnaceObjectList;
	BYTE* fAlphaData = nullptr;
	std::ifstream is(szFullPath);
	std::string line;

	BaseObject* pSphereObject = nullptr;

	while (std::getline(is, line))
	{
		std::istringstream iss(line);
		std::string fieldName;
		if (std::getline(iss, fieldName, ':'))
		{
			if (fieldName == "m_pTexture")
			{
				std::string textureName;
				iss >> textureName;
				PathChanger(textureName);
				I_Tex.Load(to_mw(textureName), &pTexture);
			}
			else if (fieldName == "m_ListTextureSplatting")
			{
				std::string texturesStr;
				std::getline(iss, texturesStr);
				std::stringstream texturesStream(texturesStr);
				std::string texturePath;
				while (std::getline(texturesStream, texturePath, ','))
				{
					if (texturePath.size() > 1)
					{
						texturePath.erase(std::remove(texturePath.begin(), texturePath.end(), ' '), texturePath.end());
						Texture* texture;
						PathChanger(texturePath);
						I_Tex.Load(to_mw(texturePath), &texture);
						pTexList.push_back(texture);
					}
				}
			}
			else if (fieldName == "m_Transform")
			{
				iss >> mapTransform;
			}
			else if (fieldName == "m_iMaxDepth")
			{
				iss >> iMaxDepth;
			}
            // ... 생략 ...
		}
	}
}
```

---

## 2. 쿼드트리 노드 공간 검색 및 지형 정점 선별
* **파일명**: `TeamProject/GameProject/FQuadTree.cpp` (`SelectVertexList`)
* **기능 개요**: 절두체 가시 영역에 활성화되어 렌더 리스트에 올라와 있는 리프 노드 목록을 탐색하여, 마우스 피킹 및 변형 브러시 반경 내의 리프 노드 범위만을 고속($O(\log N)$) 선별함.
* **코드 상세 분석**:
  - 마우스 조형 범위 상자(`T_BOX`)가 들어왔을 때, 가시 영역 내의 리프 노드 목록(`m_pDrawLeafNodeList`)을 전수 순회함.
  - 리프 노드의 AABB 박스(`node->m_Box`)와 마우스 박스 간의 교차 판정(`TCollision::BoxToBox`) 결과 양수(`ret > 0`)가 잡히는 노드들만 `selectNodeList`에 적재함으로써 피킹 높이 연산 정점 검색 효율을 압축함.

```cpp
// FQuadTree.cpp: 리프 노드 AABB 충돌 검사를 통한 정점 선별 가속화
UINT FQuadTree::SelectVertexList(T_BOX& box, std::vector<FNode*>& selectNodeList)
{
    for (auto node : m_pDrawLeafNodeList)
    {
        if (node != nullptr)
        {
            TCollisionType ret = TCollision::BoxToBox(node->m_Box, box);
            if (ret > 0)
            {
                selectNodeList.push_back(node);
            }
        }
    }
    return selectNodeList.size();
}
```

---

## 3. SpringArm OBB 충돌 및 레이 피킹 장애물 회피
* **파일명**: `TeamProject/GameProject/SpringArmCamera.cpp` (`CheckIntersectionWithMap`)
* **기능 개요**: 3인칭 뷰 시점에서 카메라와 캐릭터 사이에 배치된 지형 지물의 OBB 영역 충돌을 탐색하고, 카메라 거리를 동적으로 차감 보정하여 시야 차단을 예방함.
* **코드 상세 분석**:
  - 카메라의 뷰 회전 행렬을 기반으로 전방 방향 레이(`m_Select.m_Ray`)를 빌드하고, 활성화된 지형 리프 노드들과 1차 `OBBtoRay` 충돌을 고속 선별 검사함.
  - 충돌 노드의 삼각면 인덱스 배열로부터 3차원 정점 정보(`v0, v1, v2`)를 로드하여 삼각형 정밀 교차 판정(`ChkPick`)을 가동하여 가장 가까운 장애물 교점 거리를 산출함.
  - 이때 불필요한 주석 처리(Comment-out) 라인을 제거하여 런타임 지형 교차 검출 코드의 순수 작동 흐름만 확보함.

```cpp
// SpringArmCamera.cpp: 지형 리프 노드 OBB 및 정밀 삼각면 레이 캐스팅 검출
bool SSB::SpringArmCamera::CheckIntersectionWithMap()
{
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(-m_fCameraPitchAngle, m_fCameraYawAngle, 0);
	XMVECTOR DefaultForward = { 0, 0, 1, 0 };
	XMVECTOR camPosition = XMVector3TransformNormal(DefaultForward, camRotationMatrix);
	camPosition = XMVector3Normalize(camPosition);

	m_Select.m_Ray.vOrigin = m_vTarget;
	XMStoreFloat3(&m_Select.m_Ray.vDirection, camPosition);
	m_Select.m_Ray.vDirection.Normalize();

	float distance = _kMaxDistance;
	if (Player::GetInstance().IsUltimateSkill())
	{
		distance = 30;
	}

	for (auto node : _map->m_pDrawLeafNodeList)
	{
		if (m_Select.OBBtoRay(&node->m_Box))
		{
			if (node->m_Box.vMin.x > m_vPos.x || node->m_Box.vMax.x < m_vPos.x || node->m_Box.vMin.z > m_vPos.z || node->m_Box.vMax.z < m_vPos.z)
				continue;
			UINT index = 0;
			UINT iNumFace = node->m_IndexList.size() / 3;
			for (UINT face = 0; face < iNumFace; face++)
			{
				UINT i0 = node->m_IndexList[index + 0];
				UINT i1 = node->m_IndexList[index + 1];
				UINT i2 = node->m_IndexList[index + 2];
				TVector3 v0 = _map->m_pMap->m_ListVertex[i0].pos;
				TVector3 v1 = _map->m_pMap->m_ListVertex[i1].pos;
				TVector3 v2 = _map->m_pMap->m_ListVertex[i2].pos;
				if (m_Select.ChkPick(v0, v1, v2, distance))
				{
					return true;
				}
				index += 3;
			}
		}
	}
	return false;
}
```

---

## 4. 이항 계수 기반 n차 베지어 곡선 카메라 연출
* **파일명**: `TeamProject/GameProject/CameraCinema.cpp` (`MoveCameraBezierSpline`)
* **기능 개요**: 컷신 시네마틱 궤적 구성 시 지정된 키프레임 카메라 구조체 배열(`camMoveList`)의 위치 성분값을 n차 베지어 기저 수학식으로 보간하여 일정한 컷신 이동을 산출함.
* **코드 상세 분석**:
  - 카메라 무빙 정보 리스트(`camMoveList`)의 크기 값을 활용해 번스타인 기저 함수 가중치를 매 프레임 계산하고, 이를 각 제어점의 XYZ 위치 성분 및 Yaw/Pitch/Roll 회전 성분값에 곱하고 가중 합산하여 카메라의 뷰/시선 좌표를 최종 보간 결정함.
  - 사용되지 않는 쿼터니언 변조용 주석 처리 구절들을 전면 걷어내어 정돈함.

```cpp
// CameraCinema.cpp: 키프레임 구조체(CameraMove) 배열을 활용한 베지어 카메라 무빙 계산
void CameraCinema::MoveCameraBezierSpline(float time, float duration, std::vector<CameraMove>& camMoveList, XMFLOAT3& getPos, XMFLOAT3& getDir)
{
	float t = time / duration;
	int n = static_cast<int>(camMoveList.size()) - 1;
	float u = 1.0f - t;
	float un = pow(u, n);
	float tn = pow(t, n);
	XMFLOAT3 pos(0, 0, 0);
	XMFLOAT3 dir(0, 0, 0);
	for (int i = 0; i <= n; ++i)
	{
		float binomial = BinomialCoefficient(n, i);
		float basis = binomial * pow(u, n - i) * pow(t, i);
		pos.x += basis * camMoveList[i].camPos.x;
		pos.y += basis * camMoveList[i].camPos.y;
		pos.z += basis * camMoveList[i].camPos.z;

		dir.x += basis * camMoveList[i].fYaw;
		dir.y += basis * camMoveList[i].fPitch;
		dir.z += basis * camMoveList[i].fRoll;
	}

	getPos = pos;
	getDir = dir;
}
```

---

## 5. 1D 펄린 노이즈 기반 카메라 흔들림 연출
* **파일명**: `TeamProject/GameLib/Camera.cpp`
* **기능 개요**: 난수(`rand`) 기반 흔들림의 불규칙 가속 한계를 대체하기 위해, 해시 테이블 참조와 기울기 보간을 통해 인과 연속성이 있는 1D 펄린 노이즈를 기동하여 카메라 진동을 구현함.
* **코드 상세 분석**:
  - `UpdateCameraShake` 업데이트 시, 진동 감쇄 가중치와 주파수 인자를 연계하여 `PerlinNoise1D`를 호출함. 각각 X축 및 Y축의 펄린 노이즈 진동 분포값을 샘플링하고 최종 카메라 위치 오프셋(`noisePos`)으로 대입 갱신함.

```cpp
// Camera.cpp: 1D 펄린 노이즈 수식을 활용한 연속성 있는 흔들림 카메라 변조 기작
float Camera::PerlinNoise1D(float x)
{
	int X = (int)floor(x) & 255;
	x -= floor(x);
	float u = Fade(x);
	int A = hash[X];
	int B = hash[(X + 1) & 255];
	return Lerp(Gradient(hash[A], x), Gradient(hash[B], x - 1.0f), u);
}

void Camera::UpdateCameraShake()
{
	if (m_fShakeCurrent < m_fShakeDuration)
	{
		float shakeFactor = 1.0f - (m_fShakeCurrent / m_fShakeDuration);
		float offsetX = PerlinNoise1D(m_fShakeCurrent * m_fShakeFrequency) * m_fShakeAmplitude * shakeFactor;
		float offsetY = PerlinNoise1D((m_fShakeCurrent + 1000.0f) * m_fShakeFrequency) * m_fShakeAmplitude * shakeFactor;
		TVector3 noisePos(offsetX, offsetY, 0.0f);
		m_vPos += noisePos;
		m_fShakeCurrent += g_fSecondPerFrame;
	}
}
```

---

## 6. 비동기 InterfaceWork 프레임워크 트리 구조 UI
* **파일명**: `TeamProject/GameLib/Interface.cpp`
* **기능 개요**: 다수의 인게임 UI 객체의 트랜스폼/알파 연동과 비동기 트윈(Tween) 갱신 작업을 효율화하기 위해 부모-자식 노드 트리 및 InterfaceWork 라이프사이클 프레임워크를 조율함.
* **코드 상세 분석**:
  - `Interface::Frame()` 기동 시, 비동기 작업 관리 목록(`m_pWorkList`)을 순회하며 이미 실행 완료된 작업들을 안전하게 파괴 및 메모리 해제함.
  - 아직 가동 중인 작업에 한해 `work->Frame(this)` 를 기동해 비동기 연출을 수행하고, 자식 노드 목록(`m_pChildList`)에 대해 순차적으로 업데이트 루프(`data->Frame()`)를 재귀 전파시킴.
  - 이전에 걸려있던 주석 처리 코멘트 아웃 구문들을 완벽히 제거하여 코드를 가독성 있게 갱신함.

```cpp
// Interface.cpp: 등록된 InterfaceWork 목록을 매 프레임 업데이트 및 트리 재귀 처리
bool Interface::Frame()
{
	for (auto iter = m_pWorkList.begin(); iter != m_pWorkList.end();)
	{
		InterfaceWork* work = (*iter);
		if (work->m_isDone)
		{
			iter = m_pWorkList.erase(iter);
			if (work)
			{
				delete work;
				work = nullptr;
			}
			continue;
		}
		else
		{
			work->Frame(this);
			iter++;
		}
	}
	for (auto data : m_pChildList)
	{
		data->Frame();
	}

	return true;
}
```
