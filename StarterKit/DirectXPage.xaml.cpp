// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage.xaml class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace MoonLander;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::UI;
using namespace Windows::UI::Input;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Globalization::NumberFormatting;

DirectXPage::DirectXPage()
{
	InitializeComponent();

	m_renderer = ref new Game();

	m_renderer->Initialize(
		Window::Current->CoreWindow,
		SwapChainPanel,
		DisplayProperties::LogicalDpi
		);

	Window::Current->CoreWindow->SizeChanged +=
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &DirectXPage::OnWindowSizeChanged);

	DisplayProperties::LogicalDpiChanged +=
		ref new DisplayPropertiesEventHandler(this, &DirectXPage::OnLogicalDpiChanged);

	DisplayProperties::OrientationChanged +=
		ref new DisplayPropertiesEventHandler(this, &DirectXPage::OnOrientationChanged);

	DisplayProperties::DisplayContentsInvalidated +=
		ref new DisplayPropertiesEventHandler(this, &DirectXPage::OnDisplayContentsInvalidated);

	m_eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &DirectXPage::OnRendering));

	m_timer = ref new BasicTimer();

	m_renderer->Pause(true);
}

void DirectXPage::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	m_renderer->UpdateForWindowSizeChange();
}

void DirectXPage::OnLogicalDpiChanged(Object^ sender)
{
	m_renderer->SetDpi(DisplayProperties::LogicalDpi);
}

void DirectXPage::OnOrientationChanged(Object^ sender)
{
	m_renderer->UpdateForWindowSizeChange();
}

void DirectXPage::OnDisplayContentsInvalidated(Object^ sender)
{
	m_renderer->ValidateDevice();
}

void DirectXPage::OnRendering(Object^ sender, Object^ args)
{
	m_timer->Update();
	m_renderer->Update(m_timer->Total, m_timer->Delta);
	m_renderer->Render();
	m_renderer->Present();
	if (m_renderer->GameFinished())
	{
		m_renderer->GameFinished(false);
		m_timer = ref new BasicTimer();
	}
	if (m_renderer->GameStarted())
	{
		m_renderer->GameStarted(false);
		m_timer = ref new BasicTimer();
	}
}

void DirectXPage::SaveInternalState(IPropertySet^ state)
{
}

void DirectXPage::LoadInternalState(IPropertySet^ state)
{
}

void DirectXPage::OnTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{
	switch (e->PointerDeviceType)
	{
	case Windows::Devices::Input::PointerDeviceType::Mouse:
		break;
	}
}

void DirectXPage::OnKeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	switch (e->Key)
	{
	case Windows::System::VirtualKey::W:
		m_renderer->RotateObject(ROTATE_UP);
		break;
	case Windows::System::VirtualKey::S:
		m_renderer->RotateObject(ROTATE_DOWN);
		break;
	case Windows::System::VirtualKey::A:
		m_renderer->RotateObject(ROTATE_LEFT);
		break;
	case Windows::System::VirtualKey::D:
		m_renderer->RotateObject(ROTATE_RIGHT);
		break;
	case Windows::System::VirtualKey::E:
		m_renderer->MooveObject(MOOVE_FORWARD);
		break;
	case Windows::System::VirtualKey::Q:
		m_renderer->MooveObject(MOOVE_BACKWARD);
		break;
	case Windows::System::VirtualKey::Escape:
		if (Windows::UI::Xaml::Visibility::Visible == this->MenuButtons->Visibility)
		{
			this->MenuButtons->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
			m_renderer->Pause(false);
		}
		else if (Windows::UI::Xaml::Visibility::Collapsed == this->MenuButtons->Visibility)
		{
			this->MenuButtons->Visibility = Windows::UI::Xaml::Visibility::Visible;
			m_renderer->Pause(true);
		}
		break;
	}
}

void MoonLander::DirectXPage::New_Game_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->MenuButtons->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	this->BtnNewGame->IsEnabled = false;
	this->BtnRestart->IsEnabled = true;
	this->BtnContinue->IsEnabled = true;
	m_renderer->GameStarted(true);
	m_renderer->Pause(false);
}


void MoonLander::DirectXPage::Exit_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->MenuButtons->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	m_renderer->GameStarted(false);
	m_renderer->Pause(false);
	Application::Current->Exit();
}


void MoonLander::DirectXPage::Restart_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->MenuButtons->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	m_renderer->GameStarted(true);
	m_renderer->Pause(false);
	m_renderer->RestartGame();
}


void MoonLander::DirectXPage::Continue_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->MenuButtons->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	m_renderer->GameStarted(true);
	m_renderer->Pause(false);
}
