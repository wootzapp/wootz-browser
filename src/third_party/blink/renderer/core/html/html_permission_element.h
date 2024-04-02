// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_HTML_HTML_PERMISSION_ELEMENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_HTML_HTML_PERMISSION_ELEMENT_H_

#include <optional>

#include "base/task/single_thread_task_runner.h"
#include "base/time/time.h"
#include "third_party/blink/public/mojom/permissions/permission.mojom-blink.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/css/properties/css_property.h"
#include "third_party/blink/renderer/core/css/resolver/cascade_filter.h"
#include "third_party/blink/renderer/core/dom/events/event_target.h"
#include "third_party/blink/renderer/core/html/html_element.h"
#include "third_party/blink/renderer/core/html/shadow/permission_shadow_element.h"
#include "third_party/blink/renderer/core/intersection_observer/intersection_observer.h"
#include "third_party/blink/renderer/platform/heap/collection_support/heap_vector.h"
#include "third_party/blink/renderer/platform/mojo/heap_mojo_receiver.h"
#include "third_party/blink/renderer/platform/mojo/heap_mojo_receiver_set.h"
#include "third_party/blink/renderer/platform/mojo/heap_mojo_remote.h"
#include "third_party/blink/renderer/platform/wtf/hash_map.h"
#include "third_party/blink/renderer/platform/wtf/hash_set.h"
#include "third_party/blink/renderer/platform/wtf/vector.h"

namespace blink {

class CORE_EXPORT HTMLPermissionElement final
    : public HTMLElement,
      public mojom::blink::PermissionObserver,
      public mojom::blink::EmbeddedPermissionControlClient {
  DEFINE_WRAPPERTYPEINFO();

 public:
  explicit HTMLPermissionElement(Document&);

  ~HTMLPermissionElement() override;

  const AtomicString& GetType() const;

  DEFINE_ATTRIBUTE_EVENT_LISTENER(resolve, kResolve)
  DEFINE_ATTRIBUTE_EVENT_LISTENER(dismiss, kDismiss)

  void Trace(Visitor*) const override;

  void AttachLayoutTree(AttachContext& context) override;

  bool granted() const { return permissions_granted_; }

  // Given an input type, return permissions list. This method is for testing
  // only.
  static Vector<mojom::blink::PermissionDescriptorPtr>
  ParsePermissionDescriptorsForTesting(const AtomicString& type);

  const Member<HTMLSpanElement>& permission_text_span_for_testing() const {
    return permission_text_span_;
  }

  CascadeFilter GetCascadeFilter() const override {
    // Reject all properties for which 'kValidForPermissionElement' is false.
    return CascadeFilter(CSSProperty::kValidForPermissionElement, false);
  }

  bool IsFullyVisibleForTesting() const { return is_fully_visible_; }

 private:
  // TODO(crbug.com/1315595): remove this friend class once migration
  // to blink_unittests_v2 completes.
  friend class ClickingEnabledChecker;

  FRIEND_TEST_ALL_PREFIXES(HTMLPemissionElementIntersectionTest,
                           IntersectionChanged);
  FRIEND_TEST_ALL_PREFIXES(HTMLPemissionElementFencedFrameTest,
                           NotAllowedInFencedFrame);

  enum class DisableReason {
    // This element is temporarily disabled for a short period
    // (`kDefaultDisableTimeout`) after being attached to the DOM.
    kRecentlyAttachedToDOM,

    // This element is temporarily disabled for a short period
    // (`kDefaultDisableTimeout`) after its intersection status changed from
    // invisible to visible.
    kIntersectionChanged,
  };

  // Ensure there is a connection to the permission service and return it.
  mojom::blink::PermissionService* GetPermissionService();
  void OnPermissionServiceConnectionFailed();

  // blink::Element implements
  void AttributeChanged(const AttributeModificationParams& params) override;
  void DidAddUserAgentShadowRoot(ShadowRoot&) override;
  void AdjustStyle(ComputedStyleBuilder& builder) override;

  // blink::Node override.
  void DefaultEventHandler(Event&) override;

  // Trigger permissions requesting in browser side by calling mojo
  // PermissionService's API.
  void RequestPageEmbededPermissions();

  void RegisterPermissionObserver(
      const mojom::blink::PermissionDescriptorPtr& descriptor,
      mojom::blink::PermissionStatus current_status);

  // mojom::blink::PermissionObserver override.
  void OnPermissionStatusChange(mojom::blink::PermissionStatus status) override;

  // mojom::blink::EmbeddedPermissionControlClient override.
  void OnEmbeddedPermissionControlRegistered(
      bool allowed,
      const std::optional<Vector<mojom::blink::PermissionStatus>>& statuses)
      override;

  // Callback triggered when permission is decided from browser side.
  void OnEmbeddedPermissionsDecided(
      mojom::blink::EmbeddedPermissionControlResult result);

  // Checks whether clicking is enabled at the moment. Clicking is disabled if
  // either:
  // 1) |DisableClickingIndefinitely| has been called and |EnableClicking| has
  // not been called (yet).
  // 2) |DisableClickingTemporarily| has been called and the specified duration
  // has not yet elapsed.
  //
  // Clicking can be disabled for multiple reasons simultaneously, and it needs
  // to be re-enabled (or the temporary duration to elapse) for each independent
  // reason before it becomes enabled again.
  bool IsClickingEnabled();

  // Disables clicking indefinitely for |reason|. |EnableClicking| for the same
  // reason needs to be called to re-enable it.
  void DisableClickingIndefinitely(DisableReason reason);

  // Disables clicking temporarily for |reason|. |EnableClicking| can be called
  // to re-enable clicking, or the duration needs to elapse.
  void DisableClickingTemporarily(DisableReason reason,
                                  const base::TimeDelta& duration);

  // Removes any existing (temporary or indefinite) disable reasons.
  void EnableClicking(DisableReason reason);

  // Similar to `EnableClicking`, calling this method can override any disabled
  // duration for a given reason, but after a delay.
  void EnableClickingAfterDelay(DisableReason reason,
                                const base::TimeDelta& delay);

  void UpdateAppearance();

  void UpdateText();

  void AddConsoleError(String error);

  void OnIntersectionChanged(
      const HeapVector<Member<IntersectionObserverEntry>>& entries);

  scoped_refptr<base::SingleThreadTaskRunner> GetTaskRunner();

  HeapMojoRemote<mojom::blink::PermissionService> permission_service_;

  // Holds all `PermissionObserver` receivers connected with remotes in browser
  // process. Each of them corresponds to a permission observer of one
  // descriptor in `permission_descriptors_`.
  // This set uses `PermissionName` as context type. Once a receiver call is
  // triggered, we look into its name to determine which permission is changed.
  HeapMojoReceiverSet<mojom::blink::PermissionObserver,
                      HTMLPermissionElement,
                      HeapMojoWrapperMode::kWithContextObserver,
                      mojom::blink::PermissionName>
      permission_observer_receivers_;

  // Holds a receiver connected with a remote `EmbeddedPermissionControlClient`
  // in browser process, allowing this element to receive PEPC events from
  // browser process.
  HeapMojoReceiver<mojom::blink::EmbeddedPermissionControlClient,
                   HTMLPermissionElement>
      embedded_permission_control_receiver_;

  //  Map holds all current permission statuses, keyed by permission name.
  using PermissionStatusMap =
      HashMap<mojom::blink::PermissionName, mojom::blink::PermissionStatus>;
  PermissionStatusMap permission_status_map_;

  AtomicString type_;

  // Holds reasons for which clicking is currently disabled (if any). Each
  // entry will have an expiration time associated with it, which can be
  // |base::TimeTicks::Max()| if it's indefinite.
  HashMap<DisableReason, base::TimeTicks> clicking_disabled_reasons_;

  Member<PermissionShadowElement> shadow_element_;
  Member<HTMLSpanElement> permission_text_span_;
  Member<IntersectionObserver> intersection_observer_;

  // Set to true only if all the corresponding permissions (from `type`
  // attribute) are granted.
  bool permissions_granted_ = false;

  // Set to true only if this element is fully visible on the viewport (observed
  // by IntersectionObserver).
  bool is_fully_visible_ = true;

  // The permission descriptors that correspond to a request made from this
  // permission element. Only computed once, when the `type` attribute is set.
  Vector<mojom::blink::PermissionDescriptorPtr> permission_descriptors_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_HTML_HTML_PERMISSION_ELEMENT_H_
