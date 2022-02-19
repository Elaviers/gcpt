#pragma once
#ifdef BUILD_CLIENT
#include "engine.h"
#include "imgui.h"

namespace ImGuiUtil
{
	template <typename T> 
	void AssetImGui(T&) {}
	void AssetImGui(KickSet&);
	void AssetImGui(SheetFormat&);

	void AssetImGui(SoundClip&);
	void AssetImGui(SoundConfig&);
	void AssetImGui(Font&);
	void AssetImGui(Mesh&);
	void AssetImGui(Material&);
	void AssetImGui(Texture&);
	void AssetImGui(Text&);

	template <typename T>
	void AssetManagerImGui(AssetManager<T>& mgr)
	{
		Buffer<String> keys = mgr.GetAllPossibleKeys();
		
		char loadedCbName[10];
		int idx = 0;

		for (const String& key : keys)
		{
			snprintf(loadedCbName, sizeof(loadedCbName), "##load%d", idx++);

			SharedPointer<T> asset = mgr.Find(key);
			bool isLoaded = asset;
			if (ImGui::Checkbox(loadedCbName, &isLoaded))
				if (isLoaded && !asset)
					asset = mgr.Get(key);

			ImGui::SameLine();
			if (ImGui::TreeNode(CSTR(key)))
			{
				if (asset) AssetImGui(*asset);

				ImGui::TreePop();
			}
		}
	}
}

#endif
