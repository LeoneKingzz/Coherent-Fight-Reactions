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

			auto Protagonist = event->actor->As<RE::Actor>();

			if (!Protagonist || Protagonist->IsPlayerRef())
			{
				return RE::BSEventNotifyControl::kContinue;
			}
			// if (Protagonist->IsPlayerTeammate() || (Protagonist->IsCommandedActor() && ((Protagonist->GetCommandingActor().get()->IsPlayerRef()) || (Protagonist->GetCommandingActor().get()->IsPlayerTeammate()))))
			// {
			// 	if (auto CombatTarget = Protagonist->GetActorRuntimeData().currentCombatTarget.get().get())
			// 	{
			// 		if (CombatTarget->IsPlayerTeammate() || (CombatTarget->IsCommandedActor() && ((CombatTarget->GetCommandingActor().get()->IsPlayerRef()) || (CombatTarget->GetCommandingActor().get()->IsPlayerTeammate()))))
			// 		{
			// 			if (const auto Evaluate = RE::TESForm::LookupByEditorID<RE::MagicItem>("CFRs_CalmSpell"))
			// 			{
			// 				const auto caster = Protagonist->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
			// 				const auto caster2 = CombatTarget->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
			// 				caster->CastSpellImmediate(Evaluate, true, Protagonist, 1, false, 100.0, Protagonist);
			// 				caster2->CastSpellImmediate(Evaluate, true, CombatTarget, 1, false, 100.0, CombatTarget);
			// 			}
			// 		}
			// 	}
			// }
			auto HdSingle = RE::TESDataHandler::GetSingleton();
			if (const auto CFRs_Enable = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x801, "Coherent Fight Reactions.esp")))
			{
				if (CFRs_Enable->value == 1.0f)
				{
					return RE::BSEventNotifyControl::kContinue;
				}

				switch (event->newState.get())
				{
				case RE::ACTOR_COMBAT_STATE::kCombat:
					if (auto CombatTarget = Protagonist->GetActorRuntimeData().currentCombatTarget.get().get())
					{
						if (CombatTarget->IsPlayerRef() || CombatTarget->IsPlayerTeammate())
						{
							if (CFRs_Enable->value != 1.0f)
							{
								CFRs_Enable->value = 1.0f;
								break;
							}
						}
					}
					if (const auto combatGroup = Protagonist->GetCombatGroup())
					{
						for (auto &targetData : combatGroup->targets)
						{
							auto target = targetData.targetHandle.get();
							if (target && target.get()->IsPlayerRef())
							{
								if (CFRs_Enable->value != 1.0f)
								{
									CFRs_Enable->value = 1.0f;
									break;
								}
							}
						}
					}
					break;
				case RE::ACTOR_COMBAT_STATE::kSearching:
					break;

				case RE::ACTOR_COMBAT_STATE::kNone:
					break;

					Events::GetSingleton()->RegisterforUpdate(Protagonist, std::make_tuple(true, std::chrono::steady_clock::now(), 8000ms, "NeutralFaction_Update"));

				default:
					break;
				}
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
			if (const auto enemyhandle = event->cause.get(); enemyhandle)
			{
				if (enemyhandle->Is(RE::FormType::ActorCharacter))
				{
					RE::Actor *a_actor = enemyhandle->As<RE::Actor>();
					if (a_actor->IsPlayerTeammate() || (a_actor->IsCommandedActor() && ((a_actor->GetCommandingActor().get()->IsPlayerRef()) || (a_actor->GetCommandingActor().get()->IsPlayerTeammate()))))
					{
						if (auto CombatTarget = a_actor->GetActorRuntimeData().currentCombatTarget.get().get())
						{
							if (CombatTarget->IsPlayerTeammate() || (CombatTarget->IsCommandedActor() && ((CombatTarget->GetCommandingActor().get()->IsPlayerRef()) || (CombatTarget->GetCommandingActor().get()->IsPlayerTeammate()))))
							{
								if (const auto Evaluate = RE::TESForm::LookupByEditorID<RE::MagicItem>("CFRs_CalmSpell"))
								{
									const auto caster = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
									const auto caster2 = CombatTarget->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
									caster->CastSpellImmediate(Evaluate, true, a_actor, 1, false, 100.0, a_actor);
									caster2->CastSpellImmediate(Evaluate, true, CombatTarget, 1, false, 100.0, CombatTarget);
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
		case "MLh_SpellFire_Event"_h:
		case "MRh_SpellFire_Event"_h:
		case "Voice_SpellFire_Event"_h:
		case "PowerAttack_Start_end"_h:
		case "NextAttackInitiate"_h:
		case "preHitFrame"_h:
		case "BowFullDrawn"_h:
			if (a_actor->IsPlayerRef())
			{
				if (const auto CFRs_Enable = skyrim_cast<RE::TESGlobal *>(H->LookupForm(0x801, "Coherent Fight Reactions.esp")))
				{
					if (a_actor->IsSneaking() && !a_actor->IsInCombat())
					{
						if (CFRs_Enable->value != 0.0f)
						{
							CFRs_Enable->value = 0.0f;
						}
					}
					else
					{
						if (CFRs_Enable->value != 1.0f)
						{
							CFRs_Enable->value = 1.0f;
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

	void HitEventHandler::PreProcessHit(RE::Actor *target, RE::HitData *hitData)
	{
		auto aggressor = hitData->aggressor ? hitData->aggressor.get().get() : nullptr;

		auto HdSingle = RE::TESDataHandler::GetSingleton();

		bool ignoredamage = false;

		const auto CurrentFollowerFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CurrentFollowerFaction");
		const auto CFRs_PlayerAlliesFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_PlayerAlliesFaction");
		const auto CFRs_PlayerFriendsFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_PlayerFriendsFaction");
		const auto CFRs_NPCNeutralsFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_NPCNeutralsFaction");

		if (const auto CFRs_Currentfollow_Glob = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x805, "Coherent Fight Reactions.esp")); CFRs_Currentfollow_Glob && CFRs_Currentfollow_Glob->value == 0.0f)
		{
			CurrentFollowerFaction->SetAlly(CurrentFollowerFaction);
			CFRs_Currentfollow_Glob->value == 1.0f;
		}

		if (aggressor && target)
		{

			if (GFunc_Space::GetFactionRelation(target, aggressor, 0.0f))
			{

				if (!target->IsHostileToActor(aggressor) && target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAggression) <= 1)
				{

					if (target->IsPlayerTeammate() || (target->IsCommandedActor() && ((target->GetCommandingActor().get()->IsPlayerRef()) || (target->GetCommandingActor().get()->IsPlayerTeammate()))))
					{
						if (CurrentFollowerFaction && !target->IsInFaction(CurrentFollowerFaction))
						{

							if (CFRs_PlayerAlliesFaction && !target->IsInFaction(CFRs_PlayerAlliesFaction))
							{

								target->AsActorValueOwner()->SetActorValue(RE::ActorValue::kAssistance, 2);

								target->AddToFaction(CFRs_PlayerAlliesFaction, 0);
							}
						}
					}

					if (aggressor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && aggressor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && aggressor->IsInFaction(CurrentFollowerFaction)) )
					{
						if (const auto CFRs_Enable = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x801, "Coherent Fight Reactions.esp")); CFRs_Enable)
						{
							if (CFRs_Enable->value == 1.0f)
							{

								if (CurrentFollowerFaction && CFRs_PlayerAlliesFaction && !target->IsInFaction(CFRs_PlayerAlliesFaction) && !target->IsInFaction(CurrentFollowerFaction) && !target->IsPlayerRef())
								{

									if (CFRs_PlayerFriendsFaction && !target->IsInFaction(CFRs_PlayerFriendsFaction))
									{
										target->AddToFaction(CFRs_PlayerFriendsFaction, 0);
									}
								}

								ignoredamage = true;
							}
							else
							{

								if (CFRs_PlayerFriendsFaction && target->IsInFaction(CFRs_PlayerFriendsFaction))
								{
									RemoveFromFaction(target, CFRs_PlayerFriendsFaction);
								}
							}
						}
					}
					else
					{
						if (CurrentFollowerFaction && CFRs_PlayerAlliesFaction &&  !target->IsPlayerRef() && !target->IsInFaction(CFRs_PlayerAlliesFaction) && !target->IsInFaction(CurrentFollowerFaction))
						{
							if (CFRs_NPCNeutralsFaction)
							{
								if (!target->IsInFaction(CFRs_NPCNeutralsFaction))
								{
									target->AddToFaction(CFRs_NPCNeutralsFaction, 0);
								}

								if (!aggressor->IsInFaction(CFRs_NPCNeutralsFaction))
								{
									aggressor->AddToFaction(CFRs_NPCNeutralsFaction, 0);
								}
							}
						}
					}

					if (ignoredamage)
					{
						auto fireKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicDamageFire");
						auto frostKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicDamageFrost");
						auto shockKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicDamageShock");
						auto shoutKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicShout");
						auto stormKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicVoiceChangeWeather");

						auto TrapPoisonKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicVoiceChangeWeather");
						auto TrapGasKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicVoiceChangeWeather");

						using AX = RE::EffectSetting::Archetype;

						if (auto indv_spell = hitData->attackDataSpell; indv_spell)
						{
							for (auto indv_effect : indv_spell->effects)
							{
								if (indv_effect && indv_effect->baseEffect)
								{
									auto Archy_X = indv_effect->baseEffect->data.archetype;
									auto hasHostileflag = indv_effect->baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kHostile);
									auto Kw_ScriptHostile = clib_util::editorID::get_editorID(indv_effect->baseEffect).contains("FrostSlowFFContact");

									auto Kw_magicfire = indv_effect->baseEffect->HasKeyword(fireKeyword);
									auto Kw_magicfrost = indv_effect->baseEffect->HasKeyword(frostKeyword);
									auto Kw_magicshock = indv_effect->baseEffect->HasKeyword(shockKeyword);
									auto Kw_magicshout = indv_effect->baseEffect->HasKeyword(shoutKeyword);
									auto Kw_Exclude = indv_effect->baseEffect->HasKeyword(TrapGasKeyword) || indv_effect->baseEffect->HasKeyword(TrapPoisonKeyword) || indv_effect->baseEffect->HasKeyword(stormKeyword);
									auto Kw_Storm = indv_effect->baseEffect->HasKeyword(stormKeyword);

									if ((Kw_ScriptHostile && Archy_X == AX::kScript) || (Kw_Storm && Archy_X == AX::kStagger) || (Kw_magicshout && Archy_X == AX::kStagger) || (!Kw_Exclude && (hasHostileflag || Kw_magicfire || Kw_magicfrost || Kw_magicshock) && (Archy_X == AX::kDualValueModifier || Archy_X == AX::kValueModifier || Archy_X == AX::kPeakValueModifier || Archy_X == AX::kParalysis || Archy_X == AX::kDemoralize || Archy_X == AX::kFrenzy || Archy_X == AX::kDisarm || Archy_X == AX::kAbsorb || Archy_X == AX::kStagger)))
									{
										RE::BSTArray<RE::Effect*>::iterator position = std::find(hitData->attackDataSpell->effects.begin(), hitData->attackDataSpell->effects.end(), indv_effect);
										if (position != hitData->attackDataSpell->effects.end())
										{
											auto i = std::distance(hitData->attackDataSpell->effects.begin(), position);
											hitData->attackDataSpell->effects[i] = nullptr;
										}
									}
								}
							}
						}

						if (hitData->totalDamage)
						{
							hitData->totalDamage = 0.0f;
						}

						if (hitData->pushBack)
						{
							hitData->pushBack = 0.0f;
						}

						hitData->stagger = static_cast<uint32_t>(0.00);

						if (hitData->flags && !hitData->flags.any(RE::HitData::Flag::kIgnoreCritical))
						{
							hitData->flags |= RE::HitData::Flag::kIgnoreCritical;
						}
					}
				}
			}
		}
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
		//eventSourceHolder->AddEventSink<RE::TESSpellCastEvent>(eventSink);
		//eventSourceHolder->AddEventSink<RE::TESDeathEvent>(eventSink);
		//eventSourceHolder->AddEventSink<RE::TESHitEvent>(eventSink);
		//eventSourceHolder->AddEventSink<RE::TESMagicEffectApplyEvent>(eventSink);
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

			GetSingleton()->Process_Updates(a_actor, std::chrono::steady_clock::now());
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
								case "NeutralFaction_Update"_h:
									if (auto CFRs_NPCNeutralsFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_NPCNeutralsFaction"); CFRs_NPCNeutralsFaction && a_actor->IsInFaction(CFRs_NPCNeutralsFaction))
									{
										HitEventHandler::RemoveFromFaction(a_actor, CFRs_NPCNeutralsFaction);
									}

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