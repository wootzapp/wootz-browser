// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/system/focus_mode/focus_mode_tasks_provider.h"

#include <optional>
#include <vector>

#include "ash/api/tasks/tasks_types.h"
#include "base/strings/string_number_conversions.h"
#include "base/task/sequenced_task_runner.h"
#include "base/time/time.h"
#include "url/gurl.h"

namespace ash {

namespace {

// Struct representing everything we need to create a basic task.
struct DummyTaskData {
  const char* id;
  const char* title;
  bool completed;
  const char* due_string;
  const char* updated_string;
};

// Task data that we provide the user with initially to play around with.
constexpr DummyTaskData kTaskInitializationData[] = {
    {.id = "9",
     .title = "Task 9",
     .completed = false,
     .due_string = "12 Nov 2023 0:00 GMT",
     .updated_string = "12 Nov 2023 1:00 GMT"},
    {.id = "8",
     .title = "Task 8",
     .completed = false,
     .due_string = "19 Nov 2023 0:00 GMT",
     .updated_string = "14 Nov 2023 6:00 GMT"},
    {.id = "1",
     .title = "Task 1",
     .completed = false,
     .due_string = "20 Nov 2023 0:00 GMT",
     .updated_string = "19 Nov 2023 12:00 GMT"},
    {.id = "4",
     .title = "Task 4",
     .completed = false,
     .due_string = "21 Nov 2023 0:00 GMT",
     .updated_string = "13 Nov 2023 21:00 GMT"},
    {.id = "5",
     .title = "Task 5",
     .completed = false,
     .due_string = "23 Nov 2023 0:00 GMT",
     .updated_string = "18 Nov 2023 8:00 GMT"},
    {.id = "7",
     .title = "Task 7",
     .completed = false,
     .due_string = "24 Nov 2023 0:00 GMT",
     .updated_string = "19 Nov 2023 6:00 GMT"},
    {.id = "2",
     .title = "Task 2",
     .completed = false,
     .due_string = "30 Nov 2023 0:00 GMT",
     .updated_string = "14 Nov 2023 5:00 GMT"},
    {.id = "3",
     .title = "Task 3",
     .completed = false,
     .due_string = "",
     .updated_string = "18 Nov 2023 7:00 GMT"},
    {.id = "6",
     .title = "Task 6",
     .completed = false,
     .due_string = "",
     .updated_string = "15 Nov 2023 13:00 GMT"},
    {.id = "0",
     .title = "Task 0",
     .completed = false,
     .due_string = "",
     .updated_string = "10 Nov 2023 0:00 GMT"}};

std::unique_ptr<api::Task> GetTaskFromDummyTask(
    const DummyTaskData& task_data) {
  base::Time update_date;
  CHECK(base::Time::FromString(task_data.updated_string, &update_date));

  base::Time due_date;
  return std::make_unique<api::Task>(
      task_data.id, task_data.title,
      base::Time::FromString(task_data.due_string, &due_date)
          ? std::make_optional<base::Time>(due_date)
          : std::nullopt,
      task_data.completed,
      /*has_subtasks=*/false,
      /*has_email_link=*/false,
      /*has_notes=*/false, update_date, /*web_view_link=*/GURL());
}

}  // namespace

FocusModeTasksProvider::FocusModeTasksProvider() {
  for (const DummyTaskData& task_data : kTaskInitializationData) {
    InsertTask(GetTaskFromDummyTask(task_data));
  }
}

FocusModeTasksProvider::~FocusModeTasksProvider() = default;

std::vector<const api::Task*> FocusModeTasksProvider::GetTaskList() const {
  std::vector<const api::Task*> tasks;
  tasks.reserve(tasks_data_.size());

  for (const std::unique_ptr<api::Task>& task : tasks_data_) {
    tasks.push_back(task.get());
  }

  return tasks;
}

void FocusModeTasksProvider::AddTask(const std::string& title,
                                     OnTaskSavedCallback callback) {
  std::unique_ptr<api::Task> task = std::make_unique<api::Task>(
      /*id=*/base::NumberToString(task_id_++), title,
      /*due=*/std::nullopt, /*completed=*/false, /*has_subtasks=*/false,
      /*has_email_link=*/false,
      /*has_notes=*/false, /*updated=*/base::Time::Now(),
      /*web_view_link=*/GURL());

  api::Task* task_ptr = task.get();
  InsertTask(std::move(task));
  std::move(callback).Run(task_ptr);
}

void FocusModeTasksProvider::UpdateTaskTitle(const std::string& task_id,
                                             const std::string& title,
                                             OnTaskSavedCallback callback) {
  api::Task* found_task = nullptr;
  for (auto& task : tasks_data_) {
    if (task_id == task->id) {
      task->title = title;
      found_task = task.get();
      break;
    }
  }
  std::move(callback).Run(found_task);
}

void FocusModeTasksProvider::MarkAsCompleted(const std::string& task_id) {
  std::erase_if(tasks_data_,
                [task_id](const auto& task) { return task->id == task_id; });
}

void FocusModeTasksProvider::InsertTask(std::unique_ptr<api::Task> task) {
  for (auto it = tasks_data_.begin(); it != tasks_data_.end(); it++) {
    if ((task->due.value_or(base::Time::Max()) <
         it->get()->due.value_or(base::Time::Max())) ||
        (!it->get()->due.has_value() && task->updated > it->get()->updated)) {
      tasks_data_.insert(it, std::move(task));
      return;
    }
  }

  tasks_data_.push_back(std::move(task));
}

}  // namespace ash
