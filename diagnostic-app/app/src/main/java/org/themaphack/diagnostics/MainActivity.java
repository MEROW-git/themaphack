package org.themaphack.diagnostics;

import android.app.Activity;
import android.os.Build;
import android.os.Bundle;
import android.content.SharedPreferences;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import java.util.Arrays;

public final class MainActivity extends Activity {
    private static final String PREFERENCES = "diagnostics";
    private static final String SHOW_NATIVE_DETAILS = "show_native_details";

    private static native String nativeBuildInfo();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        String nativeStatus;
        try {
            System.loadLibrary("themaphack_diagnostics");
            nativeStatus = nativeBuildInfo();
        } catch (UnsatisfiedLinkError error) {
            nativeStatus = "Native diagnostic library failed to load: " + error.getMessage();
        }

        SharedPreferences preferences = getSharedPreferences(PREFERENCES, MODE_PRIVATE);
        LinearLayout content = new LinearLayout(this);
        content.setOrientation(LinearLayout.VERTICAL);
        int padding = (int) (24 * getResources().getDisplayMetrics().density);
        content.setPadding(padding, padding, padding, padding);

        TextView report = new TextView(this);
        report.setTextSize(16);
        report.setText("Themaphack build diagnostics\n\n"
                + "Android API: " + Build.VERSION.SDK_INT + "\n"
                + "Device ABIs: " + Arrays.toString(Build.SUPPORTED_ABIS) + "\n"
                + "64-bit ABIs: " + Arrays.toString(Build.SUPPORTED_64_BIT_ABIS) + "\n"
                + "App package: " + getPackageName() + "\n"
                + "Native library directory: " + getApplicationInfo().nativeLibraryDir + "\n\n"
                + "This app loads only its own diagnostic library. It does not inspect or modify the installed game.");

        TextView nativeReport = new TextView(this);
        nativeReport.setTextSize(16);
        nativeReport.setText("\nNative build: " + nativeStatus);
        nativeReport.setVisibility(preferences.getBoolean(SHOW_NATIVE_DETAILS, true) ? TextView.VISIBLE : TextView.GONE);

        CheckBox showDetails = new CheckBox(this);
        showDetails.setText("Show native build details");
        showDetails.setChecked(preferences.getBoolean(SHOW_NATIVE_DETAILS, true));
        showDetails.setOnCheckedChangeListener((button, checked) -> {
            preferences.edit().putBoolean(SHOW_NATIVE_DETAILS, checked).apply();
            nativeReport.setVisibility(checked ? TextView.VISIBLE : TextView.GONE);
        });

        content.addView(report);
        content.addView(showDetails);
        content.addView(nativeReport);

        ScrollView scrollView = new ScrollView(this);
        scrollView.setFitsSystemWindows(true);
        scrollView.addView(content);
        setContentView(scrollView);
    }
}
