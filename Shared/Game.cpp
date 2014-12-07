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

const float m_rotationPower = 0.01f;
const float m_moovePower = 0.5f;

Game::Game()
{
	m_rotationSpeedX = 0.0f;
	m_rotationSpeedY = 0.0f;
	m_moovementSpeed = 0.0f;
}

Game::~Game()
{
	for (Mesh* m : m_meshModels)
	{
		delete m;
	}
	m_meshModels.clear();
}

void Game::CreateWindowSizeDependentResources()
{
	GameBase::CreateWindowSizeDependentResources();

	float aspectRatio = m_windowBounds.Width / m_windowBounds.Height;

	// setup camera for our scene
	m_graphics.GetCamera().SetViewport((UINT)m_windowBounds.Width, (UINT)m_windowBounds.Height);
	m_graphics.GetCamera().SetPosition(XMFLOAT3(0.0f, 12.0f, -22.0f));
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

	m_graphics.GetCamera().SetProjection(fovAngleY, aspectRatio, 1.0f, 1000.0f);

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
}

void Game::Update(float timeTotal, float timeDelta)
{
	m_animationTime += timeDelta;
	static const float animationDuration = 0.2f;
	float rotateAnimationProgress = std::min<float>(m_animationTime / animationDuration, 0.4f);

	XMVECTOR initial = XMLoadFloat3(&m_initialRotation);
	XMVECTOR target = XMLoadFloat3(&m_targetRotation);
	XMVECTOR current = initial + rotateAnimationProgress * (target - initial);

	XMStoreFloat3(&m_currentRotation, current);
}

void Game::Render()
{
	GameBase::Render();

	// clear
	m_d3dContext->OMSetRenderTargets(
		1,
		m_d3dRenderTargetView.GetAddressOf(),
		m_d3dDepthStencilView.Get()
		);

	m_d3dContext->ClearRenderTargetView(
		m_d3dRenderTargetView.Get(),
		DirectX::Colors::DarkSlateGray
		);

	m_d3dContext->ClearDepthStencilView(
		m_d3dDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH,
		1.0f,
		0
		);

	XMMATRIX transform = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_currentRotation));
	transform *= XMMatrixTranslation(0, 0, m_moovementSpeed);
	m_starShipModel[0]->Render(m_graphics, transform);

	transform = XMMatrixIdentity();
	for (UINT i = 0; i < m_moonModel.size(); i++)
	{
		m_moonModel[i]->Render(m_graphics, transform);
	}

	// only enable MSAA if the device has enough power
	if (m_d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
	{
		// resolve multi-sample textures into single-sample textures
		UINT resourceIndex = D3D11CalcSubresource(0, 0, 1);
		m_d3dContext->ResolveSubresource(m_backBuffer.Get(), resourceIndex, m_backBufferMsaa.Get(), resourceIndex, DXGI_FORMAT_B8G8R8A8_UNORM);
	}

	UpdateObjectTarget();
}

String^ Game::OnHitObject(int x, int y)
{
	String^ result = nullptr;

	XMFLOAT3 point;
	XMFLOAT3 dir;
	m_graphics.GetCamera().GetWorldLine(x, y, &point, &dir);

	XMFLOAT4X4 world;
	XMMATRIX worldMat = XMMatrixIdentity();
	XMStoreFloat4x4(&world, worldMat);

	float closestT = FLT_MAX;
	for (Mesh* m : m_meshModels)
	{
		XMFLOAT4X4 meshTransform = world;

		auto name = ref new String(m->Name());

		float t = 0;
		bool hit = this->LineHitTest(m, &point, &dir, &meshTransform, &t);
		if (hit && t < closestT)
		{
			result = name;
		}
	}

	return result;
}

void Game::RotateObject(int rotationType)
{
	switch (rotationType)
	{
	case ROTATE_UP:
		m_rotationSpeedX -= m_rotationPower;
		break;
	case ROTATE_DOWN:
		m_rotationSpeedX += m_rotationPower;
		break;
	case ROTATE_RIGHT:
		m_rotationSpeedY -= m_rotationPower;
		break;
	case ROTATE_LEFT:
		m_rotationSpeedY += m_rotationPower;
		break;
	}

	UpdateObjectTarget();
}



void Game::MooveObject(int mooveType)
{
	switch (mooveType)
	{
	case MOOVE_FORWARD:

		break;
	case MOOVE_BACKWARD:

		break;
	}
}

void Game::UpdateObjectTarget()
{
	m_targetRotation = XMFLOAT3(m_targetRotation.x + m_rotationSpeedX, m_targetRotation.y + m_rotationSpeedY, 0.0f);

	m_initialRotation = m_currentRotation;
	m_animationTime = 0.0f;
	m_isAnimationRunning = true;
}


