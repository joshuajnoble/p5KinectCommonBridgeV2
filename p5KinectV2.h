#pragma once

#include "ofMain.h"

#include "KCBv2LIB.h"
#pragma comment (lib, "KCBv2.lib") // add path to lib additional dependency dir $(TargetDir)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// not sure this is right
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Kv2Joint
{
  public:
	Kv2Joint(){}
	Kv2Joint(const _Joint& kcbPosition, const _JointOrientation& kcbOrientation)
	{
		jointOrientation.set(kcbOrientation.Orientation.x, kcbOrientation.Orientation.y, kcbOrientation.Orientation.z, kcbOrientation.Orientation.w);
		jointPosition.set(kcbPosition.Position.X, kcbPosition.Position.Y, kcbPosition.Position.Z);
		type = kcbPosition.JointType;
	}

	ofVec3f getPosition()
	{
		return jointPosition;
	}

	ofQuaternion getOrientation()
	{
		return jointOrientation;
	}

	TrackingState getTrackingState()
	{
		return trackingState;
	}

  protected:
	float* jointPosition; // always 3
	float* jointOrientation; // always 4!
	JointType type;
	TrackingState trackingState;
};

class Kv2Skeleton
{
  public:
	bool tracked;
	map<JointType, Kv2Joint> joints;
};

class p5KinectV2 : protected ofThread {
  public:
	
	p5KinectV2();

	boolean initSensor( int id );
	boolean initDepthStream( boolean mapDepthToColor);
	boolean initColorStream(boolean mapColorToDepth, ColorImageFormat format);
	boolean initIRStream( int width, int height );
	boolean initSkeletonStream( boolean seated );
	boolean start();

	void stop();

  	/// is the current frame new?
	boolean isFrameNew();
	boolean isFrameNewVideo();
	boolean isFrameNewDepth();
	boolean isNewSkeleton();
	boolean initBodyIndexStream();

	void setDepthClipping(float nearClip, float farClip);
	
	/// updates the pixel buffers and textures
	/// make sure to call this to update to the latest incoming frames
	void update();
	unsigned char* getColorPixels() { return colorPixels; }
	unsigned char* getDepthPixels() { return depthPixels; }       ///< grayscale values
	int* getRawDepthPixels() { return depthPixelsRaw; }	///< raw 11 bit values

	/// enable/disable frame loading into textures on update()
	void setUseTexture(boolean bUse);

	// /// draw the video texture
	// void draw(float x, float y, float w, float h);
	// void draw(float x, float y);
	// void draw(int* point);

	// /// draw the grayscale depth texture
	// void drawRawDepth(float x, float y, float w, float h);
	// void drawRawDepth(float x, float y);
	// void drawRawDepth(int* point);

	// /// draw the grayscale depth texture
	// void drawDepth(float x, float y, float w, float h);
	// void drawDepth(float x, float y);
	// void drawDepth(int* point);

	// void drawIR( float x, float y, float w, float h );

	// void drawBodyIndex(float x, float y);
	// void drawSkeleton(int index, int* scale);
	// void drawAllSkeletons(int* scale);

  protected:

    KCBHANDLE hKinect;
	//KINECT_IMAGE_FRAME_FORMAT depthFormat;
	ColorImageFormat colorFormat;
	//NUI_SKELETON_FRAME k4wSkeletons;

  	bool bInited;
	bool bStarted;
	vector<Kv2Skeleton> skeletons;
	vector<Kv2Skeleton> backSkeletons;

	//quantize depth buffer to 8 bit range
	vector<unsigned char> depthLookupTable;
	void updateDepthLookupTable();
	void updateDepthPixels();
	void updateIRPixels();
	bool bNearWhite;
	float nearClipping, farClipping;

  	bool bUseTexture;
	
	bool bIsFrameNewVideo;
	bool bNeedsUpdateVideo;
	bool bIsFrameNewDepth;
	bool bNeedsUpdateDepth;
	bool bNeedsUpdateSkeleton;
	bool bIsSkeletonFrameNew;
	bool bProgrammableRenderer;

	bool bNeedsUpdateBodyIndex;

	bool bVideoIsInfrared;
	bool bUsingSkeletons;
	bool bUsingDepth;
	bool bUsingBodyIndex;

	BYTE *irPixelByteArray;

	void threadedFunction();

	bool mappingColorToDepth;
	bool mappingDepthToColor;
	bool beginMappingColorToDepth;

	KCBDepthFrame *pDepthFrame, *pDepthFrameBack;
	KCBColorFrame *pColorFrame, *pColorFrameBack;
	KCBInfraredFrame *pInfraredFrame, *pInfraredFrameBack;
	//KCBBodyFrame* pBodyFrame; // not using this yet

	JointOrientation jointOrients[JointType_Count];
	Joint joints[JointType_Count];

	KCBBodyIndexFrame *pBodyIndexFrame, *pBodyIndexFrameBack;

	KCBFrameDescription colorFrameDescription;
	KCBFrameDescription depthFrameDescription;
	KCBFrameDescription irFrameDescription;
	KCBFrameDescription bodyIndexFrameDescription;

	unsigned char* bodyIndexPixelsBack, bodyIndexPixels;
	unsigned char* irPixelsRaw, irPixelsRawBack;
	int* depthPixelsRaw, depthPixelsRawBack;
	unsigned char* depthPixels, depthPixelsBack;
	unsigned char* videoPixels, videoPixelsBack;

	pair<JointType, JointType> skeletonDrawOrder[JointType_Count];
};
