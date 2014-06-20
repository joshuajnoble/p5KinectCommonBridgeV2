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

public enum JointType
{
	JointType_Head, 
	JointType_Neck,
	JointType_SpineShoulder,
	
	JointType_SpineMid,
	JointType_SpineBase,
	JointType_ShoulderRight,
	JointType_ShoulderLeft,
	JointType_HipRight,
	JointType_HipLeft,

	// Right Arm    
	JointType_ElbowRight,
	JointType_WristRight,
	JointType_HandRight,
	JointType_HandTipRight,
	JointType_ThumbRight,

	// Left Arm
	JointType_ElbowLeft,
	JointType_WristLeft,
	JointType_HandLeft,
	JointType_HandTipLeft,
	JointType_ThumbLeft,

	// Right Leg
	JointType_KneeRight,
	JointType_AnkleRight,
	JointType_FootRight,

	// Left Leg
	JointType_KneeLeft,
	JointType_AnkleLeft,
	JointType_FootLeft

};

public class JointMapping {
	JointType first;
	JointType second;
}

public class SkeletonData {

	public TrackingState trackingState;
	public map<JointType, PVector> skeletonPositions;
	public map<JointType, PVector> skeletonOrientation;
	public map<JointType, PVector> skeletonPositionTrackingState;
	public map<JointType, PVector> skeletonPositionConfidence;

	final int SKELETON_COUNT = 6;

	public SkeletonData()
	{
		trackingState = 0;
		dwTrackingID = 0;
		position = new PVector(0.0f, 0.0f, 0.0f);

		skeletonOrientation = new PVector[NUI_SKELETON_POSITION_COUNT];
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
	ArrayList skeletonDrawOrder<JointMapping>;
	
	public P5Kinect(PApplet _p) 
	{
		parent = _p;

		////////////////////////////////////////////////////////////////////////////////

		skeletons = new SkeletonData[SKELETON_COUNT];
		// not sure if that's this is the right place for this
		skeletonDrawOrder<JointMapping> = new ArrayList<JointMapping>();

		skeletonDrawOrder.add(new JointMapping(JointType_Head, JointType_Neck));
		skeletonDrawOrder.add(new JointMapping(JointType_Neck, JointType_SpineShoulder));
		skeletonDrawOrder.add(new JointMapping(JointType_SpineShoulder, JointType_SpineMid));
		skeletonDrawOrder.add(new JointMapping(JointType_SpineMid, JointType_SpineBase));
		skeletonDrawOrder.add(new JointMapping(JointType_SpineShoulder, JointType_ShoulderRight));
		skeletonDrawOrder.add(new JointMapping(JointType_SpineShoulder, JointType_ShoulderLeft));
		skeletonDrawOrder.add(new JointMapping(JointType_SpineBase, JointType_HipRight));
		skeletonDrawOrder.add(new JointMapping(JointType_SpineBase, JointType_HipLeft));

		// Right Arm    
		skeletonDrawOrder.add(new JointMapping(JointType_ShoulderRight, JointType_ElbowRight));
		skeletonDrawOrder.add(new JointMapping(JointType_ElbowRight, JointType_WristRight));
		skeletonDrawOrder.add(new JointMapping(JointType_WristRight, JointType_HandRight));
		skeletonDrawOrder.add(new JointMapping(JointType_HandRight, JointType_HandTipRight));
		skeletonDrawOrder.add(new JointMapping(JointType_WristRight, JointType_ThumbRight));

		// Left Arm
		skeletonDrawOrder.add(new JointMapping(JointType_ShoulderLeft, JointType_ElbowLeft));
		skeletonDrawOrder.add(new JointMapping(JointType_ElbowLeft, JointType_WristLeft));
		skeletonDrawOrder.add(new JointMapping(JointType_WristLeft, JointType_HandLeft));
		skeletonDrawOrder.add(new JointMapping(JointType_HandLeft, JointType_HandTipLeft));
		skeletonDrawOrder.add(new JointMapping(JointType_WristLeft, JointType_ThumbLeft));

		// Right Leg
		skeletonDrawOrder.add(new JointMapping(JointType_HipRight, JointType_KneeRight));
		skeletonDrawOrder.add(new JointMapping(JointType_KneeRight, JointType_AnkleRight));
		skeletonDrawOrder.add(new JointMapping(JointType_AnkleRight, JointType_FootRight));

		// Left Leg
		skeletonDrawOrder.add(new JointMapping(JointType_HipLeft, JointType_KneeLeft));
		skeletonDrawOrder.add(new JointMapping(JointType_KneeLeft, JointType_AnkleLeft));
		skeletonDrawOrder.add(new JointMapping(JointType_AnkleLeft, JointType_FootLeft));

		////////////////////////////////////////////////////////////////////////////////

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
			updateSkeleton();
		}
	}

	public void updateSkeleton()
	{
		// copy all the skeletons in
		for( int i = 0; i < SKELETON_COUNT; i++ )
		{
			for (JointType joint : JointType.values()) 
			{
			  float pos[] = getJointPosition(i, joint);
			  skeletons[i].getJointPosition(joint).set()
			}
			for (JointType joint : JointType.values()) 
			{
			  float orientation[] = getJointOrientation(i, joint);
			}
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

	// thread related
	static native boolean start();
	static native void stop();

  	// is the current frame new?
	static native boolean isFrameNew();
	static native boolean isFrameNewVideo();
	static native boolean isFrameNewDepth();
	static native boolean isNewSkeleton();
	static native boolean initBodyIndexStream();

	static native void setDepthClipping(float nearClip, float farClip);
	
	// access to the pixel buffers and textures
	static native int[] getColorPixels();
	static native int[] getDepthPixels();
	static native int[] getRawDepthPixels();
	static native int[] getPlayerPixels();

	// /// draw the video texture
	void drawSkeleton(int index, int[] scale);
	void drawAllSkeletons(int[] scale);}

