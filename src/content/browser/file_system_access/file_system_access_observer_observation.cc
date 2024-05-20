// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/file_system_access/file_system_access_observer_observation.h"

#include <memory>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "build/buildflag.h"
#include "content/browser/file_system_access/file_system_access_change_source.h"
#include "content/browser/file_system_access/file_system_access_directory_handle_impl.h"
#include "content/browser/file_system_access/file_system_access_file_handle_impl.h"
#include "content/browser/file_system_access/file_system_access_handle_base.h"
#include "content/browser/file_system_access/file_system_access_manager_impl.h"
#include "content/browser/file_system_access/file_system_access_observer_host.h"
#include "content/browser/file_system_access/file_system_access_watch_scope.h"
#include "content/browser/file_system_access/file_system_access_watcher_manager.h"
#include "content/browser/renderer_host/render_frame_host_impl.h"
#include "content/public/browser/file_system_access_permission_context.h"
#include "content/public/browser/global_routing_id.h"
#include "content/public/browser/web_contents.h"
#include "storage/browser/file_system/file_system_context.h"
#include "storage/browser/file_system/file_system_url.h"
#include "third_party/blink/public/mojom/file_system_access/file_system_access_directory_handle.mojom.h"
#include "third_party/blink/public/mojom/file_system_access/file_system_access_observer.mojom.h"
#include "third_party/blink/public/mojom/permissions/permission_status.mojom-shared.h"

#if BUILDFLAG(IS_WIN)
#include "base/strings/utf_string_conversions.h"
#endif  // BUILDFLAG(IS_WIN)

namespace content {

namespace {

FileSystemAccessPermissionContext::HandleType GetHandleType(
    const absl::variant<std::unique_ptr<FileSystemAccessDirectoryHandleImpl>,
                        std::unique_ptr<FileSystemAccessFileHandleImpl>>&
        handle) {
  return absl::get_if<std::unique_ptr<FileSystemAccessDirectoryHandleImpl>>(
             &handle)
             ? FileSystemAccessPermissionContext::HandleType::kDirectory
             : FileSystemAccessPermissionContext::HandleType::kFile;
}

FileSystemAccessHandleBase& AsHandleBase(
    const absl::variant<std::unique_ptr<FileSystemAccessDirectoryHandleImpl>,
                        std::unique_ptr<FileSystemAccessFileHandleImpl>>&
        handle) {
  auto* dir_handle_ptr =
      absl::get_if<std::unique_ptr<FileSystemAccessDirectoryHandleImpl>>(
          &handle);
  if (dir_handle_ptr) {
    return *static_cast<FileSystemAccessHandleBase*>(dir_handle_ptr->get());
  }

  return *absl::get<std::unique_ptr<FileSystemAccessFileHandleImpl>>(handle);
}

// TODO(crbug.com/40105284): Move this to a helper shared with
// `FileSystemAccessDirectoryHandleImpl`.
std::vector<std::string> GetRelativePathAsVectorOfStrings(
    const base::FilePath& relative_path) {
  CHECK(!relative_path.IsAbsolute());
  CHECK(!relative_path.ReferencesParent());

  std::vector<base::FilePath::StringType> components =
      relative_path.GetComponents();
#if BUILDFLAG(IS_WIN)
  std::vector<std::string> result;
  result.reserve(components.size());
  for (const auto& component : components) {
    result.push_back(base::WideToUTF8(component));
  }
  return result;
#else
  return components;
#endif  //  BUILDFLAG(IS_WIN)
}

blink::mojom::FileSystemAccessEntryPtr CreateEntryForUrl(
    FileSystemAccessManagerImpl& manager,
    const FileSystemAccessManagerImpl::BindingContext& binding_context,
    const FileSystemAccessManagerImpl::SharedHandleState& handle_state,
    const storage::FileSystemURL& url,
    FileSystemAccessPermissionContext::HandleType handle_type) {
  switch (handle_type) {
    case FileSystemAccessPermissionContext::HandleType::kFile:
      return blink::mojom::FileSystemAccessEntry::New(
          blink::mojom::FileSystemAccessHandle::NewFile(
              manager.CreateFileHandle(binding_context, url, handle_state)),
          url.virtual_path().BaseName().AsUTF8Unsafe());
    case FileSystemAccessPermissionContext::HandleType::kDirectory:
      return blink::mojom::FileSystemAccessEntry::New(
          blink::mojom::FileSystemAccessHandle::NewDirectory(
              manager.CreateDirectoryHandle(binding_context, url,
                                            handle_state)),
          url.virtual_path().BaseName().AsUTF8Unsafe());
  }
}

}  // namespace

FileSystemAccessObserverObservation::FileSystemAccessObserverObservation(
    FileSystemAccessObserverHost* host,
    std::unique_ptr<FileSystemAccessWatcherManager::Observation> observation,
    mojo::PendingRemote<blink::mojom::FileSystemAccessObserver> remote,
    absl::variant<std::unique_ptr<FileSystemAccessDirectoryHandleImpl>,
                  std::unique_ptr<FileSystemAccessFileHandleImpl>> handle)
    : WebContentsObserver(
          WebContents::FromRenderFrameHost(RenderFrameHostImpl::FromID(
              AsHandleBase(handle).context().frame_id))),
      host_(host),
      handle_(std::move(handle)),
      observation_(std::move(observation)),
      remote_(std::move(remote)) {
  CHECK(host);
  CHECK(observation_);

  CHECK(observation_->scope().Contains(AsHandleBase(handle_).url()));

  observation_->SetCallback(
      base::BindRepeating(&FileSystemAccessObserverObservation::OnChanges,
                          weak_factory_.GetWeakPtr()));

  // `base::Unretained` is safe here because this instance owns
  // `remote_`.
  remote_.set_disconnect_handler(
      base::BindOnce(&FileSystemAccessObserverObservation::OnReceiverDisconnect,
                     base::Unretained(this)));
}

FileSystemAccessObserverObservation::~FileSystemAccessObserverObservation() =
    default;

const storage::FileSystemURL& FileSystemAccessObserverObservation::handle_url()
    const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return AsHandleBase(handle_).url();
}

void FileSystemAccessObserverObservation::OnChanges(
    const std::list<FileSystemAccessWatcherManager::Observation::Change>&
        changes) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  FileSystemAccessManagerImpl* manager = AsHandleBase(handle_).manager();
  const FileSystemAccessManagerImpl::BindingContext& binding_context =
      AsHandleBase(handle_).context();

  // Make sure the RenderFrameHost is Active before sending changes to the
  // renderer.
  GlobalRenderFrameHostId render_frame_host_id = binding_context.frame_id;
  RenderFrameHostImpl* rfh = RenderFrameHostImpl::FromID(render_frame_host_id);
  if (!rfh || !rfh->IsActive()) {
    received_changes_while_in_bf_cache_ = true;
    return;
  }

  const FileSystemAccessManagerImpl::SharedHandleState& handle_state =
      AsHandleBase(handle_).handle_state();
  const storage::FileSystemURL& handle_url = AsHandleBase(handle_).url();

  // Do not relay changes if the site has lost read permission to the handle.
  // TODO(crbug.com/40283887): Add tests for this.
  if (handle_state.read_grant->GetStatus() !=
      blink::mojom::PermissionStatus::GRANTED) {
    // TODO(crbug.com/40283887): Proactively listen for permission
    // changes, rather than (or perhaps in addition to) checking on each change.
    return;
  }

  std::vector<blink::mojom::FileSystemAccessChangePtr> mojo_changes;
  for (const auto& change : changes) {
    if (change.type->is_errored()) {
      // TODO(crbug.com/40105284): Consider destroying `observation_`...
      // Or don't bother passing along errored changes from the WatcherManager
      // to its Observations in the first place.
      continue;
    }

    // TODO(crbug.com/40105284): Consider refactoring to keep the "scope"
    // concept within the WatcherManager and its associated classes. This method
    // just needs the root url.
    //
    // It is illegal to receive a change outside of the observed scope.
    CHECK(observation_->scope().Contains(change.url));

    blink::mojom::FileSystemAccessEntryPtr root_entry =
        CreateEntryForUrl(*manager, binding_context, handle_state, handle_url,
                          GetHandleType(handle_));
    FileSystemAccessPermissionContext::HandleType changed_entry_handle_type;
    switch (change.file_path_type) {
      case FileSystemAccessChangeSource::FilePathType::kUnknown:
        // Fall back to using the same handle type as the root handle.
        changed_entry_handle_type = GetHandleType(handle_);
        break;
      case FileSystemAccessChangeSource::FilePathType::kDirectory:
        changed_entry_handle_type =
            FileSystemAccessPermissionContext::HandleType::kDirectory;
        break;
      case FileSystemAccessChangeSource::FilePathType::kFile:
        changed_entry_handle_type =
            FileSystemAccessPermissionContext::HandleType::kFile;
        break;
    }
    blink::mojom::FileSystemAccessEntryPtr changed_entry =
        CreateEntryForUrl(*manager, binding_context, handle_state, change.url,
                          changed_entry_handle_type);

    const base::FilePath& root_path = handle_url.path();
    const base::FilePath& changed_path = change.url.path();

    base::FilePath relative_path;
    if (root_path.empty()) {
      relative_path = changed_path;
    } else if (root_path.IsParent(changed_path)) {
      CHECK(root_path.AppendRelativePath(changed_path, &relative_path));
    } else {
      CHECK_EQ(root_path, changed_path);
    }

    mojo_changes.emplace_back(blink::mojom::FileSystemAccessChange::New(
        blink::mojom::FileSystemAccessChangeMetadata::New(
            std::move(root_entry), std::move(changed_entry),
            GetRelativePathAsVectorOfStrings(relative_path)),
        change.type->Clone()));
  }

  remote_->OnFileChanges(std::move(mojo_changes));
}

void FileSystemAccessObserverObservation::RenderFrameHostStateChanged(
    RenderFrameHost* render_frame_host,
    RenderFrameHost::LifecycleState old_state,
    RenderFrameHost::LifecycleState new_state) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  bool transitioned_from_bf_cache_to_active =
      new_state == RenderFrameHost::LifecycleState::kActive &&
      old_state == RenderFrameHost::LifecycleState::kInBackForwardCache;

  if (render_frame_host !=
          RenderFrameHost::FromID(AsHandleBase(handle_).context().frame_id) ||
      !transitioned_from_bf_cache_to_active ||
      !received_changes_while_in_bf_cache_) {
    return;
  }

  // RFH's state changed to kActive from kInBackForwardCache. File System
  // changes are not sent while the page is in BFCache. So, we use
  // ChangeType::kUnknown to signal to the renderer that some changes could be
  // missing.
  std::vector<blink::mojom::FileSystemAccessChangePtr> mojo_changes;
  FileSystemAccessManagerImpl* manager = AsHandleBase(handle_).manager();
  const FileSystemAccessManagerImpl::BindingContext& binding_context =
      AsHandleBase(handle_).context();
  const FileSystemAccessManagerImpl::SharedHandleState& handle_state =
      AsHandleBase(handle_).handle_state();
  const storage::FileSystemURL& handle_url = AsHandleBase(handle_).url();

  mojo_changes.emplace_back(blink::mojom::FileSystemAccessChange::New(
      blink::mojom::FileSystemAccessChangeMetadata::New(
          CreateEntryForUrl(*manager, binding_context, handle_state, handle_url,
                            GetHandleType(handle_)),
          CreateEntryForUrl(*manager, binding_context, handle_state, handle_url,
                            GetHandleType(handle_)),
          std::vector<std::string>()),
      blink::mojom::FileSystemAccessChangeType::NewUnknown(
          blink::mojom::FileSystemAccessChangeTypeUnknown::New())));
  remote_->OnFileChanges(std::move(mojo_changes));
  received_changes_while_in_bf_cache_ = false;
}

void FileSystemAccessObserverObservation::OnReceiverDisconnect() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  // Destroys `this`.
  host_->RemoveObservation(this);
}

}  // namespace content
