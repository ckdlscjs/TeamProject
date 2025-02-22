#pragma once

#include "Common.h"
#include "TMath.h"
#include "Timer.h"
#include <vector>
#include <d3d11.h>
#include "Serializeable.h"
#include "SerializeableDataType.h"

namespace SSB
{
	typedef unsigned int FrameIndex;
	typedef int BoneIndex;

	class DefaultAnimation
	{
	private:
		AnimationFrameInfo* _frameInfo;

	public:
		DefaultAnimation();
		~DefaultAnimation();

	public:
		const AnimationFrameInfo* GetAnimationFrameInfo() const;
	};

	static const DefaultAnimation kAnimationInitializer;
	typedef std::string AnimationName;

	class Animation : public Common, public Serializeable
	{
	private:
		struct FrameMatrixInfo
		{
			TMatrix BoneMatrix[255];
			TMatrix MeshMatrix[255];
		};
	public:
		float	m_fAnimTime = 0.0f;
		FrameIndex _startFrame;
		FrameIndex _endFrame;

	public:
		float _framePerSecond = 30;

	private:
		int _boneAnimationUnitMaxCount = 0;
		int _meshAnimationUnitMaxCount = 0;
		std::vector<AnimationFrameInfo*> _data;

		AnimationFrameInfo* _currentFrameInfo;
		FrameMatrixInfo _frameMatrixInfo;
		ID3D11Buffer* _animatedFrameBuffer;

		bool _isLoop = false;;

	public:
		Animation();
		~Animation();

	private:
		bool CreateAnimatedFrameInfoBuffer();
		TMatrix GetInterpolate(AnimationUnitInfo& beforeInfo, AnimationUnitInfo& afterInfo, float t);
		void UpdateFrameInfo();

	public:
		void Initialize_SetAnimationFrameData(std::vector<AnimationFrameInfo*> data);
		void Initialize_SetFrameInterval(FrameIndex start, FrameIndex end);
		void Initialize_SetAnimationUnitMaximumCount(int boneCount, int meshCount);
		void Initialize_SetLoop(bool loop);

	public:
		bool Init() override;
		bool Frame() override;
		bool PreRender();
		bool Render() override;
		bool PostRender();
		bool Release() override;
		std::string Serialize() override;
		void Deserialize(const char* buffer, int size, int& offset) override;
		// Test�� �ʿ���
		TMatrix GetCurrentBoneMatrix(BoneIndex index);

	public:
		void ResetAnimationTimer();
	};
};
