#include "chrome/browser/browser_commands.h"
#include "chrome/browser/browser_navigator.h"
#include "chrome/browser/browser_command_controller.h"
#include "chrome/browser/web_contents.h"
#include "chrome/ui/browser_window.h"
#include "chrome/ui/omnibox/location_bar.h"
#include "chrome/ui/tabs/tab_strip_model.h"
#include "chrome/browser/chrome_command_ids.h"

using namespace content;
namespace chrome{

  namespace {
    WebContents* GetOrCloneTabForDisposition(Browser* browser,
      WindowOpenDisposition disposition) {
        WebContents* current_tab = browser->tab_strip_model()->GetActiveWebContents();
        switch (disposition) {
        case NEW_FOREGROUND_TAB:
        case NEW_BACKGROUND_TAB: {
          current_tab = current_tab->Clone();
          browser->tab_strip_model()->AddWebContents(
            current_tab, -1, content::PAGE_TRANSITION_LINK,
            disposition == NEW_FOREGROUND_TAB ? TabStripModel::ADD_ACTIVE :
            TabStripModel::ADD_NONE);
          break;
        }
        case NEW_WINDOW: {
          current_tab = current_tab->Clone();
          Browser* b = new Browser(Browser::CreateParams(browser->host_desktop_type()));
          b->tab_strip_model()->AddWebContents(
            current_tab, -1, content::PAGE_TRANSITION_LINK,
            TabStripModel::ADD_ACTIVE);
          b->window()->Show();
          break;
          }
        default:
          break;
        }
        return current_tab;
    }

    void ReloadInternal(Browser* browser,
      WindowOpenDisposition disposition,
      bool ignore_cache) {
        // As this is caused by a user action, give the focus to the page.
        //
        // Also notify RenderViewHostDelegate of the user gesture; this is
        // normally done in Browser::Navigate, but a reload bypasses Navigate.
        /*WebContents* web_contents = GetOrCloneTabForDisposition(browser, disposition);
        web_contents->UserGestureDone();
        if (!web_contents->FocusLocationBarByDefault())
        web_contents->GetView()->Focus();
        if (ignore_cache)
        web_contents->GetController().ReloadIgnoringCache(true);
        else
        web_contents->GetController().Reload(true);*/
    }
  }

  bool IsCommandEnabled(Browser* browser, int command) {
    return browser->command_controller()->command_updater()->IsCommandEnabled(
      command);
  }

  bool SupportsCommand(Browser* browser, int command) {
    return browser->command_controller()->command_updater()->SupportsCommand(
      command);
  }

  bool ExecuteCommand(Browser* browser, int command) {
    return browser->command_controller()->command_updater()->ExecuteCommand(
      command);
  }

  bool ExecuteCommandWithDisposition(Browser* browser,
    int command,
    WindowOpenDisposition disposition) {
      return browser->command_controller()->command_updater()->
        ExecuteCommandWithDisposition(command, disposition);
  }

  void UpdateCommandEnabled(Browser* browser, int command, bool enabled) {
    browser->command_controller()->command_updater()->UpdateCommandEnabled(
      command, enabled);
  }

  void OpenCurrentURL(Browser* browser) {
    if(browser) {
      LocationBar* location_bar = browser->window()->GetLocationBar();
      if (!location_bar)
        return;

      GURL url(location_bar->GetDestinationURL());

      content::PageTransition page_transition = location_bar->GetPageTransition();
      WindowOpenDisposition open_disposition =
        location_bar->GetWindowOpenDisposition();


      NavigateParams params(browser, url, page_transition);
      params.disposition = open_disposition;
      // Use ADD_INHERIT_OPENER so that all pages opened by the omnibox at least
      // inherit the opener. In some cases the tabstrip will determine the group
      // should be inherited, in which case the group is inherited instead of the
      // opener.
      params.tabstrip_add_types =
        TabStripModel::ADD_FORCE_INDEX | TabStripModel::ADD_INHERIT_OPENER;
      Navigate(&params);
    }
  }

  void AddCommandObserver(Browser* browser,
    int command,
    CommandObserver* observer) {
      browser->command_controller()->command_updater()->AddCommandObserver(
        command, observer);
  }

  bool CanGoBack(const Browser* browser) {
    return true;
    /*return browser->tab_strip_model()->GetActiveWebContents()->
      GetController().CanGoBack();*/
  }

  void GoBack(Browser* browser, WindowOpenDisposition disposition) {
//     WebContents* current_tab = browser->tab_strip_model()->GetActiveWebContents();
//     if (CanGoBack(browser)) {
//       WebContents* new_tab = GetOrCloneTabForDisposition(browser, disposition);
//       // If we are on an interstitial page and clone the tab, it won't be copied
//       // to the new tab, so we don't need to go back.
//       if (current_tab->ShowingInterstitialPage() && new_tab != current_tab)
//         return;
//       new_tab->GetController().GoBack();
//     }
  }

  bool CanGoForward(const Browser* browser) {
    return true;
    //return browser->tab_strip_model()->GetActiveWebContents()->
    //  GetController().CanGoForward();
  }

  void GoForward(Browser* browser, WindowOpenDisposition disposition) {
//     content::RecordAction(UserMetricsAction("Forward"));
//     if (CanGoForward(browser)) {
//       GetOrCloneTabForDisposition(browser, disposition)->
//         GetController().GoForward();
//     }
  }

  void Reload(Browser* browser, WindowOpenDisposition disposition) {
    //content::RecordAction(UserMetricsAction("Reload"));
    ReloadInternal(browser, disposition, false);
  }

  void ReloadIgnoringCache(Browser* browser, WindowOpenDisposition disposition) {
    //content::RecordAction(UserMetricsAction("ReloadIgnoringCache"));
    ReloadInternal(browser, disposition, true);
  }

  bool CanReload(const Browser* browser) {
    return !browser->is_devtools();
  }

  void Home(Browser* browser, WindowOpenDisposition disposition) {
//     content::RecordAction(UserMetricsAction("Home"));
// 
//     std::string extra_headers;
// #if defined(ENABLE_RLZ)
//     // If the home page is a Google home page, add the RLZ header to the request.
//     PrefService* pref_service = browser->profile()->GetPrefs();
//     if (pref_service) {
//       if (google_util::IsGoogleHomePageUrl(
//         GURL(pref_service->GetString(prefs::kHomePage)))) {
//           extra_headers = RLZTracker::GetAccessPointHttpHeader(
//             RLZTracker::CHROME_HOME_PAGE);
//       }
//     }
// #endif
// 
//     OpenURLParams params(
//       browser->profile()->GetHomePage(), Referrer(), disposition,
//       content::PageTransitionFromInt(
//       content::PAGE_TRANSITION_AUTO_BOOKMARK |
//       content::PAGE_TRANSITION_HOME_PAGE),
//       false);
//     params.extra_headers = extra_headers;
//     browser->OpenURL(params);
  }

  void Stop(Browser* browser) {
    //content::RecordAction(UserMetricsAction("Stop"));
    browser->tab_strip_model()->GetActiveWebContents()->Stop();
  }

  void ClearCache(Browser* browser) {
//     BrowsingDataRemover* remover =
//       BrowsingDataRemover::CreateForUnboundedRange(browser->profile());
//     remover->Remove(BrowsingDataRemover::REMOVE_CACHE,
//       BrowsingDataHelper::UNPROTECTED_WEB);
    // BrowsingDataRemover takes care of deleting itself when done.
  }
}