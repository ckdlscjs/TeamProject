# MapTool & LoLShinSouls 핵심 기능 및 소스코드 상세 분석

본 문서는 DirectX 11 API를 기반으로 개발된 3D 맵 에디터(MapTool) 엔진과 액션 RPG 게임 클라이언트(LoLShinSouls)의 핵심 최적화 및 렌더링 기능들을 실제 소스코드의 데이터 연산 흐름을 바탕으로 상세하게 분석한 기술 보고서임.

---

## 1. 쿼드트리 공간 분할 및 가시성 선별
* **기능 개요**: 지형 정점 탐색의 시간 복잡도($O(N)$)를 제어하기 위해 전체 공간을 4진 트리 노드로 계층 분할하고, 절두체 내에 활성화되어 있는 리프 노드 목록만을 필터링하여 실시간 높이 편집 및 컬링 연산을 최적화함.
* **코드 상세 분석**:
  - 마우스 조형(Sculpting) 브러시 기동 시 전달된 범위 상자(`T_BOX`)와 절두체 가시 영역에 의해 선별된 리프 노드 목록(`m_pDrawLeafNodeList`)을 전수 순회함.
  - 각 노드의 바운딩 박스(`node->m_Box`)와 마우스 변형 영역 간의 1차 충돌 판정(`TCollision::BoxToBox`)을 가동하여 충돌 유형(`ret > 0`)이 감지된 노드 포인터만 `selectNodeList` 벡터에 삽입함.
  - 이를 통해 넓은 월드 지형 중에서 편집 영향권에 포함된 소수의 정점 인덱스 범위($O(\log N)$)만을 선별하여 정점 데이터를 동적으로 빠르게 수정할 수 있도록 계산을 간소화함.

```cpp
// FQuadTree.cpp: 절두체 내 활성 리프 노드 목록 대상의 AABB/OBB 충돌 선별 루프
UINT FQuadTree::SelectVertexList(T_BOX& box, std::vector<FNode*>& selectNodeList)
{
    for (auto node : m_pDrawLeafNodeList)
    {
        if (node != nullptr)
        {
            // 리프 노드의 Bounding Box와 피킹 박스 간의 3D 충돌 판정
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

## 2. FBX 에셋 로딩 파이프라인 및 D3D11 버퍼 GPU 자원화
* **기능 개요**: FBX SDK로 파싱된 섭메시(Sub-mesh)들의 기하 원본 데이터를 디바이스 인터페이스를 통해 직접 제어하여 D3D11 비디오 메모리 버퍼 자원으로 정밀 생성 및 전송함.
* **코드 상세 분석**:
  - `TBaseObject::CreateVertexBuffer()` 함수 내부에서 정점 생성 프로세스(`CreateVertexData()`)를 거친 정점 리스트(`m_VertexList`)의 메모리 크기를 파악하여 `D3D11_BUFFER_DESC` 구조체(`bd`)에 바인딩 크기(`ByteWidth`)를 정의함.
  - 버퍼의 속성을 디바이스 가독 전용(`D3D11_USAGE_DEFAULT`) 및 버텍스 버퍼 타겟(`D3D11_BIND_VERTEX_BUFFER`)으로 바인딩함.
  - `D3D11_SUBRESOURCE_DATA` 구조체(`sd`)의 `pSysMem` 포인터에 CPU 시스템 메모리에 적재된 정점 배열의 시작 주소(`&m_VertexList.at(0)`)를 직접 대입하고, `CreateBuffer` API를 통해 GPU 가독형 버텍스 버퍼 자원(`m_pVertexBuffer`)을 메모리에 적재함.

```cpp
// TBaseObject.cpp: 정점 리스트 데이터를 GPU 하드웨어 버텍스 버퍼로 직접 변환 및 전송
HRESULT TBaseObject::CreateVertexBuffer()
{
    HRESULT hr;
    CreateVertexData(); // 정점 데이터 생성 프로세스 호출
    
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.ByteWidth = sizeof(PNCT_VERTEX) * m_VertexList.size(); // 총 바이트 크기 설정
    bd.Usage = D3D11_USAGE_DEFAULT;                           // GPU 디폴트 가독 모드
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;                  // 정점 버퍼 타겟 바인딩

    D3D11_SUBRESOURCE_DATA sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = &m_VertexList.at(0);                         // CPU 시스템 메모리 원본 주소 연결
    
    // DirectX 11 API를 직접 제어하여 버퍼 생성 및 데이터 전송
    hr = m_pd3dDevice->CreateBuffer(&bd, &sd, &m_pVertexBuffer);
    return hr;
}
```

---

## 3. 4중 텍스처 스플래팅 랜드스케이프 픽셀 셰이더
* **기능 개요**: 지형 정점마다 여러 장의 텍스처를 선형적으로 부드럽게 합성하여 렌더링하기 위해 픽셀 셰이더 내에서 가중치 혼합 연산을 병렬 처리함.
* **코드 상세 분석**:
  - 픽셀 셰이더 레지스터 `t0`에 바인딩된 혼합 마스크 텍스처(`g_txMaskTex`)로부터 RGBA 성분의 채널별 가중치 값(`mask.rgba`)을 샘플링하여 믹싱 인자로 사용함.
  - 기본 텍스처 컬러(`texelColor`)를 베이스로 두고, 셰이더 컴파일러가 지원하는 실시간 선형 보간 연산(`lerp`) 식을 통해 디테일 텍스처 4종(`splatTex2` ~ `splatTex5`)의 컬러값을 마스크 채널 가중치에 맞춰 픽셀 단위로 실시간 합성(Modulate)함으로써 GPU 파이프라인의 상태 전환 정체 없이 부드러운 경계 랜드스케이프 색상을 연산함.

```hlsl
// MapPixelShader.hlsl: 4채널 마스크 텍스처 가중치를 활용한 4중 지형 선형 혼합 픽셀 셰이더
Texture2D g_txBaseTex  : register(t0); // 기본 베이스 텍스처
Texture2D g_txMaskTex  : register(t1); // RGBA 혼합 가중치 마스크
Texture2D g_txSplat[4] : register(t2); // 디테일 텍스처 4종 슬롯
SamplerState g_Sampler : register(s0);

float4 PS(VS_OUTPUT input) : SV_Target
{
    float4 texelColor = g_txBaseTex.Sample(g_Sampler, input.t);
    float4 mask = g_txMaskTex.Sample(g_Sampler, input.t);
    
    // RGBA 각 성분 가중치 벡터 추출
    float4 weights = mask.rgba;
    
    // 디테일 텍스처들을 마스크 가중치 비율에 맞추어 선형 보간 합성
    float4 finalColor = texelColor * (1.0f - weights.r) + g_txSplat[0].Sample(g_Sampler, input.t) * weights.r;
    finalColor = finalColor * (1.0f - weights.g) + g_txSplat[1].Sample(g_Sampler, input.t) * weights.g;
    finalColor = finalColor * (1.0f - weights.b) + g_txSplat[2].Sample(g_Sampler, input.t) * weights.b;
    finalColor = finalColor * (1.0f - weights.a) + g_txSplat[3].Sample(g_Sampler, input.t) * weights.a;
    
    return finalColor;
}
```

---

## 4. OBB 레이캐스트 기반 지형 정밀 피킹 검출
* **기능 개요**: 지형 정적 모델 및 바운딩 영역 편집 시, 전체 삼각면을 전수 조사하는 성능 정체를 해소하기 위해 2단계(Bounding Box $\rightarrow$ Triangle) 피킹 구조를 구축함.
* **코드 상세 분석**:
  - 1단계: 쿼드트리의 리프 노드(`m_pDrawLeafNodeList`)를 선회하면서 카메라 마우스 광선(`m_Ray`)의 기작 벡터와 노드의 OBB 바운딩 정보(`node->m_Box`)를 대조해 1차 충돌(`IntersectRayToOBB`) 여부를 가속 검사함.
  - 2단계: 1차 검출을 통과한 노드의 한해서만 인덱스 버퍼의 3개 인덱스 세트를 역산하여 실제 3D 월드 공간상의 정점 좌표(`v0`, `v1`, `v2`)를 로드하고, 삼각면 정밀 피킹 검사(`ChkPick`)를 가동하여 광선과의 교차 거리(`fDist`)를 추출함.
  - 1차 선별 없이 전체 삼각면을 돌았을 때의 정체 오버헤드를 극단적으로 단축하여 고속 3D 마우스 피킹 변형 성능을 확보함.

```cpp
// ToolSystemMap.cpp: 1차 OBB 리프 노드 필터링 및 2차 삼각면 정밀 피킹 교차 검사 연동 루프
bool ToolSystemMap::CheckTerrainPicking(float& outDist, XMVECTOR& outIntersection)
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
    // ... 가장 가까운 거리(first)의 교점(second)을 최종 피킹 위치로 반환 처리 ...
}
```

---

## 5. 이항 계수 기반 n차 베지어 곡선 카메라 연출
* **기능 개요**: 컷신 시네마틱 동선 구성 시 점과 점 사이의 단순 선형 이동에 따른 각진 화면 회전을 예방하고, 부드럽고 일정한 연속 이동과 회전의 카메라 동선을 산출함.
* **코드 상세 분석**:
  - 배치된 경로 제어점의 개수($N = \text{posList.size()} - 1$)를 차수로 규정하고, 곡선 가속 보간 인자($t$)를 시간 비율로 산정함.
  - 매 프레임 이항 계수(`BinomialCoefficient`)와 기저 다항식($\binom{n}{i} (1-t)^{n-i} t^i$)을 연산하여 제어 정점의 좌표 성분들에 대입 및 가중치를 합산하는 수학적 구현을 통해 끊김 없는 시네마틱 뷰 행렬 이동을 기동함.

```cpp
// Camera.cpp: 조합(Combination) 계수를 활용한 n차 베지어 곡선 다항식 카메라 위치/시선 계산
void Camera::MoveCameraBezierSpline(float time, float duration, std::vector<XMFLOAT3> posList, std::vector<XMFLOAT3> dirList, XMFLOAT3& getPos, XMFLOAT3& getDir)
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

## 6. 상속 트리 UI 프레임워크 및 마우스 충돌 판정
* **기능 개요**: 인게임 다중 UI 컴포넌트의 가시성/알파 계수 상속과 비동기 트윈(Tween) 트랙 연동을 체계적으로 구현하기 위해 부모-자식 트리 구조 배치 및 충돌 상태 기계를 구축함.
* **코드 상세 분석**:
  - `TInterface::Frame()` 호출 시 입력 장치의 스크린 좌표(`I_Input.m_ptPos`)와 UI의 월드 충돌 영역(`m_rtCollision`) 간의 점 대 사각형 충돌 검출(`RectToPoint`)을 수행함.
  - 마우스 충돌 검출 여부에 따라 `UI_HOVER` 상태로 상태를 전이하며, 마우스 왼쪽 버튼 이벤트 감지 시 `UI_PUSH` 및 `UI_SELECT` 상태를 동적으로 분기시켜 이에 상응하는 스페셜 리소스 텍스처를 스위칭함.
  - 최종 갱신 완료 시 `ScreenToNDC` 함수를 호출하여 화면 좌표계를 D3D11에 바인딩 가능한 NDC 공간 좌표(-1.0 ~ 1.0)로 정규화하고 GPU 정점 데이터를 갱신함.

```cpp
// TInterface.cpp: UI 충돌 범위 내 마우스 이벤트 전이에 따른 텍스처 상태값 갱신 및 NDC 변환
bool TInterface::Frame()
{
    POINT ptMouse = I_Input.m_ptPos;
    // 마우스가 UI 충돌 렉트 영역 내부로 진입했는지 2D 교차 검사 기동
    if (TCollision::RectToPoint(m_rtCollision, ptMouse))
    {
        m_CurrentState = UI_HOVER;
        m_pCurrentTex = m_EventState.m_pStateList[UI_HOVER];
        
        // 버튼 누름 및 홀딩에 따른 액티브 상태 업데이트
        if (I_Input.GetKey(VK_LBUTTON) == KEY_PUSH || I_Input.GetKey(VK_LBUTTON) == KEY_HOLD)
        {
            m_CurrentState = UI_PUSH;
            m_pCurrentTex = m_EventState.m_pStateList[UI_PUSH];
        }
        if (I_Input.GetKey(VK_LBUTTON) == KEY_UP) 
        {
            m_CurrentState = UI_SELECT;
        }
    }
    else
    {
        m_pCurrentTex = m_EventState.m_pStateList[UI_NORMAL];
    }
    
    ScreenToNDC();        // NDC 정규화 좌표계 변환 수행
    UpdateVertexBuffer(); // GPU 버텍스 버퍼 다이내믹 업데이트 호출
    return true;
}
```

---

## 7. RTT 미니맵 및 기하 셰이더 포인트 팽창 렌더 패스
* **기능 개요**: 미니맵 내에 수많은 캐릭터/몬스터 아이콘을 렌더링할 때 버퍼 데이터 갱신 및 드로우콜 비용을 최소화하기 위해, 단일 위치 정보 정점을 기하 셰이더 상에서 사각형 기하 도형으로 실시간 확장 생성함.
* **코드 상세 분석**:
  - CPU는 미니맵에 표시될 다수 캐릭터의 3D 월드 위치 정보만 담은 단 1개의 포인트 정점(Primitive Point) 리스트를 정렬해 GPU에 전송함.
  - 기하 셰이더(Geometry Shader) 단에서 `[maxvertexcount(4)]` 속성을 부여받아, 포인트 1개를 입력으로 하여 뷰 투영 변환 행렬을 대조해 2D 사각형 모퉁이 정점 4개를 실시간으로 연산 및 출력 스트림(`TriangleStream`)에 주입함.
  - 대량 배치에 따른 정점 자원 전송 오버헤드와 개별 2D Plane 버퍼 구성 병목을 최하단 GPU 레벨에서 최적화 렌더링함.

```hlsl
// MinimapGS.hlsl: 단일 위치 포인트 정점을 기하 셰이더 상에서 사각형 기하 정점 4개로 팽창
struct GS_INPUT
{
    float3 Pos : POSITION;
    float2 Size : SIZE;
};

struct GS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

[maxvertexcount(4)]
void GS(point GS_INPUT input[1], inout TriangleStream<GS_OUTPUT> outputStream)
{
    GS_OUTPUT output;
    float3 pos = input[0].Pos;
    float2 halfSize = input[0].Size * 0.5f;

    // 단일 포인트를 중심으로 사각형 4개 모퉁이의 상대 오프셋 좌표 실시간 연산 및 투영 변환
    float3 corners[4] = {
        float3(pos.x - halfSize.x, pos.y + halfSize.y, pos.z), // 좌상단
        float3(pos.x + halfSize.x, pos.y + halfSize.y, pos.z), // 우상단
        float3(pos.x - halfSize.x, pos.y - halfSize.y, pos.z), // 좌하단
        float3(pos.x + halfSize.x, pos.y - halfSize.y, pos.z)  // 우하단
    };

    float2 uvs[4] = {
        float2(0.0f, 0.0f), float2(1.0f, 0.0f),
        float2(0.0f, 1.0f), float2(1.0f, 1.0f)
    };

    for (int i = 0; i < 4; i++)
    {
        // 월드 위치 좌표를 동적으로 확장하여 스트림 출력
        output.Pos = mul(float4(corners[i], 1.0f), g_matViewProj);
        output.Tex = uvs[i];
        outputStream.Append(output);
    }
    outputStream.RestartStrip();
}
```
