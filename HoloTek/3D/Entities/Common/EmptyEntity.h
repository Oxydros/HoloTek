#pragma once

#include <3D\Entities\Common\Entity.h>

namespace HoloTek
{
	class EmptyEntity : public Entity
	{
	public:
		EmptyEntity(std::shared_ptr<HolographicScene> scene, std::string const &label,
			bool isRoot = false) : Entity(scene), m_label(label)
		{
			m_isRoot = isRoot;
		}

		~EmptyEntity() = default;
		// Inherited via Entity
		void DoUpdate(DX::StepTimer const & timer) override {};

		// Inherited via Entity
		std::string const GetLabel() const override { return m_label; }
	private:
		std::string m_label;
	};
}