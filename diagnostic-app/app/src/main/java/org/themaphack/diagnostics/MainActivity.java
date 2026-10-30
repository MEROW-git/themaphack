package org.themaphack.diagnostics;

import android.app.Activity;
import android.os.Build;
import android.os.Bundle;
import android.widget.ScrollView;
import android.widget.TextView;

import java.util.Arrays;

public final class MainActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        TextView report = new TextView(this);
        int padding = (int) (24 * getResources().getDisplayMetrics().density);
        report.setPadding(padding, padding, padding, padding);
        report.setTextSize(16);
        report.setText("Themaphack build diagnostics\n\n"
                + "Android API: " + Build.VERSION.SDK_INT + "\n"
                + "Device ABIs: " + Arrays.toString(Build.SUPPORTED_ABIS) + "\n"
                + "64-bit ABIs: " + Arrays.toString(Build.SUPPORTED_64_BIT_ABIS) + "\n"
                + "App package: " + getPackageName() + "\n"
                + "Native library directory: " + getApplicationInfo().nativeLibraryDir + "\n\n"
                + "This diagnostic app does not load the native library, inspect other apps, "
                + "or modify the installed game.");

        ScrollView scrollView = new ScrollView(this);
        scrollView.setFitsSystemWindows(true);
        scrollView.addView(report);
        setContentView(scrollView);
    }
}
