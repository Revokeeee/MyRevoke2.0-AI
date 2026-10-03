#pragma once

extern Revoke::Application* Revoke::CreateApplication();

int main()
{
	Revoke::Application* application = Revoke::CreateApplication();
	application->Run();
	delete application;
}

