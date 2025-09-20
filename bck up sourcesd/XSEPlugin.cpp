#include "Events.h"

void MessageHandler(SKSE::MessagingInterface::Message *a_msg)
{
	switch (a_msg->type)
	{
	case SKSE::MessagingInterface::kDataLoaded:
		Events_Space::animEventHandler::Register(true, true);
		Events_Space::Events::install();
		
		//Events_Space::Install_apply();

		Events_Space::Settings::GetSingleton()->Load();
		break;

	case SKSE::MessagingInterface::kPostPostLoad:
		// Events_Space::Events::GetSingleton()->init();
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
	// Events_Space::Events::install_protected();
	Events_Space::HitEventHandler::InstallHooks();
}

void PreLoad(){
	Events_Space::MagicApplyHandler::Register(true, true);
	// Events_Space::CastingHandler::Register(true, false);

	if (Events_Space::ExplosionCollision::registery != nullptr)
	{
		// if the thread is there, then destroy and delete it
		// if it is joinable and not running it has already finished, but needs to be joined before
		// it can be destroyed savely
		Events_Space::ExplosionCollision::registery->~thread();
		delete Events_Space::ExplosionCollision::registery;
		Events_Space::ExplosionCollision::registery = nullptr;
	}
	Events_Space::ExplosionCollision::registery = new std::thread(Events_Space::ExplosionCollision::Register);
	Events_Space::ExplosionCollision::registery->detach();
}