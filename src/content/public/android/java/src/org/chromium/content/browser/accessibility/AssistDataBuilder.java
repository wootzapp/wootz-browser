// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser.accessibility;

import android.app.assist.AssistStructure.ViewNode;
import android.view.ViewStructure;
import android.view.ViewStructure.HtmlInfo;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;

/**
 * Basic helper class to build ViewStructure objects for the WebContents in Chrome. The tree is
 * commonly referred to as the "AssistData" tree, hence the name, and to keep it separate from the
 * existing {@link ViewStructureBuilder.java} class which will be replaced by this class once the
 * unification of the code-paths is complete. This class can be used by the Native-side code {@link
 * web_contents_accessibility_android.cc} to construct objects for the virtual view hierarchy to
 * provide to the Android framework.
 */
@JNINamespace("content")
public class AssistDataBuilder {
    @CalledByNative
    public void populateBaseProperties(ViewStructure node, String className, int childCount) {
        node.setClassName(className);
        node.setChildCount(childCount);
    }

    @CalledByNative
    public void populateTextProperties(
            ViewStructure node,
            String text,
            boolean hasSelection,
            int selStart,
            int selEnd,
            int color,
            int bgcolor,
            float size,
            boolean bold,
            boolean italic,
            boolean underline,
            boolean lineThrough) {
        if (hasSelection) {
            node.setText(text, selStart, selEnd);
        } else {
            node.setText(text);
        }

        // If size is smaller than 0, then style information does not exist.
        if (size >= 0.0) {
            int style =
                    (bold ? ViewNode.TEXT_STYLE_BOLD : 0)
                            | (italic ? ViewNode.TEXT_STYLE_ITALIC : 0)
                            | (underline ? ViewNode.TEXT_STYLE_UNDERLINE : 0)
                            | (lineThrough ? ViewNode.TEXT_STYLE_STRIKE_THRU : 0);
            node.setTextStyle(size, color, bgcolor, style);
        }
    }

    // Stubbed.
    @CalledByNative
    public void populateBoundsProperties(ViewStructure node) {}

    @CalledByNative
    public void populateHTMLProperties(
            ViewStructure node, String htmlTag, String cssDisplay, String[][] htmlAttributes) {
        HtmlInfo.Builder htmlBuilder = node.newHtmlInfoBuilder(htmlTag);
        if (htmlBuilder == null) {
            return;
        }

        htmlBuilder.addAttribute("display", cssDisplay);
        for (String[] attr : htmlAttributes) {
            htmlBuilder.addAttribute(attr[0], attr[1]);
        }
        node.setHtmlInfo(htmlBuilder.build());
    }

    // Stubbed.
    @CalledByNative
    public void populateHTMLMetadataProperties(ViewStructure node) {}

    @CalledByNative
    public ViewStructure addChildNode(ViewStructure node, int childIndex) {
        return node.asyncNewChild(childIndex);
    }

    @CalledByNative
    public void commitNode(ViewStructure node) {
        node.asyncCommit();
    }
}
