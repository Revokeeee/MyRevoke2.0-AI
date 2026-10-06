#pragma once

#include <imgui.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <functional>
#include <string>

// Small widgets shared by the editor's panels, so every panel lays out and behaves the same way.
namespace Revoke::UI
{
	// Shows text as a tooltip once the last item has been hovered for a moment.
	void Tooltip(const char* text);

	// A frameless button showing only an icon. Transparent until hovered.
	bool IconButton(const char* icon, const char* tooltip = nullptr, ImVec2 size = ImVec2(0.0f, 0.0f));

	// Centered, muted text for a panel with nothing to show.
	void EmptyState(const char* icon, const char* text);

	// Text box with a search icon and a clear button. Returns true when the text changed.
	bool SearchBox(const char* id, std::string& text, const char* hint = "Search");

	// Case-insensitive "does text contain filter". An empty filter matches everything.
	bool MatchesFilter(const std::string& text, const std::string& filter);

	// Two-column "label | widget" layout used by the property panels. Wrap rows in
	// BeginProperties/EndProperties; each Property* call adds one row.
	bool BeginProperties(const char* id);
	void EndProperties();
	// Starts a row with a label and leaves the cursor in the widget column, with the next item
	// sized to fill it. For custom widgets the Property* helpers don't cover.
	void PropertyLabel(const char* label, const char* tooltip = nullptr);

	bool PropertyText(const char* label, std::string& value);
	bool PropertyFloat(const char* label, float& value, float speed = 0.1f, float min = 0.0f, float max = 0.0f, const char* format = "%.3f", const char* tooltip = nullptr);
	bool PropertyInt(const char* label, int& value, float speed = 1.0f, int min = 0, int max = 0, const char* tooltip = nullptr);
	bool PropertyBool(const char* label, bool& value, const char* tooltip = nullptr);
	bool PropertyColor(const char* label, glm::vec3& value);
	bool PropertyColor(const char* label, glm::vec4& value);
	bool PropertyCombo(const char* label, int& current, const char* const items[], int count);
	// X/Y(/Z) fields with colored axis buttons; clicking an axis button resets it to resetValue.
	bool PropertyVec2(const char* label, glm::vec2& value, float resetValue = 0.0f, float speed = 0.1f);
	bool PropertyVec3(const char* label, glm::vec3& value, float resetValue = 0.0f, float speed = 0.1f);

	enum class AssetFieldResult { None, Dropped, Cleared };
	// Shows an asset's name and takes files dragged from the Content Browser. accepts decides
	// which files may be dropped; a dropped file's path (relative to the assets folder) goes to
	// droppedPath. previewTexture, when non-zero, is drawn as a thumbnail.
	AssetFieldResult PropertyAsset(const char* label, const char* icon, const std::string& assetName, const std::string& tooltip,
		const std::function<bool(const std::filesystem::path&)>& accepts, std::filesystem::path& droppedPath, uint32_t previewTexture = 0);
}
