/**
 * A class to measure orientation on a locked display.
 *
 * To use this class do the following 3 steps in your activity:
 *
 * define 3 sensors as member variables
 *       Sensor accelerometer;
 *       Sensor magnetometer;
 *       Sensor vectorSensor;
 *       DeviceOrientation deviceOrientation;
 *
 * add this to the activity's onCreate
 *       mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
 *       accelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
 *       magnetometer = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
 *       deviceOrientation = new DeviceOrientation();
 *
 * add this to onResume
 *       mSensorManager.registerListener(deviceOrientation.getEventListener(), accelerometer, SensorManager.SENSOR_DELAY_UI);
 *       mSensorManager.registerListener(deviceOrientation.getEventListener(), magnetometer, SensorManager.SENSOR_DELAY_UI);
 *
 * add this to onPause
 *       mSensorManager.unregisterListener(deviceOrientation.getEventListener());
 *
 *
 * then, you can simply call * deviceOrientation.getOrientation() * wherever you want
 *
 *
 * This is another alternative to this class's approach:
 * http://stackoverflow.com/questions/11175599/how-to-measure-the-tilt-of-the-phone-in-xy-plane-using-accelerometer-in-android/15149421#15149421
 *
 *  @author abdelhady 
 *  @date 9/23/14.
 *  @link https://gist.github.com/abdelhady/501f6e48c1f3e32b253a#file-deviceorientation
 */
package org.libsdl.app;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.content.Context;
import android.util.DisplayMetrics;
import android.view.*;

public class DeviceOrientation {
    //private final int ORIENTATION_PORTRAIT = ExifInterface.ORIENTATION_ROTATE_90; // 6
    //private final int ORIENTATION_LANDSCAPE_REVERSE = ExifInterface.ORIENTATION_ROTATE_180; // 3
    //private final int ORIENTATION_LANDSCAPE = ExifInterface.ORIENTATION_NORMAL; // 1
    //private final int ORIENTATION_PORTRAIT_REVERSE = ExifInterface.ORIENTATION_ROTATE_270; // 8

    int smoothness = 1;
    private float averagePitch = 0;
    private float averageRoll = 0;
    private int orientation = SDLActivity.SDL_ORIENTATION_UNKNOWN;

    private float[] pitches;
    private float[] rolls;
    
    private boolean standard;

    public DeviceOrientation(boolean portrait) {
    	standard = portrait;
        pitches = new float[smoothness];
        rolls = new float[smoothness];
    }

    public SensorEventListener getEventListener() {
        return sensorEventListener;
    }

    public int getOrientation() {
        return orientation;
    }

    SensorEventListener sensorEventListener = new SensorEventListener() {
        float[] mGravity;
        float[] mGeomagnetic;

        @Override
        public void onSensorChanged(SensorEvent event) {
            if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
                mGravity = event.values;
            }
            if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD) {
                mGeomagnetic = event.values;
            }
            if (mGravity != null && mGeomagnetic != null) {
                float R[] = new float[9];
                float I[] = new float[9];
                boolean success = SensorManager.getRotationMatrix(R, I, mGravity, mGeomagnetic);
                if (success) {
                    float orientationData[] = new float[3];
                    SensorManager.getOrientation(R, orientationData);
                    averagePitch = addValue(orientationData[1], pitches);
                    averageRoll = addValue(orientationData[2], rolls);
                    orientation = calculateOrientation();
                }
            }
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            // TODO Auto-generated method stub

        }
    };

    private float addValue(float value, float[] values) {
        value = (float) Math.round((Math.toDegrees(value)));
        float average = 0;
        for (int i = 1; i < smoothness; i++) {
            values[i - 1] = values[i];
            average += values[i];
        }
        values[smoothness - 1] = value;
        average = (average + value) / smoothness;
        return average;
    }

    private int calculateOrientation() {
        Context context = SDLActivity.getContext();
        Display display = ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();

        // finding local orientation dip
        if (((orientation == SDLActivity.SDL_ORIENTATION_PORTRAIT || 
        	  orientation == SDLActivity.SDL_ORIENTATION_PORTRAIT_FLIPPED)
                && (averageRoll > -30 && averageRoll < 30))) {
            if (averagePitch > 0)
                return standard ? SDLActivity.SDL_ORIENTATION_PORTRAIT_FLIPPED : SDLActivity.SDL_ORIENTATION_LANDSCAPE_FLIPPED;
            else
                return standard ? SDLActivity.SDL_ORIENTATION_PORTRAIT : SDLActivity.SDL_ORIENTATION_LANDSCAPE;
        } else {
            // divides between all orientations
            if (Math.abs(averagePitch) >= 30) {
                if (averagePitch > 0)
                    return standard ? SDLActivity.SDL_ORIENTATION_PORTRAIT_FLIPPED : SDLActivity.SDL_ORIENTATION_LANDSCAPE_FLIPPED;
                else
                    return standard ? SDLActivity.SDL_ORIENTATION_PORTRAIT : SDLActivity.SDL_ORIENTATION_LANDSCAPE;
            } else {
                if (averageRoll > 0) {
                    return standard ? SDLActivity.SDL_ORIENTATION_LANDSCAPE_FLIPPED : SDLActivity.SDL_ORIENTATION_PORTRAIT;
                } else {
                    return standard ? SDLActivity.SDL_ORIENTATION_LANDSCAPE : SDLActivity.SDL_ORIENTATION_PORTRAIT_FLIPPED;
                }
            }
        }
    }
}