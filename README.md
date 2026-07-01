# 🛠️ MapTool & LoLShinSouls 핵심 기능 및 소스코드 상세 분석

본 문서는 DirectX 11 API를 기반으로 개발된 3D 맵 에디터(MapTool) 엔진과 액션 RPG 게임 클라이언트(LoLShinSouls)의 핵심 최적화 및 렌더링 기능들을 실제 소스코드의 데이터 연산 흐름을 바탕으로 상세하게 분석한 기술 명세서임.

---

## 1. 쿼드트리 지형 공간 분할 및 가시성 선별
* **관련 소스코드**: `FQuadTree.cpp` (`SelectVertexList`)
* **기능 개요**: 지형 정점 탐색의 시간 복잡도($O(N)$)를 최적화하기 위해 전체 공간을 4진 트리 노드로 분할하고, 충돌 영역에 걸쳐 있는 리프 노드 목록만을 빠르게 검색함.
* **코드 상세 분석**:
  - 마우스 조형(Sculpting) 브러시 기동 시 전달된 범위 상자(`T_BOX`)와 절두체 가시 영역에 의해 선별된 리프 노드 목록(`m_pDrawLeafNodeList`)을 전수 순회함.
  - 각 노드의 바운딩 박스(`node->m_Box`)와 마우스 변형 영역 간의 1차 충돌 판정(`TCollision::BoxToBox`)을 가동하여 충돌 유형(`ret > 0`)이 감지된 노드 포인터만 `selectNodeList` 벡터에 삽입함.
  - 이를 통해 넓은 월드 지형 중에서 편집 영향권에 포함된 소수의 정점 인덱스 범위($O(\log N)$)만을 선별하여 정점 데이터를 동적으로 빠르게 수정할 수 있도록 계산을 간소화함.

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

## 2. D3D11 버텍스 버퍼 GPU 자원화
* **관련 소스코드**: `VertexBuffer.cpp` (`VertexBuffer` 생성자)
* **기능 개요**: 시스템 메모리에 적재된 정점 데이터를 DirectX 11 API를 통해 GPU 비디오 메모리 버퍼 자원으로 직접 변환 및 전송함.
* **코드 상세 분석**:
  - `D3D11_BUFFER_DESC` 구조체(`bufferDesc`)를 정의하여 크기(`ByteWidth`), GPU 메모리 속성(`D3D11_USAGE_DEFAULT`), 바인딩 플래그(`D3D11_BIND_VERTEX_BUFFER`)를 설정함.
  - `D3D11_SUBRESOURCE_DATA` 구조체(`resourceData`)의 `pSysMem` 포인터에 CPU 시스템 메모리에 적재된 정점 배열의 시작 주소(`pVertices`)를 대입함.
  - D3D11 디바이스(`pDevice->CreateBuffer`) API를 호출하여 버텍스 버퍼 자원(`m_pBuffer`)을 비디오 메모리에 적재하고 바인딩을 완료함.

```cpp
// VertexBuffer.cpp: DirectX 11 API를 통한 버텍스 버퍼 생성 및 데이터 전송
VertexBuffer::VertexBuffer(ID3D11Device* pDevice, void* pVertices, UINT iSizeVertex, UINT iSizeList, INPUT_LAYOUT layout) : m_pBuffer(nullptr)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = iSizeVertex * iSizeList;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	resourceData.pSysMem = pVertices;

	m_iSizeVertex = iSizeVertex;
	m_iSizeList = iSizeList;

	pDevice->CreateBuffer(&bufferDesc, &resourceData, &m_pBuffer);
}
```

---

## 3. OBB 레이캐스트 기반 지형 정밀 피킹 검출
* **관련 소스코드**: `MyMain.cpp` (`CheckTerrainPicking`)
* **기능 개요**: 지형 정적 모델 및 바운딩 영역 편집 시, 전체 삼각면을 전수 조사하는 성능 정체를 해소하기 위해 2단계(Bounding Box $\rightarrow$ Triangle) 피킹 구조를 구축함.
* **코드 상세 분석**:
  - 1단계: 쿼드트리의 리프 노드(`m_pDrawLeafNodeList`)를 선회하면서 카메라 마우스 광선(`m_Ray`)의 기작 벡터와 노드의 OBB 바운딩 정보(`node->m_Box`)를 대조해 1차 충돌(`IntersectRayToOBB`) 여부를 가속 검사함.
  - 2단계: 1차 검출을 통과한 노드의 한해서만 인덱스 버퍼의 3개 인덱스 세트를 역산하여 실제 3D 월드 공간상의 정점 좌표(`v0`, `v1`, `v2`)를 로드하고, 삼각면 정밀 피킹 검사(`ChkPick`)를 가동하여 광선과의 교차 거리(`fDist`)를 추출함.
  - 1차 선별 없이 전체 삼각면을 돌았을 때의 정체 오버헤드를 극단적으로 단축하여 고속 3D 마우스 피킹 변형 성능을 확보함.

```cpp
// MyMain.cpp: 1차 OBB 리프 노드 필터링 및 2차 삼각면 정밀 피킹 교차 검사 연동 루프
bool MyMain::CheckTerrainPicking(float& outDist, XMVECTOR& outIntersection)
{
    std::map<float, XMVECTOR> chkDist;
    Ray ray = _PhysicsSystem.GetSelect().m_Ray;

    for (const auto& node : m_pQuadTree->m_pDrawLeafNodeList)
    {
        float fDist;
        // [1단계] 마우스 Ray와 리프 노드의 OBB 박스 간의 고속 1차 필터링 교점 연산
        if (TCollision::IntersectRayToOBB(ray.vOrigin, ray.vDirection, node->m_Box, fDist))
        {
            UINT index = 0;
            UINT iNumFace = node->m_IndexList.size() / 3;
            // [2단계] OBB 충돌 노드 내의 삼각면들만을 추출하여 정밀 폴리곤 피킹 검사 수행
            for (UINT face = 0; face < iNumFace; face++)
            {
                XMFLOAT3 v0 = m_pQuadTree->m_pMap->GetListVertex()[node->m_IndexList[index + 0]].pos;
                XMFLOAT3 v1 = m_pQuadTree->m_pMap->GetListVertex()[node->m_IndexList[index + 1]].pos;
                XMFLOAT3 v2 = m_pQuadTree->m_pMap->GetListVertex()[node->m_IndexList[index + 2]].pos;
                
                float fTriDist;
                if (_PhysicsSystem.GetSelect().ChkPick(XMLoadFloat3(&v0), XMLoadFloat3(&v1), XMLoadFloat3(&v2), fTriDist))
                {
                    chkDist.insert(std::make_pair(fTriDist, _PhysicsSystem.GetSelect().m_vIntersection));
                }
                index += 3;
            }
        }
    }
    // ... chkDist에서 최단 거리의 정점을 획득하여 반환 처리 ...
}
```

---

## 4. 이항 계수 기반 n차 베지어 곡선 카메라 연출
* **관련 소스코드**: `CameraCinema.cpp` (`MoveCameraBezierSpline`)
* **기능 개요**: 컷신 시네마틱 동선 구성 시 점과 점 사이의 단순 선형 이동에 따른 각진 화면 회전을 예방하고, 부드럽고 일정한 연속 이동과 회전의 카메라 동선을 산출함.
* **코드 상세 분석**:
  - 배치된 경로 제어점의 개수($N = \text{posList.size()} - 1$)를 차수로 규정하고, 곡선 가속 보간 인자($t$)를 시간 비율로 산정함.
  - 매 프레임 이항 계수(`BinomialCoefficient`)와 기저 다항식($\binom{n}{i} (1-t)^{n-i} t^i$)을 연산하여 제어 정점의 좌표 성분들에 대입 및 가중치를 합산하는 수학적 구현을 통해 끊김 없는 시네마틱 뷰 행렬 이동을 기동함.

```cpp
// CameraCinema.cpp: 조합(Combination) 계수를 활용한 n차 베지어 곡선 다항식 카메라 위치/시선 계산
void CameraCinema::MoveCameraBezierSpline(float time, float duration, std::vector<XMFLOAT3> posList, std::vector<XMFLOAT3> dirList, XMFLOAT3& getPos, XMFLOAT3& getDir)
{
    float t = time / duration;
    int n = static_cast<int>(posList.size()) - 1; // 베지어 차수 결정
    float u = 1.0f - t;
    XMFLOAT3 pos(0, 0, 0);
    XMFLOAT3 dir(0, 0, 0);

    for (int i = 0; i <= n; ++i)
    {
        float binomial = BinomialCoefficient(n, i);         // 이항 계수 계산
        float basis = binomial * pow(u, n - i) * pow(t, i); // 번스타인 기저 함수 값 도출
        
        pos.x += basis * posList[i].x; pos.y += basis * posList[i].y; pos.z += basis * posList[i].z;
        dir.x += basis * dirList[i].x; dir.y += basis * dirList[i].y; dir.z += basis * dirList[i].z;
    }
    getPos = pos; getDir = dir;
}
```

---

## 5. 1D 펄린 노이즈 기반 카메라 흔들림 연출
* **관련 소스코드**: `Camera.cpp` (`PerlinNoise1D` 및 `UpdateCameraShake`)
* **기능 개요**: 난수(`rand`) 기반 흔들림의 한계를 극복하기 위해 수학적 점진적 연속성이 보장되는 1D 펄린 노이즈 파동을 생성하여 카메라 쉐이크를 연출함.
* **코드 상세 분석**:
  - `InitHash`를 통해 난수 엔진(`std::mt19937`)으로 256개의 셔플 해시 테이블(`hash`)을 빌드해 둠.
  - 시간 변화량에 따른 연속값(`x`)을 입력받아 정수 바운더리 내에서 기울기 오프셋(`Gradient`)을 연산하고, 페이드 곡선(`Fade(x)`) 가중치를 통한 보간(`Lerp`) 처리를 수행하여 연속적인 1D 펄린 노이즈 분포를 산출함.
  - 진동 시간 흐름에 비례하여 감쇄 인자(`shakeFactor`)를 곱하고, X/Y 오프셋에 대해 주파수와 진폭을 적용하여 최종 뷰 좌표 오프셋(`noisePos`)을 동적 적용함.

```cpp
// Camera.cpp: 1D 펄린 노이즈 생성 및 카메라 쉐이크 오프셋 갱신 처리
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
        // ... 생략 ...
    }
}
```
