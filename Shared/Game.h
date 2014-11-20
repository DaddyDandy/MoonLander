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

#include "ShipRotationTypes.h"

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

	void RotateShip(int rotationType);

private:
    std::vector<VSD3DStarter::Mesh*> m_meshModels;

	bool m_isAnimationRunning;
	float m_animationTime;
	DirectX::XMFLOAT3 m_initialRotation;
	DirectX::XMFLOAT3 m_currentRotation;
	DirectX::XMFLOAT3 m_targetRotation;
	float m_currentTranslationY;

	RotationTypes m_rotationType;
};
