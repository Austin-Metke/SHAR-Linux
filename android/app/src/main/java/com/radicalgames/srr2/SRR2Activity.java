package com.radicalgames.srr2;

import org.libsdl.app.SDLActivity;

public class SRR2Activity extends SDLActivity {

    @Override
    protected String[] getLibraries() {
        return new String[]{
            "SDL2",
            "SRR2"
        };
    }
}
