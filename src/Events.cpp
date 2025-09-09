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
			
			auto HdSingle = RE::TESDataHandler::GetSingleton();
			if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
			{
				if (CFRs_FriendlyFire_Off->value == 1.0f)
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
							if (Settings::GetSingleton()->general.bDynamicToggle && CFRs_FriendlyFire_Off->value != 1.0f)
							{
								CFRs_FriendlyFire_Off->value = 1.0f;
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
								if (Settings::GetSingleton()->general.bDynamicToggle && CFRs_FriendlyFire_Off->value != 1.0f)
								{
									CFRs_FriendlyFire_Off->value = 1.0f;
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
				if (const auto targethandle = event->target.get(); targethandle)
				{
					if (enemyhandle->Is(RE::FormType::ActorCharacter) && targethandle->Is(RE::FormType::ActorCharacter))
					{
						RE::Actor *enemy = enemyhandle->As<RE::Actor>();
						RE::Actor *target = targethandle->As<RE::Actor>();
						const auto CurrentFollowerFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CurrentFollowerFaction");
						const auto CFRs_PlayerAlliesFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_PlayerAlliesFaction");

						if (enemy && target && CFRs_PlayerAlliesFaction && CurrentFollowerFaction)
						{
							if ((enemy->IsInFaction(CFRs_PlayerAlliesFaction) || enemy->IsInFaction(CurrentFollowerFaction) || enemy->IsPlayerRef()) && (target->IsInFaction(CFRs_PlayerAlliesFaction) || target->IsInFaction(CurrentFollowerFaction) || target->IsPlayerRef()))
							{
								if (Events::GetFactionReaction(target, enemy) != RE::FIGHT_REACTION::kAlly)
								{
									if (const auto CFRs_CalmSpell = RE::TESForm::LookupByEditorID<RE::MagicItem>("CFRs_CalmSpell"); CFRs_CalmSpell)
									{
										if (!enemy->IsPlayerRef() && Actor_GetCombatState(enemy) != RE::ACTOR_COMBAT_STATE::kCombat)
										{
											const auto caster_enemy = enemy->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
											caster_enemy->CastSpellImmediate(CFRs_CalmSpell, true, enemy, 1, false, 100.0, enemy);
										}
										if(!target->IsPlayerRef() && Actor_GetCombatState(target) != RE::ACTOR_COMBAT_STATE::kCombat){
											const auto caster_target = target->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
											caster_target->CastSpellImmediate(CFRs_CalmSpell, true, target, 1, false, 100.0, target);
										}
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
		case "MLh_SpellFire_Event"_h:
		case "MRh_SpellFire_Event"_h:
		case "Voice_SpellFire_Event"_h:
		case "PowerAttack_Start_end"_h:
		case "NextAttackInitiate"_h:
		case "preHitFrame"_h:
		case "BowRelease"_h:
		case "arrowRelease"_h:
		case "bowDraw"_h:

			if (a_actor->IsPlayerRef())
			{
				if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(H->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
				{
					if (a_actor->IsSneaking() && !a_actor->IsInCombat())
					{
						if (Settings::GetSingleton()->general.bDynamicToggle && CFRs_FriendlyFire_Off->value != 0.0f)
						{
							CFRs_FriendlyFire_Off->value = 0.0f;
						}
					}
					else
					{
						if (Settings::GetSingleton()->general.bDynamicToggle && CFRs_FriendlyFire_Off->value != 1.0f)
						{
							CFRs_FriendlyFire_Off->value = 1.0f;
						}
					}
				}

			}else{

				if (const auto enemy = a_actor->GetActorRuntimeData().currentCombatTarget.get().get(); enemy)
				{
					const auto CurrentFollowerFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CurrentFollowerFaction");
					const auto CFRs_PlayerAlliesFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_PlayerAlliesFaction");

					if (enemy && CFRs_PlayerAlliesFaction && CurrentFollowerFaction)
					{
						if ((enemy->IsInFaction(CFRs_PlayerAlliesFaction) || enemy->IsInFaction(CurrentFollowerFaction) || enemy->IsPlayerRef()) && (a_actor->IsInFaction(CFRs_PlayerAlliesFaction) || a_actor->IsInFaction(CurrentFollowerFaction)))
						{
							if (const auto CFRs_CalmSpell = RE::TESForm::LookupByEditorID<RE::MagicItem>("CFRs_CalmSpell"); CFRs_CalmSpell)
							{
								if (!enemy->IsPlayerRef() && GFunc_Space::Has_Magiceffect_Keyword(enemy, RE::TESForm::LookupByEditorID<RE::BGSKeyword>("CFRs_Calm_Key"), 0.0f))
								{
									const auto caster_enemy = enemy->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
									caster_enemy->CastSpellImmediate(CFRs_CalmSpell, true, enemy, 1, false, 100.0, enemy);
								}
								if (GFunc_Space::Has_Magiceffect_Keyword(a_actor, RE::TESForm::LookupByEditorID<RE::BGSKeyword>("CFRs_Calm_Key"), 0.0f))
								{
									const auto caster_target = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
									caster_target->CastSpellImmediate(CFRs_CalmSpell, true, a_actor, 1, false, 100.0, a_actor);
								}
							}
						}
					}
				}
				
			}
			break;

		case "BowFullDrawn"_h:
		case "BeginCastLeft"_h:
		case "BeginCastRight"_h:
		case "BeginCastVoice"_h:

			if (!a_actor->IsPlayerRef())
			{
				if (const auto enemy = a_actor->GetActorRuntimeData().currentCombatTarget.get().get(); enemy)
				{
					const auto CurrentFollowerFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CurrentFollowerFaction");
					const auto CFRs_PlayerAlliesFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_PlayerAlliesFaction");

					if (enemy && CFRs_PlayerAlliesFaction && CurrentFollowerFaction)
					{
						if ((enemy->IsInFaction(CFRs_PlayerAlliesFaction) || enemy->IsInFaction(CurrentFollowerFaction) || enemy->IsPlayerRef()) && (a_actor->IsInFaction(CFRs_PlayerAlliesFaction) || a_actor->IsInFaction(CurrentFollowerFaction)))
						{
							if (const auto CFRs_CalmSpell = RE::TESForm::LookupByEditorID<RE::MagicItem>("CFRs_CalmSpell"); CFRs_CalmSpell)
							{
								if (!enemy->IsPlayerRef() && GFunc_Space::Has_Magiceffect_Keyword(enemy, RE::TESForm::LookupByEditorID<RE::BGSKeyword>("CFRs_Calm_Key"), 0.0f))
								{
									const auto caster_enemy = enemy->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
									caster_enemy->CastSpellImmediate(CFRs_CalmSpell, true, enemy, 1, false, 100.0, enemy);
								}
								if (GFunc_Space::Has_Magiceffect_Keyword(a_actor, RE::TESForm::LookupByEditorID<RE::BGSKeyword>("CFRs_Calm_Key"), 0.0f))
								{
									const auto caster_target = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
									caster_target->CastSpellImmediate(CFRs_CalmSpell, true, a_actor, 1, false, 100.0, a_actor);
								}
							}
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




	bool HitEventHandler::PreProcessMagic(RE::Actor *target, RE::Actor *aggressor, RE::Effect *a_effect)
	{
		auto HdSingle = RE::TESDataHandler::GetSingleton();

		bool ignoredamage = false;

		const auto CurrentFollowerFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CurrentFollowerFaction");
		const auto CFRs_PlayerAlliesFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_PlayerAlliesFaction");
		const auto CFRs_PlayerFriendsFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_PlayerFriendsFaction");
		const auto CFRs_NPCNeutralsFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_NPCNeutralsFaction");

		// GFunc_Space::GetQuestRunning(target, RE::TESForm::LookupByEditorID<RE::TESQuest>("MQ101"), RE::CONDITION_ITEM_DATA::OpCode::kEqualTo, 0.0f);
		//bool GFunc_Space::IsInScene(const RE::Actor *a_actor, float a_comparison_value);

		if (aggressor && target)
		{
			if ((!target->IsHostileToActor(aggressor) && (target->AsActorValueOwner() && target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAggression) <= 1) && target->IsPlayerTeammate()) || (target->IsCommandedActor() && target->GetCommandingActor().get() && ((target->GetCommandingActor().get()->IsPlayerRef()) || (target->GetCommandingActor().get()->IsPlayerTeammate()))))
			{
				if (CFRs_PlayerAlliesFaction && !target->IsInFaction(CFRs_PlayerAlliesFaction))
				{
					target->AddToFaction(CFRs_PlayerAlliesFaction, 0);
				}
				if (target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAssistance) != 2)
				{
					target->AsActorValueOwner()->SetActorValue(RE::ActorValue::kAssistance, 2);
				}
				if ((target->formFlags & RE::TESObjectREFR::RecordFlags::kIgnoreFriendlyHits) == 0)
				{
					target->formFlags |= RE::TESObjectREFR::RecordFlags::kIgnoreFriendlyHits;
				}
			}else if(CFRs_PlayerAlliesFaction && target->IsInFaction(CFRs_PlayerAlliesFaction)){
				Events::RemoveFromFaction(target, CFRs_PlayerAlliesFaction);

			}
			if ((!aggressor->IsHostileToActor(target) && (aggressor->AsActorValueOwner() && aggressor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAggression) <= 1) && aggressor->IsPlayerTeammate()) || (aggressor->IsCommandedActor() && aggressor->GetCommandingActor().get() && ((aggressor->GetCommandingActor().get()->IsPlayerRef()) || (aggressor->GetCommandingActor().get()->IsPlayerTeammate()))))
			{
				if (CFRs_PlayerAlliesFaction && !aggressor->IsInFaction(CFRs_PlayerAlliesFaction))
				{
					aggressor->AddToFaction(CFRs_PlayerAlliesFaction, 0);
				}
				if (aggressor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAssistance) != 2)
				{
					aggressor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kAssistance, 2);
				}
				if ((aggressor->formFlags & RE::TESObjectREFR::RecordFlags::kIgnoreFriendlyHits) == 0)
				{
					aggressor->formFlags |= RE::TESObjectREFR::RecordFlags::kIgnoreFriendlyHits;
				}
			}else if(CFRs_PlayerAlliesFaction && aggressor->IsInFaction(CFRs_PlayerAlliesFaction)){
				Events::RemoveFromFaction(aggressor, CFRs_PlayerAlliesFaction);

			}
			
			if (CFRs_NPCNeutralsFaction)
			{
				if (target->IsInFaction(CFRs_NPCNeutralsFaction))
				{
					Events::RemoveFromFaction(target, CFRs_NPCNeutralsFaction);
				}

				if (aggressor->IsInFaction(CFRs_NPCNeutralsFaction))
				{
					Events::RemoveFromFaction(aggressor, CFRs_NPCNeutralsFaction);
				}
			}

			if (Events::GetFactionReaction(target, aggressor) == RE::FIGHT_REACTION::kNeutral)
			{
				// logger::info("Neutral Branch Active");

				if (!target->IsHostileToActor(aggressor) && target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAggression) <= 1)
				{

					if (aggressor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && aggressor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && aggressor->IsInFaction(CurrentFollowerFaction)) )
					{
						if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
						{
							if (CFRs_FriendlyFire_Off->value == 1.0f)
							{

								ignoredamage = true;
							}
		
						}
					}
					else
					{
						if (CurrentFollowerFaction && CFRs_PlayerAlliesFaction &&  !target->IsPlayerRef() && !target->IsInFaction(CFRs_PlayerAlliesFaction) && !target->IsInFaction(CurrentFollowerFaction))
						{
							// if (CFRs_NPCNeutralsFaction)
							// {
							// 	if (!target->IsInFaction(CFRs_NPCNeutralsFaction) && GFunc_Space::IsInScene(target, 0.0f))
							// 	{
							// 		target->AddToFaction(CFRs_NPCNeutralsFaction, 0);
							// 	}

							// 	if (!aggressor->IsInFaction(CFRs_NPCNeutralsFaction) && GFunc_Space::IsInScene(aggressor, 0.0f))
							// 	{
							// 		aggressor->AddToFaction(CFRs_NPCNeutralsFaction, 0);
							// 	}
							// }

						}else{
							ignoredamage = true;
						}
					}
				}
			}
			else if (Events::GetFactionReaction(target, aggressor) >= RE::FIGHT_REACTION::kAlly)
			{
				// logger::info("Allied Branch Active");

				if (!target->IsHostileToActor(aggressor))
				{
					if (aggressor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && aggressor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && aggressor->IsInFaction(CurrentFollowerFaction)))
					{
						if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
						{
							if (CFRs_FriendlyFire_Off->value == 1.0f)
							{

								ignoredamage = true;
							}
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
				auto TrapPoisonKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicTrapPoison");
				auto TrapGasKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("'MagicTrapGas");
				auto WeaponSpeedKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicWeaponSpeed");
				auto CFRs_Exclude_MagicEffect_Key = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("CFRs_Exclude_MagicEffect_Key");
				RE::BSFixedString CFRs_exclude = "_CFRs_exclude";

				using AX = RE::EffectSetting::Archetype;

				if (a_effect && a_effect->baseEffect && !clib_util::editorID::get_editorID(a_effect->baseEffect).contains(CFRs_exclude) && CFRs_Exclude_MagicEffect_Key && !a_effect->baseEffect->HasKeyword(CFRs_Exclude_MagicEffect_Key))
				{
					RE::BSFixedString VoiceDragonrend = "VoiceDragonrend";
					RE::BSFixedString FrostSlow = "FrostSlow";
					auto Archy_X = a_effect->baseEffect->data.archetype;
					auto PrimAV = a_effect->baseEffect->data.primaryAV;
					auto SecAV = a_effect->baseEffect->data.secondaryAV;
					auto hasHostileflag = a_effect->baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kHostile);
					auto hasdoublehostile = a_effect->baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kHostile) || a_effect->baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kDetrimental);
					auto Kw_ScriptHostile = clib_util::editorID::get_editorID(a_effect->baseEffect).contains(FrostSlow);
					auto Kw_DragonRend = clib_util::editorID::get_editorID(a_effect->baseEffect).contains(VoiceDragonrend);
					auto Kw_DragonRendeffects = (Archy_X == AX::kScript) || (Archy_X == AX::kStagger) || (PrimAV == RE::ActorValue::kDragonRend);
					auto Kw_magicfire = fireKeyword && a_effect->baseEffect->HasKeyword(fireKeyword);
					auto Kw_magicfrost = frostKeyword && a_effect->baseEffect->HasKeyword(frostKeyword);
					auto Kw_magicshock = shockKeyword && a_effect->baseEffect->HasKeyword(shockKeyword);
					auto Kw_magicshout = shoutKeyword && a_effect->baseEffect->HasKeyword(shoutKeyword);
					auto Kw_Exclude = (TrapGasKeyword && a_effect->baseEffect->HasKeyword(TrapGasKeyword)) || (TrapPoisonKeyword && a_effect->baseEffect->HasKeyword(TrapPoisonKeyword)) || (stormKeyword && a_effect->baseEffect->HasKeyword(stormKeyword)) || (WeaponSpeedKeyword && a_effect->baseEffect->HasKeyword(WeaponSpeedKeyword));
					auto Kw_Storm = stormKeyword && a_effect->baseEffect->HasKeyword(stormKeyword);

					if ((Kw_DragonRend && Kw_DragonRendeffects) || (!Kw_Exclude && (hasdoublehostile && Archy_X == AX::kScript)) || (Kw_ScriptHostile && Archy_X == AX::kScript) || (Kw_Storm && Archy_X == AX::kStagger) || (Kw_magicshout && Archy_X == AX::kStagger) || (!Kw_Exclude && (hasHostileflag || Kw_magicfire || Kw_magicfrost || Kw_magicshock) && (Archy_X == AX::kDualValueModifier || Archy_X == AX::kValueModifier || Archy_X == AX::kPeakValueModifier || Archy_X == AX::kParalysis || Archy_X == AX::kDemoralize || Archy_X == AX::kFrenzy || Archy_X == AX::kDisarm || Archy_X == AX::kAbsorb || Archy_X == AX::kStagger)))
					{
						// harmful effect - ignore;
						if(Settings::GetSingleton()->general.bDebugMode){
							logger::info("{} ignored magicEffect from {}. effectID: {} ", target->GetName(), aggressor->GetName(), clib_util::editorID::get_editorID(a_effect->baseEffect));
						}
						
					}else{

						ignoredamage = false;
					}
				}
			}
		}

		return ignoredamage;
	}

	bool HitEventHandler::PreProcessHit(RE::Actor *target, RE::HitData *hitData)
	{
		auto aggressor = hitData->aggressor ? hitData->aggressor.get().get() : nullptr;

		auto HdSingle = RE::TESDataHandler::GetSingleton();

		bool ignoredamage = false;

		const auto CurrentFollowerFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CurrentFollowerFaction");
		const auto CFRs_PlayerAlliesFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_PlayerAlliesFaction");
		const auto CFRs_PlayerFriendsFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_PlayerFriendsFaction");
		const auto CFRs_NPCNeutralsFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CFRs_NPCNeutralsFaction");

		

		if (aggressor && target)
		{
			if ((!target->IsHostileToActor(aggressor) && (target->AsActorValueOwner() && target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAggression) <= 1) && target->IsPlayerTeammate()) || (target->IsCommandedActor() && target->GetCommandingActor().get() && ((target->GetCommandingActor().get()->IsPlayerRef()) || (target->GetCommandingActor().get()->IsPlayerTeammate()))))
			{
				if (CFRs_PlayerAlliesFaction && !target->IsInFaction(CFRs_PlayerAlliesFaction))
				{
					target->AddToFaction(CFRs_PlayerAlliesFaction, 0);
				}
				if (target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAssistance) != 2)
				{
					target->AsActorValueOwner()->SetActorValue(RE::ActorValue::kAssistance, 2);
				}
				if ((target->formFlags & RE::TESObjectREFR::RecordFlags::kIgnoreFriendlyHits) == 0)
				{
					target->formFlags |= RE::TESObjectREFR::RecordFlags::kIgnoreFriendlyHits;
				}
			}
			else if (CFRs_PlayerAlliesFaction && target->IsInFaction(CFRs_PlayerAlliesFaction))
			{
				Events::RemoveFromFaction(target, CFRs_PlayerAlliesFaction);
			}
			if ((!aggressor->IsHostileToActor(target) && (aggressor->AsActorValueOwner() && aggressor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAggression) <= 1) && aggressor->IsPlayerTeammate()) || (aggressor->IsCommandedActor() && aggressor->GetCommandingActor().get() && ((aggressor->GetCommandingActor().get()->IsPlayerRef()) || (aggressor->GetCommandingActor().get()->IsPlayerTeammate()))))
			{
				if (CFRs_PlayerAlliesFaction && !aggressor->IsInFaction(CFRs_PlayerAlliesFaction))
				{
					aggressor->AddToFaction(CFRs_PlayerAlliesFaction, 0);
				}
				if (aggressor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAssistance) != 2)
				{
					aggressor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kAssistance, 2);
				}
				if ((aggressor->formFlags & RE::TESObjectREFR::RecordFlags::kIgnoreFriendlyHits) == 0)
				{
					aggressor->formFlags |= RE::TESObjectREFR::RecordFlags::kIgnoreFriendlyHits;
				}
			}
			else if (CFRs_PlayerAlliesFaction && aggressor->IsInFaction(CFRs_PlayerAlliesFaction))
			{
				Events::RemoveFromFaction(aggressor, CFRs_PlayerAlliesFaction);
			}

			if (CFRs_NPCNeutralsFaction)
			{
				if (target->IsInFaction(CFRs_NPCNeutralsFaction))
				{
					Events::RemoveFromFaction(target, CFRs_NPCNeutralsFaction);
				}

				if (aggressor->IsInFaction(CFRs_NPCNeutralsFaction))
				{
					Events::RemoveFromFaction(aggressor, CFRs_NPCNeutralsFaction);
				}
			}

			if (Events::GetFactionReaction(target, aggressor) == RE::FIGHT_REACTION::kNeutral)
			{
				// logger::info("Neutral Branch Active");

				if (!target->IsHostileToActor(aggressor) && target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAggression) <= 1)
				{

					if (aggressor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && aggressor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && aggressor->IsInFaction(CurrentFollowerFaction)))
					{
						if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
						{
							if (CFRs_FriendlyFire_Off->value == 1.0f)
							{

								ignoredamage = true;
							}
						}
					}
					else
					{
						if (CurrentFollowerFaction && CFRs_PlayerAlliesFaction && !target->IsPlayerRef() && !target->IsInFaction(CFRs_PlayerAlliesFaction) && !target->IsInFaction(CurrentFollowerFaction))
						{
							// do nothing
							
						}else{
							ignoredamage = true;
						}
					}
				}
			}
			else if (Events::GetFactionReaction(target, aggressor) >= RE::FIGHT_REACTION::kAlly)
			{
				// logger::info("Allied Branch Active");

				if (!target->IsHostileToActor(aggressor))
				{
					if (aggressor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && aggressor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && aggressor->IsInFaction(CurrentFollowerFaction)))
					{
						if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
						{
							if (CFRs_FriendlyFire_Off->value == 1.0f)
							{

								ignoredamage = true;
							}
						}
					}
				}
			}

			if (ignoredamage && Actor_GetCombatState(target) != RE::ACTOR_COMBAT_STATE::kCombat){
				if (Settings::GetSingleton()->general.bDebugMode)
				{
					logger::info("{} ignored attack from {} ", target->GetName(), aggressor->GetName());
				}
			}else{
				ignoredamage = false;
			}


		}else if (target && hitData){

			if (const auto sourceHandle = hitData->sourceRef; sourceHandle)
			{
				if (const auto sourcePtr = sourceHandle.get(); sourcePtr)
				{
					if (const auto sourceRef = sourcePtr.get(); sourceRef)
					{
						if (sourceRef->AsExplosion() && (sourceRef->AsExplosion()->actorCause || sourceRef->AsExplosion()->actorOwner || sourceRef->AsExplosion()->magicCaster))
						{
							// logger::info("Explosion Branch Active");
							if (sourceRef->AsExplosion()->actorCause && sourceRef->AsExplosion()->actorCause.get() && sourceRef->AsExplosion()->actorCause.get()->actor)
							{
								if (const auto blameActorHandle = sourceRef->AsExplosion()->actorCause.get()->actor; blameActorHandle)
								{
									if(const auto blameActorPtr = blameActorHandle.get(); blameActorPtr){
										if(const auto blameActor = blameActorPtr.get(); blameActor){

											if (Events::GetFactionReaction(target, blameActor) == RE::FIGHT_REACTION::kNeutral){
												// logger::info("Neutral Branch Active");
												if (!target->IsHostileToActor(blameActor) && target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAggression) <= 1){

													if (blameActor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && blameActor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && blameActor->IsInFaction(CurrentFollowerFaction)))
													{
														if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
														{
															if (CFRs_FriendlyFire_Off->value == 1.0f){
                                                                // aggressor is in player team
																ignoredamage = true;
															}
														}
													}
													else{
														if (CurrentFollowerFaction && CFRs_PlayerAlliesFaction && !target->IsPlayerRef() && !target->IsInFaction(CFRs_PlayerAlliesFaction) && !target->IsInFaction(CurrentFollowerFaction)){
															// do nothing; aggressor and target aren't the player team.
														}
														else{
															// target is in the player team.
															ignoredamage = true;
														}
													}
												}
											}
											else if (Events::GetFactionReaction(target, blameActor) >= RE::FIGHT_REACTION::kAlly){
												// logger::info("Allied Branch Active");
												if (!target->IsHostileToActor(blameActor))
												{
													if (blameActor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && blameActor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && blameActor->IsInFaction(CurrentFollowerFaction)))
													{
														if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
														{
															if (CFRs_FriendlyFire_Off->value == 1.0f){

																ignoredamage = true;
															}
														}
													}
												}
											}

											if (ignoredamage && Settings::GetSingleton()->general.bDebugMode){
												logger::info("{} ignored explosion from {} ", target->GetName(), blameActor->GetName());
											}
										}
									}
								}

							}else if (sourceRef->AsExplosion()->actorOwner)
							{
								if (const auto blameActorHandle = sourceRef->AsExplosion()->actorOwner; blameActorHandle)
								{
									if (const auto blameActorPtr = blameActorHandle.get(); blameActorPtr)
									{
										if (const auto blameActor = blameActorPtr.get(); blameActor)
										{
											if (Events::GetFactionReaction(target, blameActor) == RE::FIGHT_REACTION::kNeutral){
												// logger::info("Neutral Branch Active");
												if (!target->IsHostileToActor(blameActor) && target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAggression) <= 1){

													if (blameActor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && blameActor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && blameActor->IsInFaction(CurrentFollowerFaction)))
													{
														if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
														{
															if (CFRs_FriendlyFire_Off->value == 1.0f){
																// aggressor is in player team
																ignoredamage = true;
															}
														}
													}
													else{
														if (CurrentFollowerFaction && CFRs_PlayerAlliesFaction && !target->IsPlayerRef() && !target->IsInFaction(CFRs_PlayerAlliesFaction) && !target->IsInFaction(CurrentFollowerFaction)){
															// do nothing; aggressor and target aren't the player team.
														}
														else{
															// target is in the player team.
															ignoredamage = true;
														}
													}
												}
											}
											else if (Events::GetFactionReaction(target, blameActor) >= RE::FIGHT_REACTION::kAlly){
												// logger::info("Allied Branch Active");
												if (!target->IsHostileToActor(blameActor))
												{
													if (blameActor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && blameActor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && blameActor->IsInFaction(CurrentFollowerFaction)))
													{
														if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
														{
															if (CFRs_FriendlyFire_Off->value == 1.0f){

																ignoredamage = true;
															}
														}
													}
												}
											}

											if (ignoredamage && Settings::GetSingleton()->general.bDebugMode){
												logger::info("{} ignored explosion from {} ", target->GetName(), blameActor->GetName());
											}
										}
									}
								}

							}else if (sourceRef->AsExplosion()->magicCaster && sourceRef->AsExplosion()->magicCaster->blameActor)
							{
								if (const auto blameActorHandle = sourceRef->AsExplosion()->magicCaster->blameActor; blameActorHandle)
								{
									if (const auto blameActorPtr = blameActorHandle.get(); blameActorPtr)
									{
										if (const auto blameActor = blameActorPtr.get(); blameActor)
										{
											if (Events::GetFactionReaction(target, blameActor) == RE::FIGHT_REACTION::kNeutral){
												// logger::info("Neutral Branch Active");
												if (!target->IsHostileToActor(blameActor) && target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAggression) <= 1){

													if (blameActor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && blameActor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && blameActor->IsInFaction(CurrentFollowerFaction)))
													{
														if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
														{
															if (CFRs_FriendlyFire_Off->value == 1.0f){
																// aggressor is in player team
																ignoredamage = true;
															}
														}
													}
													else{
														if (CurrentFollowerFaction && CFRs_PlayerAlliesFaction && !target->IsPlayerRef() && !target->IsInFaction(CFRs_PlayerAlliesFaction) && !target->IsInFaction(CurrentFollowerFaction)){
															// do nothing; aggressor and target aren't the player team.
														}
														else{
															// target is in the player team.
															ignoredamage = true;
														}
													}
												}
											}
											else if (Events::GetFactionReaction(target, blameActor) >= RE::FIGHT_REACTION::kAlly){
												// logger::info("Allied Branch Active");

												if (!target->IsHostileToActor(blameActor))
												{
													if (blameActor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && blameActor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && blameActor->IsInFaction(CurrentFollowerFaction)))
													{
														if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
														{
															if (CFRs_FriendlyFire_Off->value == 1.0f){

																ignoredamage = true;
															}
														}
													}
												}
											}

											if (ignoredamage && Settings::GetSingleton()->general.bDebugMode){
												logger::info("{} ignored explosion from {} ", target->GetName(), blameActor->GetName());
											}
										}
									}
								}
							}
						}else if(sourceRef->Is(RE::FormType::PlacedHazard) && sourceRef->As<RE::Hazard>() && sourceRef->As<RE::Hazard>()->GetHazardRuntimeData().ownerActor)
						{
							// logger::info("Hazard Branch Active");
							if (const auto blameActorHandle = sourceRef->As<RE::Hazard>()->GetHazardRuntimeData().ownerActor; blameActorHandle)
							{
								if (const auto blameActorPtr = blameActorHandle.get(); blameActorPtr)
								{
									if (const auto blameActor = blameActorPtr.get(); blameActor)
									{
										if (Events::GetFactionReaction(target, blameActor) == RE::FIGHT_REACTION::kNeutral){
											// logger::info("Neutral Branch Active");
											if (!target->IsHostileToActor(blameActor) && target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAggression) <= 1){

												if (blameActor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && blameActor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && blameActor->IsInFaction(CurrentFollowerFaction)))
												{
													if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
													{
														if (CFRs_FriendlyFire_Off->value == 1.0f){
															// aggressor is in player team
															ignoredamage = true;
														}
													}
												}
												else{
													if (CurrentFollowerFaction && CFRs_PlayerAlliesFaction && !target->IsPlayerRef() && !target->IsInFaction(CFRs_PlayerAlliesFaction) && !target->IsInFaction(CurrentFollowerFaction)){
														// do nothing; aggressor and target aren't the player team.
													}
													else{
														// target is in the player team.
														ignoredamage = true;
													}
												}
											}
										}
										else if (Events::GetFactionReaction(target, blameActor) >= RE::FIGHT_REACTION::kAlly){
											// logger::info("Allied Branch Active");
											if (!target->IsHostileToActor(blameActor))
											{
												if (blameActor->IsPlayerRef() || (CFRs_PlayerAlliesFaction && blameActor->IsInFaction(CFRs_PlayerAlliesFaction)) || (CurrentFollowerFaction && blameActor->IsInFaction(CurrentFollowerFaction)))
												{
													if (const auto CFRs_FriendlyFire_Off = skyrim_cast<RE::TESGlobal *>(HdSingle->LookupForm(0x804, "Coherent Fight Reactions.esp")); CFRs_FriendlyFire_Off)
													{
														if (CFRs_FriendlyFire_Off->value == 1.0f){

															ignoredamage = true;
														}
													}
												}
											}
										}

										if (ignoredamage && Settings::GetSingleton()->general.bDebugMode){
											logger::info("{} ignored hazard from {} ", target->GetName(), blameActor->GetName());
										}
									}
								}
							}
						}
					}
				}
			}
		}

		return ignoredamage;
	}

	struct MagicTargetApply
	{
		static bool thunk(RE::MagicTarget *a_this, RE::MagicTarget::AddTargetData *a_data)
		{
			if (const auto target = a_this && a_data ? a_this->GetTargetStatsObject() : nullptr; target)
			{
				const auto effect = a_data->effect;
				if (const auto baseEffect = effect ? effect->baseEffect : nullptr; baseEffect)
				{
					const auto caster = a_data->caster;
					if (const auto casterActor = caster && caster->Is(RE::FormType::ActorCharacter)? caster->As<RE::Actor>(): nullptr; casterActor)
					{
						if (const auto targetActor = target->Is(RE::FormType::ActorCharacter)? target->As<RE::Actor>(): nullptr; targetActor)
						{
							if(targetActor != casterActor){
								const auto magicitem = a_data->magicItem;
								if (const auto valid = magicitem ? (((magicitem->GetSpellType() == RE::MagicSystem::SpellType::kStaffEnchantment) && magicitem->GetDelivery() 
								!= RE::MagicSystem::Delivery::kTouch) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kScroll) || (magicitem->GetSpellType() 
								== RE::MagicSystem::SpellType::kLesserPower) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kPower) || (magicitem->GetSpellType() 
								== RE::MagicSystem::SpellType::kSpell) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kVoicePower)) : false; valid){

									if (HitEventHandler::GetSingleton()->PreProcessMagic(targetActor, casterActor, effect))
									{
										if (const auto new_item = RE::TESForm::LookupByEditorID<RE::MagicItem>("CFRs_BlankSpell"); new_item)
										{
											if (const auto new_itemEffect = RE::TESForm::LookupByEditorID<RE::EffectSetting>("CFRs_BlankEffect"); new_itemEffect)
											{
												RE::Effect *neweffect = new RE::Effect;
												neweffect->cost = 0.0f;
												neweffect->effectItem.area = 0;
												neweffect->effectItem.duration = 0;
												neweffect->effectItem.magnitude = 0.0f;
												neweffect->baseEffect = new_itemEffect;
												a_data->magicItem = new_item;
												a_data->effect = neweffect;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			return func(a_this, a_data);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	void Install_apply()
	{
		REL::Relocation<std::uintptr_t> target{RELOCATION_ID(33742, 34526), OFFSET(0x1E8, 0x20B)};
		stl::write_thunk_call<MagicTargetApply>(target.address());

		logger::info("Hooked Magic Effect Apply");
	}

	bool MagicApplyHandler::Character::Thunk(RE::MagicTarget *a_this, RE::MagicTarget::AddTargetData *a_data){

		if (const auto target = a_this && a_data ? a_this->GetTargetStatsObject() : nullptr; target)
		{
			const auto effect = a_data->effect;
			if (const auto baseEffect = effect ? effect->baseEffect : nullptr; baseEffect)
			{
				const auto caster = a_data->caster;
				if (const auto casterActor = caster && caster->Is(RE::FormType::ActorCharacter) ? caster->As<RE::Actor>() : nullptr; casterActor)
				{
					if (const auto targetActor = target->Is(RE::FormType::ActorCharacter) ? target->As<RE::Actor>() : nullptr; targetActor)
					{
						if (targetActor != casterActor)
						{
							const auto magicitem = a_data->magicItem;
							if (const auto valid = magicitem ? (((magicitem->GetSpellType() == RE::MagicSystem::SpellType::kStaffEnchantment) && magicitem->GetDelivery() != RE::MagicSystem::Delivery::kTouch) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kScroll) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kLesserPower) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kPower) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kSpell) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kVoicePower)) : false; valid)
							{

								if (HitEventHandler::GetSingleton()->PreProcessMagic(targetActor, casterActor, effect))
								{
									if (const auto new_item = RE::TESForm::LookupByEditorID<RE::MagicItem>("CFRs_BlankSpell"); new_item)
									{
										if (const auto new_itemEffect = RE::TESForm::LookupByEditorID<RE::EffectSetting>("CFRs_BlankEffect"); new_itemEffect)
										{
											RE::Effect *neweffect = new RE::Effect;
											neweffect->cost = 0.0f;
											neweffect->effectItem.area = 0;
											neweffect->effectItem.duration = 0;
											neweffect->effectItem.magnitude = 0.0f;
											neweffect->baseEffect = new_itemEffect;
											a_data->magicItem = new_item;
											a_data->effect = neweffect;

											a_data->explosionPoint.x = 0.0f;
											a_data->explosionPoint.y = 0.0f;
											a_data->explosionPoint.z = 0.0f;

											logger::info("eX: {:.2f} eY: {:.2f} eZ: {:.2f}", a_data->explosionPoint.x, a_data->explosionPoint.y, a_data->explosionPoint.z);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return _func(a_this, a_data);
	}

	bool MagicApplyHandler::Player::Thunk(RE::MagicTarget *a_this, RE::MagicTarget::AddTargetData *a_data)
	{
		if (const auto target = a_this && a_data ? a_this->GetTargetStatsObject() : nullptr; target)
		{
			const auto effect = a_data->effect;
			if (const auto baseEffect = effect ? effect->baseEffect : nullptr; baseEffect)
			{
				const auto caster = a_data->caster;
				if (const auto casterActor = caster && caster->Is(RE::FormType::ActorCharacter) ? caster->As<RE::Actor>() : nullptr; casterActor)
				{
					if (const auto targetActor = target->Is(RE::FormType::ActorCharacter) ? target->As<RE::Actor>() : nullptr; targetActor)
					{
						if (targetActor != casterActor)
						{
							const auto magicitem = a_data->magicItem;
							if (const auto valid = magicitem ? (((magicitem->GetSpellType() == RE::MagicSystem::SpellType::kStaffEnchantment) && magicitem->GetDelivery() != RE::MagicSystem::Delivery::kTouch) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kScroll) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kLesserPower) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kPower) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kSpell) || (magicitem->GetSpellType() == RE::MagicSystem::SpellType::kVoicePower)) : false; valid)
							{

								if (HitEventHandler::GetSingleton()->PreProcessMagic(targetActor, casterActor, effect))
								{
									if (const auto new_item = RE::TESForm::LookupByEditorID<RE::MagicItem>("CFRs_BlankSpell"); new_item)
									{
										if (const auto new_itemEffect = RE::TESForm::LookupByEditorID<RE::EffectSetting>("CFRs_BlankEffect"); new_itemEffect)
										{
											RE::Effect *neweffect = new RE::Effect;
											neweffect->cost = 0.0f;
											neweffect->effectItem.area = 0;
											neweffect->effectItem.duration = 0;
											neweffect->effectItem.magnitude = 0.0f;
											neweffect->baseEffect = new_itemEffect;
											a_data->magicItem = new_item;
											a_data->effect = neweffect;

											a_data->explosionPoint.x = 0.0f;
											a_data->explosionPoint.y = 0.0f;
											a_data->explosionPoint.z = 0.0f;

											logger::info("eX: {:.2f} eY: {:.2f} eZ: {:.2f}", a_data->explosionPoint.x, a_data->explosionPoint.y, a_data->explosionPoint.z);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return _func(a_this, a_data);
	}


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
		// eventSourceHolder->AddEventSink<RE::TESHitEvent>(eventSink);
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
										Events::RemoveFromFaction(a_actor, CFRs_NPCNeutralsFaction);
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
		constexpr auto path = "Data\\SKSE\\Plugins\\CoherentFightReactions.ini";

		CSimpleIniA ini;
		ini.SetUnicode();

		ini.LoadFile(path);

		general.Load(ini);

		// include_spells_mods.Load(ini);
		// include_spells_keywords.Load(ini);
		// exclude_spells_mods.Load(ini);
		// exclude_spells_keywords.Load(ini);

		ini.SaveFile(path);
	}

	void Settings::General_Settings::Load(CSimpleIniA &a_ini)
	{
		static const char *section = "General_Settings";

		auto DS = GetSingleton();

		DS->general.bDebugMode = a_ini.GetBoolValue(section, "bDebugMode", DS->general.bDebugMode);

		DS->general.bDynamicToggle = a_ini.GetBoolValue(section, "bDynamicToggle", DS->general.bDynamicToggle);

		a_ini.SetBoolValue(section, "bDebugMode", DS->general.bDebugMode, ";The log shows which actors ignored attacks and from whom. Also shows EDIDs of any relevant MagicEffects");

		a_ini.SetBoolValue(section, "bDynamicToggle", DS->general.bDynamicToggle, ";Dynamically toggles FF at appropriate moments. FF is on while sneaking outside combat. FF is off in combat against non-hostile actors");
		
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

// if (CurrentFollowerFaction && CFRs_PlayerAlliesFaction && !target->IsInFaction(CFRs_PlayerAlliesFaction) && !target->IsInFaction(CurrentFollowerFaction) && !target->IsPlayerRef())
// {

// 	if (CFRs_PlayerFriendsFaction && !target->IsInFaction(CFRs_PlayerFriendsFaction))
// 	{
// 		target->AddToFaction(CFRs_PlayerFriendsFaction, 0);
// 	}
// }

// if (CFRs_PlayerFriendsFaction && target->IsInFaction(CFRs_PlayerFriendsFaction))
// {
// 	Events::RemoveFromFaction(target, CFRs_PlayerFriendsFaction);
// }

// if (sourceRef->AsProjectile() && sourceRef->AsProjectile()->GetProjectileRuntimeData().ammoSource && sourceRef->AsProjectile()->GetProjectileRuntimeData().weaponSource)
// {
// 	sourceRef->AsProjectile()->GetProjectileRuntimeData().castingSource;
// }

// if (CFRs_NPCNeutralsFaction)
// {
// 	if (!target->IsInFaction(CFRs_NPCNeutralsFaction) && GFunc_Space::IsInScene(target, 0.0f))
// 	{
// 		target->AddToFaction(CFRs_NPCNeutralsFaction, 0);
// 	}

// 	if (!aggressor->IsInFaction(CFRs_NPCNeutralsFaction) && GFunc_Space::IsInScene(aggressor, 0.0f))
// 	{
// 		aggressor->AddToFaction(CFRs_NPCNeutralsFaction, 0);
// 	}
// }