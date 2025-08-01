#include "SKSE/Trampoline.h"
#include "ClibUtil/editorID.hpp"
#include <SimpleIni.h>
#include <iterator>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
#include <tuple>
#include <thread>
#pragma warning(disable: 4100)
#pragma warning(disable : 4189)

namespace GFunc_Space{

	using uniqueLocker = std::unique_lock<std::shared_mutex>;
	using sharedLocker = std::shared_lock<std::shared_mutex>;

#define PI 3.14159265358979323846f

	using tActor_IsMoving = bool (*)(RE::Actor *a_this);
	static REL::Relocation<tActor_IsMoving> IsMoving{ REL::VariantID(36928, 37953, 0x6116C0) };

	typedef float (*tActor_GetReach)(RE::Actor *a_this);
	static REL::Relocation<tActor_GetReach> Actor_GetReach{RELOCATION_ID(37588, 38538)};

	typedef void(_fastcall *tPushActorAway_sub_14067D4A0)(RE::AIProcess *a_causer, RE::Actor *a_target, RE::NiPoint3 &a_origin, float a_magnitude);
	inline static REL::Relocation<tPushActorAway_sub_14067D4A0> pushActorAway{RELOCATION_ID(38858, 39895)};

	typedef void(_fastcall *_shakeCamera)(float strength, RE::NiPoint3 source, float duration);
	inline static REL::Relocation<_shakeCamera> shakeCamera{RELOCATION_ID(32275, 33012)};

	typedef void(_fastcall *_destroyProjectile)(RE::Projectile *a_projectile);
	inline static REL::Relocation<_destroyProjectile> destroyProjectile{RELOCATION_ID(42930, 44110)};
	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::milliseconds ms;
	typedef std::chrono::seconds secs;
	typedef std::chrono::duration<float> fsec;

	union ConditionParam
	{
		char c;
		std::int32_t i;
		float f;
		RE::TESForm *form;
	};

	bool GetshouldHelp(const RE::Actor *p_ally, const RE::Actor *a_actor);
	void StartCombat(const RE::Actor *subject, const RE::Actor *target);
	bool Has_Magiceffect_Keyword(const RE::Actor *a_actor, const RE::BGSKeyword *a_key, float a_comparison_value);
	bool HasBoundWeaponEquipped(const RE::Actor *a_actor, RE::MagicSystem::CastingSource type);
	bool GetPairedAnimation(const RE::Actor *a_actor, float a_comparison_value);
	bool GetIsGhost(const RE::Actor *a_actor, float a_comparison_value);
	bool IsAllowedToFly(const RE::Actor *a_actor, float a_comparison_value);
	bool IsScenePackageRunning(const RE::Actor *a_actor, float a_comparison_value);

	class GFunc
	{
	public: 
		static GFunc *GetSingleton()
		{
			static GFunc avInterface;
			return &avInterface;
		}

		static void Set_iFrames(RE::Actor *actor);
		static void Reset_iFrames(RE::Actor *actor);
		static void dispelEffect(RE::MagicItem *spellForm, RE::Actor *a_target);

		static void InterruptAttack(RE::Actor *a_actor);

		static void EquipfromInvent(RE::Actor *a_actor, RE::FormID a_formID, bool unequip = false);

		static bool isPowerAttacking(RE::Actor *a_actor);
		static bool IsCasting(RE::Actor *a_actor);
		static void UpdateCombatTarget(RE::Actor *a_actor);
		static bool isHumanoid(RE::Actor *a_actor);
		static std::vector<RE::TESForm *> GetEquippedForm(RE::Actor *actor, bool right = false, bool left = false);
		static int GetEquippedItemType(RE::Actor *actor, bool lefthand);
		static bool IsWeaponOut(RE::Actor *actor);
		float AV_Mod(RE::Actor *a_actor, int a_aggression, float input, float mod);
		int GenerateRandomInt(int value_a, int value_b);
		float GenerateRandomFloat(float value_a, float value_b);
		static bool IsMeleeOnly(RE::Actor *a_actor);
		void UnequipAll(RE::Actor *a_actor);
		void Re_EquipAll(RE::Actor *a_actor);
		RE::BGSAttackData *get_attackData(RE::Actor *a);
		float get_personal_threatRatio(RE::Actor *protagonist, RE::Actor *combat_target);
		float get_personal_survivalRatio(RE::Actor *protagonist, RE::Actor *combat_target);
		static void PushActorAway(RE::Actor *causer, RE::Actor *target, float magnitude);
		float get_angle_he_me(RE::Actor *me, RE::Actor *he, RE::BGSAttackData *attackdata);
		static void set_tupledata(std::tuple<bool, std::chrono::steady_clock::time_point, GFunc_Space::ms, std::string> data, bool a, std::chrono::steady_clock::time_point b, GFunc_Space::ms c, std::string d);
		static bool GetBoolVariable(RE::Actor *a_actor, std::string a_string);
		static int GetIntVariable(RE::Actor *a_actor, std::string a_string);
		static float GetFloatVariable(RE::Actor *a_actor, std::string a_string);
		static float GetActorValuePercent(RE::Actor *a_actor, RE::ActorValue a_value);
		void wait(int a_duration);

		struct PolarAngle
		{
			float alpha;
			operator float() const { return alpha; }
			PolarAngle(float x = 0.0f) : alpha(x)
			{
				while (alpha > 360.0f)
					alpha -= 360.0f;
				while (alpha < 0.0f)
					alpha += 360.0f;
			}
			PolarAngle(const RE::NiPoint3 &p)
			{
				const float y = p.y;
				if (y == 0.0)
				{
					if (p.x <= 0.0)
						alpha = PI * 1.5f;
					else
						alpha = PI * 0.5f;
				}
				else
				{
					alpha = atanf(p.x / y);
					if (y < 0.0)
						alpha += PI;
				}
				alpha = alpha * 180.0f / PI;
			}
			PolarAngle add(const PolarAngle &r) const
			{
				float ans = alpha + r.alpha;
				if (ans >= 360.0f)
					return {ans - 360.0f};
				else
					return {ans};
			}
			PolarAngle sub(const PolarAngle &r) const
			{
				return this->add({360.0f - r.alpha});
			}
			float to_normangle() const
			{
				if (alpha > 180.0f)
					return alpha - 360.0f;
				else
					return alpha;
			}
			float to_normangle_abs() const
			{
				return abs(to_normangle());
			}
			static bool ordered(PolarAngle alpha, PolarAngle beta, PolarAngle gamma)
			{
				gamma = gamma.sub(alpha);
				beta = beta.sub(alpha);
				return gamma >= beta;
			}
			static float dist(float r, PolarAngle alpha)
			{
				auto ans = r * (alpha) / 180.0f * PI;
				return ans;
			}
		};

		template <class T>
		static std::vector<T *> get_all(const std::vector<RE::BGSKeyword *> &a_keywords)
		{
			std::vector<T *> result;

			if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler)
			{
				for (const auto &form : dataHandler->GetFormArray<T>())
				{
					if (!form || !a_keywords.empty() && !form->HasKeywordInArray(a_keywords, false))
					{
						continue;
					}
					result.push_back(form);
				}
			}

			return result;
		}

		template <class T>
		static std::vector<T *> get_in_mod(const RE::TESFile *a_modInfo, const std::vector<RE::BGSKeyword *> &a_keywords)
		{
			std::vector<T *> result;

			if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler)
			{
				for (const auto &form : dataHandler->GetFormArray<T>())
				{
					if (!form || !a_modInfo->IsFormInMod(form->formID) || !a_keywords.empty() && !form->HasKeywordInArray(a_keywords, false))
					{
						continue;
					}
					result.push_back(form);
				}
			}

			return result;
		}

		static std::vector<const RE::TESFile *> LookupMods(const std::vector<std::string> &modInfo_List)
		{
			std::vector<const RE::TESFile *> result;

			for (auto limbo_mod : modInfo_List)
			{
				if (!limbo_mod.empty())
				{
					const auto dataHandler = RE::TESDataHandler::GetSingleton();
					const auto modInfo = dataHandler ? dataHandler->LookupModByName(limbo_mod) : nullptr;

					if (modInfo)
					{
						result.push_back(modInfo);
					}
				}
			}

			return result;
		}

		static std::vector<RE::BGSKeyword *> LookupKeywords(const std::vector<std::string> &keyword_List)
		{
			std::vector<RE::BGSKeyword *> result;

			for (auto limbo_key : keyword_List)
			{
				if (!limbo_key.empty())
				{
					const auto key = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(limbo_key);

					if (key)
					{
						result.push_back(key);
					}
				}
			}

			return result;
		}

		template <class T>
		static std::vector<T *> get_valid_spellList(const std::vector<const RE::TESFile *> &exclude_modInfo_List, const std::vector<RE::BGSKeyword *> &exclude_keywords, bool whiteList_approach)
		{
			std::vector<T *> result;

			if (whiteList_approach)
			{
				if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler)
				{
					for (const auto &form : dataHandler->GetFormArray<T>())
					{
						if (!form)
						{
							continue;
						}
						bool valid = false;
						for (const auto a_modInfo : exclude_modInfo_List)
						{
							if (a_modInfo && a_modInfo->IsFormInMod(form->formID))
							{
								valid = true;
								break;
							}
						}
						if (form->HasKeywordInArray(exclude_keywords, false))
						{
							valid = true;
						}
						if (valid)
						{
							result.push_back(form);
						}
					}
				}
			}
			else
			{

				if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler)
				{
					for (const auto &form : dataHandler->GetFormArray<T>())
					{
						if (!form)
						{
							continue;
						}
						bool invalid = false;
						for (const auto a_modInfo : exclude_modInfo_List)
						{
							if (a_modInfo && a_modInfo->IsFormInMod(form->formID))
							{
								invalid = true;
								break;
							}
						}
						if (form->HasKeywordInArray(exclude_keywords, false))
						{
							invalid = true;
						}
						if (!invalid)
						{
							result.push_back(form);
						}
					}
				}
			}

			return result;
		}

		static void playSound(RE::Actor *a, RE::BGSSoundDescriptorForm *a_descriptor);

		inline auto ApplyHavokImpulse(RE::TESObjectREFR *self, float afX, float afY, float afZ, float afMagnitude)
		{
			using func_t = void(RE::BSScript::Internal::VirtualMachine *, RE::VMStackID, RE::TESObjectREFR *, float, float, float, float);
			RE::VMStackID frame = 0;

			REL::Relocation<func_t> func{RE::VTABLE_SkyrimScript____ApplyHavokImpulseFunctor[0]};
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();

			return func(vm, frame, self, afX, afY, afZ, afMagnitude);
		}

		inline auto AddItem(RE::TESObjectREFR *self, RE::TESForm* item, bool abSilent)
		{
			using func_t = void(RE::BSScript::Internal::VirtualMachine *, RE::VMStackID, RE::TESObjectREFR *, RE::TESForm *, bool);
			RE::VMStackID frame = 0;

			REL::Relocation<func_t> func{RE::VTABLE_SkyrimScript____AddItemFunctor[0]};
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();

			return func(vm, frame, self, item, abSilent);
		}

		static void SetForcedLandingMarker(RE::Actor *self, RE::TESObjectREFR *target)
		{
			using func_t = decltype(&SetForcedLandingMarker);
			REL::Relocation<func_t> func{RE::VTABLE_ExtraForcedLandingMarker[0]};
			return func(self, target);
		}

		static void RemoveFromFaction(RE::Actor *self, RE::TESFaction *a_faction)
		{
			using func_t = decltype(&RemoveFromFaction);
			REL::Relocation<func_t> func{RELOCATION_ID(36680, 37688)};
			return func(self, a_faction);
		}

		static bool PlayImpactEffect(RE::TESObjectREFR *a_ref, RE::BGSImpactDataSet *a_impactEffect, const RE::BSFixedString &a_nodeName, float afPickDirX, float afPickDirY, float afPickDirZ, float afPickLength, bool abApplyNodeRotation, bool abUseNodeLocalRotation)
		{
			using func_t = bool(RE::BSScript::Internal::VirtualMachine *, RE::VMStackID, RE::TESObjectREFR *, RE::BGSImpactDataSet * , const RE::BSFixedString &, float , float , float, float , bool, bool);
			RE::VMStackID frame = 0;

			REL::Relocation<func_t> func{RE::VTABLE_BSScript__NativeFunction8_TESObjectREFR_bool_BGSImpactDataSet___BSFixedString_const___float_float_float_float_bool_bool_[0]};
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();

			return func(vm, frame, a_ref, a_impactEffect, a_nodeName, afPickDirX, afPickDirY, afPickDirZ, afPickLength, abApplyNodeRotation, abUseNodeLocalRotation);
		}

		bool play_impact_1(RE::Actor *actor, const RE::BSFixedString &nodeName);
		bool play_impact_2(RE::TESObjectREFR *a, RE::BGSImpactData *impact, RE::NiPoint3 *P_V, RE::NiPoint3 *P_from, RE::NiNode *bone);
		bool play_impact_3(RE::TESObjectCELL *cell, float a_lifetime, const char *model, RE::NiPoint3 *a_rotation, RE::NiPoint3 *a_position, float a_scale, uint32_t a_flags, RE::NiNode *a_target);

		static void Call_Papyrus_Function(RE::Actor *a_actor, RE::BSFixedString scriptName, RE::BSFixedString functionName, RE::BSScript::IFunctionArguments *a_args)
		{
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();

			RE::TESForm *form = std::move(a_actor);

			auto policy = vm->GetObjectHandlePolicy();
			RE::VMHandle handle = policy->GetHandleForObject(form->GetFormType(), form);

			if (handle == policy->EmptyHandle())
			{
				return;
			}

			RE::BSTSmartPointer<RE::BSScript::Object> object;
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> result;

			if (vm->FindBoundObject(handle, scriptName.c_str(), object))
			{
				vm->DispatchMethodCall1(object, functionName, a_args, result);
			}
		}
		void Set_Handle(RE::Actor *actor, RE::BSSoundHandle &a_handle);
		void scan_activeHandles(RE::Actor *a_actor, RE::BSSoundHandle &a_handle, bool insert = false, bool clear = false, bool clear_all = false);
		std::pair<bool, RE::BSSoundHandle> Get_Handle(RE::Actor *a_actor);

	private:
		// GFunc() = default;
		// GFunc(const GFunc &) = delete;
		// GFunc(GFunc &&) = delete;
		// ~GFunc() = default;

		// GFunc &operator=(const GFunc &) = delete;
		// GFunc &operator=(GFunc &&) = delete;

		std::random_device rd;

		std::unordered_map<RE::Actor *, std::vector<RE::TESBoundObject *>> _Inventory;
		std::shared_mutex mtx_Inventory;
		std::unordered_map<RE::Actor *, std::vector<RE::BSSoundHandle>> _Handles;
		std::shared_mutex mtx_Handles;

		static int soundHelper_a(void *manager, RE::BSSoundHandle *a2, int a3, int a4) // sub_140BEEE70
		{
			using func_t = decltype(&soundHelper_a);
			REL::Relocation<func_t> func{RELOCATION_ID(66401, 67663)};
			return func(manager, a2, a3, a4);
		}

		static void soundHelper_b(RE::BSSoundHandle *a1, RE::NiAVObject *source_node) // sub_140BEDB10
		{
			using func_t = decltype(&soundHelper_b);
			REL::Relocation<func_t> func{RELOCATION_ID(66375, 67636)};
			return func(a1, source_node);
		}

		static char __fastcall soundHelper_c(RE::BSSoundHandle *a1) // sub_140BED530
		{
			using func_t = decltype(&soundHelper_c);
			REL::Relocation<func_t> func{RELOCATION_ID(66355, 67616)};
			return func(a1);
		}

		static char set_sound_position(RE::BSSoundHandle *a1, float x, float y, float z)
		{
			using func_t = decltype(&set_sound_position);
			REL::Relocation<func_t> func{RELOCATION_ID(66370, 67631)};
			return func(a1, x, y, z);
		}

	protected:
		GFunc() = default;
		GFunc(const GFunc &) = delete;
		GFunc(GFunc &&) = delete;
		~GFunc() = default;

		GFunc &operator=(const GFunc &) = delete;
		GFunc &operator=(GFunc &&) = delete;
	};
}

constexpr uint32_t hash(const char *data, size_t const size) noexcept
{
	uint32_t hash = 5381;

	for (const char *c = data; c < data + size; ++c)
	{
		hash = ((hash << 5) + hash) + (unsigned char)*c;
	}

	return hash;
}

constexpr uint32_t operator"" _h(const char *str, size_t size) noexcept
{
	return hash(str, size);
}

// static void ApplyHavokImpulse(RE::TESObjectREFR *self, float afX, float afY, float afZ, float afMagnitude)
// {
// 	using func_t = decltype(&ApplyHavokImpulse);
// 	REL::Relocation<func_t> func{RE::VTABLE_SkyrimScript____ApplyHavokImpulseFunctor[0]};
// 	return func(self, afX, afY, afZ, afMagnitude);
// }