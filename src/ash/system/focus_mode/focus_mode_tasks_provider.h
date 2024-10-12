// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_SYSTEM_FOCUS_MODE_FOCUS_MODE_TASKS_PROVIDER_H_
#define ASH_SYSTEM_FOCUS_MODE_FOCUS_MODE_TASKS_PROVIDER_H_

#include <string>
#include <vector>

#include "ash/ash_export.h"
#include "base/containers/flat_set.h"
#include "base/functional/callback_forward.h"
#include "base/time/time.h"
#include "ui/base/models/list_model.h"

namespace ash {

namespace api {
struct Task;
}

class TaskFetcher;

// Represents a task.
struct ASH_EXPORT FocusModeTask {
  FocusModeTask();
  ~FocusModeTask();
  FocusModeTask(const FocusModeTask&);
  FocusModeTask(FocusModeTask&&);
  FocusModeTask& operator=(const FocusModeTask&);
  FocusModeTask& operator=(FocusModeTask&&);

  // TODO: Replace the condition below with `FocusModeTask::IsValid()`.
  bool empty() const { return task_list_id.empty(); }

  std::string task_list_id;
  std::string task_id;
  std::string title;

  // The time when this task was last updated.
  base::Time updated;

  // Optional due time for the task.
  std::optional<base::Time> due;
};

// A specialized interface that Focus Mode can use to fetch a filtered list of
// tasks to display. Currently only provides dummy data.
class ASH_EXPORT FocusModeTasksProvider {
 public:
  // Done callback for `AddTask` and `UpdateTaskTitle`. If the request completes
  // successfully, `task_entry` points to the newly created or updated
  // `FocusModeTask`, or an empty `FocusModeTask` with nullptr members
  // otherwise.
  using OnTaskSavedCallback =
      base::OnceCallback<void(const FocusModeTask& task_entry)>;

  using OnGetTasksCallback =
      base::OnceCallback<void(const std::vector<FocusModeTask>& tasks)>;

  FocusModeTasksProvider();
  FocusModeTasksProvider(const FocusModeTasksProvider&) = delete;
  FocusModeTasksProvider& operator=(const FocusModeTasksProvider&) = delete;
  ~FocusModeTasksProvider();

  // Provides a sorted list of `FocusModeTask` instances that can be displayed
  // in Focus Mode. The provided `callback` is invoked asynchronously when tasks
  // have been fetched.
  void GetSortedTaskList(OnGetTasksCallback callback);

  // Creates a new task with name `title` and adds it to `task_list_`. Returns
  // the added `FocusModeTask` in `callback`, or an empty `FocusModeTask` if an
  // error has occurred. Note that this will clear the internal cache.
  void AddTask(const std::string& title, OnTaskSavedCallback callback);

  // Finds the task by `task_list_id` and `task_id` and updates the task title
  // and completion status. Returns a `FocusModeTask` in `callback`, or an empty
  // `FocusModeTask` if the task could not be found or an error has occurred.
  void UpdateTask(const std::string& task_list_id,
                  const std::string& task_id,
                  const std::string& title,
                  bool completed,
                  OnTaskSavedCallback callback);

  // This kicks off a fetch of tasks from the backend.
  void ScheduleTaskListUpdate();

 private:
  void OnTasksFetched();
  void OnTaskSaved(const std::string& task_list_id,
                   const std::string& task_id,
                   bool completed,
                   OnTaskSavedCallback callback,
                   const api::Task* api_task);

  // Returns cached tasks according to this sort order:
  // 1. Entries added/updated by the user during the lifetime of this provider.
  // 2. Entries containing `Task`s which are past due.
  // 3. Entries containing `Task`s which are due in the next 24 hours.
  // 4. All other entries.
  // Entries within each group are sorted by their `Task`'s update date.
  std::vector<FocusModeTask> GetSortedTasksImpl();

  // Cache of tasks retrieved from the API.
  std::vector<FocusModeTask> tasks_;

  // Pending UI requests to get all tasks.
  std::vector<OnGetTasksCallback> get_tasks_requests_;

  // The ID of the task list to use when creating new tasks. This will be empty
  // until tasks have been fetched.
  std::string task_list_for_new_task_;

  // Holds a set of tasks that have been created or updated during the lifetime
  // of the provider. These tasks are pushed to the front of the sort order.
  base::flat_set<std::string> created_task_ids_;

  // Holds a set of tasks that have been deleted during the lifetime of the
  // provider.
  base::flat_set<std::string> deleted_task_ids_;

  // Populated when the provider is requesting tasks from the API, otherwise
  // empty.
  std::unique_ptr<TaskFetcher> task_fetcher_;

  // The timestamp of the last task fetch.
  base::Time task_fetch_time_;

  base::WeakPtrFactory<FocusModeTasksProvider> weak_factory_{this};
};

}  // namespace ash

#endif  // ASH_SYSTEM_FOCUS_MODE_FOCUS_MODE_TASKS_PROVIDER_H_
