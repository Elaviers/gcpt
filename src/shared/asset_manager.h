#pragma once
#include "asset.h"
#include "debug.h"
#include "io.h"
#include "byte_reader.h"
#include "concepts.h"
#include "t_hashmap.h"
#include "shared_ptr.h"
#include "string.h"
#include "iterator_utils.h"

class AssetManagerBase
{
protected:
	Buffer<String> _paths;

	const Buffer<String> _extensions;

	bool _fallbackAnywhere; //Allows the asset manager fallback even if _all.txt is not present for a subdir for an asset
protected:
	template <typename T>
	friend class AssetManager;

	AssetManagerBase(const Array<String>& extensions) : _extensions(extensions), _fallbackAnywhere(false) 
	{ 
		_paths.Add(""); /*Root dir*/ 
	}

public:
	virtual ~AssetManagerBase() {}

	virtual void GetAllPossibleKeys(Buffer<String>& resultsOut, const String& path = "", bool recursive = false) const = 0;

protected:
	template <typename T>
	void _DefaultReadResourceImpl(T& resource, const Array<byte>& data, const String& extension) { Debug::PrintLine("Warning: default _ResourceRead used!"); }

	template <typename T>
	requires Concepts::DerivedFrom<T, Asset>
	void _DefaultReadResourceImpl(T& resource, const Array<byte>& data, const String& extension)
	{
		Asset* asset = dynamic_cast<Asset*>(&resource);
		if (asset)
		{
			if (extension == ".txt")
			{
				asset->ReadText(String(data.begin(), data.GetSize()));
			}
			else
			{
				ByteReader reader = data;
				asset->Read(reader);
			}
		}
	}
};

#include "asset.h"

template<typename T>
class AssetManager : protected AssetManagerBase
{
private:
	Hashmap<String, SharedPointerData<T>> _map;
	
protected:
	virtual T* _CreateResource(const Array<byte>& data, const String& name, const String& extension) = 0;
	virtual void _ResourceRead(T& resource, const Array<byte>& data, const String& extension)
	{
		_DefaultReadResourceImpl<T>(resource, data, extension);
	}

	//Called when the specified name could not be loaded
	virtual bool _CreateAlternative(T*& resource, const String& name) { return false; }

	//Called before deleting a resource
	virtual void _DestroyResource(T& resource) {}

	//Returns a list of managers who's assets can be used to finalise an asset for this manager
	virtual Buffer<const AssetManagerBase*> _GetFallbackManagers() const { return {}; }
	
	//Extensions are in priority order (names that specify the extension override this)
	AssetManager(const Buffer<String>& extensions = { ".txt" }) : AssetManagerBase(extensions) { }
	AssetManager(const AssetManager&) = delete;

	virtual ~AssetManager() { _DestroyAll(); }

	void _DestroyAll()
	{
		for (Pair<const String, SharedPointerData<T>>& kv : _map)
		{
			_DestroyResource(*kv.second.GetPtr());
			delete kv.second.GetPtr();
			kv.second.SetPtr(nullptr);
		}
	}

	SharedPointerData<T>& _MapValue(const String& name)
	{
		SharedPointerData<T>* found = _map.TryGet(name);

		if (found)
			return *found;

		return _map.Emplace(name, nullptr, 0); 
	}

public:
	constexpr const Hashmap<String, SharedPointerData<T>> GetCurrentAssets() noexcept { return _map; }

	virtual void Initialise() {}

	constexpr bool NameIsValid(const String& name) const noexcept
	{
		if (name.GetLength() > 0 && name[name.GetLength() - 1] != '/')
		{
			auto slash = IteratorUtils::IndexOfRev(name.begin(), name.end(), '/');
			return name[(slash != IteratorUtils::INVALID_INDEX) ? (slash + 1) : 0] != '_';
		}

		return false;
	}

	void Refresh()
	{
		_DestroyAll();
		Initialise();

		//Reload all assets that are still in use
		for (const Pair<const String, SharedPointerData<T>>& kv : _map)
		{
			if (kv.second.GetReferenceCount())
				auto loadNewAsset = Get(kv.first);
		}
	}

	constexpr const Hashmap<String, SharedPointerData<T>>& GetMap() const noexcept { return _map; }

	//Only adds if key does not exist, otherwise returns false
	SharedPointer<T> Add(const String& name, T* value) noexcept
	{
		if (_map.TryGet(name))
			return SharedPointer<T>();

		return _map.Emplace(name, value, 0);
	}

	struct ResourcePathInfo
	{
		const String* path = nullptr; //Path chosen for the file (w/ trailing slash)
		String extension; //(e.g. ".txt")
		String fullpath; //Path + name + extension
	};

	//info is an output variable and is only valid if function returns true
	constexpr bool GetPath(const String& name, ResourcePathInfo &info) const noexcept
	{
		info.extension = Utilities::GetExtension(name);
		if (info.extension.GetLength())
		{
			bool validExtension = false;
			for (const String& ext : _extensions)
				if (info.extension == ext)
				{
					validExtension = true;
					break;
				}

			if (validExtension)
			{
				for (const String& path : _paths)
				{
					info.fullpath = path + name;
					if (IO::FileExists(info.fullpath.begin()))
					{
						info.path = &path;
						return true;
					}
				}
			}
		}
		else
		{
			for (const String& path : _paths)
			{
				for (const String& ext : _extensions)
				{
					info.fullpath = path + name + ext;
					if (IO::FileExists(info.fullpath.begin()))
					{
						info.extension = ext;
						info.path = &path;
						return true;
					}
				}
			}
		}

		return false;
	}
	
	SharedPointer<T> Get(const String& name) noexcept
	{
		if (name.GetLength() == 0)
			return SharedPointer<T>();

		String lowerName = name.ToLower();
		SharedPointerData<T>* existing = _map.TryGet(lowerName);
		if (existing && existing->GetPtr()) 
			return SharedPointer<T>(*existing);

		if (!NameIsValid(name))
			return SharedPointer<T>();

		T* resource = nullptr;
		ResourcePathInfo info;
		if (GetPath(name, info))
		{
			//Run any _all.txt files in preceding dirs & paths
			for (const String& path : _paths)
			{
				size_t start = 0;
				String searchPath = path;
				do
				{
					String preFile = searchPath + "_all.txt";
					if (IO::FileExists(preFile.begin()))
					{
						if (resource) _ResourceRead(*resource, IO::ReadFile(preFile.begin()), ".txt");
						else resource = _CreateResource(IO::ReadFile(preFile.begin()), lowerName, ".txt");
					}

					auto stop = IteratorUtils::IndexOf(name.begin(), name.end(), '/', start);
					if (stop != IteratorUtils::INVALID_INDEX)
					{
						searchPath += name.SubString(start, stop) + '/';
						start = stop + 1;
					}
					else
					{
						searchPath += '/';
						start = searchPath.GetLength();
					}

				} while (start < name.GetLength());

				if (&path == info.path)
					break;
			}
			
			//..Then the resource
			if (resource) _ResourceRead(*resource, IO::ReadFile(info.fullpath.begin()), info.extension);
			else resource = _CreateResource(IO::ReadFile(info.fullpath.begin()), lowerName, info.extension);

			if (resource)
			{
				if (existing)
				{
					existing->SetPtr(resource);
					return SharedPointer<T>(*existing);
				}

				return SharedPointer<T>(_map.Emplace(lowerName, resource, 0));
			}
		}
		else
		{
			//Okay, so there isn't a file for this specific resource...
			//Let's just run every applicable _all.txt

			for (const String& path : _paths)
			{
				size_t start = 0;
				String searchPath = path;
				do
				{
					String preFile = searchPath + "_all.txt";
					if (IO::FileExists(preFile.begin()))
					{
						if (resource) _ResourceRead(*resource, IO::ReadFile(preFile.begin()), ".txt");
						else resource = _CreateResource(IO::ReadFile(preFile.begin()), lowerName, ".txt");
					}

					auto stop = IteratorUtils::IndexOf(name.begin(), name.end(), '/', start);
					if (stop != IteratorUtils::INVALID_INDEX)
					{
						searchPath += name.SubString(start, stop) + '/';
						start = stop + 1;
					}
					else
					{
						searchPath += '/';
						start = searchPath.GetLength();
					}
				} while (IO::DirectoryExists(searchPath.begin()) && start < name.GetLength());
			}

			if (_CreateAlternative(resource, name))
			{
				if (existing)
				{
					existing->SetPtr(resource);
					return SharedPointer<T>(*existing);
				}

				return SharedPointer<T>(_map.Emplace(lowerName, resource, 0));
			}
			else if (resource)
				delete resource;
		}

		return SharedPointer<T>();
	}

	constexpr SharedPointer<T> Find(const String& name) noexcept
	{
		SharedPointerData<T>* found = _map.TryGet(name.ToLower());
		return found ? SharedPointer<T>(*found) : SharedPointer<T>();
	}

	constexpr SharedPointer<T> Find(const T* ptr) noexcept
	{
		for (Pair<const String, SharedPointerData<T>>& kv : _map)
			if (kv.second.GetPtr() == ptr)
				return SharedPointer<T>(kv.second);

		return SharedPointer<T>();
	}

	constexpr String FindNameOf(const T* resource) const noexcept
	{
		for (const Pair<const String, SharedPointerData<T>>& kv : _map)
			if (kv.second.GetPtr() == resource)
				return kv.first;

		return "None";
	}

	void SetRootPath(const String &root) { _paths[0] = root; }
	const String &GetRootPath() { return _paths[0]; }

	void AddPath(const String &root) { _paths.Add(root); }

	const Buffer<String>& GetPaths() { return _paths; }

	Buffer<String> GetAllPossibleKeys(const String& path = "", bool recursive = true) 
	{ 
		Buffer<String> b; 
		GetAllPossibleKeys(b, path, recursive); 
		return b;
	}

	virtual void GetAllPossibleKeys(Buffer<String>& resultsOut, const String& path = "", bool recursive = true) const override
	{
		for (const Pair<const String, SharedPointerData<T>>& kv : _map)
		{
			if (path.GetLength() > 0)
			{
				size_t slash = IteratorUtils::IndexOf(kv.first.begin(), kv.first.end(), '/');
				if (slash == IteratorUtils::INVALID_INDEX || kv.first.SubString(0, slash + 1) != path)
					continue;
			}

			resultsOut.Insert(
				IteratorUtils::FirstGreaterPosition(resultsOut.begin(), resultsOut.end(), kv.first) - resultsOut.begin(), 
				kv.first);
		}

		Buffer<const AssetManagerBase*> fallbacks = _GetFallbackManagers();
		const Buffer<const AssetManagerBase*>* pfallbacks = &fallbacks;

		if (_fallbackAnywhere)
		{
			for (const AssetManagerBase* f : fallbacks)
				f->GetAllPossibleKeys(resultsOut, path, false);

			pfallbacks = nullptr;
		}
		else
		{
			for (const String& p : _paths)
			{
				if (path.GetLength())
				{
					String check = p;

					for (size_t i = 0;;)
					{
						if (IO::FileExists(CSTR(check, "_all.txt")))
						{
							for (const AssetManagerBase* f : fallbacks)
								f->GetAllPossibleKeys(resultsOut, path, false);

							pfallbacks = nullptr;
							break;
						}

						size_t s = i;
						i = IteratorUtils::IndexOf(path.begin(), path.end(), '/', s);
						if (i == IteratorUtils::INVALID_INDEX || i == path.GetLength() - 1)
							break;

						check += path.SubString(s, i) + '/';
					}
				}

				_FindAllKeysForPath(p, path, resultsOut, pfallbacks, recursive);
			}
		}
	}


private:
	void _FindAllKeysForPath(
		const String& path, const String& dir, 
		Buffer<String>& results, 
		const Buffer<const AssetManagerBase*>* fallbacks,
		bool recursive) const
	{
		String d = path + dir;

		if (!IO::DirectoryExists(d.begin()))
			return;

		if (fallbacks)
		{
			if (IO::FileExists(CSTR(d, "_all.txt")))
			{
				for (const AssetManagerBase* f : *fallbacks)
					f->GetAllPossibleKeys(results, dir, true);

				fallbacks = nullptr;
			}
		}

		for (const String& filename : IO::FindFilesInDirectory(CSTR(d, "*.*")))
		{
			String f = dir + filename;
			String full = path + f;

			if (IO::IsDirectory(full.begin()))
			{
				if (recursive)
					_FindAllKeysForPath(path, f + '/', results, fallbacks, true);
			}
			else
			{
				auto extIndex = IteratorUtils::IndexOf(f.begin(), f.end(), '.');
				if (extIndex != IteratorUtils::INVALID_INDEX)
				{
					String fpath = f.ToLower();
					String name = fpath.SubString(0, extIndex);
					String ext = fpath.SubString(extIndex, fpath.GetLength());

					for (const String& extension : _extensions)
						if (ext == extension)
						{
							if (NameIsValid(name) && IteratorUtils::FirstEqualPosition(results.begin(), results.end(), name) == results.end())
								results.Insert(
									IteratorUtils::FirstGreaterPosition(results.begin(), results.end(), name) - results.begin(),
									name);

							break;
						}
				}
			}
		}
	}
};
