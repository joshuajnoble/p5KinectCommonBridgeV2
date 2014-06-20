#include "p5KinectV2.h"

p5KinectV2::p5KinectV2(){
	hKinect = NULL;

	pDepthFrame = NULL;
	pColorFrame = NULL;
	pBodyIndexFrame = NULL;

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
	
	setDepthClipping();
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
bool p5KinectV2::isFrameNew()
{
	return isFrameNewVideo() || isFrameNewDepth();
}

bool p5KinectV2::isFrameNewVideo()
{
	return bIsFrameNewVideo;
}

bool p5KinectV2::isFrameNewDepth()
{
	return bIsFrameNewDepth;
}

bool p5KinectV2::isNewSkeleton() 
{
	return bNeedsUpdateSkeleton;
}

/// updates the pixel buffers and textures
/// make sure to call this to update to the latest incoming frames
void p5KinectV2::update()
{
	if(!bStarted) 
	{
		return;
	}

	// update color or IR pixels and textures if necessary
	if(bNeedsUpdateVideo)
	{
		bIsFrameNewVideo = true;
		bNeedsUpdateVideo = false;
		swap(pColorFrame, pColorFrameBack);

	} 
	else 
	{
		bIsFrameNewVideo = false;
	}


	// update depth pixels
	if(bNeedsUpdateDepth) 
	{
		swap(pDepthFrameBack, pDepthFrame);

		if(mappingColorToDepth) 
		{
			beginMappingColorToDepth = true;
		}

		bIsFrameNewDepth = true;
		bNeedsUpdateDepth = false;

		for(int i = 0; i < DEPTH_SIZE; i++) 
		{
			depthPixels[i] = depthLookupTable[ofClamp(pDepthFrame->Buffer[i] >> 4, 0, depthLookupTable.size() - 1)];
			pDepthFrame->Buffer[i] = pDepthFrame->Buffer[i] >> 4;
		}

	} 
	else 
	{
		bIsFrameNewDepth = false;
	}

	// update skeletons if necessary
	if(bUsingSkeletons && bNeedsUpdateSkeleton) 
	{
		swap(skeletons, backSkeletons);
		bNeedsUpdateSkeleton = false;

	} 


	if (bNeedsUpdateBodyIndex) 
	{
		swap(pBodyIndexFrame, pBodyIndexFrame);
		bNeedsUpdateBodyIndex = false;
	}
}


bool p5KinectV2::initSensor( int id )
{
	if(bStarted)
	{
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

	if(bStarted)
	{
		return false;
	}

	HRESULT hr;
	hr = KCBGetDepthFrameDescription(hKinect, &depthFrameDescription);

	//hr = KCBCreateDepthFrame(depthFrameDescription, &pDepthFrame);

	if(bProgrammableRenderer) 
	{
		depthPixels.allocate(depthFrameDescription.width, depthFrameDescription.height, OF_IMAGE_COLOR);
		depthPixelsBack.allocate(depthFrameDescription.width, depthFrameDescription.height, OF_IMAGE_COLOR);
	} 
	else 
	{
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
	if(bStarted)
	{
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
	//ofLogError("p5KinectV2::initIRStream") << "cannot initialize stream";
	return true;
}

bool p5KinectV2::initBodyIndexStream()
{
	if (bStarted)
	{
		return false;
	}
	HRESULT hr = KCBGetBodyIndexFrameDescription(hKinect, &bodyIndexFrameDescription);

	if (!SUCCEEDED(hr))
	{
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
	if(bStarted)
	{
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
	if(bStarted)
	{
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

float * p5KinectV2::getJointPosition( int skeletonIndex, int jointType )
{
	return skeletons[skeletonIndex].get(jointType).getPosition();
}

float * p5KinectV2::getJointOrientation( int skeletonIndex, int jointType )
{
	return skeletons[skeletonIndex].get(jointType).getOrientation();
}

float p5KinectV2::getJointCertainty( int skeletonIndex, int jointType )
{
	// erm, gotta get this
	//return skeletons[skeletonIndex].get(jointType).getOrientation();
	return 1.0;
}

//
// this now being called by the java wrapper, not a windows thread
//
void p5KinectV2::threadFunction()
{

	LONGLONG timestamp;
	
	//how can we tell?
	while(isThreadRunning())
	{

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
							// how to log errors?
						}

						if (FAILED(hrOrient))
						{
							// how to log errors?
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
