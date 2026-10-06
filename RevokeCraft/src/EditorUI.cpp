#include "rvpch.h"
#include "EditorUI.h"

#include "AssetType.h"

#include "MyRevoke/ImGui/ImGuiLayer.h"
#include "MyRevoke/ImGui/ImGuiTheme.h"
#include "MyRevoke/ImGui/ImGuiIcons.h"

#include <glm/gtc/type_ptr.hpp>

#include <cstdio>

namespace Revoke::UI
{
	void Tooltip(const char* text)
	{
		if (!text || !ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_AllowWhenDisabled))
			return;

		// Callers may have a large or bold font pushed; tooltips always use the regular one.
		ImGui::PushFont(ImGuiLayer::GetFont(FontStyle::Regular));
		ImGui::SetTooltip("%s", text);
		ImGui::PopFont();
	}

	bool IconButton(const char* icon, const char* tooltip, ImVec2 size)
	{
		if (size.x <= 0.0f || size.y <= 0.0f)
			size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::SurfaceHovered);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::SurfaceActive);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		bool pressed = ImGui::Button(icon, size);
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(3);

		Tooltip(tooltip);
		return pressed;
	}

	void EmptyState(const char* icon, const char* text)
	{
		ImVec2 available = ImGui::GetContentRegionAvail();
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + available.x);

		ImVec2 iconSize = ImGui::CalcTextSize(icon);
		ImVec2 textSize = ImGui::CalcTextSize(text, nullptr, false, available.x);
		float top = ImGui::GetCursorPosY() + (available.y - iconSize.y - textSize.y) * 0.4f;
		if (top > ImGui::GetCursorPosY())
			ImGui::SetCursorPosY(top);

		ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextDisabled);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (available.x - iconSize.x) * 0.5f);
		ImGui::TextUnformatted(icon);
		float textX = (available.x - textSize.x) * 0.5f;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (textX > 0.0f ? textX : 0.0f));
		ImGui::TextUnformatted(text);
		ImGui::PopStyleColor();

		ImGui::PopTextWrapPos();
	}

	bool SearchBox(const char* id, std::string& text, const char* hint)
	{
		ImGui::PushID(id);

		char buffer[256];
		std::snprintf(buffer, sizeof(buffer), "%s", text.c_str());

		const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
		const float clearWidth = text.empty() ? 0.0f : ImGui::GetFrameHeight() + spacing;
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - clearWidth);

		std::string hintText = std::string(RV_ICON_SEARCH "  ") + hint;
		bool changed = ImGui::InputTextWithHint("##search", hintText.c_str(), buffer, sizeof(buffer));
		if (changed)
			text = buffer;

		if (!text.empty())
		{
			ImGui::SameLine(0.0f, spacing);
			if (IconButton(RV_ICON_CLOSE, "Clear search"))
			{
				text.clear();
				changed = true;
			}
		}

		ImGui::PopID();
		return changed;
	}

	bool MatchesFilter(const std::string& text, const std::string& filter)
	{
		if (filter.empty())
			return true;

		auto lower = [](unsigned char c) { return (char)std::tolower(c); };
		std::string haystack(text.size(), '\0');
		std::string needle(filter.size(), '\0');
		std::transform(text.begin(), text.end(), haystack.begin(), lower);
		std::transform(filter.begin(), filter.end(), needle.begin(), lower);
		return haystack.find(needle) != std::string::npos;
	}

	bool BeginProperties(const char* id)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f * ImGuiLayer::GetUIScale(), 2.0f * ImGuiLayer::GetUIScale()));
		if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PadOuterX))
		{
			ImGui::PopStyleVar();
			return false;
		}

		ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.38f);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.62f);
		return true;
	}

	void EndProperties()
	{
		ImGui::EndTable();
		ImGui::PopStyleVar();
	}

	void PropertyLabel(const char* label, const char* tooltip)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextMuted);
		ImGui::TextUnformatted(label);
		ImGui::PopStyleColor();
		Tooltip(tooltip);

		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
	}

	bool PropertyText(const char* label, std::string& value)
	{
		PropertyLabel(label);

		char buffer[256];
		std::snprintf(buffer, sizeof(buffer), "%s", value.c_str());

		ImGui::PushID(label);
		bool changed = ImGui::InputText("##value", buffer, sizeof(buffer));
		ImGui::PopID();

		if (changed)
			value = buffer;
		return changed;
	}

	bool PropertyFloat(const char* label, float& value, float speed, float min, float max, const char* format, const char* tooltip)
	{
		PropertyLabel(label, tooltip);
		ImGui::PushID(label);
		bool changed = ImGui::DragFloat("##value", &value, speed, min, max, format, min < max ? ImGuiSliderFlags_AlwaysClamp : ImGuiSliderFlags_None);
		ImGui::PopID();
		return changed;
	}

	bool PropertyInt(const char* label, int& value, float speed, int min, int max, const char* tooltip)
	{
		PropertyLabel(label, tooltip);
		ImGui::PushID(label);
		bool changed = ImGui::DragInt("##value", &value, speed, min, max, "%d", min < max ? ImGuiSliderFlags_AlwaysClamp : ImGuiSliderFlags_None);
		ImGui::PopID();
		return changed;
	}

	bool PropertyBool(const char* label, bool& value, const char* tooltip)
	{
		PropertyLabel(label, tooltip);
		ImGui::PushID(label);
		bool changed = ImGui::Checkbox("##value", &value);
		ImGui::PopID();
		return changed;
	}

	bool PropertyColor(const char* label, glm::vec3& value)
	{
		PropertyLabel(label);
		ImGui::PushID(label);
		bool changed = ImGui::ColorEdit3("##value", glm::value_ptr(value));
		ImGui::PopID();
		return changed;
	}

	bool PropertyColor(const char* label, glm::vec4& value)
	{
		PropertyLabel(label);
		ImGui::PushID(label);
		bool changed = ImGui::ColorEdit4("##value", glm::value_ptr(value), ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
		ImGui::PopID();
		return changed;
	}

	bool PropertyCombo(const char* label, int& current, const char* const items[], int count)
	{
		PropertyLabel(label);
		ImGui::PushID(label);

		bool changed = false;
		const char* preview = current >= 0 && current < count ? items[current] : "";
		if (ImGui::BeginCombo("##value", preview))
		{
			for (int i = 0; i < count; i++)
			{
				bool selected = i == current;
				if (ImGui::Selectable(items[i], selected))
				{
					current = i;
					changed = true;
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::PopID();
		return changed;
	}

	static bool AxisFields(const char* label, float* values, int count, float resetValue, float speed)
	{
		static const char* axisNames[] = { "X", "Y", "Z" };
		static const ImVec4 axisColors[] = { Theme::AxisX, Theme::AxisY, Theme::AxisZ };

		PropertyLabel(label);
		ImGui::PushID(label);

		const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
		const float buttonWidth = ImGui::GetFrameHeight() * 0.85f;
		const float fieldWidth = (ImGui::GetContentRegionAvail().x - spacing * (count - 1)) / count - buttonWidth;

		bool changed = false;
		for (int i = 0; i < count; i++)
		{
			ImGui::PushID(i);
			if (i > 0)
				ImGui::SameLine(0.0f, spacing);

			ImVec4 hovered = ImVec4(axisColors[i].x * 1.15f, axisColors[i].y * 1.15f, axisColors[i].z * 1.15f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, axisColors[i]);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, axisColors[i]);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::PushFont(ImGuiLayer::GetFont(FontStyle::Bold));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ImGui::GetStyle().FramePadding.y));
			if (ImGui::Button(axisNames[i], ImVec2(buttonWidth, ImGui::GetFrameHeight())))
			{
				values[i] = resetValue;
				changed = true;
			}
			ImGui::PopStyleVar();
			ImGui::PopFont();
			ImGui::PopStyleColor(4);

			char tooltip[32];
			std::snprintf(tooltip, sizeof(tooltip), "Reset %s to %g", axisNames[i], resetValue);
			Tooltip(tooltip);

			ImGui::SameLine(0.0f, 0.0f);
			ImGui::SetNextItemWidth(fieldWidth);
			changed |= ImGui::DragFloat("##value", &values[i], speed, 0.0f, 0.0f, "%.2f");
			ImGui::PopID();
		}

		ImGui::PopID();
		return changed;
	}

	bool PropertyVec2(const char* label, glm::vec2& value, float resetValue, float speed)
	{
		return AxisFields(label, glm::value_ptr(value), 2, resetValue, speed);
	}

	bool PropertyVec3(const char* label, glm::vec3& value, float resetValue, float speed)
	{
		return AxisFields(label, glm::value_ptr(value), 3, resetValue, speed);
	}

	AssetFieldResult PropertyAsset(const char* label, const char* icon, const std::string& assetName, const std::string& tooltip,
		const std::function<bool(const std::filesystem::path&)>& accepts, std::filesystem::path& droppedPath, uint32_t previewTexture)
	{
		PropertyLabel(label);
		ImGui::PushID(label);

		AssetFieldResult result = AssetFieldResult::None;
		const ImGuiStyle& style = ImGui::GetStyle();
		const float frameHeight = ImGui::GetFrameHeight();

		if (previewTexture)
		{
			ImVec2 cursor = ImGui::GetCursorScreenPos();
			float size = frameHeight;
			ImGui::GetWindowDrawList()->AddRectFilled(cursor, ImVec2(cursor.x + size, cursor.y + size), ImGui::GetColorU32(ImGuiCol_FrameBg), style.FrameRounding);
			ImGui::Image((ImTextureID)(intptr_t)previewTexture, ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
		}

		const float clearWidth = assetName.empty() ? 0.0f : frameHeight + style.ItemInnerSpacing.x;
		const float fieldWidth = ImGui::GetContentRegionAvail().x - clearWidth;

		// A button dressed as an input frame, so it reads as a field that takes a value.
		ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_FrameBg]);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_FrameBgHovered]);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.Colors[ImGuiCol_FrameBgActive]);
		ImGui::PushStyleColor(ImGuiCol_Text, assetName.empty() ? Theme::TextDisabled : Theme::Text);
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
		std::string text = std::string(icon) + "  " + (assetName.empty() ? std::string("None (drop a file here)") : assetName);
		ImGui::Button(text.c_str(), ImVec2(fieldWidth, frameHeight));
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(4);
		Tooltip(tooltip.empty() ? nullptr : tooltip.c_str());

		if (ImGui::BeginDragDropTarget())
		{
			// Peek before the drop so a wrong file type shows red instead of silently doing nothing.
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(ContentBrowserPayload, ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
			{
				std::filesystem::path path((const wchar_t*)payload->Data);
				bool accepted = accepts(path);

				ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
					ImGui::GetColorU32(accepted ? Theme::Accent : Theme::Error), style.FrameRounding, 0, 2.0f);

				if (accepted && payload->IsDelivery())
				{
					droppedPath = path;
					result = AssetFieldResult::Dropped;
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (!assetName.empty())
		{
			ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
			if (IconButton(RV_ICON_CLOSE, "Clear"))
				result = AssetFieldResult::Cleared;
		}

		ImGui::PopID();
		return result;
	}
}
