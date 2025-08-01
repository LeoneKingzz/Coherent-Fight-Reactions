#include "lib/PrecisionAPI.h"
#include "Coding.h"
//using std::string;
static float& g_deltaTime = (*(float*)RELOCATION_ID(523660, 410199).address());

namespace Events_Space
{
	using uniqueLocker = std::unique_lock<std::shared_mutex>;
	using sharedLocker = std::shared_lock<std::shared_mutex>;

	using VM = RE::BSScript::Internal::VirtualMachine;
	using StackID = RE::VMStackID;
#define STATIC_ARGS [[maybe_unused]] VM *a_vm, [[maybe_unused]] StackID a_stackID, RE::StaticFunctionTag *

	using EventResult = RE::BSEventNotifyControl;

	template <typename T>
	T &Singleton()
	{
		static T single;
		return single;
	}

	class HitEventHandler
	{
		// friend EldenParry;
	public:
		[[nodiscard]] static HitEventHandler *GetSingleton()
		{
			static HitEventHandler singleton;
			return std::addressof(singleton);
		}

		static void InstallHooks()
		{
			Hooks::Physical_Install();
		}

		static void InstallMageHooks()
		{
			Hooks::Magic_Install();
		}

		bool PreProcessHit(RE::Actor *target, RE::HitData *hitData);
		bool PreProcessMagic(RE::Actor *target, RE::Actor *aggressor, RE::MagicTarget::AddTargetData *hitData);

	protected:
		struct Hooks
		{
			struct ProcessHitEvent
			{
				static void thunk(RE::Actor *target, RE::HitData *hitData)
				{
					auto handler = GetSingleton();
					if (handler->PreProcessHit(target, hitData))
					{
						return;
					}
					return func(target, hitData);
				}
				static inline REL::Relocation<decltype(thunk)> func;
			};

			static void Physical_Install()
			{
				stl::write_thunk_call<ProcessHitEvent>(REL::RelocationID(37673, 38627).address() + REL::Relocate(0x3C0, 0x4A8, 0x3C0)); // 1.5.97 140628C20
			}

			struct MagicTargetApply
			{
				static bool thunk(RE::MagicTarget *a_this, RE::MagicTarget::AddTargetData *a_data)
				{
					auto handler = GetSingleton();
					if (auto target = a_this && a_data ? a_this->GetTargetStatsObject() : nullptr; target)
					{
						if (target->Is(RE::FormType::ActorCharacter) && a_data->caster && a_data->caster->Is(RE::FormType::ActorCharacter))
						{
							if (handler->PreProcessMagic(target->As<RE::Actor>(), a_data->caster->As<RE::Actor>(), a_data))
							{
								return func(a_this, a_data);
							}
						}
					}

					return func(a_this, a_data);
				}

				static inline REL::Relocation<decltype(thunk)> func;
			};

			static void Magic_Install()
			{
				REL::Relocation<std::uintptr_t> target{RELOCATION_ID(33742, 34526), OFFSET(0x1E8, 0x20B)};
				stl::write_thunk_call<MagicTargetApply>(target.address());

				logger::info("Hooked Magic Effect Apply");
			}
		};

	private:
		// static void PoiseCallback_Post(const PRECISION_API::PrecisionHitData& a_precisionHitData, const RE::HitData& hitData);
		constexpr HitEventHandler() noexcept = default;
		HitEventHandler(const HitEventHandler &) = delete;
		HitEventHandler(HitEventHandler &&) = delete;

		~HitEventHandler() = default;

		HitEventHandler &operator=(const HitEventHandler &) = delete;
		HitEventHandler &operator=(HitEventHandler &&) = delete;
	};

	class animEventHandler
	{
	private:
		template <class Ty>
		static Ty SafeWrite64Function(uintptr_t addr, Ty data)
		{
			DWORD oldProtect;
			void* _d[2];
			memcpy(_d, &data, sizeof(data));
			size_t len = sizeof(_d[0]);

			VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
			Ty olddata;
			memset(&olddata, 0, sizeof(Ty));
			memcpy(&olddata, (void*)addr, len);
			memcpy((void*)addr, &_d[0], len);
			VirtualProtect((void*)addr, len, oldProtect, &oldProtect);
			return olddata;
		}

		typedef RE::BSEventNotifyControl (animEventHandler::*FnProcessEvent)(RE::BSAnimationGraphEvent& a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* dispatcher);

		RE::BSEventNotifyControl HookedProcessEvent(RE::BSAnimationGraphEvent& a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* src);

		static void HookSink(uintptr_t ptr)
		{
			FnProcessEvent fn = SafeWrite64Function(ptr + 0x8, &animEventHandler::HookedProcessEvent);
			fnHash.insert(std::pair<uint64_t, FnProcessEvent>(ptr, fn));
		}

	public:
		static animEventHandler* GetSingleton()
		{
			static animEventHandler singleton;
			return &singleton;
		}

		/*Hook anim event sink*/
		static void Register(bool player, bool NPC)
		{
			if (player) {
				logger::info("Sinking animation event hook for player");
				REL::Relocation<uintptr_t> pcPtr{ RE::VTABLE_PlayerCharacter[2] };
				HookSink(pcPtr.address());
			}
			if (NPC) {
				logger::info("Sinking animation event hook for NPC");
				REL::Relocation<uintptr_t> npcPtr{ RE::VTABLE_Character[2] };
				HookSink(npcPtr.address());
			}
			logger::info("Sinking complete.");
		}

		static void RegisterForPlayer()
		{
			Register(true, false);
		}

	protected:
		static std::unordered_map<uint64_t, FnProcessEvent> fnHash;
	};

	class Events
	{
	public:

		static Events* GetSingleton()
		{
			static Events avInterface;
			return &avInterface;
		}

		static void install();
		static void install_pluginListener();
		static void install_protected(){
			Install_Update();
		}
		void init();

		static bool BindPapyrusFunctions(VM* vm);
		void Update(RE::Actor* a_actor, float a_delta);
		void Process_Updates(RE::Actor *a_actor, std::chrono::steady_clock::time_point time_now);
		void RegisterforUpdate(RE::Actor *a_actor, std::tuple<bool, std::chrono::steady_clock::time_point, GFunc_Space::ms, std::string> data);

		static void RemoveFromFaction(RE::Actor *a_actor, RE::TESFaction *a_faction)
		{
			using func_t = decltype(&RemoveFromFaction);
			REL::Relocation<func_t> func{REL::RelocationID(36680, 37688)};
			func(a_actor, a_faction);
		};

		static RE::FIGHT_REACTION GetFactionReaction(RE::Actor *a_subject, RE::Actor *a_other)
		{
			using func_t = decltype(&GetFactionReaction);
			static REL::Relocation<func_t> func{RELOCATION_ID(36658, 37666)};
			return func(a_subject, a_other);
		};

	private:
		Events() = default;
		Events(const Events&) = delete;
		Events(Events&&) = delete;
		~Events() = default;

		Events& operator=(const Events&) = delete;
		Events& operator=(Events&&) = delete;

		PRECISION_API::IVPrecision1* _precision_API;
		static void PrecisionWeaponsCallback_Post(const PRECISION_API::PrecisionHitData& a_precisionHitData, const RE::HitData& a_hitdata);

	protected:

		struct Actor_Update
		{
			static void thunk(RE::Actor* a_actor, float a_delta)
			{
				func(a_actor, a_delta);
				GetSingleton()->Update(a_actor, g_deltaTime);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install_Update(){
			stl::write_vfunc<RE::Character, 0xAD, Actor_Update>();
		}

		std::unordered_map<RE::Actor *, std::vector<std::tuple<bool, std::chrono::steady_clock::time_point, std::chrono::milliseconds, std::string>>> _Timer;
		std::shared_mutex mtx_Timer;
	};


	class Settings
	{
	public:
		static Settings* GetSingleton()
		{
			static Settings avInterface;
			return &avInterface;
		}

		void Load();

		struct General_Settings
		{
			void Load(CSimpleIniA &a_ini);
			bool bWhiteListApproach = false;

		} general;

		struct Exclude_AllSpells_inMods
		{
			void Load(CSimpleIniA& a_ini);

			std::string exc_mods_joined = "Heroes of Yore.esp|VampireLordSeranaAssets.esp|VampireLordSerana.esp|TheBeastWithin.esp|TheBeastWithinHowls.esp";

			std::vector<std::string> exc_mods;

		} exclude_spells_mods;

		struct Exclude_AllSpells_withKeywords
		{
			void Load(CSimpleIniA& a_ini);
			std::string exc_keywords_joined = "HoY_MagicShoutSpell|LDP_MagicShoutSpell|NSV_CActorSpell_Exclude";

			std::vector<std::string> exc_keywords;

		} exclude_spells_keywords;

		struct Include_AllSpells_inMods
		{
			void Load(CSimpleIniA &a_ini);

			std::string inc_mods_joined = "Skyrim.esm|Dawnguard.esm|Dragonborn.esm";

			std::vector<std::string> inc_mods;

		} include_spells_mods;

		struct Include_AllSpells_withKeywords
		{
			void Load(CSimpleIniA &a_ini);
			std::string inc_keywords_joined = "DummyKey|ImposterKey";

			std::vector<std::string> inc_keywords;

		} include_spells_keywords;

	private:
		Settings() = default;
		Settings(const Settings&) = delete;
		Settings(Settings&&) = delete;
		~Settings() = default;

		Settings& operator=(const Settings&) = delete;
		Settings& operator=(Settings&&) = delete;
	};
};
