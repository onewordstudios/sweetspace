package fun.onewordstudios.sweetspace;

import org.libsdl.app.*;
import android.os.Bundle;

public class Sweetspace extends SDLActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // Make sure this is before calling super.onCreate
        setTheme(R.style.CUGLTheme);
        super.onCreate(savedInstanceState);
    }

}
