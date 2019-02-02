#pragma once

#include <3D\Entities\Common\InteractableEntity.h>

using namespace winrt::Windows::Foundation::Numerics;

namespace HoloTek
{

	class GUIEntity : public InteractableEntity
	{
	protected:
		float3				m_size;
		float4				m_color;
		std::wstring		m_label;

	public:
		GUIEntity(std::shared_ptr<HolographicScene> scene,
			Spatial::SpatialGestureSettings settings = Spatial::SpatialGestureSettings::None)
			: InteractableEntity(scene, settings) {}
		virtual ~GUIEntity() = default;

	public:
		/// <summary>	Executes the label changed action. </summary>
		virtual void OnLabelChanged() {}
		
		bool OnGetFocus() override { return false; }
		bool OnLostFocus() override { return false; }

	public:

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Sets a size. </summary>
		///
		/// <param name="size">	The size. </param>
		///-------------------------------------------------------------------------------------------------
		void setSize(float3 size) { m_size = size; }

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Sets a color. </summary>
		///
		/// <param name="color">	The color. </param>
		///-------------------------------------------------------------------------------------------------
		void setColor(float4 color) { m_color = color; }

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Sets a label. </summary>
		///
		/// <param name="label">	The label. </param>
		///-------------------------------------------------------------------------------------------------
		void setLabel(std::wstring const &label) {
			m_label = label;
			OnLabelChanged();
		}

	public:

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Gets the size. </summary>
		///
		/// <returns>	The size. </returns>
		///-------------------------------------------------------------------------------------------------
		float3 getSize() const { return m_size; }

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Gets the color. </summary>
		///
		/// <returns>	The color. </returns>
		///-------------------------------------------------------------------------------------------------
		float4 getColor() const { return m_color; }

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Gets the label. </summary>
		///
		/// <returns>	The label. </returns>
		///-------------------------------------------------------------------------------------------------
		std::wstring const &getLabel() const { return m_label; }
	};
}