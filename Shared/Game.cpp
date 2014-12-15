// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "pch.h"
#include "Game.h"
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <algorithm>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace VSD3DStarter;

const float ROTATION_POWER = 0.01f;
const float MOOVE_POWER = 0.1f;
const float ANIMATION_DURATION = 0.2f;
const float GRAVITATION_ANIMATION_DURATION = 1.0f;
const float MOON_GA = 1.6f;

const float START_CAM_POS_X = 0.0f;
const float START_CAM_POS_Y = 0.5f;
const float START_CAM_POS_Z = -2.5f;

Game::Game()
{
	m_rotationSpeedX = 0.0f;
	m_rotationSpeedY = 0.0f;
	m_rotationSpeedZ = 0.0f;
	m_translationSpeed = 0.0f;

	m_isGameStarted = false;
	m_isPause = false;
	m_isMultiplayer = false;

	m_landingX = 25.0f;
	m_landingY = -50.0f;
	m_landingZ = 100.0f;
}

Game::~Game()
{
	for (Mesh* m : m_backModel)
	{
		delete m;
	}
	m_backModel.clear();

	for (Mesh* m : m_moonModel)
	{
		delete m;
	}
	m_moonModel.clear();

	for (Mesh* m : m_landingPointModel)
	{
		delete m;
	}
	m_landingPointModel.clear();

	for (Mesh* m : m_starShipModel)
	{
		delete m;
	}
	m_starShipModel.clear();
}

void Game::CreateWindowSizeDependentResources()
{
	GameBase::CreateWindowSizeDependentResources();

	float aspectRatio = m_windowBounds.Width / m_windowBounds.Height;

	// setup camera for our scene
	m_graphics.GetCamera().SetViewport((UINT)m_windowBounds.Width, (UINT)m_windowBounds.Height);
	m_graphics.GetCamera().SetPosition(XMFLOAT3(START_CAM_POS_X, START_CAM_POS_Y, START_CAM_POS_Z));
	m_graphics.GetCamera().SetLookAt(XMFLOAT3(0.0f, 0.0f, 0.0f));

	float fovAngleY = 70.0f * XM_PI / 180.0f;

	if (aspectRatio < 1.0f)
	{
		/// portrait or snap view
		m_graphics.GetCamera().SetUpVector(XMFLOAT3(1.0f, 0.0f, 0.0f));
		fovAngleY = 120.0f * XM_PI / 180.0f;
	}
	else
	{
		/// landscape view
		m_graphics.GetCamera().SetUpVector(XMFLOAT3(0.0f, 1.0f, 0.0f));
	}

	m_graphics.GetCamera().SetProjection(fovAngleY, aspectRatio, 1.0f, 1500.0f);

	// setup lighting for our scene
	XMFLOAT3 pos = XMFLOAT3(5.0f, 5.0f, -2.5f);
	XMVECTOR vPos = XMLoadFloat3(&pos);

	XMFLOAT3 dir;
	XMStoreFloat3(&dir, XMVector3Normalize(vPos));

	m_lightConstants.ActiveLights = 1;
	m_lightConstants.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_lightConstants.IsPointLight[0] = false;
	m_lightConstants.LightColor[0] = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_lightConstants.LightDirection[0].x = dir.x;
	m_lightConstants.LightDirection[0].y = dir.y;
	m_lightConstants.LightDirection[0].z = dir.z;
	m_lightConstants.LightDirection[0].w = 0;
	m_lightConstants.LightSpecularIntensity[0].x = 2;

	m_graphics.UpdateLightConstants(m_lightConstants);
}

void Game::Initialize()
{
	Mesh::LoadFromFile(m_graphics, L"StarShip.cmo", L"", L"", m_starShipModel);
	Mesh::LoadFromFile(m_graphics, L"TheMoon.cmo", L"", L"", m_moonModel);
	Mesh::LoadFromFile(m_graphics, L"LandingPoint.cmo", L"", L"", m_landingPointModel);
	Mesh::LoadFromFile(m_graphics, L"Back.cmo", L"", L"", m_backModel);
}

void Game::Clear()
{
	// clear
	m_d3dContext->OMSetRenderTargets(
		1,
		m_d3dRenderTargetView.GetAddressOf(),
		m_d3dDepthStencilView.Get()
		);

	m_d3dContext->ClearRenderTargetView(
		m_d3dRenderTargetView.Get(),
		DirectX::Colors::Black
		);

	m_d3dContext->ClearDepthStencilView(
		m_d3dDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH,
		1.0f,
		0
		);
}

String^ Game::OnHitObject(int x, int y)
{
	return "";
}

void Game::Update(float timeTotal, float timeDelta)
{
	if (!Pause())
	{
		m_animationTime += timeDelta;
		m_animationGravTime += timeDelta;
		float rotateAnimationProgress = std::min<float>(m_animationTime / ANIMATION_DURATION, 0.4f);

		XMVECTOR initial = XMLoadFloat3(&m_initialRotation);
		XMVECTOR target = XMLoadFloat3(&m_targetRotation);
		XMVECTOR current = initial + rotateAnimationProgress * (target - initial);
		XMStoreFloat3(&m_currentRotation, current);

		UseTranslation();
		UseGravitation();

		// reset camera
		m_graphics.GetCamera().SetPosition(XMFLOAT3(START_CAM_POS_X, START_CAM_POS_Y + m_gravitationTranslation, START_CAM_POS_Z + m_currentTranslation + m_currentTranslation));
		m_graphics.GetCamera().SetLookAt(XMFLOAT3(0.0f, m_currentTranslation - m_currentGT, m_currentTranslation));

		UpdateObjectTarget();
	}

}

void Game::UseTranslation()
{
	float animationProgres = std::min<float>(m_animationTime / ANIMATION_DURATION, 0.4f);
	float initialT = m_initialTranslation;
	float targetT = m_targetTranslation;
	float currentT = initialT + animationProgres * (targetT - initialT);
	m_currentTranslation = currentT;
}

void Game::UseGravitation()
{
	float fallAnimationProgress = std::min<float>(m_animationTime / ANIMATION_DURATION, 0.4f);
	float initial = m_initialGT;
	float target = m_targetGT;
	float current = initial + fallAnimationProgress * (target - initial);
	m_currentGT = current;
}

void Game::Render()
{
	GameBase::Render();
	Clear();	

	// render ship
	XMMATRIX transform = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_currentRotation));
	transform *= XMMatrixTranslation(0.0f, m_currentTranslation - m_currentGT, m_currentTranslation);
	for (UINT i = 0; i < m_starShipModel.size(); i++)
	{
		m_starShipModel[i]->Render(m_graphics, transform);
	}

	// render Moon
	transform = XMMatrixTranslation(0.0f, -250.0f, 0.0f);
	for (UINT i = 0; i < m_moonModel.size(); i++)
	{
		m_moonModel[i]->Render(m_graphics, transform);
	}

	// render Landing point
	transform = XMMatrixTranslation(m_landingX, m_landingY, m_landingZ);
	for (UINT i = 0; i < m_landingPointModel.size(); i++)
	{
		m_landingPointModel[i]->Render(m_graphics, transform);
	}

	// render Back model
	transform = XMMatrixTranslation(0.0f, -250.0f, m_currentTranslation);
	for (UINT i = 0; i < m_backModel.size(); i++)
	{
		m_backModel[i]->Render(m_graphics, transform);
	}

	// only enable MSAA if the device has enough power
	if (m_d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
	{
		// resolve multi-sample textures into single-sample textures
		UINT resourceIndex = D3D11CalcSubresource(0, 0, 1);
		m_d3dContext->ResolveSubresource(m_backBuffer.Get(), resourceIndex, m_backBufferMsaa.Get(), resourceIndex, DXGI_FORMAT_B8G8R8A8_UNORM);
	}

	if (!Pause())
	{
		
	}	
}

void Game::RotateObject(int rotationType)
{
	if (!Pause())
	{
		switch (rotationType)
		{
		case ROTATE_UP:
			m_rotationSpeedX += ROTATION_POWER;			
			if (m_targetRotation.z != 0.0f)
			{
				m_rotationSpeedX -= ROTATION_POWER;
				m_rotationSpeedY += (ROTATION_POWER * m_targetRotation.z) / XM_PIDIV2;
				m_rotationSpeedX = 0.0f;
			}
			break;
		case ROTATE_DOWN:
			m_rotationSpeedX -= ROTATION_POWER;			
			if (m_targetRotation.z != 0.0f)
			{
				m_rotationSpeedX += ROTATION_POWER;
				m_rotationSpeedY -= (ROTATION_POWER * m_targetRotation.z) / XM_PIDIV2;
				m_rotationSpeedX = 0.0f;
			}
			break;
		case ROTATE_RIGHT:
			m_rotationSpeedZ += ROTATION_POWER;						
			break;
		case ROTATE_LEFT:
			m_rotationSpeedZ -= ROTATION_POWER;			
			break;
		}				

		UpdateObjectTarget();
	}
}



void Game::MooveObject(int mooveType)
{
	if (!Pause())
	{
		switch (mooveType)
		{
		case MOOVE_FORWARD:
			m_translationSpeed += MOOVE_POWER;
			break;
		case MOOVE_BACKWARD:
			m_translationSpeed -= MOOVE_POWER;
			break;
		}

		UpdateObjectTarget();
	}
}

void Game::UpdateObjectTarget()
{
	m_targetRotation = XMFLOAT3(
		m_targetRotation.x + m_rotationSpeedX, 
		m_targetRotation.y + m_rotationSpeedY, 
		m_targetRotation.z + m_rotationSpeedZ
		);
	m_initialRotation = m_currentRotation;
	
	m_targetTranslation += m_translationSpeed;
	m_initialTranslation = m_currentTranslation;

	m_targetGT += 0.05f;
	m_initialGT = m_currentGT;

	m_animationGravTime = 0.0f;
	m_animationTime = 0.0f;
	m_isAnimationRunning = true;
}
