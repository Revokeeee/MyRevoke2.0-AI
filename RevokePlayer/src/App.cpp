#include <MyRevoke.h>
#include "MyRevoke/Core/EntryPoint.h"

#include "PlayerLayer.h"

namespace Revoke {

	class RevokePlayer : public Revoke::Application
	{
	public:
		RevokePlayer()
			: Application(WindowSettings("MyRevoke", 1280, 720, false))
		{
			PushLayer(new PlayerLayer());
		}
	};

	Revoke::Application* Revoke::CreateApplication()
	{
		return new RevokePlayer();
	}
};
