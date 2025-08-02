#include "Events.h"

void MessageHandler(SKSE::MessagingInterface::Message *a_msg)
{
	switch (a_msg->type)
	{
	case SKSE::MessagingInterface::kDataLoaded:
		Events_Space::animEventHandler::Register(true, false);
		Events_Space::Events::install();
		Events_Space::Install_apply();
		Events_Space::Settings::GetSingleton()->Load();
		break;

	case SKSE::MessagingInterface::kPostPostLoad:
		//Events_Space::Events::GetSingleton()->init();
		break;

	default:

		break;
	}
}

void Init()
{
	//Events_Space::Events::install_pluginListener();
	auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener("SKSE", MessageHandler);
}

void Load(){
	Events_Space::Events::install_protected();
	Events_Space::HitEventHandler::InstallHooks();
}