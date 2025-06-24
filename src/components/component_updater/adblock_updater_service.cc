#include "components/component_updater/adblock_updater_service.h"

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/task/single_thread_task_runner.h"
#include "base/threading/thread_checker.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "url/gurl.h"
#include "base/strings/safe_sprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"


namespace adblock_updater {

// all constants express seconds
// these could be made configurable
const int initial_check_delay = 5,
      next_check_delay = 60*60*24*7, // 1 week
      on_demand_check_delay = 60; // minimum 1 minute between each on-demand check

AdBlockUpdaterService::AdBlockUpdaterService(scoped_refptr<network::SharedURLLoaderFactory> shared_url_network_factory, std::unique_ptr<component_updater::UpdateScheduler> scheduler,
   subresource_filter::RulesetService* ruleset_service, std::string filters_url)
 : ruleset_service_(ruleset_service), shared_url_network_factory_(shared_url_network_factory), scheduler_(std::move(scheduler)) {
  DCHECK(ruleset_service);
  filters_url_ = filters_url;
}

AdBlockUpdaterService::~AdBlockUpdaterService() {
  DCHECK(thread_checker_.CalledOnValidThread());
}

void AdBlockUpdaterService::AddObserver(Observer* observer) {
  DCHECK(thread_checker_.CalledOnValidThread());
  observer_list_.AddObserver(observer);
}

void AdBlockUpdaterService::RemoveObserver(Observer* observer) {
  DCHECK(thread_checker_.CalledOnValidThread());
  observer_list_.RemoveObserver(observer);
}

void AdBlockUpdaterService::NotifyObservers(Event event) {
  DCHECK(thread_checker_.CalledOnValidThread());
  for (auto& observer : observer_list_)
    observer.OnEvent(event);
}

void AdBlockUpdaterService::Start() {
  DCHECK(thread_checker_.CalledOnValidThread());

  // avoid multiple scheduling
  if (scheduled_) {
    return;
  }
  scheduled_ = true;

  LOG(INFO) << "AdBlock: Starting up. "
          << "First update attempt will take place in "
          << initial_check_delay << " seconds. "
          << "Next update attempt will take place in "
          << next_check_delay << " seconds. ";

  scheduler_->Schedule(
      base::Seconds(initial_check_delay),
      base::Seconds(next_check_delay),
      base::BindRepeating(&AdBlockUpdaterService::OnDemandScheduledUpdate,
                 base::Unretained(this)), base::DoNothing());
}

void AdBlockUpdaterService::OnDemandScheduledUpdate(component_updater::UpdateScheduler::OnFinishedCallback on_finished) {
  //TODO: call on_finished
  OnDemandUpdateAsNeeded(false, Callback());
}

bool AdBlockUpdaterService::OnDemandUpdate(Callback on_finished) {
  return OnDemandUpdateAsNeeded(true, std::move(on_finished));
}

bool AdBlockUpdaterService::OnDemandUpdateAsNeeded(bool is_foreground, Callback on_finished) {
  DCHECK(thread_checker_.CalledOnValidThread());

  // Check if the request is too early
  if (!last_update_.is_null()) {
    base::TimeDelta delta =
        base::TimeTicks::Now() - last_update_;
    if (delta < base::Seconds(on_demand_check_delay)) {
      return false;
    }
  }

  if (is_updating_) {
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(std::move(on_finished),
                                    Error::UPDATE_IN_PROGRESS));
    return false;
  }
  is_updating_ = true;

  base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(&AdBlockUpdaterService::NotifyObservers, base::Unretained(this), Event::ADBLOCK_CHECKING_FOR_UPDATES));

  base::Time::Exploded e = {0};
  base::Time min_last_modified = base::Time();
  auto version = ruleset_service_->GetMostRecentlyIndexedVersion();
  std::vector<std::string> tokens =
      base::SplitString(version.content_version, ".", base::KEEP_WHITESPACE, base::SPLIT_WANT_ALL);
  int i = 0;
  bool failed = false;
  for (const std::string& token : tokens) {
    // parse as number
    int n = 0;
    if (!base::StringToInt(token, &n)) {
      failed = true;
      break;
    }

    switch (i++) {
      case 0:
        e.year = 2019 + n;
        break;
      case 1:
        e.month = n + 1;
        break;
      case 2:
        e.day_of_month = n + 1;
        break;
      case 3:
        e.second = n % 60;
        n -= e.second;
        n /= 60;
        e.minute = n % 60;
        e.hour = n / 60;
        break;
      default:
        failed = true;
        break;
    }
  }

  if (failed) {
    LOG(WARNING) << "AdBlock: Failed to parse most recent version as x.y.z.w dot-separated integers";
  } else {
    if (!base::Time::FromUTCExploded(e, &min_last_modified))
      LOG(WARNING) << "AdBlock: Failed to convert version to time.";
    else
      LOG(INFO) << "AdBlock: Parsed version timestamp: " << min_last_modified;
  }

  // avoid making a new request if version-based time is recent enough
  if (!failed) {
    base::TimeDelta delta =
        base::Time::Now() - min_last_modified;
    if (delta < base::Seconds(on_demand_check_delay)) {
      LOG(INFO) << "AdBlock: Update check not yet needed - version is " << delta.InSeconds() << " seconds old";
      is_updating_ = false;
      return false;
    }
  }

  last_update_ = base::TimeTicks::Now();
  LOG(INFO) << "AdBlock: Starting download of filters from " << filters_url_;
  auto task = base::MakeRefCounted<DownloadFiltersTask>(
      shared_url_network_factory_,
      is_foreground, filters_url_,
      min_last_modified,
      base::BindOnce(&AdBlockUpdaterService::OnUpdateComplete, base::Unretained(this),
                     std::move(on_finished)));

  // run task now; task is responsible for downloading the filters (if Last-Modified header is more recent)
  // and then clearing the 'is_updating' status
  base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(FROM_HERE,
      base::BindOnce(&DownloadFiltersTask::Run, base::Unretained(task.get())));
  tasks_.insert(task);

  base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(&AdBlockUpdaterService::NotifyObservers, base::Unretained(this), Event::ADBLOCK_UPDATE_DOWNLOADING));

  return true;
}

void AdBlockUpdaterService::OnUpdateComplete(Callback on_finished,
                                        scoped_refptr<DownloadFiltersTask> task,
                                        Error error) {
  DCHECK(thread_checker_.CalledOnValidThread());
  LOG(INFO) << "AdBlock: Update completed with error code: " << static_cast<int>(error);

  auto file_path = task->file_path();
  if (error == Error::NONE) {
    LOG(INFO) << "AdBlock: Successfully downloaded filters to " << file_path;
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(&AdBlockUpdaterService::NotifyObservers, base::Unretained(this), Event::ADBLOCK_UPDATE_READY));

    subresource_filter::UnindexedRulesetInfo ruleset_info;
    ruleset_info.ruleset_path = file_path;
    ruleset_info.delete_ruleset_path = true;
    ruleset_info.content_version = "0.0.0.0";
    DCHECK(!ruleset_info.ruleset_path.empty());

    // convert Last-Modified timestamp fetched by the DownloadFiltersTask to a semver version
    auto t = task->last_modified();
    bool ignore_version = t.is_null();
    if (!ignore_version) {
      base::Time::Exploded e;
      t.UTCExplode(&e);

      // convert time to version
      const int major = e.year - 2019,
                minor = e.month - 1,
                patch = e.day_of_month - 1,
                revision = (e.hour*60+e.minute)*60 + e.second;
      if (major < 0) {
         LOG(WARNING) << "AdBlock: Too old Last-Modified header (" << t << "), ignoring version check.";
      } else {
        char version_buffer[32];
        base::strings::SafeSNPrintf(version_buffer, sizeof(version_buffer), "%d.%d.%d.%d",
                                major, minor, patch, revision);

        ruleset_info.content_version = version_buffer;

      }
    } else
      LOG(WARNING) << "AdBlock: Invalid Last-Modified header, ignoring version check.";
    ruleset_service_->IndexAndStoreAndPublishRulesetIfNeeded(ruleset_info, ignore_version);

    base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(&AdBlockUpdaterService::NotifyObservers, base::Unretained(this), Event::ADBLOCK_UPDATED));
  } else if (error == Error::UPDATE_NOT_NEEDED) {
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(&AdBlockUpdaterService::NotifyObservers, base::Unretained(this), Event::ADBLOCK_NOT_UPDATED));
  } else {
    LOG(ERROR) << "AdBlock: Update failed with error: " << static_cast<int>(error);
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(&AdBlockUpdaterService::NotifyObservers, base::Unretained(this), Event::ADBLOCK_UPDATE_ERROR));
  }

  //TODO: run these only when index-and-store is actually finished?
  // would require exposing the callback in IndexAndStoreAndPublishRulesetIfNeeded
  if (!on_finished.is_null()) {
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(std::move(on_finished), error));
  }

  // mark as not updating
  is_updating_ = false;
  tasks_.erase(task);
}

}  // namespace adblock_updater