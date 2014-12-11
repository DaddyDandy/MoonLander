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
    
    Platform::String^ OnHitObject(int x, int y);

	void RotateObject(int rotationType);
	void MooveObject(int mooveType);
	void UpdateObjectTarget();

	/*bool AnimationRunning() { return m_isAnimationRunning; }
	void AnimationRunning(bool val) { m_isAnimationRunning = val; }*/
private:
    std::vector<VSD3DStarter::Mesh*> m_meshModels;
	std::vector<VSD3DStarter::Mesh*> m_moonModel;
	std::vector<VSD3DStarter::Mesh*> m_starShipModel;

	float m_rotationX;
	float m_rotationY;
	float m_rotationZ;

	float m_translationX;
	float m_translationY;
	float m_translationZ;

	/*bool m_isAnimationRunning;	
	float m_animationTime;

	DirectX::XMFLOAT3 m_initialRotation;
	DirectX::XMFLOAT3 m_currentRotation;
	DirectX::XMFLOAT3 m_targetRotation;

	DirectX::XMFLOAT3 m_initialMoove;
	DirectX::XMFLOAT3 m_currentMoove;
	DirectX::XMFLOAT3 m_tergetMoove;

	float m_rotationSpeedX;
	float m_rotationSpeedY;
	float m_rotationSpeedZ;

	float m_moovementSpeed;*/
};
