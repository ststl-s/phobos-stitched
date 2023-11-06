#pragma once

// include this file whenever something is to be saved.

#include "Savegame.h"

#include <vector>
#include <deque>
#include <set>
#include <map>
#include <unordered_map>
#include <bitset>
#include <memory>

#include <ArrayClasses.h>
#include <FileSystem.h>
#include <FileFormats/SHP.h>
#include <RulesClass.h>
#include <SidebarClass.h>
#include <ScenarioClass.h>

#include <Utilities/Constructs.h>

#include "Swizzle.h"
#include "Debug.h"

namespace Savegame
{
	template <typename T>
	concept ImplementsUpperCaseSaveLoad = requires (PhobosStreamWriter & stmWriter, PhobosStreamReader & stmReader, T & value, bool registerForChange)
	{
		value.Save(stmWriter);
		value.Load(stmReader, registerForChange);
	};

	template <typename T>
	concept ImplementsLowerCaseSaveLoad = requires (PhobosStreamWriter & stmWriter, PhobosStreamReader & stmReader, T & value, bool registerForChange)
	{
		value.save(stmWriter);
		value.load(stmReader, registerForChange);
	};

#pragma warning(push)
#pragma warning(disable: 4702) // MSVC isn't smart enough and yells about unreachable code

	template <typename T>
	bool ReadPhobosStream(PhobosStreamReader& stm, T& value, bool registerForChange)
	{
		if constexpr (ImplementsUpperCaseSaveLoad<T>)
			return value.Load(stm, registerForChange);

		else if constexpr (ImplementsLowerCaseSaveLoad<T>)
			return value.load(stm, registerForChange);

		PhobosStreamObject<T> item;
		return item.ReadFromStream(stm, value, registerForChange);
	}

	template <typename T>
	bool WritePhobosStream(PhobosStreamWriter& stm, const T& value)
	{
		if constexpr (ImplementsUpperCaseSaveLoad<T>)
			return value.Save(stm);

		if constexpr (ImplementsLowerCaseSaveLoad<T>)
			return value.save(stm);

		PhobosStreamObject<T> item;
		return item.WriteToStream(stm, value);
	}

#pragma warning(pop)


	template <typename T>
	T* RestoreObject(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		T* ptrOld = nullptr;
		if (!Stm.Load(ptrOld))
			return nullptr;

		if (ptrOld)
		{
			std::unique_ptr<T> ptrNew = ObjectFactory<T>()(Stm);

			if (Savegame::ReadPhobosStream(Stm, *ptrNew, RegisterForChange))
			{
				PhobosSwizzle::Instance.RegisterChange(ptrOld, ptrNew.get());
				return ptrNew.release();
			}
		}

		return nullptr;
	}

	template <typename T>
	bool PersistObject(PhobosStreamWriter& Stm, const T* pValue)
	{
		if (!Savegame::WritePhobosStream(Stm, pValue))
			return false;

		if (pValue)
			return Savegame::WritePhobosStream(Stm, *pValue);

		return true;
	}

	template <typename T>
	bool PhobosStreamObject<T>::ReadFromStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange) const
	{
		bool ret = Stm.Load(Value);

		if (RegisterForChange)
			Swizzle swizzle(Value);

		return ret;
	}

	template <typename T>
	bool PhobosStreamObject<T>::WriteToStream(PhobosStreamWriter& Stm, const T& Value) const
	{
		Stm.Save(Value);
		return true;
	}


	// specializations

	template <typename T>
	struct Savegame::PhobosStreamObject<VectorClass<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, VectorClass<T>& Value, bool RegisterForChange) const
		{
			Value.Clear();
			int Capacity = 0;

			if (!Stm.Load(Capacity))
				return false;

			Value.Reserve(Capacity);

			for (auto ix = 0; ix < Capacity; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value.Items[ix], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const VectorClass<T>& Value) const
		{
			Stm.Save(Value.Capacity);

			for (auto ix = 0; ix < Value.Capacity; ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value.Items[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<DynamicVectorClass<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, DynamicVectorClass<T>& Value, bool RegisterForChange) const
		{
			Value.Clear();
			int Capacity = 0;

			if (!Stm.Load(Capacity))
				return false;

			Value.Reserve(Capacity);

			if (!Stm.Load(Value.Count) || !Stm.Load(Value.CapacityIncrement))
				return false;

			for (auto ix = 0; ix < Value.Count; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value.Items[ix], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const DynamicVectorClass<T>& Value) const
		{
			Stm.Save(Value.Capacity);
			Stm.Save(Value.Count);
			Stm.Save(Value.CapacityIncrement);

			for (auto ix = 0; ix < Value.Count; ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value.Items[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<TypeList<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, TypeList<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream<DynamicVectorClass<T>>(Stm, Value, RegisterForChange))
				return false;

			return Stm.Load(Value.unknown_18);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const TypeList<T>& Value) const
		{
			if (!Savegame::WritePhobosStream<DynamicVectorClass<T>>(Stm, Value))
				return false;

			Stm.Save(Value.unknown_18);
			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<Vector3D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, Vector3D<T>& Value, bool RegisterForChange) const
		{
			if (!(Savegame::ReadPhobosStream<int>(Stm, Value.X, RegisterForChange)
				&& Savegame::ReadPhobosStream<int>(Stm, Value.Y, RegisterForChange)
				&& Savegame::ReadPhobosStream<int>(Stm, Value.Z, RegisterForChange)))
				return false;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const Vector3D<T>& Value) const
		{
			if (!(Savegame::WritePhobosStream(Stm, Value.X)
				&& Savegame::WritePhobosStream(Stm, Value.Y)
				&& Savegame::WritePhobosStream(Stm, Value.Z)))
				return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<TintStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, TintStruct& Value, bool RegisterForChange) const
		{
			if (!(Savegame::ReadPhobosStream<int>(Stm, Value.Red, RegisterForChange)
				&& Savegame::ReadPhobosStream<int>(Stm, Value.Green, RegisterForChange)
				&& Savegame::ReadPhobosStream<int>(Stm, Value.Blue, RegisterForChange)))
				return false;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const TintStruct& Value) const
		{
			if (!(Savegame::WritePhobosStream(Stm, Value.Red)
				&& Savegame::WritePhobosStream(Stm, Value.Green)
				&& Savegame::WritePhobosStream(Stm, Value.Blue)))
				return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<LightingStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, LightingStruct& Value, bool RegisterForChange) const
		{
			PhobosStreamObject<TintStruct> item;
			if (!(item.ReadFromStream(Stm, Value.Tint, RegisterForChange)
				&& Savegame::ReadPhobosStream<int>(Stm, Value.Ground, RegisterForChange)
				&& Savegame::ReadPhobosStream<int>(Stm, Value.Level, RegisterForChange)))
				return false;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const LightingStruct& Value) const
		{
			PhobosStreamObject<TintStruct> item;

			if (!(item.WriteToStream(Stm, Value.Tint)
				&& Savegame::WritePhobosStream<int>(Stm, Value.Ground)
				&& Savegame::WritePhobosStream<int>(Stm, Value.Level)))
				return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<CounterClass>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, CounterClass& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream<VectorClass<int>>(Stm, Value, RegisterForChange))
				return false;

			return Stm.Load(Value.Total);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const CounterClass& Value) const
		{
			if (!Savegame::WritePhobosStream<VectorClass<int>>(Stm, Value))
				return false;

			Stm.Save(Value.Total);
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<WeaponStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, WeaponStruct& Value, bool RegisterForChange) const
		{
			return Savegame::ReadPhobosStream(Stm, Value.WeaponType, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.FLH, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.BarrelLength, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.BarrelThickness, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.TurretLocked, RegisterForChange);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const WeaponStruct& Value) const
		{
			return Savegame::WritePhobosStream(Stm, Value.WeaponType)
				&& Savegame::WritePhobosStream(Stm, Value.FLH)
				&& Savegame::WritePhobosStream(Stm, Value.BarrelLength)
				&& Savegame::WritePhobosStream(Stm, Value.BarrelThickness)
				&& Savegame::WritePhobosStream(Stm, Value.TurretLocked);
		}
	};

	template <size_t Size>
	struct Savegame::PhobosStreamObject<std::bitset<Size>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::bitset<Size>& Value, bool RegisterForChange) const
		{
			unsigned char value = 0;
			for (auto i = 0u; i < Size; ++i)
			{
				auto pos = i % 8;

				if (pos == 0 && !Stm.Load(value))
					return false;

				Value.set(i, ((value >> pos) & 1) != 0);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::bitset<Size>& Value) const
		{
			unsigned char value = 0;
			for (auto i = 0u; i < Size; ++i)
			{
				auto pos = i % 8;

				if (Value[i])
					value |= 1 << pos;

				if (pos == 7 || i == Size - 1)
				{
					Stm.Save(value);
					value = 0;
				}
			}

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<std::string>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::string& Value, bool RegisterForChange) const
		{
			size_t length = 0;

			if (Stm.Load(length))
			{
				Value.resize(length);
				bool success = true;

				for (char& c : Value)
				{
					success &= Stm.Load(c);
				}

				return success;
			}
			return false;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::string& Value) const
		{
			Stm.Save(Value.length());

			for (char c : Value)
			{
				Stm.Save(c);
			}

			return true;
		}
	};

	template <typename _Ty, typename _Deleter>
	struct Savegame::PhobosStreamObject<std::unique_ptr<_Ty, _Deleter>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::unique_ptr<_Ty, _Deleter>& Value, bool RegisterForChange) const
		{
			Value.reset(RestoreObject<_Ty>(Stm, RegisterForChange));
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::unique_ptr<_Ty, _Deleter>& Value) const
		{
			return PersistObject(Stm, Value.get());
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::vector<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::vector<T>& Value, bool RegisterForChange) const
		{
			Value.clear();

			size_t Capacity = 0;

			if (!Stm.Load(Capacity))
				return false;

			Value.reserve(Capacity);

			size_t Count = 0;

			if (!Stm.Load(Count))
				return false;

			Value.resize(Count);

			for (auto ix = 0u; ix < Count; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value[ix], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::vector<T>& Value) const
		{
			Stm.Save(Value.capacity());
			Stm.Save(Value.size());

			for (auto ix = 0u; ix < Value.size(); ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value[ix]))
					return false;
			}

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<std::vector<bool>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::vector<bool>& Value, bool RegisterForChange) const
		{
			Value.clear();

			size_t Count = 0;

			if (!Stm.Load(Count))
				return false;

			for (size_t i = 0; i < Count; i++)
			{
				bool item;

				if (!Savegame::ReadPhobosStream(Stm, item, false))
					return false;

				Value.emplace_back(item);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::vector<bool>& Value) const
		{
			Stm.Save(Value.size());

			for (bool item : Value)
			{
				if (!Savegame::WritePhobosStream(Stm, item))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::deque<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::deque<T>& Value, bool RegisterForChange) const
		{
			Value.clear();

			size_t count = 0;

			if (!Stm.Load(count))
				return false;

			Value.resize(count);

			for (auto ix = 0u; ix < count; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value[ix], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::deque<T>& Value) const
		{
			Stm.Save(Value.size());

			for (auto ix = 0u; ix < Value.size(); ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename _Kty, typename _Pr>
	struct Savegame::PhobosStreamObject<std::set<_Kty, _Pr>>
	{
		static void is_pointer(std::true_type) = delete;

		static void is_pointer(std::false_type) { }

		bool ReadFromStream(PhobosStreamReader& Stm, std::set<_Kty, _Pr>& Value, bool RegisterForChange) const
		{
			// use pointer in set is unswizzleable
			is_pointer(typename std::is_pointer<_Kty>::type());

			Value.clear();

			size_t Size = 0;

			if (!Stm.Load(Size))
				return false;

			for (auto ix = 0u; ix < Size; ++ix)
			{
				_Kty buffer = _Kty();
				if (!Savegame::ReadPhobosStream(Stm, buffer, false))
				{
					return false;
				}
				Value.emplace(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::set<_Kty, _Pr>& Value) const
		{
			// use pointer in set is unswizzleable
			is_pointer(typename std::is_pointer<_Kty>::type());

			Stm.Save(Value.size());

			for (const auto& item : Value)
			{
				if (!Savegame::WritePhobosStream(Stm, item))
					return false;
			}

			return true;
		}
	};

	template <typename _Ty1, typename _Ty2>
	struct Savegame::PhobosStreamObject<std::pair<_Ty1, _Ty2>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::pair<_Ty1, _Ty2>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.first, RegisterForChange)
				|| !Savegame::ReadPhobosStream(Stm, Value.second, RegisterForChange))
			{
				return false;
			}
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::pair<_Ty1, _Ty2>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.first)
				|| !Savegame::WritePhobosStream(Stm, Value.second))
			{
				return false;
			}
			return true;
		}
	};

	template <typename TKey, typename TValue, typename Cmp>
	struct Savegame::PhobosStreamObject<std::map<TKey, TValue, Cmp>>
	{
		inline static void is_pointer(std::true_type) = delete;

		inline static void is_pointer(std::false_type) { }

		bool ReadFromStream(PhobosStreamReader& Stm, std::map<TKey, TValue, Cmp>& Value, bool RegisterForChange) const
		{
			Value.clear();

			// use pointer as key of map is unswizzleable
			is_pointer(typename std::is_pointer<TKey>::type());

			size_t Count = 0;
			if (!Stm.Load(Count))
			{
				return false;
			}

			for (auto ix = 0u; ix < Count; ++ix)
			{
				TKey key = TKey();

				if (!Savegame::ReadPhobosStream(Stm, key, false))
					return false;

				Value.emplace(key, TValue());

				if (!Savegame::ReadPhobosStream(Stm, Value[key], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::map<TKey, TValue, Cmp>& Value) const
		{
			// use pointer as key of map is unswizzleable
			is_pointer(typename std::is_pointer<TKey>::type());

			Stm.Save(Value.size());

			for (const auto& item : Value)
			{
				if (!Savegame::WritePhobosStream(Stm, item.first)
					|| !Savegame::WritePhobosStream(Stm, item.second))
				{
					return false;
				}
			}
			return true;
		}
	};

	template <typename TKey, typename TValue>
	struct Savegame::PhobosStreamObject<std::unordered_map<TKey, TValue>>
	{
		inline static void is_pointer(std::true_type) = delete;

		inline static void is_pointer(std::false_type) { }

		bool ReadFromStream(PhobosStreamReader& Stm, std::unordered_map<TKey, TValue>& Value, bool RegisterForChange) const
		{
			Value.clear();

			// use pointer as key of map is unswizzleable
			is_pointer(typename std::is_pointer<TKey>::type());

			size_t Count = 0;
			if (!Stm.Load(Count))
			{
				return false;
			}

			for (auto ix = 0u; ix < Count; ++ix)
			{
				TKey key = TKey();

				if (!Savegame::ReadPhobosStream(Stm, key, false))
					return false;

				Value.emplace(key, TValue());
			}

			for (auto& item : Value)
			{
				if (!Savegame::ReadPhobosStream(Stm, item.second, RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::unordered_map<TKey, TValue>& Value) const
		{
			// use pointer as key of map is unswizzleable
			is_pointer(typename std::is_pointer<TKey>::type());

			Stm.Save(Value.size());

			for (const auto& item : Value)
			{
				if (!Savegame::WritePhobosStream(Stm, item.first))
					return false;
			}

			for (const auto& item : Value)
			{
				if (!Savegame::WritePhobosStream(Stm, item.second))
					return false;
			}

			return true;
		}
	};

	template <typename TKey, typename TValue, typename Cmp>
	struct Savegame::PhobosStreamObject<std::multimap<TKey, TValue, Cmp>>
	{
		static void is_pointer(std::true_type) = delete;

		static void is_pointer(std::false_type) { }

		bool ReadFromStream(PhobosStreamReader& Stm, std::multimap<TKey, TValue, Cmp>& Value, bool RegisterForChange) const
		{
			Value.clear();

			// use pointer as key of map is unswizzleable
			is_pointer(typename std::is_pointer<TKey>::type());

			size_t Count = 0;
			if (!Stm.Load(Count))
			{
				return false;
			}

			for (auto ix = 0u; ix < Count; ++ix)
			{
				TKey key = TKey();

				if (!Savegame::ReadPhobosStream(Stm, key, false))
					return false;

				Value.emplace(key, TValue());

				if (!Savegame::ReadPhobosStream(Stm, Value.rbegin()->second, RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::multimap<TKey, TValue, Cmp>& Value) const
		{
			// use pointer as key of map is unswizzleable
			is_pointer(typename std::is_pointer<TKey>::type());

			Stm.Save(Value.size());

			for (const auto& item : Value)
			{
				if (!Savegame::WritePhobosStream(Stm, item.first)
					|| !Savegame::WritePhobosStream(Stm, item.second))
				{
					return false;
				}
			}
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<SHPStruct*>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, SHPStruct*& Value, bool RegisterForChange) const
		{
			if (Value && !Value->IsReference())
				Debug::Log("Value contains SHP file data. Possible leak.\n");

			Value = nullptr;

			bool hasValue = true;
			if (Savegame::ReadPhobosStream(Stm, hasValue) && hasValue)
			{
				std::string name;
				if (Savegame::ReadPhobosStream(Stm, name))
				{
					if (auto pSHP = FileSystem::LoadSHPFile(name.c_str()))
					{
						Value = pSHP;
						return true;
					}
				}
			}

			return !hasValue;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, SHPStruct* const& Value) const
		{
			const char* filename = nullptr;
			if (Value)
			{
				if (auto pRef = Value->AsReference())
					filename = pRef->Filename;
				else
					Debug::Log("Cannot save SHPStruct, because it isn't a reference.\n");
			}

			if (Savegame::WritePhobosStream(Stm, filename != nullptr))
			{
				if (filename)
				{
					std::string file(filename);
					return Savegame::WritePhobosStream(Stm, file);
				}
			}

			return filename == nullptr;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<RocketStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, RocketStruct& Value, bool RegisterForChange) const
		{
			if (!Stm.Load(Value))
				return false;

			if (RegisterForChange)
				Swizzle swizzle(Value.Type);

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const RocketStruct& Value) const
		{
			Stm.Save(Value);
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<BuildType>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, BuildType& Value, bool RegisterForChange) const
		{
			if (!Stm.Load(Value))
				return false;

			if (RegisterForChange)
				Swizzle swizzle(Value.CurrentFactory);

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const BuildType& Value) const
		{
			Stm.Save(Value);
			return true;
		}
	};
}
