#include <pch.h>
#include <3D\Objects\GUI\ButtonObject.h>

HoloTek::ButtonObject::ButtonObject(std::shared_ptr<DX::DeviceResources> devicesResources,
											winrt::Windows::Foundation::Numerics::float2 size,
											winrt::Windows::Foundation::Numerics::float4 backgroundColor)
	: ATextObject(devicesResources), m_size(size), m_color(backgroundColor)
{
	/*_backgroundMesh = std::make_unique<ColoredRectangle>(devicesResources, size, backgroundColor);*/
}

std::future<void> HoloTek::ButtonObject::CreateDeviceDependentResourcesAsync()
{
	m_textMesh = std::make_unique<TextObject>(m_deviceResources, m_size, m_color);
	co_await m_textMesh->CreateDeviceDependentResourcesAsync();
	//_backgroundMesh->CreateDeviceDependentResources();
	co_return;
}

void HoloTek::ButtonObject::ReleaseDeviceDependentResources()
{
	m_textMesh->ReleaseDeviceDependentResources();
	//_backgroundMesh->ReleaseDeviceDependentResources();
}

void HoloTek::ButtonObject::Render()
{
	m_textMesh->Render();
	//_backgroundMesh->Render();
}

void HoloTek::ButtonObject::SetModelTransform(DirectX::XMMATRIX const &modelTransform)
{
	//_backgroundMesh->ApplyMatrix(modelTransform);
	m_textMesh->SetModelTransform(modelTransform);
}

void HoloTek::ButtonObject::GetBoundingBox(DirectX::BoundingOrientedBox &boundingBox)
{
	return (m_textMesh->GetBoundingBox(boundingBox));
}

std::wstring const & HoloTek::ButtonObject::getText() const
{
	return (m_textMesh->getText());
}

void HoloTek::ButtonObject::setText(std::wstring const & text)
{
	m_textMesh->setText(text);
}

float const HoloTek::ButtonObject::getFontSize() const
{
	return m_textMesh->getFontSize();
}

void HoloTek::ButtonObject::setFontSize(float font)
{
	m_textMesh->setFontSize(font);
}

void HoloTek::ButtonObject::setColor(winrt::Windows::Foundation::Numerics::float4 color)
{
	m_color = color;
	m_textMesh->setColor(color);
}

winrt::Windows::Foundation::Numerics::float4 HoloTek::ButtonObject::getColor() const
{
	return m_color;
}
