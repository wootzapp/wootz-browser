// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_SESSIONS_SESSION_RESTORATION_SERVICE_H_
#define IOS_CHROME_BROWSER_SESSIONS_SESSION_RESTORATION_SERVICE_H_

#include <memory>
#include <set>
#include <string>

#include "base/functional/callback_forward.h"
#include "components/keyed_service/core/keyed_service.h"

class Browser;
class SessionRestorationObserver;

namespace web {
namespace proto {
class WebStateStorage;
}  // namespace proto
class WebState;
}  // namespace web

// Service responsible for session saving and restoration.
class SessionRestorationService : public KeyedService {
 public:
  // Callback invoked when data for a WebState has been loaded.
  using WebStateStorageCallback =
      base::OnceCallback<void(web::proto::WebStateStorage)>;

  SessionRestorationService() = default;
  ~SessionRestorationService() override = default;

  // Registers `observer` to be notified about session restoration events.
  virtual void AddObserver(SessionRestorationObserver* observer) = 0;

  // Unregisters `observer` from the list of observers to notify about
  // session restoration events.
  virtual void RemoveObserver(SessionRestorationObserver* observer) = 0;

  // Requests that all pending changes to be saved to storage as soon as
  // possible. Can be called at any time.
  virtual void SaveSessions() = 0;

  // Requests that all pending changes to be saved to storage when possible.
  // Can be called at any time.
  virtual void ScheduleSaveSessions() = 0;

  // Sets the `identifier` used to save/load the session for `browser`. The
  // identifier is used to derive the location of the file on storage, thus
  // must be consistent across application restart.
  //
  // Must be called before `LoadSession()` or `Disconnect()` can be called.
  virtual void SetSessionID(Browser* browser,
                            const std::string& identifier) = 0;

  // Loads the session for `browser` from storage synchronously. This must
  // only be used during application startup.
  //
  // `SetSessionID()` must have been called before with this `browser`.
  virtual void LoadSession(Browser* browser) = 0;

  // Asynchronously loads data for `web_state` owned by `browser` that is
  // unrealized. Invokes `callback` with the data loaded from disk on the
  // calling sequence.
  //
  // If the method `SetSessionID(...)` has not been called for `browser`,
  // or if the method `Disconnect(...)` has already been called, then the
  // callback is not invoked.
  //
  // The callback may be invoked with an empty WebStateStorage (e.g. if
  // the data could not be read from disk for any reason).
  virtual void LoadWebStateStorage(Browser* browser,
                                   web::WebState* web_state,
                                   WebStateStorageCallback callback) = 0;

  // Attaches `backup` as a backup Browser for `browser`.
  //
  // A backup Browser allows freely moving WebState to/from the original
  // Browser, but without saving the state of `backup`. If any WebState state
  // changes, the code ensures the data on disk is saved. This can be used to
  // implement a "close all tabs" operation that can be reverted.
  //
  // The `browser` must have been registered via `SetSessionID(...)` before the
  // call to `AttachBackup(...)`. The `backup` Browser must be detached with
  // `Disconnect(...)` before `browser`.
  virtual void AttachBackup(Browser* browser, Browser* backup) = 0;

  // Forgets about `browser` (after writing to disk any pending changes).
  //
  // Must be called before the `browser` is destroyed if `SetSessionID()`
  // has been called for it.
  virtual void Disconnect(Browser* browser) = 0;

  // Creates an unrealized WebState from `storage` that will be inserted
  // in `browser` at a later point. It is required to use this method to
  // create those WebState as the service cannot retrieve the state from
  // an unrealized WebState.
  //
  // Must be called after `SetSessionID()` and before `Disconnect()`.
  virtual std::unique_ptr<web::WebState> CreateUnrealizedWebState(
      Browser* browser,
      web::proto::WebStateStorage storage) = 0;

  // Deletes all data for sessions with `identifiers` and invoke `closure`
  // on the calling sequence when the data has been deleted. Can be called
  // at any time.
  virtual void DeleteDataForDiscardedSessions(
      const std::set<std::string>& identifiers,
      base::OnceClosure closure) = 0;

  // Requests that `closure` is invoked when all pending background tasks
  // are complete. The `closure` may be invoked on a background sequence,
  // so it must be safe to be called from any sequence. Consider using
  // `base::BindPostTask(...)` if the closure needs to be executed on a
  // specific sequence.
  virtual void InvokeClosureWhenBackgroundProcessingDone(
      base::OnceClosure closure) = 0;

  // Removes any persisted data that is no longer needed and invokes
  // `closure` on the calling sequence when done.
  virtual void PurgeUnassociatedData(base::OnceClosure closure) = 0;

  // Returns whether placeholder tabs support is enabled.
  virtual bool PlaceholderTabsEnabled() const = 0;
};

#endif  // IOS_CHROME_BROWSER_SESSIONS_SESSION_RESTORATION_SERVICE_H_
