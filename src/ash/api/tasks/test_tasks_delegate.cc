// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/api/tasks/test_tasks_delegate.h"

#include "ash/api/tasks/tasks_client.h"
#include "base/notreached.h"
#include "components/account_id/account_id.h"

namespace ash::api {

TestTasksDelegate::TestTasksDelegate() = default;

TestTasksDelegate::~TestTasksDelegate() = default;

void TestTasksDelegate::UpdateClientForProfileSwitch(
    const AccountId& account_id) {
  NOTIMPLEMENTED();
}

void TestTasksDelegate::GetTaskLists(
    bool force_fetch,
    TasksClient::GetTaskListsCallback callback) {
  NOTIMPLEMENTED();
}

void TestTasksDelegate::GetTasks(const std::string& task_list_id,
                                 bool force_fetch,
                                 TasksClient::GetTasksCallback callback) {
  NOTIMPLEMENTED();
}

void TestTasksDelegate::AddTask(const std::string& task_list_id,
                                const std::string& title,
                                TasksClient::OnTaskSavedCallback callback) {
  NOTIMPLEMENTED();
}

void TestTasksDelegate::UpdateTask(const std::string& task_list_id,
                                   const std::string& task_id,
                                   const std::string& title,
                                   bool completed,
                                   TasksClient::OnTaskSavedCallback callback) {
  NOTIMPLEMENTED();
}

}  // namespace ash::api
