#include "p5KinectV2.h"

p5KinectV2::p5KinectV2(){
	hKinect = NULL;

	pDepthFrame = NULL;
	pColorFrame = NULL;

	beginMappingColorToDepth = false;
	bNeedsUpdateSkeleton = false;
	bUsingBodyIndex = false;
	bNeedsUpdateBodyIndex = false;
	bIsFrameNewVideo = false;
	bNeedsUpdateVideo = false;
	bIsFrameNewDepth = false;
	bNeedsUpdateDepth = false;
	bVideoIsInfrared = false;
	bInited = false;
	bStarted = false;

	mappingColorToDepth = false;
	mappingDepthToColor = false;

	bUsingSkeletons = false;
  	bUseTexture = true;
	bProgrammableRenderer = false;
	
	setDepthClipping();


	skeletonDrawOrder[0] = make_pair<JointType, JointType>(JointType_Head, JointType_Neck);
	skeletonDrawOrder[1] = make_pair<JointType, JointType>(JointType_Neck, JointType_SpineShoulder);
	skeletonDrawOrder[2] = make_pair<JointType, JointType>(JointType_SpineShoulder, JointType_SpineMid);
	skeletonDrawOrder[3] = make_pair<JointType, JointType>(JointType_SpineMid, JointType_SpineBase);
	skeletonDrawOrder[4] = make_pair<JointType, JointType>(JointType_SpineShoulder, JointType_ShoulderRight);
	skeletonDrawOrder[5] = make_pair<JointType, JointType>(JointType_SpineShoulder, JointType_ShoulderLeft);
	skeletonDrawOrder[6] = make_pair<JointType, JointType>(JointType_SpineBase, JointType_HipRight);
	skeletonDrawOrder[7] = make_pair<JointType, JointType>(JointType_SpineBase, JointType_HipLeft);

	// Right Arm    
	skeletonDrawOrder[8] = make_pair<JointType, JointType>(JointType_ShoulderRight, JointType_ElbowRight);
	skeletonDrawOrder[9] = make_pair<JointType, JointType>(JointType_ElbowRight, JointType_WristRight);
	skeletonDrawOrder[10] = make_pair<JointType, JointType>(JointType_WristRight, JointType_HandRight);
	skeletonDrawOrder[11] = make_pair<JointType, JointType>(JointType_HandRight, JointType_HandTipRight);
	skeletonDrawOrder[12] = make_pair<JointType, JointType>(JointType_WristRight, JointType_ThumbRight);

	// Left Arm
	skeletonDrawOrder[13] = make_pair<JointType, JointType>(JointType_ShoulderLeft, JointType_ElbowLeft);
	skeletonDrawOrder[14] = make_pair<JointType, JointType>(JointType_ElbowLeft, JointType_WristLeft);
	skeletonDrawOrder[15] = make_pair<JointType, JointType>(JointType_WristLeft, JointType_HandLeft);
	skeletonDrawOrder[16] = make_pair<JointType, JointType>(JointType_HandLeft, JointType_HandTipLeft);
	skeletonDrawOrder[17] = make_pair<JointType, JointType>(JointType_WristLeft, JointType_ThumbLeft);

	// Right Leg
	skeletonDrawOrder[18] = make_pair<JointType, JointType>(JointType_HipRight, JointType_KneeRight);
	skeletonDrawOrder[19] = make_pair<JointType, JointType>(JointType_KneeRight, JointType_AnkleRight);
	skeletonDrawOrder[20] = make_pair<JointType, JointType>(JointType_AnkleRight, JointType_FootRight);

	// Left Leg
	skeletonDrawOrder[21] = make_pair<JointType, JointType>(JointType_HipLeft, JointType_KneeLeft);
	skeletonDrawOrder[22] = make_pair<JointType, JointType>(JointType_KneeLeft, JointType_AnkleLeft);
	skeletonDrawOrder[23] = make_pair<JointType, JointType>(JointType_AnkleLeft, JointType_FootLeft);

}

//-----------------
void p5KinectV2::setDepthClipping(float nearClip, float farClip){
	nearClipping = nearClip;
	farClipping = farClip;
	updateDepthLookupTable();
}

//-----------------
void p5KinectV2::updateDepthLookupTable()
{
	unsigned char nearColor = bNearWhite ? 255 : 0;
	unsigned char farColor = bNearWhite ? 0 : 255;
	unsigned int maxDepthLevels = 10001;
	depthLookupTable.resize(maxDepthLevels);
	depthLookupTable[0] = 0;
	for(unsigned int i = 1; i < maxDepthLevels; i++)
	{
		depthLookupTable[i] = ofMap(i, nearClipping, farClipping, nearColor, farColor, true);
	}
}

/// is the current frame new?
bool p5KinectV2::isFrameNew(){
	return isFrameNewVideo() || isFrameNewDepth();
}

bool p5KinectV2::isFrameNewVideo(){
	return bIsFrameNewVideo;
}

bool p5KinectV2::isFrameNewDepth(){
	return bIsFrameNewDepth;
}

bool p5KinectV2::isNewSkeleton() {
	return bNeedsUpdateSkeleton;
}

//vector<Skeleton> &p5KinectV2::getSkeletons() {
//	return skeletons;
//}
/// updates the pixel buffers and textures
/// make sure to call this to update to the latest incoming frames
void p5KinectV2::update()
{
	if(!bStarted)
	{
		ofLogError("p5KinectV2::update") << "Kinect not started";
		return;
	}

	// update color or IR pixels and textures if necessary
	if(bNeedsUpdateVideo)
	{
		bIsFrameNewVideo = true;
		bNeedsUpdateVideo = false;

		//swap(videoPixelsBack, videoPixels);
		swap(pColorFrame, pColorFrameBack);

	} else {
		bIsFrameNewVideo = false;
	}


	// update depth pixels and texture if necessary
	if(bNeedsUpdateDepth) {

		//swap(depthPixelsRawBack, depthPixelsRaw);
		swap(pDepthFrameBack, pDepthFrame);

		if(mappingColorToDepth) {
			beginMappingColorToDepth = true;
		}

		bIsFrameNewDepth = true;
		bNeedsUpdateDepth = false;

		for(int i = 0; i < DEPTH_SIZE; i++) {
			depthPixels[i] = depthLookupTable[ofClamp(pDepthFrame->Buffer[i] >> 4, 0, depthLookupTable.size() - 1)];
			pDepthFrame->Buffer[i] = pDepthFrame->Buffer[i] >> 4;
		}

	} else {
		bIsFrameNewDepth = false;
	}

	// update skeletons if necessary
	if(bUsingSkeletons && bNeedsUpdateSkeleton) {	
		swap(skeletons, backSkeletons);
		bNeedsUpdateSkeleton = false;

	} 


	if (bNeedsUpdateBodyIndex) {
		
		swap(pBodyIndexFrame, pBodyIndexFrame);

		bNeedsUpdateBodyIndex = false;
	}
}


bool p5KinectV2::initSensor( int id )
{
	if(bStarted){
		ofLogError("p5KinectV2::initSensor") << "Cannot configure once the sensor has already started" << endl;
		return false;
	}
	if (ofGetCurrentRenderer()->getType() == ofGLProgrammableRenderer::TYPE)
	{
		bProgrammableRenderer = true;
	}

	hKinect = KCBOpenDefaultSensor();

	return true;
}

bool p5KinectV2::initDepthStream( bool mapDepthToColor )
{

	mappingDepthToColor = mapDepthToColor;

	if(bStarted){
		ofLogError("p5KinectV2::initDepthStream") << " Cannot configure once the sensor has already started";
		return false;
	}

	HRESULT hr;
	hr = KCBGetDepthFrameDescription(hKinect, &depthFrameDescription);

	//hr = KCBCreateDepthFrame(depthFrameDescription, &pDepthFrame);

	if(bProgrammableRenderer) {
		depthPixels.allocate(depthFrameDescription.width, depthFrameDescription.height, OF_IMAGE_COLOR);
		depthPixelsBack.allocate(depthFrameDescription.width, depthFrameDescription.height, OF_IMAGE_COLOR);
	} else {
		depthPixels.allocate(depthFrameDescription.width, depthFrameDescription.height, OF_IMAGE_GRAYSCALE);
		depthPixelsBack.allocate(depthFrameDescription.width, depthFrameDescription.height, OF_IMAGE_GRAYSCALE);
	}

	depthPixelsRaw.allocate(depthFrameDescription.width, depthFrameDescription.height, OF_IMAGE_GRAYSCALE);
	depthPixelsRawBack.allocate(depthFrameDescription.width, depthFrameDescription.height, OF_IMAGE_GRAYSCALE);

	pDepthFrame = new KCBDepthFrame();
	depthPixelsRaw = new int[depthFrameDescription.lengthInPixels];
	pDepthFrame->Buffer = depthPixelsRaw;
	pDepthFrame->Size = depthFrameDescription.lengthInPixels;

	pDepthFrameBack = new KCBDepthFrame();
	depthPixelsRawBack = new int[depthFrameDescription.lengthInPixels];
	pDepthFrameBack->Buffer = depthPixelsRawBack;
	pDepthFrameBack->Size = depthFrameDescription.lengthInPixels;

	if(bUseTexture){

		if(bProgrammableRenderer) {
			//int w, int h, int glInternalFormat, bool bUseARBExtention, int glFormat, int pixelType
			depthTex.allocate(depthFrameDescription.width, depthFrameDescription.height, GL_R8);//, true, GL_R8, GL_UNSIGNED_BYTE);
			depthTex.setRGToRGBASwizzles(true);

			//rawDepthTex.allocate(K2_IR_WIDTH, K2_IR_HEIGHT, GL_R16, true, GL_RED, GL_UNSIGNED_SHORT);
			/*rawDepthTex.allocate(depthPixelsRaw, true);
			rawDepthTex.setRGToRGBASwizzles(true);

			cout << rawDepthTex.getWidth() << " " << rawDepthTex.getHeight() << endl;*/
			//depthTex.allocate(K2_IR_WIDTH, K2_IR_HEIGHT, GL_RGB);
		} else {
			depthTex.allocate(depthFrameDescription.width, depthFrameDescription.height, GL_LUMINANCE);
			rawDepthTex.allocate(depthFrameDescription.width, depthFrameDescription.height, GL_LUMINANCE16);
		}
	}
	
	
	return bInited;
}

bool p5KinectV2::initColorStream( bool mapColorToDepth, ColorImageFormat format)
{

	KCBGetColorFrameDescription(hKinect, ColorImageFormat_Rgba, &colorFrameDescription);

	if (format != ColorImageFormat_Rgba)
	{
		videoPixels = new unsigned char[colorFrameDescription.width * colorFrameDescription.height * 2];
		videoPixelsBack = new unsigned char[colorFrameDescription.width * colorFrameDescription.height * 2];
	}
	else
	{
		videoPixels = new unsigned char[colorFrameDescription.width * colorFrameDescription.height * 4];
		videoPixelsBack = new unsigned char[colorFrameDescription.width * colorFrameDescription.height * 4];
	}

	pColorFrame = new KCBColorFrame();
	pColorFrame->Buffer = videoPixels;
	pColorFrame->Size = colorFrameDescription.lengthInPixels * colorFrameDescription.bytesPerPixel;
	pColorFrame->Format = format;


	pColorFrameBack = new KCBColorFrame();
	pColorFrameBack->Buffer = videoPixelsBack;
	pColorFrameBack->Size = colorFrameDescription.lengthInPixels * colorFrameDescription.bytesPerPixel;
	pColorFrameBack->Format = format;

	//HRESULT hr = KCBCreateColorFrame(ColorImageFormat_Rgba, colorFrameDescription, &pColorFrame);
	return true;
}

bool p5KinectV2::initIRStream( int width, int height )
{
	if(bStarted){
		ofLogError("p5KinectV2::startIRStream") << " Cannot configure when the sensor has already started";
		return false;
	}

	bVideoIsInfrared = true;

	KCBGetInfraredFrameDescription(hKinect, &irFrameDescription);

	irPixelsRaw.allocate(irFrameDescription.width, irFrameDescription.height, OF_IMAGE_GRAYSCALE);

	pInfraredFrameBack = new KCBInfraredFrame();
	irPixelsRawBack = new unsigned char[irFrameDescription.lengthInPixels];
	pInfraredFrameBack->Buffer = irPixelsRawBack;
	pInfraredFrameBack->Size = irFrameDescription.lengthInPixels;

	pInfraredFrame = new KCBInfraredFrame();
	irPixelsRaw = new unsigned char[irFrameDescription.lengthInPixels];
	pInfraredFrame->Buffer = irPixelsRaw;
	pInfraredFrame->Size = irFrameDescription.lengthInPixels;
	bInited = true;
	ofLogError("p5KinectV2::initIRStream") << "cannot initialize stream";
	return true;
}

bool p5KinectV2::initBodyIndexStream()
{
	if (bStarted){
		ofLogError("p5KinectV2::initBodyIndexStream") << "Cannot configure once the sensor has already started";
		return false;
	}
	HRESULT hr = KCBGetBodyIndexFrameDescription(hKinect, &bodyIndexFrameDescription);

	if (!SUCCEEDED(hr))
	{
		ofLogError("p5KinectV2::initBodyIndexStream") << "cannot initialize stream";
		return false;
	}

	bodyIndexPixels.allocate(bodyIndexFrameDescription.width, bodyIndexFrameDescription.height, OF_IMAGE_GRAYSCALE);
	bodyIndexPixelsBack.allocate(bodyIndexFrameDescription.width, bodyIndexFrameDescription.height, OF_IMAGE_GRAYSCALE);

	pBodyIndexFrameBack = new KCBBodyIndexFrame();
	bodyIndexPixelsBack = new unsigned char[bodyIndexFrameDescription.lengthInPixels];
	pBodyIndexFrameBack->Buffer = bodyIndexPixelsBack;
	pBodyIndexFrameBack->Size = bodyIndexFrameDescription.lengthInPixels;

	pBodyIndexFrame = new KCBBodyIndexFrame();
	bodyIndexPixels = new unsigned char[bodyIndexFrameDescription.lengthInPixels];
	pBodyIndexFrame->Buffer = bodyIndexPixels;
	pBodyIndexFrame->Size = bodyIndexFrameDescription.lengthInPixels;

	bUsingBodyIndex = true;

	return true; //??
}

bool p5KinectV2::initSkeletonStream( bool seated )
{
	if(bStarted){
		ofLogError("p5KinectV2::initSkeletonStream") << "Cannot configure once the sensor has already started";
		return false;
	}

	skeletons.resize(6);
	backSkeletons.resize(6);
	bUsingSkeletons = true;
	return true;
}

//
bool p5KinectV2::start()
{
	
	startThread(true, false);
	bStarted = true;	
	return true;
}

//
void p5KinectV2::stop() {
	if(bStarted){
		waitForThread(true);
		bStarted = false;

		KCBCloseSensor(&hKinect);

		KCBReleaseBodyIndexFrame(&pBodyIndexFrame);
		KCBReleaseColorFrame(&pColorFrame);
		KCBReleaseDepthFrame(&pDepthFrame);
		KCBReleaseInfraredFrame(&pInfraredFrame);
		//KCBReleaseLongExposureInfraredFrame(_Inout_ KCBLongExposureInfraredFrame** pLongExposureInfraredFrame);

	}
}	

//
// this now being called by the java wrapper, not a windows thread
//
void p5KinectV2::threadFunction(){

	LONGLONG timestamp;
	
	//how can we tell?
	while(isThreadRunning()) {

		if (SUCCEEDED(KCBGetDepthFrame(hKinect, pDepthFrame)))
		{
			bNeedsUpdateDepth = true;
		}

		if(bUsingSkeletons) 
		{
			LONGLONG timestamp;
			IBodyFrame* pBodyFrame = NULL;
			IBody* ppBodies[BODY_COUNT] = { 0 };
			if (SUCCEEDED(KCBGetIBodyFrame(hKinect, &pBodyFrame)))
			{
				HRESULT hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, ppBodies);
				bNeedsUpdateSkeleton = true;

				// buffer for later
				for (int i = 0; i < BODY_COUNT; ++i)
				{
					backSkeletons[i].joints.clear();
					backSkeletons[i].tracked = false;

					IBody *pBody = ppBodies[i];
					BOOLEAN isTracked = false;

					if (pBody == NULL)
					{
						continue;
					}

					HRESULT hr = pBody->get_IsTracked(&isTracked);
					if (isTracked)
					{
						HRESULT hrJoints = ppBodies[i]->GetJoints(JointType_Count, joints);
						HRESULT hrOrient = ppBodies[i]->GetJointOrientations(JointType_Count, jointOrients);
						if (FAILED(hrJoints))
						{
							ofLogError("p5KinectV2::threadedFunction") << "Failed to get joints";
						}

						if (FAILED(hrOrient))
						{
							ofLogError("p5KinectV2::threadedFunction") << "Failed to get orientations";
						}

						if (SUCCEEDED(hrJoints) && SUCCEEDED(hrOrient))
						{
							for (int j = 0; j < JointType_Count; ++j)
							{
								backSkeletons[i].joints[joints[j].JointType] = Kv2Joint(joints[j], jointOrients[j]);
							}
						}
						backSkeletons[i].tracked = true;
					}
					pBody->Release();
				}

				// all done clean up
				pBodyFrame->Release();
			}
		}

		if (bUsingBodyIndex)
		{
			
			if (SUCCEEDED(KCBGetBodyIndexFrame(hKinect, pBodyIndexFrame)))
			{
				bNeedsUpdateBodyIndex = true;
			}
		}

		if (bVideoIsInfrared)
		{
			if (SUCCEEDED(KCBGetInfraredFrame(hKinect, pInfraredFrame)))
			{
				bNeedsUpdateVideo = true;
				// do we need to do this anymore?
				for (int i = 0; i <colorFrameDescription.width * colorFrameDescription.height; i++)
				{
					videoPixels[i] = reinterpret_cast<USHORT*>(irPixelByteArray)[i] >> 8;
				}
			}
		}
		else
		{
			if (SUCCEEDED(KCBGetColorFrame(hKinect, pColorFrame)))
			{
				bNeedsUpdateVideo = true;
			}
		}
	}
}
