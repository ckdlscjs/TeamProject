# DirectX 3D Team Project : LolShinSouls

![234498154-ffaa7cd6-9bbb-4f14-b423-c42b302dc000.PNG](https://postfiles.pstatic.net/MjAyNDAyMDlfMTkx/MDAxNzA3NDczODQ0Mzg3.R-OnPPhk45OirD-AtXJuGcIA9oFXtQXoYxDBV6-xBKcg.68hipY759BXn9Z5ya-fWH7sv6OXCKpR8jlahN_cuqHMg.PNG.sinsin63/234498154-ffaa7cd6-9bbb-4f14-b423-c42b302dc000.png?type=w580)

## 개요
- 장르
    - 3D RPG
- 제작 인원
    - 3명
- 제작 기간
    - 2달

## 기술 스택
- C++
- DirectX
- FBX SDK
- FMOD

## 역할
![dx_4](https://github.com/ckdlscjs/TeamProject/assets/41976800/ffcd1bc5-122f-40b8-a9c3-d4287dbf065f)
- **MapTool, Map**
    - **LandScape**
        - **Frustum Culling**을 통한 Rendering최적화
        - **Collider ObbBox, FbxMesh**의 **RayCast**를 이용한 충돌 구현
    - **FBXObject**
        - FBXSDK를 통해 리소스를 로드, **Vertex를 Index**로 구축하는 알고리즘을 구현해 최적화
        - 로드된 객체는 **Singleton Manager**에서 관리
    - **SkyDome**
        - 입력받은 Slice, Stack 값을 통한 알고리즘을 이용해 SkyDome을 구현
    - **Camera**
        - **n차 VezierSpline**을 통해 CinemaCamera를 구현
        - 펄린노이즈값을 이용해 Amplitue, Frequency을 이용하는 CameraShake구현
    - **Fog**
        - 플레이어의 좌표를 기준으로 하는 거리Fog 구현, 이를 프로젝트 렌더링 엔진의
        **MRT에 사용하여 최종 SRV**에 안개 적용
          
 ![Untitled2](https://github.com/ckdlscjs/TeamProject/assets/41976800/a7b14cf1-81f3-4bc0-afc0-592d8eab7d8e)
- **UI**
    - **Fade In, Out / Gage**
        - Frame당 **Vertex의 R,G,B,A** 값을 조절해 Shader에서 사용
    - **Billboard**
        - 투영행렬을 사용하는 **UI를 빌보드화** 하여 Damage, Gage 등에 사용
    - **Text**
        - DXWrite를 이용, Scenario등 텍스트를 띄우고 Fade In, Out을 같이 사용
    - **ClockWiseFade**
        - PixelShader내에서 ConstantBuffer에서 넘어온 **Degree값을 기반으로 atan**를 
        이용해 계산하여 시계방향 으로 Fade되는 효과를 구현
    - **MiniMap**
        - 추가 카메라를 통해 별도의 **RTT, ViewPort를 이용해 렌더링** 플레이어와 Enemy는
        GeometryShader를 이용한 Box로 Render
    - **AlphaMapBlur**
        - 가우시안값, AlphaMapWeight을 통해 **가중치에 따른 SRV의 Blur** 구현

## 동영상
https://youtu.be/4ECHTmTD6h4?si=qe3qtL75Kr7zCrz7 (LoLShinSouls)
https://youtu.be/_ik09B4ptmo?si=l5gQW1pp_IqxO2Hu (MapTools)
## 다운로드
http://naver.me/55RIqKpf (게임)
