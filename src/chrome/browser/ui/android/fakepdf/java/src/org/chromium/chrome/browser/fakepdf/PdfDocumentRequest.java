// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.fakepdf;

import android.net.Uri;

import androidx.annotation.NonNull;
import androidx.core.util.Preconditions;

import java.io.File;

/** Fake PDF viewer API. Will be removed once real APIs become available. */
public class PdfDocumentRequest {
    private final Uri mUri;
    private final File mFile;

    public PdfDocumentRequest(@NonNull Builder builder) {
        Preconditions.checkNotNull(builder);
        this.mUri = builder.mUri;
        this.mFile = builder.mFile;
    }

    public Uri getUri() {
        return this.mUri;
    }

    public File getFile() {
        return this.mFile;
    }

    public static class Builder {
        private Uri mUri;
        private File mFile;
        private PdfViewSettings mPdfViewSettings;

        public Builder() {}

        @NonNull
        public Builder setUri(Uri uri) {
            this.mUri = uri;
            return this;
        }

        @NonNull
        public Builder setFile(File file) {
            this.mFile = file;
            return this;
        }

        @NonNull
        public Builder setPdfViewSettings(PdfViewSettings settings) {
            this.mPdfViewSettings = settings;
            return this;
        }

        @NonNull
        public PdfDocumentRequest build() {
            return new PdfDocumentRequest(this);
        }
    }
}
