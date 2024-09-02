package org.chromium.chrome.browser.hub;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.LinearLayout;
import android.view.Gravity;
import android.view.View;

import org.chromium.chrome.browser.flags.ChromeFeatureList;

public class ReversedLinearLayout extends LinearLayout {
    private boolean mReversedLayout;

	public ReversedLinearLayout(Context context) {
		super(context);
        setReversedLayout();
	}

	public ReversedLinearLayout(Context context, AttributeSet attrs) {
		super(context, attrs);
        setReversedLayout();
	}

	public ReversedLinearLayout(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
        setReversedLayout();
	}

    private void setReversedLayout() {
        mReversedLayout = ChromeFeatureList.sMoveTopToolbarToBottom.isEnabled();
        if (mReversedLayout)
            setGravity(Gravity.START | Gravity.BOTTOM);
    }

    @Override
    public View getChildAt(int i) {
        if (mReversedLayout)
		    return super.getChildAt(getChildCount() - 1 - i);
        else
            return super.getChildAt(i);
	}
}