
// class Kv2Joint
// {
//   public:
// 	Kv2Joint(const _Joint& kcbPosition, const _JointOrientation& kcbOrientation);

// 	int[] getPosition();

// 	//ofQuaternion getOrientation();
// 	TrackingState getTrackingState();
// }

TrackingState_NotTracked
TrackingState_Inferred
TrackingState_Tracked

ColorImageFormat_Yuv
ColorImageFormat_None
ColorImageFormat_Bgra
ColorImageFormat_Bayer
ColorImageFormat_Yuy2

class p5KinectCommonBridge 
{
	
	static {  
		System.loadLibrary("KinectCommonBridge");
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
	
	/// updates the pixel buffers and textures
	/// make sure to call this to update to the latest incoming frames
	static native void update();
	static native int[] getColorPixels();
	static native int[] getDepthPixels();       ///< grayscale values
	static native int[] getRawDepthPixels();	///< raw 11 bit values

	/// enable/disable frame loading into textures on update()
	static native void setUseTexture(boolean bUse);

	/// draw the video texture
	static native void draw(float x, float y, float w, float h);
	static native void draw(float x, float y);
	static native void draw(int[] point);

	/// draw the grayscale depth texture
	static native void drawRawDepth(float x, float y, float w, float h);
	static native void drawRawDepth(float x, float y);
	static native void drawRawDepth(int[] point);

	/// draw the grayscale depth texture
	static native void drawDepth(float x, float y, float w, float h);
	static native void drawDepth(float x, float y);
	static native void drawDepth(int[] point);

	static native void drawIR( float x, float y, float w, float h );

	static native void drawBodyIndex(float x, float y);
	static native void drawSkeleton(int index, int[] scale);
	static native void drawAllSkeletons(int[] scale);
}
