package org.chromium.chrome.browser.firstrun;

import org.chromium.base.supplier.OneshotSupplier;
import org.chromium.chrome.browser.profiles.ProfileProvider;
import org.chromium.components.signin.AccountManagerFacadeProvider;

public abstract class WootzFirstRunFlowSequencer extends FirstRunFlowSequencer {
    public WootzFirstRunFlowSequencer(OneshotSupplier<ProfileProvider> profileSupplier) {
        super(
                profileSupplier,
                new ChildAccountStatusSupplier(
                        AccountManagerFacadeProvider.getInstance(),
                        FirstRunAppRestrictionInfo.takeMaybeInitialized()));
    }

    @Override
    public void start() {
        super.start();
    }
}