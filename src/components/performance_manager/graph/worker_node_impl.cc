// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/performance_manager/graph/worker_node_impl.h"

#include "base/memory/raw_ptr.h"
#include "components/performance_manager/graph/frame_node_impl.h"
#include "components/performance_manager/graph/process_node_impl.h"
#include "content/public/browser/browser_thread.h"

namespace performance_manager {

// static
constexpr char WorkerNodeImpl::kDefaultPriorityReason[] =
    "default worker priority";

using PriorityAndReason = execution_context_priority::PriorityAndReason;

WorkerNodeImpl::WorkerNodeImpl(const std::string& browser_context_id,
                               WorkerType worker_type,
                               ProcessNodeImpl* process_node,
                               const blink::WorkerToken& worker_token)
    : browser_context_id_(browser_context_id),
      worker_type_(worker_type),
      process_node_(process_node),
      worker_token_(worker_token) {
  // Nodes are created on the UI thread, then accessed on the PM sequence.
  // `weak_this_` can be returned from GetWeakPtrOnUIThread() and dereferenced
  // on the PM sequence.
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DETACH_FROM_SEQUENCE(sequence_checker_);
  weak_this_ = weak_factory_.GetWeakPtr();

  DCHECK(process_node);
}

WorkerNodeImpl::~WorkerNodeImpl() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(client_frames_.empty());
  DCHECK(client_workers_.empty());
  DCHECK(child_workers_.empty());
  DCHECK(!execution_context_);
}

WorkerNode::WorkerType WorkerNodeImpl::GetWorkerType() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return worker_type_;
}

const std::string& WorkerNodeImpl::GetBrowserContextID() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return browser_context_id_;
}

const blink::WorkerToken& WorkerNodeImpl::GetWorkerToken() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return worker_token_;
}

resource_attribution::WorkerContext WorkerNodeImpl::GetResourceContext() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return resource_attribution::WorkerContext::FromWorkerNode(this);
}

const GURL& WorkerNodeImpl::GetURL() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return url_;
}

const PriorityAndReason& WorkerNodeImpl::GetPriorityAndReason() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return priority_and_reason_.value();
}

uint64_t WorkerNodeImpl::GetResidentSetKbEstimate() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return resident_set_kb_estimate_;
}

uint64_t WorkerNodeImpl::GetPrivateFootprintKbEstimate() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return private_footprint_kb_estimate_;
}

void WorkerNodeImpl::AddClientFrame(FrameNodeImpl* frame_node) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  bool inserted = client_frames_.insert(frame_node).second;
  DCHECK(inserted);

  frame_node->AddChildWorker(this);

  for (auto* observer : GetObservers())
    observer->OnClientFrameAdded(this, frame_node);
}

void WorkerNodeImpl::RemoveClientFrame(FrameNodeImpl* frame_node) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  for (auto* observer : GetObservers())
    observer->OnBeforeClientFrameRemoved(this, frame_node);

  frame_node->RemoveChildWorker(this);

  size_t removed = client_frames_.erase(frame_node);
  DCHECK_EQ(1u, removed);
}

void WorkerNodeImpl::AddClientWorker(WorkerNodeImpl* worker_node) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  switch (worker_type_) {
    case WorkerType::kDedicated:
      // Nested dedicated workers are only available from other dedicated
      // workers in Chrome.
      DCHECK_EQ(worker_node->GetWorkerType(), WorkerType::kDedicated);
      break;
    case WorkerType::kShared:
      // Nested shared workers are not available in Chrome.
      NOTREACHED();
      break;
    case WorkerType::kService:
      // A service worker may not control another service worker.
      DCHECK_NE(worker_node->GetWorkerType(), WorkerType::kService);
      break;
  }

  bool inserted = client_workers_.insert(worker_node).second;
  DCHECK(inserted);

  worker_node->AddChildWorker(this);

  for (auto* observer : GetObservers())
    observer->OnClientWorkerAdded(this, worker_node);
}

void WorkerNodeImpl::RemoveClientWorker(WorkerNodeImpl* worker_node) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  for (auto* observer : GetObservers())
    observer->OnBeforeClientWorkerRemoved(this, worker_node);

  worker_node->RemoveChildWorker(this);

  size_t removed = client_workers_.erase(worker_node);
  DCHECK_EQ(removed, 1u);
}

void WorkerNodeImpl::SetPriorityAndReason(
    const PriorityAndReason& priority_and_reason) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  priority_and_reason_.SetAndMaybeNotify(this, priority_and_reason);
}

void WorkerNodeImpl::SetResidentSetKbEstimate(uint64_t rss_estimate) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  resident_set_kb_estimate_ = rss_estimate;
}

void WorkerNodeImpl::SetPrivateFootprintKbEstimate(uint64_t pmf_estimate) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  private_footprint_kb_estimate_ = pmf_estimate;
}

void WorkerNodeImpl::OnFinalResponseURLDetermined(const GURL& url) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(url_.is_empty());
  url_ = url;

  for (auto* observer : GetObservers())
    observer->OnFinalResponseURLDetermined(this);
}

ProcessNodeImpl* WorkerNodeImpl::process_node() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return process_node_;
}

const base::flat_set<raw_ptr<FrameNodeImpl, CtnExperimental>>&
WorkerNodeImpl::client_frames() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return client_frames_;
}

const base::flat_set<raw_ptr<WorkerNodeImpl, CtnExperimental>>&
WorkerNodeImpl::client_workers() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return client_workers_;
}

const base::flat_set<raw_ptr<WorkerNodeImpl, CtnExperimental>>&
WorkerNodeImpl::child_workers() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return child_workers_;
}

base::WeakPtr<WorkerNodeImpl> WorkerNodeImpl::GetWeakPtrOnUIThread() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return weak_this_;
}

base::WeakPtr<WorkerNodeImpl> WorkerNodeImpl::GetWeakPtr() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return weak_factory_.GetWeakPtr();
}

void WorkerNodeImpl::OnJoiningGraph() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  // Make sure all weak pointers, even `weak_this_` that was created on the UI
  // thread in the constructor, can only be dereferenced on the graph sequence.
  weak_factory_.BindToCurrentSequence(
      base::subtle::BindWeakPtrFactoryPassKey());

  process_node_->AddWorker(this);
}

void WorkerNodeImpl::OnBeforeLeavingGraph() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  process_node_->RemoveWorker(this);
}

void WorkerNodeImpl::RemoveNodeAttachedData() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  execution_context_.reset();
}

const ProcessNode* WorkerNodeImpl::GetProcessNode() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return process_node();
}

const base::flat_set<const FrameNode*> WorkerNodeImpl::GetClientFrames() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  base::flat_set<const FrameNode*> client_frames;
  for (FrameNodeImpl* client : client_frames_) {
    client_frames.insert(static_cast<const FrameNode*>(client));
  }
  DCHECK_EQ(client_frames.size(), client_frames_.size());
  return client_frames;
}

bool WorkerNodeImpl::VisitClientFrames(const FrameNodeVisitor& visitor) const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  for (FrameNodeImpl* node : client_frames_) {
    if (!visitor(node)) {
      return false;
    }
  }
  return true;
}

const base::flat_set<const WorkerNode*> WorkerNodeImpl::GetClientWorkers()
    const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  base::flat_set<const WorkerNode*> client_workers;
  for (WorkerNodeImpl* client : client_workers_) {
    client_workers.insert(static_cast<const WorkerNode*>(client));
  }
  DCHECK_EQ(client_workers.size(), client_workers_.size());
  return client_workers;
}

bool WorkerNodeImpl::VisitClientWorkers(
    const WorkerNodeVisitor& visitor) const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  for (WorkerNodeImpl* node : client_workers_) {
    if (!visitor(node)) {
      return false;
    }
  }
  return true;
}

const base::flat_set<const WorkerNode*> WorkerNodeImpl::GetChildWorkers()
    const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  base::flat_set<const WorkerNode*> child_workers;
  for (WorkerNodeImpl* child : child_workers_) {
    child_workers.insert(static_cast<const WorkerNode*>(child));
  }
  DCHECK_EQ(child_workers.size(), child_workers_.size());
  return child_workers;
}

bool WorkerNodeImpl::VisitChildDedicatedWorkers(
    const WorkerNodeVisitor& visitor) const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  for (WorkerNodeImpl* worker_node_impl : child_workers_) {
    const WorkerNode* node = worker_node_impl;
    if (node->GetWorkerType() == WorkerType::kDedicated && !visitor(node)) {
      return false;
    }
  }
  return true;
}

void WorkerNodeImpl::AddChildWorker(WorkerNodeImpl* worker_node) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  bool inserted = child_workers_.insert(worker_node).second;
  DCHECK(inserted);
}

void WorkerNodeImpl::RemoveChildWorker(WorkerNodeImpl* worker_node) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  size_t removed = child_workers_.erase(worker_node);
  DCHECK_EQ(removed, 1u);
}

}  // namespace performance_manager
