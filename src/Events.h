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

	using tActor_GetCombatState = RE::ACTOR_COMBAT_STATE (*)(RE::Actor *a_this);
	static REL::Relocation<tActor_GetCombatState> Actor_GetCombatState{REL::VariantID(37603, 38556, 0x62DD00)}; // 624E90, 64A520, 62DD00

	template <typename T>
	T &Singleton()
	{
		static T single;
		return single;
	}

	void Install_apply();

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

		bool PreProcessHit(RE::Actor *target, RE::HitData *hitData);
		bool PreProcessMagic(RE::Actor *target, RE::Actor *aggressor, RE::Effect * a_effect);
		bool PreProcessExplosion(RE::Actor *target, RE::Actor *blameActor);
		bool PreProcessResolve(RE::HitData *a_hitData, bool a_ignoreBlocking);

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

			struct ProcessHitResolve
			{
				static bool thunk(RE::HitData *a_hitData, bool a_ignoreBlocking)
				{
					auto handler = GetSingleton();
					if (handler->PreProcessResolve(a_hitData, a_ignoreBlocking))
					{
						// return false;
						return func(a_hitData, a_ignoreBlocking);
					}
					return func(a_hitData, a_ignoreBlocking);
				}
				static inline REL::Relocation<decltype(thunk)> func;
			};


			static void Physical_Install()
			{
				stl::write_thunk_call<ProcessHitEvent>(REL::RelocationID(37673, 38627).address() + REL::Relocate(0x3C0, 0x4A8, 0x3C0)); // 1.5.97 140628C20

				// stl::write_thunk_call<ProcessHitResolve>(REL::RelocationID(42832, 44001).address() + REL::Relocate(0x37C, 0x358, 0x3CF));
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

		static PRECISION_API::CollisionFilterComparisonResult Add_Collision_Filter(RE::bhkCollisionFilter *a_collisionFilter, uint32_t a_filterInfoA, uint32_t a_filterInfoB);

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

		struct PlayerCharacter_Update
		{
			static void thunk(RE::PlayerCharacter *a_player, float a_delta)
			{
				func(a_player, a_delta);
				GetSingleton()->Update(a_player, g_deltaTime);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install_Update(){
			stl::write_vfunc<RE::Character, 0xAD, Actor_Update>();
			stl::write_vfunc<RE::PlayerCharacter, 0xAD, PlayerCharacter_Update>();
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
			bool bDebugMode = false;
			bool bDynamicToggle = true;

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

	class MagicApplyHandler
	{
	private:
		struct Character
		{
			static bool Thunk(RE::MagicTarget *a_this, RE::MagicTarget::AddTargetData *a_data);

			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

		struct Player
		{
			static bool Thunk(RE::MagicTarget *a_this, RE::MagicTarget::AddTargetData *a_data);

			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

	public:
		static MagicApplyHandler *GetSingleton()
		{
			static MagicApplyHandler singleton;
			return &singleton;
		}

		/*Hook magic apply sink*/
		static void Register(bool player, bool NPC)
		{
			if (player)
			{
				logger::info("Sinking magic apply hook for player");
				REL::Relocation<uintptr_t> pcPtr{RE::VTABLE_PlayerCharacter[4]};
				Player::_func = pcPtr.write_vfunc(0x1, Player::Thunk);
			}
			if (NPC)
			{
				logger::info("Sinking magic apply hook for NPC");
				REL::Relocation<uintptr_t> npcPtr{RE::VTABLE_Character[4]};
				Character::_func = npcPtr.write_vfunc(0x1, Character::Thunk);
			}
			logger::info("Sinking complete.");
		}

		static void RegisterForPlayer()
		{
			Register(true, false);
		}

	protected:
		
	};

	class ExplosionCollision
	{
	private:

	public:
		static ExplosionCollision *GetSingleton()
		{
			static ExplosionCollision singleton;
			return &singleton;
		}

		
		RE::FIGHT_REACTION Process_Hit(RE::Actor *a_subject, RE::Actor *a_target, RE::FIGHT_REACTION a_reaction);
		bool Process_HitHandle(RE::TESObjectREFR *a_target, RE::TESObjectREFR *a_source, RE::HitData *a_hitData);
		bool Analyse_Hits(RE::hkpAllCdPointCollector *a_AllCdPointCollector);
		bool Analyse_Hits1(RE::TESObjectREFR *a_source, RE::TESObjectREFR *a_target);

		/*Hook Explosion sink*/
		static void Register()
		{
			Install();
		}

	protected:
		struct ExplosionHandler
		{
			static void thunk(RE::ActorMagicCaster* a_this, RE::MagicItem *a_spell, bool a_noHitEffectArt, RE::TESObjectREFR *a_target, float a_effectiveness, bool a_hostileEffectivenessOnly, float a_magnitudeOverride, RE::Actor *a_blameActor)
			{
				
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct GetFactionFightReaction
		{
			static RE::FIGHT_REACTION thunk(RE::Actor *a_subject, RE::Actor *a_player)
			{
				const auto fightReaction = func(a_subject, a_player);
				if (a_subject && a_player)
				{
					return GetSingleton()->Process_Hit(a_subject, a_player, fightReaction);
				}
				return fightReaction;
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct HitHandle1{

			static void thunk(RE::HitData *a_this, RE::TESObjectREFR *a_source, RE::TESObjectREFR *a_target, RE::InventoryEntryData *a_weapon, bool a_bIsOffhand)
			{
				if (GetSingleton()->Process_HitHandle(a_target, a_source, a_this))
				{
					return;
				}
				return func(a_this, a_source, a_target, a_weapon, a_bIsOffhand);
			}
			static inline REL::Relocation<decltype(thunk)> func;
			
		};

		struct HitHandle2{

			static void thunk(RE::HitData *a_this, RE::TESObjectREFR *a_source, RE::TESObjectREFR *a_target, RE::InventoryEntryData *a_weapon, bool a_bIsOffhand)
			{
				if (GetSingleton()->Process_HitHandle(a_target, a_source, a_this))
				{
					return;
				}
				return func(a_this, a_source, a_target, a_weapon, a_bIsOffhand);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct HitHandle3{

			static bool thunk(RE::hkpAllCdPointCollector *a_collector, RE::bhkWorld *a_world, RE::NiPoint3 &a_origin, RE::NiPoint3 &a_direction, float a_length)
			{
				if (GetSingleton()->Analyse_Hits(a_collector))
				{
					return false;
				}
				return func(a_collector, a_world, a_origin, a_direction, a_length);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			// stl::write_vfunc<RE::ActorMagicCaster, 0x1, ExplosionHandler>();

			// REL::Relocation<std::uintptr_t> target{RELOCATION_ID(36658, 37666), OFFSET(0x130, 0x120)};
			// stl::write_thunk_call<GetFactionFightReaction>(target.address());

			// REL::Relocation<std::uintptr_t> hook2{RELOCATION_ID(37673, 38627), OFFSET(0x1B7, 0x1C6)};
			// stl::write_thunk_call<HitHandle1>(hook2.address());

			// REL::Relocation<std::uintptr_t> hook3{RELOCATION_ID(37674, 38628), OFFSET(0xEB, 0x110)};
			// stl::write_thunk_call<HitHandle2>(hook3.address());

			// REL::Relocation<std::uintptr_t> hook4{RELOCATION_ID(37674, 38628), OFFSET(0x26A, 0x294)};
			// stl::write_thunk_call<HitHandle3>(hook4.address());
		}
	};

	class CastingHandler
	{
	private:
		struct Character
		{
			static void Thunk(RE::ActorMagicCaster *a_this, RE::MagicItem *a_spell, bool a_noHitEffectArt, RE::TESObjectREFR *a_target, float a_effectiveness, bool a_hostileEffectivenessOnly, float a_magnitudeOverride, RE::Actor *a_blameActor)
			{
				if (GetSingleton()->Analyse(a_this, a_spell, a_target, a_hostileEffectivenessOnly, a_blameActor))
				{
					return func(a_this, a_spell, a_noHitEffectArt, a_target, a_effectiveness, true, a_magnitudeOverride, a_blameActor);
				}
				return func(a_this, a_spell, a_noHitEffectArt, a_target, a_effectiveness, a_hostileEffectivenessOnly, a_magnitudeOverride, a_blameActor);
			}

			inline static REL::Relocation<decltype(&Thunk)> func;
		};

		struct Player
		{
			static void Thunk(RE::ActorMagicCaster *a_this, RE::MagicItem *a_spell, bool a_noHitEffectArt, RE::TESObjectREFR *a_target, float a_effectiveness, bool a_hostileEffectivenessOnly, float a_magnitudeOverride, RE::Actor *a_blameActor)
			{
				if (GetSingleton()->Analyse(a_this, a_spell, a_target, a_hostileEffectivenessOnly, a_blameActor))
				{
					return func(a_this, a_spell, a_noHitEffectArt, a_target, a_effectiveness, true, a_magnitudeOverride, a_blameActor);
				}
				return func(a_this, a_spell, a_noHitEffectArt, a_target, a_effectiveness, a_hostileEffectivenessOnly, a_magnitudeOverride, a_blameActor);
			}

			inline static REL::Relocation<decltype(&Thunk)> func;
		};

	public:
		static CastingHandler *GetSingleton()
		{
			static CastingHandler singleton;
			return &singleton;
		}

		bool Analyse(RE::ActorMagicCaster *a_this, RE::MagicItem *a_spell, RE::TESObjectREFR *a_target, bool a_hostileEffectivenessOnly, RE::Actor *a_blameActor);

		/*Hook magic apply sink*/
		static void Register(bool player, bool NPC)
		{
			if (player)
			{
				logger::info("Sinking casting hook for player");
				REL::Relocation<uintptr_t> pcPtr{RE::VTABLE_PlayerCharacter[3]};
				Player::func = pcPtr.write_vfunc(0x1, Player::Thunk);
			}
			if (NPC)
			{
				logger::info("Sinking casting hook for NPC");
				REL::Relocation<uintptr_t> npcPtr{RE::VTABLE_Character[3]};
				Character::func = npcPtr.write_vfunc(0x1, Character::Thunk);
			}
			logger::info("Sinking complete.");
		}

		static void RegisterForPlayer()
		{
			Register(true, false);
		}

	protected:
		
	};
};
