// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_GLANCEABLES_TASKS_GLANCEABLES_TASKS_VIEW_H_
#define ASH_GLANCEABLES_TASKS_GLANCEABLES_TASKS_VIEW_H_

#include <memory>

#include "ash/api/tasks/tasks_client.h"
#include "ash/api/tasks/tasks_types.h"
#include "ash/ash_export.h"
#include "ash/glanceables/glanceables_metrics.h"
#include "ash/glanceables/tasks/glanceables_tasks_error_type.h"
#include "ash/system/unified/glanceable_tray_child_bubble.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/base/models/list_model.h"
#include "ui/views/layout/flex_layout_view.h"
#include "ui/views/view.h"
#include "ui/views/view_observer.h"

class GURL;

namespace views {
class LabelButton;
}  // namespace views

namespace ash {

class Combobox;
class GlanceablesListFooterView;
class GlanceablesProgressBarView;
class GlanceablesTaskViewV2;
class TasksComboboxModel;

// Temporary interface to allow smooth migration from `TasksBubbleView` to
// `GlanceablesTasksView`.
class ASH_EXPORT GlanceablesTasksViewBase : public GlanceableTrayChildBubble {
  METADATA_HEADER(GlanceablesTasksViewBase, GlanceableTrayChildBubble)

 public:
  explicit GlanceablesTasksViewBase(bool use_glanceables_container_style);
  GlanceablesTasksViewBase(const GlanceablesTasksViewBase&) = delete;
  GlanceablesTasksViewBase& operator=(const GlanceablesTasksViewBase&) = delete;
  ~GlanceablesTasksViewBase() override = default;

  // Invalidates any pending tasks, or tasks lists requests. Called when the
  // glanceables bubble widget starts closing to avoid unnecessary UI updates.
  virtual void CancelUpdates() = 0;
};

// Glanceables view responsible for interacting with Google Tasks.
class ASH_EXPORT GlanceablesTasksView : public GlanceablesTasksViewBase,
                                        public views::ViewObserver {
  METADATA_HEADER(GlanceablesTasksView, GlanceablesTasksViewBase)

 public:
  explicit GlanceablesTasksView(const ui::ListModel<api::TaskList>* task_lists);
  GlanceablesTasksView(const GlanceablesTasksView&) = delete;
  GlanceablesTasksView& operator=(const GlanceablesTasksView&) = delete;
  ~GlanceablesTasksView() override;

  // views::View:
  void ChildPreferredSizeChanged(View* child) override;

  // GlanceablesTasksViewBase:
  void CancelUpdates() override;

  // views::ViewObserver:
  void OnViewFocused(views::View* view) override;

  // Updates the cached task lists to `task_lists` and the tasks that are
  // supposed to show.
  void UpdateTaskLists(const ui::ListModel<api::TaskList>* task_lists);

 private:
  // The context of why the current task list is shown.
  enum class ListShownContext {
    // The list is a cached one that will be updated later after the lists data
    // are fetched.
    kCachedList,
    // The list that is loaded by default when users open glanceables.
    kInitialList,
    // The list is manually selected by the users through
    // `task_list_combo_box_view_`.
    kUserSelectedList
  };

  // Handles press behavior for `add_new_task_button_`.
  void AddNewTaskButtonPressed();

  // Creates a `GlanceablesTaskViewV2` instance with bound callbacks.
  std::unique_ptr<GlanceablesTaskViewV2> CreateTaskView(
      const std::string& task_list_id,
      const api::Task* task);

  // Handles switching between tasks lists.
  void SelectedTasksListChanged();
  void ScheduleUpdateTasks(ListShownContext context);
  void UpdateTasksInTaskList(const std::string& task_list_id,
                             const std::string& task_list_title,
                             ListShownContext context,
                             bool fetch_success,
                             const ui::ListModel<api::Task>* tasks);

  // Announces text describing the task list state through a screen
  // reader, using `task_list_combo_box_view_` view accessibility helper.
  void AnnounceListStateOnComboBoxAccessibility();

  // Marks the specified task as completed.
  void MarkTaskAsCompleted(const std::string& task_list_id,
                           const std::string& task_id,
                           bool completed);

  // Handles press behavior for icons that are used to open Google Tasks in the
  // browser.
  void ActionButtonPressed(TasksLaunchSource source, const GURL& target_url);

  // Saves the task (either creates or updates the existing one).
  // `view`     - individual task view which triggered this request.
  // `callback` - done callback passed from an individual task view.
  void SaveTask(const std::string& task_list_id,
                base::WeakPtr<GlanceablesTaskViewV2> view,
                const std::string& task_id,
                const std::string& title,
                api::TasksClient::OnTaskSavedCallback callback);

  // Handles completion of `SaveTask`.
  // `view`     - individual task view which triggered this request.
  // `callback` - callback passed from an individual task view via `SaveTask`.
  // `task`     - newly created or edited task if the request completes
  //              successfully, `nullptr` otherwise.
  void OnTaskSaved(base::WeakPtr<GlanceablesTaskViewV2> view,
                   const std::string& task_id,
                   api::TasksClient::OnTaskSavedCallback callback,
                   const api::Task* task);

  // Returns the current showing task list.
  const api::TaskList* GetActiveTaskList() const;

  // Creates and shows `error_message_` that depends on the `error_type`.
  void ShowErrorMessageWithType(GlanceablesTasksErrorType error_type);

  // Returns the string to show on `error_message_` according to the
  // `error_type`.
  std::u16string GetErrorString(GlanceablesTasksErrorType error_type) const;

  // Removes `task_view` from the tasks container.
  void RemoveTaskView(base::WeakPtr<GlanceablesTaskViewV2> task_view);

  // Creates and initializes `task_list_combo_box_view_`.
  void CreateComboBoxView();

  // This function should be called with `is_loading` = true if `this` is
  // waiting for fetched data to be returned. After the data arrives, resets the
  // states by calling with `is_loading` = false.
  void SetIsLoading(bool is_loading);

  // Model for the combobox used to change the active task list.
  std::unique_ptr<TasksComboboxModel> tasks_combobox_model_;

  // The number of times that the tasks list has been changed during the
  // lifetime of this view.
  int tasks_list_change_count_ = 0;

  // Whether the first task list has been shown during the lifetime of this
  // view.
  bool first_task_list_shown_ = false;

  // Owned by views hierarchy.
  raw_ptr<views::FlexLayoutView> tasks_header_view_ = nullptr;
  raw_ptr<Combobox> task_list_combo_box_view_ = nullptr;
  raw_ptr<views::FlexLayoutView> button_container_ = nullptr;
  raw_ptr<views::View> task_items_container_view_ = nullptr;
  raw_ptr<views::LabelButton> add_new_task_button_ = nullptr;
  raw_ptr<GlanceablesListFooterView> list_footer_view_ = nullptr;
  raw_ptr<GlanceablesProgressBarView> progress_bar_ = nullptr;

  // Records the time when the bubble was about to request a task list. Used for
  // metrics.
  base::TimeTicks tasks_requested_time_;

  // Cached to reset the value of the index of `task_list_combo_box_view_` when
  // the target task list failed to be loaded.
  std::optional<size_t> cached_selected_list_index_ = std::nullopt;

  // Number of tasks added by the user for the currently selected task list.
  // Task is considered "added" if task creation was requested via tasks API.
  // The count is reset when the selected task list changes.
  int added_tasks_ = 0;

  // Whether the current task list was empty when it got selected.
  bool task_list_initially_empty_ = false;

  // Whether the user had a single task list with no tasks when the current task
  // list was selected.
  bool user_with_no_tasks_ = false;

  // Callback that recreates `task_list_combo_box_view_`.
  base::OnceClosure recreate_combobox_callback_;

  base::ScopedObservation<views::View, views::ViewObserver>
      combobox_view_observation_{this};

  base::WeakPtrFactory<GlanceablesTasksView> weak_ptr_factory_{this};
};

}  // namespace ash

#endif  // ASH_GLANCEABLES_TASKS_GLANCEABLES_TASKS_VIEW_H_
