package com.radicalentertainment.shar;

import android.os.Bundle;
import android.os.Environment;
import android.view.WindowManager;

import java.io.File;

import org.libsdl.app.SDLActivity;

/**
 * SDL Activity wrapper for The Simpsons: Hit &amp; Run.
 *
 * <p>This activity extends {@link SDLActivity} provided by SDL2/SDL3 and
 * configures the native libraries, game‐data path, and window flags required
 * by the SRR2 engine on Android.</p>
 */
public class SHARActivity extends SDLActivity {

    /** Default directory for game data on external storage. */
    private static final String GAME_DATA_DIR = "SHAR";

    // ---------------------------------------------------------------
    //  SDLActivity overrides
    // ---------------------------------------------------------------

    /**
     * Return the list of shared libraries to load before entering the
     * native entry‐point.  Order matters – dependencies must come first.
     */
    @Override
    protected String[] getLibraries() {
        return new String[]{
            "SDL2",
            "openal",
            "SRR2"
        };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // Keep the screen on while the game is in the foreground.
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onResume() {
        super.onResume();
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    @Override
    protected void onPause() {
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        super.onPause();
    }

    // ---------------------------------------------------------------
    //  Helpers exposed to native code via JNI
    // ---------------------------------------------------------------

    /**
     * Return the absolute path to the game‐data directory.
     *
     * <p>The method checks, in order:</p>
     * <ol>
     *   <li>External storage {@code /SHAR/} directory</li>
     *   <li>The application's internal files directory as a fallback</li>
     * </ol>
     *
     * <p>Called from native code through JNI to resolve the asset root.</p>
     */
    public String getGameDataPath() {
        // Prefer external storage so users can drop files alongside the APK.
        File external = new File(
            Environment.getExternalStorageDirectory(), GAME_DATA_DIR);
        if (external.isDirectory()) {
            return external.getAbsolutePath();
        }

        // Fall back to app-private storage.
        return getFilesDir().getAbsolutePath();
    }
}
