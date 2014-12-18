// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <memory>
#include <vector>

#include <wrl.h>
#include <DirectXMath.h>

#include "VSD3DStarter.h"
#include "GameBase.h"

#include "StarShipMoovementTypes.h"
#include "PhysicVariables.h"

ref class Game sealed : public GameBase
{
public:
	Game();

private:
	~Game();

public:
	virtual void CreateWindowSizeDependentResources() override;
	virtual void Initialize() override;
	virtual void Update(float timeTotal, float timeDelta) override;
	virtual void Render() override;
	virtual void Clear();

	Platform::String^ OnHitObject(int x, int y);

	void RotateObject(int rotationType);
	void MooveObject(int mooveType);
	void UpdateObjectTarget();

	void UseRotation();
	void UseGravitation();
	void UseTranslation();

	void CountTranslation();
	void CountRotation();
	void CountGravitation();

	void UpdateCameraPosition();

	bool Pause() { return m_isPause; }
	void Pause(bool val) { m_isPause = val; }
	bool Multiplayer() { return m_isMultiplayer; }
	void Multiplayer(bool val) { m_isMultiplayer = val; }
	bool GameStarted() { return m_isGameStarted; }
	void GameStarted(bool val) { m_isGameStarted = val; }	

	bool AnimationRunning() { return m_isAnimationRunning; }
	void AnimationRunning(bool val) { m_isAnimationRunning = val; }
private:

	std::vector<VSD3DStarter::Mesh*> m_moonModel;
	std::vector<VSD3DStarter::Mesh*> m_landingPointModel;
	std::vector<VSD3DStarter::Mesh*> m_starShipModel;
	std::vector<VSD3DStarter::Mesh*> m_backModel;

	bool m_isGameStarted;
	bool m_isPause;
	bool m_isMultiplayer;

	bool m_isAnimationRunning;
	float m_animationTime;
	float m_generalAnimationProgress;
	float m_totalTime;

	float m_animationGravTime;

	DirectX::XMFLOAT3 m_initialRotation;
	DirectX::XMFLOAT3 m_currentRotation;
	DirectX::XMFLOAT3 m_targetRotation;

	DirectX::XMVECTOR m_initialTranslation_v_x;
	DirectX::XMVECTOR m_currentTranslation_v_x;
	DirectX::XMVECTOR m_targetTranslation_v_x;

	DirectX::XMVECTOR m_initialTranslation_v_y;
	DirectX::XMVECTOR m_currentTranslation_v_y;
	DirectX::XMVECTOR m_targetTranslation_v_y;

	DirectX::XMVECTOR m_initialTranslation_v_z;
	DirectX::XMVECTOR m_currentTranslation_v_z;
	DirectX::XMVECTOR m_targetTranslation_v_z;

	DirectX::XMVECTOR m_basicTranlsation_x;
	DirectX::XMVECTOR m_basicTranlsation_y;
	DirectX::XMVECTOR m_basicTranlsation_z;
	DirectX::XMVECTOR m_basicVector;

	float m_initialGT;
	float m_currentGT;
	float m_targetGT;
	float m_gravitationTranslation;

	float m_gravitationTime;

	float m_rotationSpeedX;
	float m_rotationSpeedY;
	float m_rotationSpeedZ;

	float m_translationSpeed;

	float m_landingX;
	float m_landingY;
	float m_landingZ;
};
