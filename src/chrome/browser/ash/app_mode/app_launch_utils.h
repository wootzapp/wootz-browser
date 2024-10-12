// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_APP_MODE_APP_LAUNCH_UTILS_H_
#define CHROME_BROWSER_ASH_APP_MODE_APP_LAUNCH_UTILS_H_

#include <optional>
#include <string>

#include "chrome/browser/ash/app_mode/kiosk_app_launcher.h"
#include "chrome/browser/ash/app_mode/kiosk_app_types.h"
#include "chrome/browser/ash/app_mode/lacros_launcher.h"

class PrefService;
class Profile;

namespace base {
class CommandLine;
}

namespace ash {

// Attempts to launch the app given by `kiosk_app_id` in app mode or exit on
// failure. This function will not show any launch UI during the launch.
void LaunchAppOrDie(Profile* profile, const KioskAppId& kiosk_app_id);

// Removes obsolete preferences left out by previous user session;
void ResetEphemeralKioskPreferences(PrefService* prefs);
// Replace the list of preferences which are reset in tests.
void SetEphemeralKioskPreferencesListForTesting(std::vector<std::string>*);

// Checks whether kiosk auto launch should be started.
bool ShouldAutoLaunchKioskApp(const base::CommandLine& command_line,
                              const PrefService& local_state);

void CreateKioskSystemSession(const KioskAppId& kiosk_app_id,
                              Profile* profile,
                              const std::optional<std::string>& app_name);

// Returns true if a kiosk app should auto-launch just this one time.
// This happens after a Lacros migration, which requires a full relaunch of
// the running kiosk app.
bool ShouldOneTimeAutoLaunchKioskApp(const base::CommandLine& command_line,
                                     const PrefService& local_state);
// Retrieves the app id that should be launched one time, and removes it
// from the `local_state` so the app does not get launched again the next time.
KioskAppId ExtractOneTimeAutoLaunchKioskAppId(PrefService& local_state);
// Retrieves the app id that should be launched one time, or nullopt if no app
// should be one time auto launched.
// This method will not change `local_state`.
std::optional<KioskAppId> GetOneTimeAutoLaunchKioskAppId(
    const PrefService& local_state);
void SetOneTimeAutoLaunchKioskAppId(PrefService& local_state,
                                    const KioskAppId& kiosk_app_id);

}  // namespace ash

#endif  // CHROME_BROWSER_ASH_APP_MODE_APP_LAUNCH_UTILS_H_
