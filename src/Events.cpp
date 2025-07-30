#include "Events.h"

namespace Events_Space
{
	class OurEventSink :
		public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent>,
		public RE::BSTEventSink<RE::TESEquipEvent>,
		public RE::BSTEventSink<RE::TESCombatEvent>,
		public RE::BSTEventSink<RE::TESActorLocationChangeEvent>,
		public RE::BSTEventSink<RE::TESSpellCastEvent>,
		public RE::BSTEventSink<RE::TESDeathEvent>,
		public RE::BSTEventSink<SKSE::ModCallbackEvent>,
        public RE::BSTEventSink<RE::TESHitEvent>,
		public RE::BSTEventSink<RE::TESMagicEffectApplyEvent>
	{
		OurEventSink() = default;
		OurEventSink(const OurEventSink&) = delete;
		OurEventSink(OurEventSink&&) = delete;
		OurEventSink& operator=(const OurEventSink&) = delete;
		OurEventSink& operator=(OurEventSink&&) = delete;

	public:
		static OurEventSink* GetSingleton()
		{
			static OurEventSink singleton;
			return &singleton;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*)
		{
			auto a_actor = event->subject->As<RE::Actor>();

			if (!a_actor) {
				return RE::BSEventNotifyControl::kContinue;
			}

			return RE::BSEventNotifyControl::kContinue;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent *event, RE::BSTEventSource<RE::TESDeathEvent> *)
		{
			auto a_actor = event->actorDying->As<RE::Actor>();

			if (!a_actor)
			{
				return RE::BSEventNotifyControl::kContinue;
			}

			return RE::BSEventNotifyControl::kContinue;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*){
			auto a_actor = event->actor->As<RE::Actor>();

			if (!a_actor) {
				return RE::BSEventNotifyControl::kContinue;
			}

			if (a_actor->IsPlayerRef())
			{
				return RE::BSEventNotifyControl::kContinue;
			}

			
			auto item = event->originalRefr;
			if (item && event->equipped)
			{
				auto form = RE::TESForm::LookupByID<RE::TESForm>(item);
				if (form && form->Is(RE::FormType::Spell)){
					auto a_spell = form->As<RE::SpellItem>();
					if (a_spell)
					{
					}
				}
			}

			return RE::BSEventNotifyControl::kContinue;
		}

		RE::BSEventNotifyControl ProcessEvent(const SKSE::ModCallbackEvent* event, RE::BSTEventSource<SKSE::ModCallbackEvent>*)
		{
			auto Ename = event->eventName;

			if (Ename != "KID_KeywordDistributionDone")
			{
				return RE::BSEventNotifyControl::kContinue;
			}

			//logger::info("recieved KID finished event"sv);

			//Settings::GetSingleton()->Load();

			return RE::BSEventNotifyControl::kContinue;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* event, RE::BSTEventSource<RE::TESCombatEvent>*){
			auto a_actor = event->actor->As<RE::Actor>();

			if (!a_actor || !a_actor->Is(RE::FormType::ActorCharacter)) {
				return RE::BSEventNotifyControl::kContinue;
			}

			switch (event->newState.get()) {
			case RE::ACTOR_COMBAT_STATE::kCombat:
				a_actor->SetGraphVariableBool("bNUB_IsinCombat", true);
				
				break;
			case RE::ACTOR_COMBAT_STATE::kSearching:
				a_actor->SetGraphVariableBool("bNUB_IsinCombat", false);
				break;

			case RE::ACTOR_COMBAT_STATE::kNone:
				a_actor->SetGraphVariableBool("bNUB_IsinCombat", false);
				
				break;

			default:
				break;
			}

			return RE::BSEventNotifyControl::kContinue;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESActorLocationChangeEvent* event, RE::BSTEventSource<RE::TESActorLocationChangeEvent>*)
		{
			auto a_actor = event->actor->As<RE::Actor>();

			if (!a_actor || !a_actor->IsPlayerRef()) {
				return RE::BSEventNotifyControl::kContinue;
			}


			return RE::BSEventNotifyControl::kContinue;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent *event, RE::BSTEventSource<RE::TESHitEvent> *)
		{
			auto a_actor = event->target->As<RE::Actor>();

			if (!a_actor)
			{
				return RE::BSEventNotifyControl::kContinue;
			}

			if (a_actor->HasKeywordString("ActorTypeNPC"))
			{
				if (!GFunc_Space::GFunc::GetBoolVariable(a_actor, "bNUB_IsinCombat"))
				{
					return RE::BSEventNotifyControl::kContinue;
				}

				if (const auto enemyhandle = event->cause.get(); enemyhandle)
				{
					if (enemyhandle->Is(RE::FormType::ActorCharacter))
					{
						RE::Actor *enemy = enemyhandle->As<RE::Actor>();
						if (enemy->IsHostileToActor(a_actor))
						{
							
						}
					}
				}
			}else{
				if (const auto enemyhandle = event->cause.get(); enemyhandle)
				{
					if (enemyhandle->Is(RE::FormType::ActorCharacter))
					{
						RE::Actor* enemy = enemyhandle->As<RE::Actor>();
						if (GFunc_Space::GFunc::GetBoolVariable(enemy, "bNUB_IsinCombat") && enemy->IsHostileToActor(a_actor))
						{
							if (auto form = RE::TESForm::LookupByID<RE::TESForm>(event->source))
							{
								if (form)
								{
									switch (form->GetFormType())
									{
									case RE::FormType::Spell:
										if (auto a_spell = form->As<RE::SpellItem>())
										{
											std::string Lsht = (clib_util::editorID::get_editorID(a_spell));
											switch (hash(Lsht.c_str(), Lsht.size()))
											{
											case "Null"_h:
												
												break;

											default:
												break;
											}
										}
										break;

									default:
										break;
									}
								}
							}
						}
					}
				}
			}

			return RE::BSEventNotifyControl::kContinue;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESMagicEffectApplyEvent *event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent> *)
		{
			auto a_actor = event->target->As<RE::Actor>();

			if (!a_actor)
			{
				return RE::BSEventNotifyControl::kContinue;
			}

			if (const auto enemyhandle = event->caster.get(); enemyhandle)
			{
				if (enemyhandle->Is(RE::FormType::ActorCharacter))
				{
					RE::Actor *enemy = enemyhandle->As<RE::Actor>();
					if (GFunc_Space::GFunc::GetBoolVariable(enemy, "bNUB_IsinCombat"))
					{
						if (auto form = RE::TESForm::LookupByID<RE::TESForm>(event->magicEffect))
						{
							switch (form->GetFormType())
							{
							case RE::FormType::MagicEffect:
								if (auto a_effect = form->As<RE::EffectSetting>())
								{
									std::string Lsht = (clib_util::editorID::get_editorID(a_effect));
									switch (hash(Lsht.c_str(), Lsht.size()))
									{
									case "Null"_h:
										
										break;

									default:
										break;
									}
								}
								break;

							default:
								break;
							}
						}
					}
				}
			}

			return RE::BSEventNotifyControl::kContinue;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESSpellCastEvent* event, RE::BSTEventSource<RE::TESSpellCastEvent>*)
		{
			auto a_actor = event->object->As<RE::Actor>();

			if (!a_actor) {
				return RE::BSEventNotifyControl::kContinue;
			}

			if (a_actor->IsPlayerRef()) {
				return RE::BSEventNotifyControl::kContinue;
			}

			auto H = RE::TESDataHandler::GetSingleton();
			const auto caster = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);

			if (auto form = RE::TESForm::LookupByID(event->spell)){

				switch (form->GetFormType())
				{
				case RE::FormType::Spell:
					if (auto a_spell = form->As<RE::SpellItem>())
					{
						std::string Lsht = (clib_util::editorID::get_editorID(a_spell));
						switch (hash(Lsht.c_str(), Lsht.size()))
						{
						case "Null"_h:
							
							break;

						default:
							break;
						}
					}
					break;

				default:
					break;
				}
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	RE::BSEventNotifyControl animEventHandler::HookedProcessEvent(RE::BSAnimationGraphEvent& a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* src)
	{
		FnProcessEvent fn = fnHash.at(*(uint64_t*)this);

		if (!a_event.holder) {
			return fn ? (this->*fn)(a_event, src) : RE::BSEventNotifyControl::kContinue;
		}
		auto H = RE::TESDataHandler::GetSingleton();
		RE::Actor* a_actor = const_cast<RE::TESObjectREFR*>(a_event.holder)->As<RE::Actor>();
		switch (hash(a_event.tag.c_str(), a_event.tag.size())) {
		case "AnimObjLoad"_h:
			if (a_actor->Is3DLoaded() && a_actor->GetParentCell() && a_actor->GetParentCell()->cellState == RE::TESObjectCELL::CellState::kAttached && a_actor->IsInCombat())
			{
				if (a_event.payload == "AnimObjectDrinkPotion")
				{
					if (const auto potionSafety = RE::TESForm::LookupByEditorID<RE::MagicItem>("UAPNG_Potion_Safety"); potionSafety)
					{
						if (const auto caster = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant); caster)
						{
							caster->CastSpellImmediate(potionSafety, true, a_actor, 1, false, 999.0, a_actor);
						}
					}
				}
				else if (GFunc_Space::GFunc::GetBoolVariable(a_actor, "bEasState"))
				{
					if (const auto eatingSafety = RE::TESForm::LookupByEditorID<RE::MagicItem>("UAPNG_Eating_Safety"); eatingSafety)
					{
						if (const auto caster = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant); caster)
						{
							caster->CastSpellImmediate(eatingSafety, true, a_actor, 1, false, 999.0, a_actor);
						}
					}
				}
			}
			
			break;

		default:
			break;
		}

		return fn ? (this->*fn)(a_event, src) : RE::BSEventNotifyControl::kContinue;
	}

	std::unordered_map<uint64_t, animEventHandler::FnProcessEvent> animEventHandler::fnHash;

	void Events::install(){

		auto eventSink = OurEventSink::GetSingleton();

		// ScriptSource
		auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
		// eventSourceHolder->AddEventSink<RE::TESSwitchRaceCompleteEvent>(eventSink);
		//eventSourceHolder->AddEventSink<RE::TESEquipEvent>(eventSink);
		eventSourceHolder->AddEventSink<RE::TESCombatEvent>(eventSink);
		//eventSourceHolder->AddEventSink<RE::TESActorLocationChangeEvent>(eventSink);
		eventSourceHolder->AddEventSink<RE::TESSpellCastEvent>(eventSink);
		eventSourceHolder->AddEventSink<RE::TESDeathEvent>(eventSink);
		eventSourceHolder->AddEventSink<RE::TESHitEvent>(eventSink);
		eventSourceHolder->AddEventSink<RE::TESMagicEffectApplyEvent>(eventSink);
	}

	void Events::install_pluginListener(){
		auto eventSink = OurEventSink::GetSingleton();
		SKSE::GetModCallbackEventSource()->AddEventSink(eventSink);
	}

	

	bool Events::BindPapyrusFunctions(VM* vm)
	{
		//vm->RegisterFunction("XXXX", "XXXXX", XXXX);
		return true;
	}

	void Events::RegisterforUpdate(RE::Actor *a_actor, std::tuple<bool, GFunc_Space::Time::time_point, GFunc_Space::ms, std::string> data)
	{
		auto itt = _Timer.find(a_actor);
		if (itt == _Timer.end())
		{
			std::vector<std::tuple<bool, GFunc_Space::Time::time_point, GFunc_Space::ms, std::string>> Hen;
			Hen.push_back(data);
			_Timer.insert({a_actor, Hen});
		}
		else
		{
			itt->second.push_back(data);
		}
	}

	void Events::Update(RE::Actor* a_actor, [[maybe_unused]] float a_delta)
	{
		if (a_actor->GetActorRuntimeData().currentProcess && a_actor->GetActorRuntimeData().currentProcess->InHighProcess() && a_actor->Is3DLoaded()){
			if (GFunc_Space::GFunc::GetBoolVariable(a_actor, "bNUB_IsinCombat"))
			{
				GetSingleton()->Process_Updates(a_actor, std::chrono::steady_clock::now());

				//GFunc_Space::GFunc::Call_Papyrus_Function(a_actor, "dragonActorSCRIPT", "LDP_PlayImpactEffect", RE::MakeFunctionArguments(std::bit_cast<RE::BGSImpactDataSet *>(FXDragonTakeOffImpactSet), std::bit_cast<const char *>(NPCPelvis)));

				// std::tuple<bool, std::chrono::steady_clock::time_point, GFunc_Space::ms, std::string> data;
				// GFunc_Space::GFunc::set_tupledata(data, true, std::chrono::steady_clock::now(), 100ms, "BusyState_Update");
				// GFunc_Space::GFunc::GetSingleton()->RegisterforUpdate(a_actor, data);

				// int a = 100;
				// std::jthread waitThread([&a]() { GFunc_Space::GFunc::GetSingleton()->wait(a); });
				// GetSingleton()->RegisterforUpdate(a_actor, std::make_tuple(true, std::chrono::steady_clock::now(), 100ms, "BusyState_Update")
			}
		}
	}

	void Events::Process_Updates(RE::Actor *a_actor, GFunc_Space::Time::time_point time_now)
	{
		for (auto it = _Timer.begin(); it != _Timer.end(); ++it)
		{
			if (it->first == a_actor)
			{
				if (!it->second.empty())
				{
					for (auto data : it->second)
					{
						bool update;
						std::chrono::steady_clock::time_point time_initial;
						std::chrono::milliseconds time_required;
						std::string function;
						std::tie(update, time_initial, time_required, function) = data;
						if (update)
						{
							if (duration_cast<std::chrono::milliseconds>(time_now - time_initial).count() >= time_required.count())
							{
								auto H = RE::TESDataHandler::GetSingleton();
								switch (hash(function.c_str(), function.size()))
								{
								case "Decoy_Update"_h:
									
									break;

								default:
									break;
								}
								std::vector<std::tuple<bool, GFunc_Space::Time::time_point, GFunc_Space::ms, std::string>>::iterator position = std::find(it->second.begin(), it->second.end(), data);
								if (position != it->second.end())
								{
									it->second.erase(position);
								}
							}
						}
					}
				}
				else
				{
					_Timer.erase(it);
				}
				break;
			}
		}
	}

	void Events::init()
	{
		_precision_API = reinterpret_cast<PRECISION_API::IVPrecision1*>(PRECISION_API::RequestPluginAPI());
		if (_precision_API) {
			_precision_API->AddPostHitCallback(SKSE::GetPluginHandle(), PrecisionWeaponsCallback_Post);
			logger::info("Enabled compatibility with Precision");
		}
	}

	void Events::PrecisionWeaponsCallback_Post(const PRECISION_API::PrecisionHitData& a_precisionHitData, const RE::HitData& a_hitdata)
	{
		if (!a_precisionHitData.target || !a_precisionHitData.target->Is(RE::FormType::ActorCharacter)) {
			return;
		}
		return;
	}




	void Settings::Load(){
		constexpr auto path = "Data\\SKSE\\Plugins\\NPCSpellVariance.ini";

		CSimpleIniA ini;
		ini.SetUnicode();

		ini.LoadFile(path);

		general.Load(ini);

		include_spells_mods.Load(ini);
		include_spells_keywords.Load(ini);
		exclude_spells_mods.Load(ini);
		exclude_spells_keywords.Load(ini);

		ini.SaveFile(path);
	}

	void Settings::General_Settings::Load(CSimpleIniA &a_ini)
	{
		static const char *section = "General_Settings";

		auto DS = GetSingleton();

		DS->general.bWhiteListApproach = a_ini.GetBoolValue(section, "bWhiteListApproach", DS->general.bWhiteListApproach);

		a_ini.SetBoolValue(section, "bWhiteListApproach", DS->general.bWhiteListApproach, ";If set to true, only the include mods and keywords are considered. Else only the exclude approach is used");
		//
	}

	void Settings::Include_AllSpells_withKeywords::Load(CSimpleIniA &a_ini)
	{
		static const char *section = "Include_AllSpells_withKeywords";

		auto DS = GetSingleton();

		DS->include_spells_keywords.inc_keywords_joined = a_ini.GetValue(section, "inc_keywords", DS->include_spells_keywords.inc_keywords_joined.c_str());

		std::istringstream f(DS->include_spells_keywords.inc_keywords_joined);
		std::string s;
		while (getline(f, s, '|'))
		{
			DS->include_spells_keywords.inc_keywords.push_back(s);
		}

		a_ini.SetValue(section, "inc_keywords", DS->include_spells_keywords.inc_keywords_joined.c_str(), ";Enter keywords for which all associated spells are included. Seperate keywords with | ");
	}

	void Settings::Include_AllSpells_inMods::Load(CSimpleIniA& a_ini){
		static const char* section = "Include_AllSpells_inMods";

		auto DS = GetSingleton();

		DS->include_spells_mods.inc_mods_joined = a_ini.GetValue(section, "inc_mods", DS->include_spells_mods.inc_mods_joined.c_str());

		std::istringstream f(DS->include_spells_mods.inc_mods_joined);
		std::string  s;
		while (getline(f, s, '|')) {
			DS->include_spells_mods.inc_mods.push_back(s);
		}

		a_ini.SetValue(section, "inc_mods", DS->include_spells_mods.inc_mods_joined.c_str(), ";Enter Mod Names of which all spells within are included. Seperate names with | ");
		//
	}

	void Settings::Exclude_AllSpells_withKeywords::Load(CSimpleIniA& a_ini)
	{
		static const char* section = "Exclude_AllSpells_withKeywords";

		auto DS = GetSingleton();

		DS->exclude_spells_keywords.exc_keywords_joined = a_ini.GetValue(section, "exc_keywords", DS->exclude_spells_keywords.exc_keywords_joined.c_str());

		std::istringstream f(DS->exclude_spells_keywords.exc_keywords_joined);
		std::string  s;
		while (getline(f, s, '|')) {
			DS->exclude_spells_keywords.exc_keywords.push_back(s);
		}

		a_ini.SetValue(section, "exc_keywords", DS->exclude_spells_keywords.exc_keywords_joined.c_str(), ";Enter keywords for which all associated spells are excluded. Seperate keywords with | ");
	}

	void Settings::Exclude_AllSpells_inMods::Load(CSimpleIniA &a_ini)
	{
		static const char *section = "Exclude_AllSpells_inMods";

		auto DS = GetSingleton();

		DS->exclude_spells_mods.exc_mods_joined = a_ini.GetValue(section, "exc_mods", DS->exclude_spells_mods.exc_mods_joined.c_str());

		std::istringstream f(DS->exclude_spells_mods.exc_mods_joined);
		std::string s;
		while (getline(f, s, '|'))
		{
			DS->exclude_spells_mods.exc_mods.push_back(s);
		}

		a_ini.SetValue(section, "exc_mods", DS->exclude_spells_mods.exc_mods_joined.c_str(), ";Enter Mod Names of which all spells within are excluded. Seperate names with | ");
		//
	}

	
}