// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.pdf;

import static org.mockito.Mockito.doReturn;

import android.app.Activity;
import android.content.Context;
import android.graphics.Rect;
import android.view.View;
import android.view.ViewGroup;

import androidx.test.ext.junit.rules.ActivityScenarioRule;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import org.chromium.base.supplier.DestroyableObservableSupplier;
import org.chromium.base.test.BaseRobolectricTestRunner;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.ui.native_page.NativePageHost;
import org.chromium.components.embedder_support.util.UrlConstants;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.ui.base.TestActivity;

@RunWith(BaseRobolectricTestRunner.class)
public class PdfPageUnitTest {

    @Rule
    public ActivityScenarioRule<TestActivity> mActivityScenarioRule =
            new ActivityScenarioRule<>(TestActivity.class);

    @Mock private NativePageHost mMockNativePageHost;
    @Mock private Profile mMockProfile;
    @Mock private DestroyableObservableSupplier<Rect> mMarginSupplier;
    @Mock private Context mContext;
    @Mock private LoadUrlParams mLoadUrlParams;
    private Activity mActivity;
    private AutoCloseable mCloseableMocks;

    private static final String CONTENT_URL = "content://media/external/downloads/1000000022";
    private static final String FILE_URL = "file:///media/external/downloads/sample.pdf";
    private static final String PDF_LINK = "https://www.foo.com/testfiles/pdf/sample.pdf";
    private static final String FILE_PATH = "/media/external/downloads/sample.pdf";
    private static final String FILE_NAME = "sample.pdf";
    private static final String IMAGE_FILE_URL = "file:///media/external/downloads/sample.jpg";

    @Before
    public void setUp() {
        mCloseableMocks = MockitoAnnotations.openMocks(this);
        mActivityScenarioRule
                .getScenario()
                .onActivity(
                        activity -> {
                            mActivity = activity;
                            doReturn(activity).when(mMockNativePageHost).getContext();
                        });
        doReturn(mMarginSupplier).when(mMockNativePageHost).createDefaultMarginSupplier();
        PdfUtils.setUseAndroidPdfViewerForTesting(true);
    }

    @After
    public void tearDown() throws Exception {
        PdfUtils.setUseAndroidPdfViewerForTesting(false);
        mCloseableMocks.close();
    }

    @Test
    public void testCreatePdfPage_WithContentUri() {
        PdfPage pdfPage = new PdfPage(mMockNativePageHost, mMockProfile, mActivity, CONTENT_URL);
        Assert.assertNotNull(pdfPage);
        Assert.assertEquals(
                "Pdf page host should match.", UrlConstants.PDF_HOST, pdfPage.getHost());
        Assert.assertEquals("Pdf page url should match.", CONTENT_URL, pdfPage.getUrl());
        Assert.assertFalse(
                "Pdf should not be loaded when the view is not attached to window.",
                pdfPage.mPdfCoordinator.getIsPdfLoadedForTesting());

        // Simulate tab brought from background to foreground
        View view = pdfPage.mPdfCoordinator.getView();
        ViewGroup contentView = mActivity.findViewById(android.R.id.content);
        contentView.addView(view);
        Assert.assertTrue(
                "Pdf should be loaded when the view is attached to window.",
                pdfPage.mPdfCoordinator.getIsPdfLoadedForTesting());
        contentView.removeView(view);
    }

    @Test
    public void testCreatePdfPage_WithFileUri() {
        PdfPage pdfPage = new PdfPage(mMockNativePageHost, mMockProfile, mActivity, FILE_URL);
        Assert.assertNotNull(pdfPage);
        Assert.assertEquals("Pdf page title should match.", FILE_NAME, pdfPage.getTitle());
        Assert.assertEquals(
                "Pdf page host should match.", UrlConstants.PDF_HOST, pdfPage.getHost());
        Assert.assertEquals("Pdf page url should match.", FILE_URL, pdfPage.getUrl());
        Assert.assertFalse(
                "Pdf should not be loaded when the view is not attached to window.",
                pdfPage.mPdfCoordinator.getIsPdfLoadedForTesting());

        // Simulate tab brought from background to foreground
        View view = pdfPage.mPdfCoordinator.getView();
        ViewGroup contentView = mActivity.findViewById(android.R.id.content);
        contentView.addView(view);
        Assert.assertTrue(
                "Pdf should be loaded when the view is attached to window.",
                pdfPage.mPdfCoordinator.getIsPdfLoadedForTesting());
        contentView.removeView(view);
    }

    @Test
    public void testCreatePdfPage_WithPdfLink() {
        PdfPage pdfPage = new PdfPage(mMockNativePageHost, mMockProfile, mActivity, PDF_LINK);
        Assert.assertNotNull(pdfPage);
        Assert.assertFalse(
                "Pdf should not be loaded when the download is not completed.",
                pdfPage.mPdfCoordinator.getIsPdfLoadedForTesting());

        // Simulate download complete
        pdfPage.onDownloadComplete(FILE_NAME, FILE_PATH);
        Assert.assertEquals("Pdf page title should match.", FILE_NAME, pdfPage.getTitle());
        Assert.assertEquals(
                "Pdf page host should match.", UrlConstants.PDF_HOST, pdfPage.getHost());
        Assert.assertEquals("Pdf page url should match.", PDF_LINK, pdfPage.getUrl());
        Assert.assertFalse(
                "Pdf should not be loaded when the view is not attached to window.",
                pdfPage.mPdfCoordinator.getIsPdfLoadedForTesting());

        // Simulate tab brought from background to foreground
        View view = pdfPage.mPdfCoordinator.getView();
        ViewGroup contentView = mActivity.findViewById(android.R.id.content);
        contentView.addView(view);
        Assert.assertTrue(
                "Pdf should be loaded when the view is attached to window.",
                pdfPage.mPdfCoordinator.getIsPdfLoadedForTesting());
        contentView.removeView(view);
    }

    @Test
    public void testGetFileNameFromUrl() {
        String filename = PdfUtils.getFileNameFromUrl(FILE_URL);
        Assert.assertEquals("Filename does not match for file url.", FILE_NAME, filename);

        filename = PdfUtils.getFileNameFromUrl(PDF_LINK);
        Assert.assertEquals(
                "Filename does not match for pdf link.", PdfUtils.PDF_PAGE_TITLE, filename);
    }

    @Test
    public void testIsPdfNavigation_FileScheme() {
        boolean result = PdfUtils.isPdfNavigation(FILE_URL, null);
        Assert.assertTrue("It is pdf navigation when file extension is pdf.", result);

        result = PdfUtils.isPdfNavigation(IMAGE_FILE_URL, null);
        Assert.assertFalse("It is not pdf navigation when file extension is not pdf.", result);
    }

    @Test
    public void testIsPdfNavigation_PdfLink() {
        doReturn(true).when(mLoadUrlParams).getIsPdf();
        boolean result = PdfUtils.isPdfNavigation(PDF_LINK, mLoadUrlParams);
        Assert.assertTrue("It is pdf navigation when IsPdf is set in LoadUrlParams.", result);

        doReturn(false).when(mLoadUrlParams).getIsPdf();
        result = PdfUtils.isPdfNavigation(PDF_LINK, mLoadUrlParams);
        Assert.assertFalse(
                "It is not pdf navigation when IsPdf is not set in LoadUrlParams.", result);

        result = PdfUtils.isPdfNavigation(PDF_LINK, null);
        Assert.assertFalse("It is not pdf navigation when LoadUrlParams is null.", result);
    }

    @Test
    public void testIsPdfNavigation_SchemeNotMatch() {
        boolean result = PdfUtils.isPdfNavigation(UrlConstants.HISTORY_URL, null);
        Assert.assertFalse(
                "It is not pdf navigation when the scheme is not one of content/file/http/https.",
                result);
    }
}
