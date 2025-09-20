#include "GeneralFunctions.h"

namespace Coding_Space{
	using uniqueLocker = std::unique_lock<std::shared_mutex>;
	using sharedLocker = std::shared_lock<std::shared_mutex>;

	class Coding
	{
	public:
		static Coding *GetSingleton()
		{
			static Coding avInterface;
			return &avInterface;
		}

	private:
		Coding() = default;
		Coding(const Coding &) = delete;
		Coding(Coding &&) = delete;
		~Coding() = default;

		Coding &operator=(const Coding &) = delete;
		Coding &operator=(Coding &&) = delete;

		std::random_device rd;

	protected:
		std::unordered_map<RE::Actor *, std::tuple<int, std::vector<int>, std::vector<int>, std::vector<int>>> _attackList;
		std::shared_mutex mtx_attackList;
		std::unordered_map<RE::Actor *, std::vector<RE::TESObjectREFR *>> _Boxes;
		std::shared_mutex mtx_Boxes;
	};
}


