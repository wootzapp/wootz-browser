#ifndef COMPONENTS_DOWNLOAD_FILTERS_TASK_H_
#define COMPONENTS_DOWNLOAD_FILTERS_TASK_H_

#include <string>
#include <vector>

#include "base/functional/callback.h"
#include "base/memory/ref_counted.h"
#include "base/threading/thread_checker.h"
#include "components/update_client/network.h"
#include "url/gurl.h"
#include "base/files/file_path.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"

namespace adblock_updater {

// Errors generated as a result of calling Run() or by the service itself (UPDATE_IN_PROGRESS or UPDATE_CANCELED)
enum class Error {
  NONE = 0,
  UPDATE_IN_PROGRESS = 1,
  UPDATE_CANCELED = 2,
  UPDATE_NOT_NEEDED = 3,
  DOWNLOAD_ERROR = 4,
  INVALID_ARGUMENT = 5,
  MAX_VALUE,
};

enum class Event {
      // Sent before the update client does an update check.
      ADBLOCK_CHECKING_FOR_UPDATES = 1,

      // Sent after the new filters have been downloaded but before the install
      // or the upgrade is attempted.
      ADBLOCK_UPDATE_READY,

      // Sent when filters are being downloaded.
      ADBLOCK_UPDATE_DOWNLOADING,

      // Sent when filters have been successfully updated.
      ADBLOCK_UPDATED,

      // Sent when filters have not been updated because there
      // was no new version available
      //TODO: implement this with the headers check
      ADBLOCK_NOT_UPDATED,

      // Sent when an error ocurred during an update for any reason, including
      // the update check itself failed, or the download of the update payload
      // failed, or applying the update failed.
      ADBLOCK_UPDATE_ERROR,
};

// Defines a specialized task for updating AdBlock filters.
class DownloadFiltersTask : public base::RefCounted<DownloadFiltersTask> {
 public:
  DownloadFiltersTask(const DownloadFiltersTask&) = delete;
  DownloadFiltersTask& operator=(const DownloadFiltersTask&) = delete;

  using Callback =
      base::OnceCallback<void(scoped_refptr<DownloadFiltersTask> task, Error error)>;

  // |shared_url_network_factory| is injected here for the URL loader factory.
  // |is_foreground| is true when the update task is initiated by the user.
  // |filters_url| is the URL to load filters from.
  // |complete_callback| is called to return the execution flow back to creator of
  //    this task when the task is done.
  DownloadFiltersTask(scoped_refptr<network::SharedURLLoaderFactory> shared_url_network_factory,
                       bool is_foreground, const std::string& filters_url, base::Time min_last_modified,
                       Callback complete_callback);

  void Run();

  void Cancel();

  base::FilePath file_path();

  base::Time last_modified();

 private:
  ~DownloadFiltersTask();

  void OnDownloadProgress(uint64_t current);
  void OnResponseStarted(const GURL& final_url,
                         const network::mojom::URLResponseHead& response_head);
  void OnDownloadComplete(base::FilePath file_path);
  void OnHeadersDownloadComplete(scoped_refptr<net::HttpResponseHeaders> headers);
  void createSimpleURLLoader(bool);
  void internalDownload();

  // Called when the task has completed either because the task has run or
  // it has been canceled.
  void TaskComplete(Error error);

  base::ThreadChecker thread_checker_;
  scoped_refptr<network::SharedURLLoaderFactory> shared_url_network_factory_;
  const bool is_foreground_;
  Callback complete_callback_;
  std::unique_ptr<network::SimpleURLLoader> simple_url_loader_;

  // fields populated while downloading
  base::TimeTicks download_start_time_;
  GURL final_url_, filters_url_;
  int response_code_;
  base::Time last_modified_, min_last_modified_;
  base::FilePath file_path_;

  friend class base::RefCounted<DownloadFiltersTask>;
};

}  // namespace update_client

#endif  // COMPONENTS_DOWNLOAD_FILTERS_TASK_H_