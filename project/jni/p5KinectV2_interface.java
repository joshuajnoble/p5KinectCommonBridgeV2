package p5Kinect;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PImage;

public enum TrackingState 
{
	TrackingState_NotTracked,
	TrackingState_Inferred,
	TrackingState_Tracked
};

public enum ColorImageFormat {

	ColorImageFormat_Yuv,
	ColorImageFormat_None,
	ColorImageFormat_Bgra,
	ColorImageFormat_Bayer,
	ColorImageFormat_Yuy2
};

public class SkeletonData {

	public TrackingState trackingState;
	public PVector[] skeletonPositions;
	public int[] skeletonPositionTrackingState;
	public int[] skeletonPositionConfidence;

	final int SKELETON_COUNT = 6;

	public SkeletonData() {
		trackingState = 0;
		dwTrackingID = 0;
		position = new PVector(0.0f, 0.0f, 0.0f);

		skeletonPositions = new PVector[NUI_SKELETON_POSITION_COUNT];
		skeletonPositionTrackingState = new int[NUI_SKELETON_POSITION_COUNT];
		skeletonPositionConfidence = new int[NUI_SKELETON_POSITION_COUNT];

		for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++) {
			skeletonPositions[i] = new PVector(0.0f, 0.0f, 0.0f);
			skeletonPositionTrackingState[i] = 0;
		}
	}

	public void copy(SkeletonData _s) {
		this.trackingState = _s.trackingState;
		this.dwTrackingID = _s.dwTrackingID;

		this.position.x = _s.position.x;
		this.position.y = _s.position.y;
		this.position.z = _s.position.z;

		for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++) {
			this.skeletonPositions[i].x = _s.skeletonPositions[i].x;
			this.skeletonPositions[i].y = _s.skeletonPositions[i].y;
			this.skeletonPositions[i].z = _s.skeletonPositions[i].z;
			this.skeletonPositionTrackingState[i] = _s.skeletonPositionTrackingState[i];
		}
	}
}


class p5KinectV2 implements Runnable 
{

	PApplet parent;
	
	public P5Kinect(PApplet _p) {
		parent = _p;

		skeletons = new SkeletonData[SKELETON_COUNT];

		for (int i = 0; i < SKELETON_COUNT; i++) {
			skeletons[i] = new SkeletonData();
		}

		// parent.registerDispose(this);
		init();
		welcome();
		(new Thread(this)).start();

	}

	public void run() {
		int fr = PApplet.round(1000.0f / parent.frameRate);
		while (true) {
			try {
				Thread.sleep(fr);
			} catch (InterruptedException e) {
				e.printStackTrace();
				return;
			}
			threadFunction();
		}
	}

	public PImage GetImage() {
		PImage img = parent.createImage(WIDTH, HEIGHT, ARGB);
		PApplet.arrayCopy(getColorPixels(), img.pixels);
		img.updatePixels();
		return img;
	}

	public PImage GetDepth() {
		PImage img = parent.createImage(WIDTH, HEIGHT, ARGB);
		PApplet.arrayCopy(getDepthPixels(), img.pixels);
		img.updatePixels();
		return img;
	}

	public PImage GetMask() {
		PImage img = parent.createImage(WIDTH, HEIGHT, ARGB);
		PApplet.arrayCopy(getPlayerPixels(), img.pixels);
		img.updatePixels();
		return img;
	}


	// new API
	static native boolean initSensor( int id );
	static native boolean initDepthStream( boolean mapDepthToColor);
	static native boolean initColorStream(boolean mapColorToDepth, ColorImageFormat format);
	static native boolean initIRStream( int width, int height );
	static native boolean initSkeletonStream( boolean seated );
	static native boolean start();

	static native void stop();

  	/// is the current frame new?
	static native boolean isFrameNew();
	static native boolean isFrameNewVideo();
	static native boolean isFrameNewDepth();
	static native boolean isNewSkeleton();
	static native boolean initBodyIndexStream();

	static native void setDepthClipping(float nearClip, float farClip);
	
	/// access to the pixel buffers and textures
	static native int[] getColorPixels();
	static native int[] getDepthPixels();       ///< grayscale values
	static native int[] getRawDepthPixels();	///< raw 11 bit values
	static native int[] getPlayerPixels();

	/// enable/disable frame loading into textures on update()
	static native void setUseTexture(boolean bUse);

	// /// draw the video texture
	void drawSkeleton(int index, int[] scale);
	void drawAllSkeletons(int[] scale);}

