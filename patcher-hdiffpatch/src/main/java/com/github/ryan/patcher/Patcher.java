package com.github.ryan.patcher;

public class Patcher {

    static {
        System.loadLibrary("patcher");
    }

    public native int applyPatch(String oldFilePath, String diffFilePath, String newFilePath);

    public int patch(String oldFilePath, String diffFilePath, String newFilePath) {
        return applyPatch(oldFilePath, diffFilePath, newFilePath);
    }
}
