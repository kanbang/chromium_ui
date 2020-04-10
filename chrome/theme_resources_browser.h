#ifndef THEMERESOURE_BROWSER_H_
#define THEMERESOURE_BROWSER_H_
#include <map>

#include "ui/base/resource/resource_bundle.h"
#include "chrome/theme_resources_id.h"
#include "ui/base/default_theme_provider.h"

namespace themeresoure
{
	class ThemeRes : public ui::ResourceBundle::Delegate {
	public:
    ThemeRes();
		virtual base::FilePath GetPathForResourcePack(
			const base::FilePath& pack_path,
			ui::ScaleFactor scale_factor) OVERRIDE;

		virtual base::FilePath GetPathForLocalePack(
			const base::FilePath& pack_path,
			const std::string& locale) OVERRIDE;
		virtual gfx::Image GetImageNamed(int resource_id) OVERRIDE;

		virtual gfx::Image GetNativeImageNamed(int resource_id, ui::ResourceBundle::ImageRTL rtl) OVERRIDE;

		virtual base::RefCountedStaticMemory* LoadDataResourceBytes(
			int resource_id,
			ui::ScaleFactor scale_factor) OVERRIDE { return NULL;};

		// Retrieve a raw data resource. Return true if a resource was provided or
		// false to attempt retrieval of the default resource.
		virtual bool GetRawDataResource(int resource_id,
			ui::ScaleFactor scale_factor,
			base::StringPiece* value) OVERRIDE {return false;} ;

		// Retrieve a localized string. Return true if a string was provided or
		// false to attempt retrieval of the default string.
		virtual bool GetLocalizedString(int message_id, string16* value) OVERRIDE;

		// Returns a font or NULL to attempt retrieval of the default resource.
		virtual scoped_ptr<gfx::Font> GetFont(ui::ResourceBundle::FontStyle style) OVERRIDE {return scoped_ptr<gfx::Font>();};

    std::map<int, std::string> res_ids;
    std::map<int, base::string16> string_ids;

    void init_theme_res();
    void init_theme_string();
	};

  class BrowserThemeProvider : public ui::DefaultThemeProvider
  {
  public:
    virtual SkColor GetColor(int id) const OVERRIDE;
  };
}

#endif