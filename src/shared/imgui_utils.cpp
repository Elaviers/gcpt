#include "imgui_utils.h"

#ifdef BUILD_CLIENT

EKickRotTransfer imguiTransfers[] = {
	EKickRotTransfer::START2RIGHT,
	EKickRotTransfer::START2FLIP,
	EKickRotTransfer::START2LEFT,
	EKickRotTransfer::RIGHT2START,
	EKickRotTransfer::RIGHT2FLIP,
	EKickRotTransfer::RIGHT2LEFT,
	EKickRotTransfer::FLIP2START,
	EKickRotTransfer::FLIP2RIGHT,
	EKickRotTransfer::FLIP2LEFT,
	EKickRotTransfer::LEFT2START,
	EKickRotTransfer::LEFT2RIGHT,
	EKickRotTransfer::LEFT2FLIP
};

const char* imguiTransferNames[] = {
	"Start2Right",
	"Start2Flip",
	"Start2Left",
	"Right2Start",
	"Right2Flip",
	"Right2Left",
	"Flip2Start",
	"Flip2Right",
	"Flip2Left",
	"Left2Start",
	"Left2Right",
	"Left2Flip"
};

void ImGuiUtil::AssetImGui(KickSet& asset)
{
	for (size_t dataIdx = 0; dataIdx < asset.rotSets.GetSize(); ++dataIdx)
	{
		if (ImGui::TreeNodeEx(CSTR(dataIdx), ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& data = asset.rotSets[dataIdx];
			const size_t sz = Utilities::ArrayLength(imguiTransfers);
			for (size_t i = 0; i < sz; ++i)
			{
				List<Vector2>& testList = data.TestList(imguiTransfers[i]);

				if (testList.GetSize() > 0)
				{
					ImGui::Text(imguiTransferNames[i]);

					int counter = 0;
					ImGui::Indent();
					for (Vector2& offset : testList)
						ImGui::Text(CSTR(counter++, "\t[", offset.x, ", ", offset.y, "]"));

					ImGui::Unindent();
				}
			}

			ImGui::TreePop();
		}
	}

	ImGui::Text("Offset table mappings:");
	ImGui::Indent();
	for (const auto& kv : asset.part2rotSetIdx)
		ImGui::Text(CSTR((char)kv.first, "->", (int)kv.second));
	ImGui::Unindent();
}


static void ImguiSpriteSet(SpriteSet& ss)
{
	for (Pair<const String, SpriteSet>& kv : ss.subsets)
	{
		if (ImGui::TreeNode(CSTR(kv.first)))
		{
			ImguiSpriteSet(kv.second);
			ImGui::TreePop();
		}
	}

	for (Pair<const String, SpriteDef>& kv : ss.sprites)
		ImGui::Text(CSTR(kv.first, " [x:", kv.second.x, ", y:", kv.second.y, ", w:", kv.second.w, ", h:", kv.second.h, ']'));
}

void ImGuiUtil::AssetImGui(SheetFormat& asset)
{
	ImguiSpriteSet(asset.sprites);
}

#define IMGUIMEMBERFIELD(ASSET, FIELD) ImGui::Text(CSTR(#FIELD, ASSET.##FIELD))

static void PropertyImGui(const PropertyCollection& props, void* obj)
{
	for (Property* prop : props.GetAll())
		ImGui::Text(CSTR(prop->GetName(), ":", prop->GetAsString(obj)));
}

static void ImGuiSoundBucket(SoundBucket& bucket)
{
	const char* pickModeString = "(unknown)";
	switch (bucket.picker.mode)
	{
	case ESoundBucketPickMode::FIRST: pickModeString = "first"; break;
	case ESoundBucketPickMode::RANDOM: pickModeString = "random"; break;
	case ESoundBucketPickMode::SEQUENTIAL: pickModeString = "sequential"; break;
	case ESoundBucketPickMode::SHUFFLE: pickModeString = "shuffle"; break;
	}

	ImGui::Text(CSTR("Pick mode: ", pickModeString));
	ImGui::Text(CSTR("Pick counter: ", bucket.picker.counter));

	for (SoundBucket::Entry& entry : bucket.entries)
	{
		const char* fieldString = "";
		String valString;
		switch (entry.condition.field)
		{
		case ESoundContextData::COUNT: fieldString = "count"; valString = String::FromInt(entry.condition.mInt); break;
		case ESoundContextData::FROM: fieldString = "from"; valString = entry.condition.mString; break;
		case ESoundContextData::LINES: fieldString = "lines"; valString = String::FromInt(entry.condition.mInt); break;
		}

		String compString = "(unknown)";
		switch (entry.condition.comparison)
		{
		case SoundConditional::Comparison::ALWAYS: compString = "always"; break;
		case SoundConditional::Comparison::IS_SET: compString = String(fieldString) + "is set"; break;
		case SoundConditional::Comparison::IS_NOT_SET: compString = String(fieldString) + "is not set"; break;
		case SoundConditional::Comparison::EQUAL: compString = String(fieldString) + " == " + valString; break;
		case SoundConditional::Comparison::NOT_EQUAL: compString = String(fieldString) + " != " + valString; break;
		case SoundConditional::Comparison::GREATER: compString = String(fieldString) + " > " + valString; break;
		case SoundConditional::Comparison::GREATER_EQUAL: compString = String(fieldString) + " >= " + valString; break;
		case SoundConditional::Comparison::LESS: compString = String(fieldString) + " < " + valString; break;
		case SoundConditional::Comparison::LESS_EQUAL: compString = String(fieldString) + " <= " + valString; break;
		}

		if (entry.bucket)
		{
			if (ImGui::TreeNode(CSTR("Bucket [", compString, ']')))
			{
				ImGuiSoundBucket(*entry.bucket);
				ImGui::TreePop();
			}
		}
		else
		{
			ImGui::Text(CSTR(g_engine.sounds->FindNameOf(entry.sound.Ptr()), " [", compString, ']'));
		}
	}
}

void ImGuiUtil::AssetImGui(SoundClip& asset)
{
	IMGUIMEMBERFIELD(asset, bitsPerSample);
	IMGUIMEMBERFIELD(asset, byteRate);
	IMGUIMEMBERFIELD(asset, channelCount);
	IMGUIMEMBERFIELD(asset, dataSize);
	IMGUIMEMBERFIELD(asset, format);
	IMGUIMEMBERFIELD(asset, frameSize);
	IMGUIMEMBERFIELD(asset, sampleRate);
	IMGUIMEMBERFIELD(asset, volume);
}

void ImGuiUtil::AssetImGui(SoundConfig& asset)
{
	for (int i = 0; i < (int)ESoundEvent::_COUNT; ++i)
	{
		if (asset.sounds[i])
		{
			if (ImGui::TreeNode(g_soundTypeNames[i]))
			{
				ImGuiSoundBucket(*asset.sounds[i]);
				ImGui::TreePop();
			}
		}
		else
			ImGui::Text(g_soundTypeNames[i]);
	}
}

#include "font_ttf.h"

void ImGuiUtil::AssetImGui(Font& asset)
{
	PropertyImGui(asset.GetProperties(), &asset);
	int cols = Maths::Clamp((int)(ImGui::GetContentRegionAvail().x / 40), 1, 64);

	if (ImGui::BeginTable("Glyphs", cols))
	{
		if (FontTTF* ttf = dynamic_cast<FontTTF*>(&asset))
		{
			const float scale = 32.f / ttf->GetPointSize();

			for (const auto& kv : ttf->GetGlyphMap())
			{
				ImVec2 sz;
				sz.x = kv.second.size.x * scale;
				sz.y = kv.second.size.y * scale;

				ImGui::Image((ImTextureID)kv.second.texID, sz);
				ImGui::Text("%c", kv.first);
				ImGui::TableNextColumn();
			}
		}

		ImGui::EndTable();
	}
}

void ImGuiUtil::AssetImGui(Mesh& asset)
{
	PropertyImGui(asset.GetProperties(), &asset);
}

void ImGuiUtil::AssetImGui(Material& asset)
{
	PropertyImGui(asset.GetProperties(), &asset);
}

void ImGuiUtil::AssetImGui(Texture& asset)
{
	ImGui::Image((ImTextureID)asset.GL().GetID(), ImVec2(256, 256));
	ImGui::SameLine();
	ImGui::BeginChild("details", ImVec2(-FLT_MIN, 256));
	PropertyImGui(asset.GetProperties(), &asset);
	ImGui::Text("width: %d", asset.GetWidth());
	ImGui::Text("height: %d", asset.GetHeight());
	ImGui::EndChild();
}

void ImGuiUtil::AssetImGui(Text& asset)
{
}

#endif
