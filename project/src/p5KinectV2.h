#pragma once

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

	float * getPosition()
	{
		return jointPosition;
	}

	float * getOrientation()
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

class p5KinectV2 {
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
	int* getPlayerIndexPixels() { return playerIndexPixels; }

	float * getJointPosition( int skeletonIndex, int jointType );
	float * getJointOrientation( int skeletonIndex, int jointType );
	float getJointCertainty( int skeletonIndex, int jointType );

	/// enable/disable frame loading into textures on update()
	void setUseTexture(boolean bUse);

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

	void threadFunction();

	bool mappingColorToDepth;
	bool mappingDepthToColor;
	bool beginMappingColorToDepth;

	KCBDepthFrame *pDepthFrame, *pDepthFrameBack;
	KCBColorFrame *pColorFrame, *pColorFrameBack;
	KCBInfraredFrame *pInfraredFrame, *pInfraredFrameBack;
	KCBPlayerFrame *pPlayerFrame, *pPlayerFrameBack;
	//KCBBodyFrame* pBodyFrame; // not using this yet

	JointOrientation jointOrients[JointType_Count];
	Joint joints[JointType_Count];

	KCBBodyIndexFrame *pBodyIndexFrame, *pBodyIndexFrameBack;

	KCBFrameDescription colorFrameDescription;
	KCBFrameDescription depthFrameDescription;
	KCBFrameDescription irFrameDescription;
	KCBFrameDescription bodyIndexFrameDescription;

	unsigned char* playerIndexPixels;
	unsigned char* irPixelsRaw, irPixelsRawBack;
	int* depthPixelsRaw, depthPixelsRawBack;
	unsigned char* depthPixels, depthPixelsBack;
	unsigned char* videoPixels, videoPixelsBack;

	pair<JointType, JointType> skeletonDrawOrder[JointType_Count];
};
