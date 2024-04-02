// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/file_system_provider/cloud_file_system.h"

#include <memory>
#include <utility>

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/notreached.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"
#include "base/timer/timer.h"
#include "chrome/browser/ash/file_manager/fileapi_util.h"
#include "chrome/browser/ash/file_system_provider/provided_file_system_interface.h"
#include "chrome/browser/ash/file_system_provider/queue.h"
#include "url/origin.h"

namespace ash::file_system_provider {

namespace {

// The frequency that the FSP syncs with the cloud when the File Manager is a
// watcher.
constexpr base::TimeDelta kFileManagerWatcherInterval = base::Seconds(15);

// TODO(b/317137739): Remove this once a proper API call is introduced.
// Temp custom action to request ODFS sync with the cloud.
constexpr char kODFSSyncWithCloudAction[] = "HIDDEN_SYNC_WITH_CLOUD";

const GURL GetContentCacheURL() {
  return GURL("chrome://content-cache/");
}

const base::FilePath RootFilePath() {
  return base::FilePath("/");
}

std::ostream& operator<<(std::ostream& out,
                         const std::vector<base::FilePath>& entry_paths) {
  for (size_t i = 0; i < entry_paths.size(); ++i) {
    out << entry_paths[i];
    if (i < entry_paths.size() - 1) {
      out << ", ";
    }
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, OpenFileMode mode) {
  switch (mode) {
    case OpenFileMode::OPEN_FILE_MODE_READ:
      return out << "OPEN_FILE_MODE_READ";
    case OpenFileMode::OPEN_FILE_MODE_WRITE:
      return out << "OPEN_FILE_MODE_WRITE";
  }
  NOTREACHED_NORETURN() << "Unknown OpenFileMode: " << mode;
}

std::ostream& operator<<(std::ostream& out,
                         storage::WatcherManager::ChangeType type) {
  using ChangeType = storage::WatcherManager::ChangeType;
  switch (type) {
    case ChangeType::CHANGED:
      return out << "CHANGED";
    case ChangeType::DELETED:
      return out << "DELETED";
  }
  NOTREACHED_NORETURN() << "Unknown ChangeType: " << type;
}

std::ostream& operator<<(std::ostream& out,
                         ProvidedFileSystemObserver::Changes changes) {
  if (changes.empty()) {
    return out << "none";
  }
  for (size_t i = 0; i < changes.size(); ++i) {
    const auto& [entry_path, change_type] = changes[i];
    out << entry_path << ": " << change_type;
    if (i < changes.size() - 1) {
      out << ", ";
    }
  }
  return out;
}

}  // namespace

CloudFileSystem::CloudFileSystem(
    std::unique_ptr<ProvidedFileSystemInterface> file_system)
    : CloudFileSystem(std::move(file_system), nullptr) {}

CloudFileSystem::CloudFileSystem(
    std::unique_ptr<ProvidedFileSystemInterface> file_system,
    CacheManager* cache_manager)
    : file_system_(std::move(file_system)) {
  if (!cache_manager) {
    return;
  }

  cache_manager->InitializeForProvider(
      file_system_->GetFileSystemInfo().mount_path().BaseName(),
      base::BindOnce(&CloudFileSystem::OnContentCacheInitialized,
                     weak_ptr_factory_.GetWeakPtr()));

  // Add watcher to keep content cache up to date. Notifications are received
  // though Notify() so no notification_callback is needed.
  AddWatcher(GetContentCacheURL(), RootFilePath(),
             /*recursive=*/true, /*persistent=*/false,
             base::BindOnce([](base::File::Error result) {
               VLOG(1) << "Added file watcher on root: " << result;
             }),
             base::DoNothing());
}

CloudFileSystem::~CloudFileSystem() {
  if (content_cache_) {
    RemoveWatcher(GetContentCacheURL(), RootFilePath(),
                  /*recursive=*/true,
                  base::BindOnce([](base::File::Error result) {
                    VLOG(1) << "Removed file watcher on root: " << result;
                  }));
  }
}

void CloudFileSystem::OnContentCacheInitialized(
    base::FileErrorOr<std::unique_ptr<ContentCache>> error_or_cache) {
  LOG_IF(ERROR, !error_or_cache.has_value())
      << "Error initializing the content cache: " << error_or_cache.error();
  if (error_or_cache.has_value()) {
    content_cache_ = std::move(error_or_cache.value());
  }
}

AbortCallback CloudFileSystem::RequestUnmount(
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(2) << "RequestUnmount {fsid = " << GetFileSystemId() << "}";
  return file_system_->RequestUnmount(std::move(callback));
}

AbortCallback CloudFileSystem::GetMetadata(const base::FilePath& entry_path,
                                            MetadataFieldMask fields,
                                            GetMetadataCallback callback) {
  VLOG(2) << "GetMetadata {fsid = '" << GetFileSystemId() << "', entry_path = '"
          << entry_path << "', fields = '" << fields << "'}";
  return file_system_->GetMetadata(entry_path, fields, std::move(callback));
}

AbortCallback CloudFileSystem::GetActions(
    const std::vector<base::FilePath>& entry_paths,
    GetActionsCallback callback) {
  VLOG(2) << "GetActions {fsid = '" << GetFileSystemId() << "', entry_paths = '"
          << entry_paths << "'}";
  return file_system_->GetActions(entry_paths, std::move(callback));
}

AbortCallback CloudFileSystem::ExecuteAction(
    const std::vector<base::FilePath>& entry_paths,
    const std::string& action_id,
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(2) << "ExecuteAction {fsid = '" << GetFileSystemId()
          << "', entry_paths = '" << entry_paths << "', action_id = '"
          << action_id << "'}";
  return file_system_->ExecuteAction(entry_paths, action_id,
                                     std::move(callback));
}

AbortCallback CloudFileSystem::ReadDirectory(
    const base::FilePath& directory_path,
    storage::AsyncFileUtil::ReadDirectoryCallback callback) {
  VLOG(1) << "ReadDirectory {fsid = '" << GetFileSystemId()
          << "', directory_path = '" << directory_path << "'}";
  return file_system_->ReadDirectory(directory_path, callback);
}

AbortCallback CloudFileSystem::ReadFile(int operation_id,
                                        net::IOBuffer* buffer,
                                        int64_t offset,
                                        int length,
                                        ReadChunkReceivedCallback callback) {
  VLOG(1) << "ReadFile {fsid = '" << GetFileSystemId() << "', operation_id = '"
          << operation_id << "', offset = '" << offset << "', length = '"
          << length << "'}";
  DCHECK(operation_id_to_file_handle_[operation_id]);
  return file_system_->ReadFile(operation_id_to_file_handle_[operation_id],
                                buffer, offset, length, callback);
}

AbortCallback CloudFileSystem::OpenFile(const base::FilePath& file_path,
                                         OpenFileMode mode,
                                         OpenFileCallback callback) {
  VLOG(1) << "OpenFile {fsid = '" << GetFileSystemId() << "', file_path = '"
          << file_path << "', mode = '" << mode << "'}";
  return file_system_->OpenFile(
      file_path, mode,
      base::BindOnce(&CloudFileSystem::OnOpenFileCompleted,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
}

AbortCallback CloudFileSystem::CloseFile(
    int operation_id,
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(1) << "CloseFile {fsid = '" << GetFileSystemId() << "', operation_id = '"
          << operation_id << "'}";
  DCHECK(operation_id_to_file_handle_[operation_id]);
  return file_system_->CloseFile(
      operation_id_to_file_handle_[operation_id],
      base::BindOnce(&CloudFileSystem::OnCloseFileCompleted,
                     weak_ptr_factory_.GetWeakPtr(), operation_id,
                     std::move(callback)));
}

AbortCallback CloudFileSystem::CreateDirectory(
    const base::FilePath& directory_path,
    bool recursive,
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(1) << "CreateDirectory {fsid = '" << GetFileSystemId()
          << "', directory_path = '" << directory_path << "', recursive = '"
          << recursive << "'}";
  return file_system_->CreateDirectory(directory_path, recursive,
                                       std::move(callback));
}

AbortCallback CloudFileSystem::DeleteEntry(
    const base::FilePath& entry_path,
    bool recursive,
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(1) << "DeleteEntry {fsid = '" << GetFileSystemId() << "', entry_path = '"
          << entry_path << "', recursive = '" << recursive << "'}";
  return file_system_->DeleteEntry(entry_path, recursive, std::move(callback));
}

AbortCallback CloudFileSystem::CreateFile(
    const base::FilePath& file_path,
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(1) << "CreateFile {fsid = '" << GetFileSystemId() << "', file_path = '"
          << file_path << "'}";
  return file_system_->CreateFile(file_path, std::move(callback));
}

AbortCallback CloudFileSystem::CopyEntry(
    const base::FilePath& source_path,
    const base::FilePath& target_path,
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(1) << "CopyEntry {fsid = '" << GetFileSystemId() << "', source_path = '"
          << source_path << "', target_path = '" << target_path << "'}";
  return file_system_->CopyEntry(source_path, target_path, std::move(callback));
}

AbortCallback CloudFileSystem::WriteFile(
    int operation_id,
    net::IOBuffer* buffer,
    int64_t offset,
    int length,
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(1) << "WriteFile {fsid = '" << GetFileSystemId() << "', operation_id = '"
          << operation_id << "', offset = '" << offset << "', length = '"
          << length << "'}";
  DCHECK(operation_id_to_file_handle_[operation_id]);
  return file_system_->WriteFile(operation_id_to_file_handle_[operation_id],
                                 buffer, offset, length, std::move(callback));
}

AbortCallback CloudFileSystem::FlushFile(
    int operation_id,
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(1) << "FlushFile {fsid = '" << GetFileSystemId() << "', operation_id = '"
          << operation_id << "'}";
  DCHECK(operation_id_to_file_handle_[operation_id]);
  return file_system_->FlushFile(operation_id_to_file_handle_[operation_id],
                                 std::move(callback));
}

AbortCallback CloudFileSystem::MoveEntry(
    const base::FilePath& source_path,
    const base::FilePath& target_path,
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(1) << "MoveEntry {fsid = '" << GetFileSystemId() << "', source_path = '"
          << source_path << "', target_path = '" << target_path << "'}";
  return file_system_->MoveEntry(source_path, target_path, std::move(callback));
}

AbortCallback CloudFileSystem::Truncate(
    const base::FilePath& file_path,
    int64_t length,
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(1) << "Truncate {fsid = '" << GetFileSystemId() << "', file_path = '"
          << file_path << "', length = '" << length << "'}";
  return file_system_->Truncate(file_path, length, std::move(callback));
}

AbortCallback CloudFileSystem::AddWatcher(
    const GURL& origin,
    const base::FilePath& entry_path,
    bool recursive,
    bool persistent,
    storage::AsyncFileUtil::StatusCallback callback,
    storage::WatcherManager::NotificationCallback notification_callback) {
  VLOG(2) << "AddWatcher {fsid = '" << GetFileSystemId() << "', origin = '"
          << origin.spec() << "', entry_path = '" << entry_path
          << "', recursive = '" << recursive << "', persistent = '"
          << persistent << "'}";

  // Set timer if the File Manager is a watcher.
  file_manager_watchers_ +=
      file_manager::util::IsFileManagerURL(origin) ? 1 : 0;
  if (file_manager_watchers_ > 0 && !timer_.IsRunning()) {
    timer_.Start(FROM_HERE, kFileManagerWatcherInterval,
                 base::BindRepeating(&CloudFileSystem::OnTimer,
                                     weak_ptr_factory_.GetWeakPtr()));
  }

  return file_system_->AddWatcher(origin, entry_path, recursive, persistent,
                                  std::move(callback),
                                  std::move(notification_callback));
}

void CloudFileSystem::RemoveWatcher(
    const GURL& origin,
    const base::FilePath& entry_path,
    bool recursive,
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(2) << "RemoveWatcher {fsid = '" << GetFileSystemId() << "', origin = '"
          << origin.spec() << "', entry_path = '" << entry_path
          << "', recursive = '" << recursive << "'}";

  // Stop timer if the File Manager is not a watcher.
  file_manager_watchers_ -=
      file_manager::util::IsFileManagerURL(origin) ? 1 : 0;
  if (file_manager_watchers_ == 0 && timer_.IsRunning()) {
    timer_.Stop();
  }

  file_system_->RemoveWatcher(origin, entry_path, recursive,
                              std::move(callback));
}

const ProvidedFileSystemInfo& CloudFileSystem::GetFileSystemInfo() const {
  return file_system_->GetFileSystemInfo();
}

OperationRequestManager* CloudFileSystem::GetRequestManager() {
  return file_system_->GetRequestManager();
}

Watchers* CloudFileSystem::GetWatchers() {
  return file_system_->GetWatchers();
}

const OpenedFiles& CloudFileSystem::GetOpenedFiles() const {
  return file_system_->GetOpenedFiles();
}

void CloudFileSystem::AddObserver(ProvidedFileSystemObserver* observer) {
  file_system_->AddObserver(observer);
}

void CloudFileSystem::RemoveObserver(ProvidedFileSystemObserver* observer) {
  file_system_->RemoveObserver(observer);
}

void CloudFileSystem::Notify(
    const base::FilePath& entry_path,
    bool recursive,
    storage::WatcherManager::ChangeType change_type,
    std::unique_ptr<ProvidedFileSystemObserver::Changes> changes,
    const std::string& tag,
    storage::AsyncFileUtil::StatusCallback callback) {
  VLOG(2) << "Notify {fsid = '" << GetFileSystemId() << "', recursive = '"
          << recursive << "', change_type = '" << change_type << "', tag = '"
          << tag << "', changes = {"
          << (changes ? *changes : ProvidedFileSystemObserver::Changes())
          << "}}";
  return file_system_->Notify(entry_path, recursive, change_type,
                              std::move(changes), tag, std::move(callback));
}

void CloudFileSystem::Configure(
    storage::AsyncFileUtil::StatusCallback callback) {
  return file_system_->Configure(std::move(callback));
}

base::WeakPtr<ProvidedFileSystemInterface> CloudFileSystem::GetWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

std::unique_ptr<ScopedUserInteraction>
CloudFileSystem::StartUserInteraction() {
  return file_system_->StartUserInteraction();
}

const std::string CloudFileSystem::GetFileSystemId() const {
  return file_system_->GetFileSystemInfo().file_system_id();
}

void CloudFileSystem::OnTimer() {
  VLOG(2) << "OnTimer";
  // TODO(b/317137739): Replace this with a proper API call once one is
  // introduced.
  // Request that the file system syncs with the Cloud. The entry path is
  // insignficant, just pass it root.
  ExecuteAction({base::FilePath("/")}, kODFSSyncWithCloudAction,
                base::BindOnce([](base::File::Error result) {
                  VLOG(1) << "Action " << kODFSSyncWithCloudAction
                          << " completed: " << result;
                }));
}

void CloudFileSystem::OnOpenFileCompleted(OpenFileCallback callback,
                                          int file_handle,
                                          base::File::Error result) {
  int returned_id = file_handle;
  // If the file is opened successfully then hold the operation_id until the
  // file is closed.
  if (result == base::File::FILE_OK) {
    const int operation_id = ++next_operation_id_;
    operation_id_to_file_handle_[operation_id] = file_handle;
    returned_id = operation_id;
  }
  std::move(callback).Run(returned_id, result);
}

void CloudFileSystem::OnCloseFileCompleted(
    int operation_id,
    storage::AsyncFileUtil::StatusCallback callback,
    base::File::Error result) {
  // Closing is always final. Even if an error happened, the file is considered
  // closed on the C++ side.
  operation_id_to_file_handle_.erase(operation_id);

  std::move(callback).Run(result);
}

}  // namespace ash::file_system_provider
