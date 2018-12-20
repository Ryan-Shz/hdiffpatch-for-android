package com.github.patcher;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.github.ryan.patcher.Patcher;

import java.io.File;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private static final String TAG = "PatcherV3";
    private static final String TEST_DIR = Environment.getExternalStorageDirectory()
            + File.separator + "patch_test" + File.separator;
    private EditText mPrefixEd;
    private Patcher mPatcher;
    private Button mApplyPatchBtn;
    private ExecutorService mExecutorService;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mPrefixEd = findViewById(R.id.prefix_ed);
        mApplyPatchBtn = findViewById(R.id.apply_btn);
        mApplyPatchBtn.setOnClickListener(this);
        mPatcher = new Patcher();
        mExecutorService = Executors.newCachedThreadPool();
    }

    @Override
    public void onClick(View v) {
        String prefix = mPrefixEd.getText().toString().trim();
        if (TextUtils.isEmpty(prefix)) {
            showToast("please enter apk name.");
            return;
        }
        if (v == mApplyPatchBtn) {
            applyPatch(prefix);
        }
    }

    private void applyPatch(final String apkName) {
        mExecutorService.execute(new Runnable() {
            @Override
            public void run() {
                String oldApk = TEST_DIR + apkName + "_old.apk";
                String newApk = TEST_DIR + apkName + "_new_1.apk";
                String patchApk = TEST_DIR + apkName + "_patch.patch";
                long startTime = System.currentTimeMillis();
                int ret = mPatcher.patch(oldApk, patchApk, newApk);
                final long time = System.currentTimeMillis() - startTime;
                Log.d(TAG, "apply patch ret: " + ret + ", time: " + time);
                showToast("apply done: " + time);
            }
        });
    }

    private void showToast(final String message) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(MainActivity.this, message, Toast.LENGTH_SHORT).show();
            }
        });
    }
}
