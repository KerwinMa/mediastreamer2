/*
AndroidVideoApi5JniWrapper.java
Copyright (C) 2010  Belledonne Communications, Grenoble, France

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
package org.linphone.mediastream.video.capture;

import java.util.List;

import org.linphone.mediastream.video.capture.hwconf.AndroidCameraConfiguration;
import org.linphone.mediastream.video.capture.hwconf.AndroidCameraConfiguration.AndroidCamera;

import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.util.Log;
import android.view.SurfaceView;
 
/**
 * Wrapper for Android Camera API. Used by Mediastreamer to record 
 * video from webcam.
 * This file depends only on Android SDK >= 5
 */
public class AndroidVideoApi5JniWrapper {
	
	static public native void setAndroidSdkVersion(int version);
	public static native void putImage(long nativePtr, byte[] buffer);
	
	static public int detectCameras(int[] indexes, int[] frontFacing, int[] orientation) {
		Log.d("mediastreamer", "detectCameras\n");
		AndroidCamera[] cameras = AndroidCameraConfiguration.retrieveCameras();
		
		int nextIndex = 0;
		for (AndroidCamera androidCamera : cameras) {
			if (nextIndex == 2) {
				Log.w("mediastreamer", "Returning only the 2 first cameras (increase buffer size to retrieve all)");
				break;
			}
			// skip already added cameras
			indexes[nextIndex] = androidCamera.id;
			frontFacing[nextIndex] = androidCamera.frontFacing?1:0;
			orientation[nextIndex] = androidCamera.orientation;
			nextIndex++;
		}
		return cameras.length;
	}
	
	/**
	 * Return the hw-available available resolution best matching the requested one.
	 * Best matching meaning :
	 * - try to find the same one
	 * - try to find one just a little bigger (ex: CIF when asked QVGA)
	 * - as a fallback the nearest smaller one
	 * @param requestedW Requested video size width
	 * @param requestedH Requested video size height
	 * @return int[width, height] of the chosen resolution, may be null if no
	 * resolution can possibly match the requested one
	 */
	static public int[] selectNearestResolutionAvailable(int cameraId, int requestedW, int requestedH) {
		Log.d("mediastreamer", "selectNearestResolutionAvailable: " + cameraId + ", " + requestedW + "x" + requestedH);
		
		return selectNearestResolutionAvailableForCamera(cameraId, requestedW, requestedH);
	}
	
	public static Object startRecording(int cameraId, int width, int height, int fps, int rotation, final long nativePtr) {
		Log.d("mediastreamer", "startRecording(" + cameraId + ", " + width + ", " + height + ", " + fps + ", " + rotation + ", " + nativePtr + ")");
		Camera camera = Camera.open(); 
		
		applyCameraParameters(camera, width, height, fps);
		  
		camera.setPreviewCallback(new Camera.PreviewCallback() {
			public void onPreviewFrame(byte[] data, Camera camera) {
				// forward image data to JNI
				putImage(nativePtr, data);
			}
		});
		 
		camera.startPreview();
		Log.d("mediastreamer", "Returning camera object: " + camera);
		return camera; 
	} 
	
	public static void stopRecording(Object cam) {
		Log.d("mediastreamer", "stopRecording(" + cam + ")"); 
		Camera camera = (Camera) cam;
		 
		if (camera != null) {
			camera.setPreviewCallback(null);
			camera.stopPreview();
			camera.release(); 
		} else {
			Log.i("mediastreamer", "Cannot stop recording ('camera' is null)");
		}
	} 
	
	public static void setPreviewDisplaySurface(Object cam, Object surf) {
		Log.d("mediastreamer", "setPreviewDisplaySurface(" + cam + ", " + surf + ")");
		Camera camera = (Camera) cam;
		SurfaceView surface = (SurfaceView) surf;
		try {
			camera.setPreviewDisplay(surface.getHolder());
		} catch (Exception exc) {
			exc.printStackTrace(); 
		}
	}
	
	protected static int[] selectNearestResolutionAvailableForCamera(int id, int requestedW, int requestedH) {
		// inversing resolution since webcams only support landscape ones
		if (requestedH > requestedW) {
			int t = requestedH;
			requestedH = requestedW;
			requestedW = t;
		}
				
		AndroidCamera[] cameras = AndroidCameraConfiguration.retrieveCameras();
		List<Size> supportedSizes = null;
		for(AndroidCamera c: cameras) {
			if (c.id == id)
				supportedSizes = c.resolutions;
		}
		if (supportedSizes == null) {
		Log.e("mediastreamer", "Failed to retrieve supported resolutions.");
			return null;
		}
		Log.d("mediastreamer", supportedSizes.size() + " supported resolutions :");
		for(Size s : supportedSizes) {
			Log.d("mediastreamer", "\t" + s.width + "x" + s.height);
		}
		int rW = Math.max(requestedW, requestedH);
		int rH = Math.min(requestedW, requestedH);
		
		try {
			// look for nearest size
			Size result = null;
			int req = rW * rH;
			int minDist = Integer.MAX_VALUE;
			for(Size s: supportedSizes) {
				int dist = Math.abs(req - s.width * s.height);
				if (dist < minDist) {
					minDist = dist;
					result = s;
				}
				if (s.width == rW && s.height == rH)
					return new int[] {s.width, s.height};
			}
			return new int[] {result.width, result.height};
		} catch (Exception exc) {
			exc.printStackTrace();
			return null;
		} finally {
			Log.d("mediastreamer", "resolution selection done");
		}		
	}
	
	protected static void applyCameraParameters(Camera camera, int width, int height, int requestedFps) {
		Parameters params = camera.getParameters();
		 
		params.setPreviewSize(width, height); 
		
		List<Integer> supported = params.getSupportedPreviewFrameRates();
		if (supported != null) {
			int nearest = Integer.MAX_VALUE;
			for(Integer fr: supported) {
				int diff = Math.abs(fr.intValue() - requestedFps);
				if (diff < nearest) {
					nearest = diff;
					params.setPreviewFrameRate(fr.intValue());
				}
			}
			Log.d("mediastreamer", "Preview framerate set:" + params.getPreviewFrameRate());
		}
		
		camera.setParameters(params);		
	}
}