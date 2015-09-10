#include "aaOcean_texture.h"
#include <lxu_math.hpp>
#include <fstream>
#include <assert.h>
#include <lxvmath.h>

using namespace aaOceanTextureNamespace;

LXtTagInfoDesc	 aaOceanTexture::descInfo[] = {
        { LXsSRV_USERNAME,	"aaOcean Texture" },
        { LXsSRV_LOGSUBSYSTEM,	"val-texture"	},
        { 0 }
};

#define SRVs_TEXTURE		"aaOcean.texture"
#define SRVs_ITEMTYPE		SRVs_TEXTURE
// OS X defines this. MSVC doesn't.
#ifndef M_PI
#define M_PI        3.14159265358979323846264338327950288   /* pi             */
#endif

aaOceanTexture::aaOceanTexture ()
{
    my_type = LXiTYPE_NONE;
}

aaOceanTexture::~aaOceanTexture ()
{
}

LXtItemType aaOceanTexture::MyType ()
{
    if (my_type != LXiTYPE_NONE)
            return my_type;

    CLxUser_SceneService	 svc;

    my_type = svc.ItemType (SRVs_ITEMTYPE);
    return my_type;
}

static LXtTextValueHint hint_resolution[] = {
    4,			"Map size : 16",
    5,			"Map size : 32",
    6,			"Map size : 64",
    7,			"Map size : 128",
    8,			"Map size : 256",
    9,			"Map size : 512",
    10,			"Map size : 1024",
    11,			"Map size : 2048",
    12,			"Map size : 4096",
    13,			"Map size : 8192 (memory hungry!)",
    14,			"Map size : 16384 (very memory hungry!)",
    4,			NULL
};

static LXtTextValueHint hint_outputType[] = {
    0,			"Heightfield and chop",
    1,          "Foam",
    2,          "Eigenminus",
    3,			"Eigenplus",
    4,          "Tangent h and c",
    5,          "Tangent h and c ff",
    6,          "Testing",
    0,			NULL
};

static LXtTextValueHint hint_boolLimit[] = {
    0,			"&min",		// int min 0
    1,			"&max",		// int max 1
    -1,			NULL
};

/*
 * Setup channels for the item type.
 */
LxResult aaOceanTexture::vtx_SetupChannels (ILxUnknownID addChan)
{
	CLxUser_AddChannel	 ac (addChan);

    ac.NewChannel  ("outputType",	LXsTYPE_INTEGER);
	ac.SetDefault  (0.0, 0);
    ac.SetHint(hint_outputType);

    ac.NewChannel  ("resolution",	LXsTYPE_INTEGER);
	ac.SetDefault  (0.0, 4);
    ac.SetHint(hint_resolution);
    
	ac.NewChannel  ("oceanSize",			LXsTYPE_FLOAT);
	ac.SetDefault  (100.0f, 0);
	
	ac.NewChannel  ("waveHeight",	LXsTYPE_FLOAT);
	ac.SetDefault  (2.0f, 0);
    
	ac.NewChannel  ("surfaceTension",	LXsTYPE_FLOAT);
	ac.SetDefault  (0.0f, 0);
    
	ac.NewChannel  ("waveAlign",	LXsTYPE_INTEGER);
	ac.SetDefault  (0.0, 1);
    
	ac.NewChannel  ("waveSmooth",	LXsTYPE_FLOAT);
	ac.SetDefault  (0.0f, 0);
    
	ac.NewChannel  ("waveDirection",	LXsTYPE_FLOAT);
	ac.SetDefault  (45.0f, 0);
    
    ac.NewChannel  ("waveReflection",	LXsTYPE_FLOAT);
	ac.SetDefault  (0.0f, 0);
    
	ac.NewChannel  ("waveSpeed",	LXsTYPE_FLOAT);
	ac.SetDefault  (1.0f, 0);
    
	ac.NewChannel  ("waveChop",	LXsTYPE_FLOAT);
	ac.SetDefault  (2.0f, 0);
    
	ac.NewChannel  ("oceanDepth",	LXsTYPE_FLOAT);
	ac.SetDefault  (10000.0f, 0);
    
    ac.NewChannel  ("waveSize",	LXsTYPE_FLOAT);
	ac.SetDefault  (4.0f, 0);
    
    ac.NewChannel  ("seed",	LXsTYPE_INTEGER);
	ac.SetDefault  (0.0f, 1);
    
    ac.NewChannel  ("repeatTime",	LXsTYPE_FLOAT);
	ac.SetDefault  (1000.0f, 0);
    
	ac.NewChannel  ("doFoam",	LXsTYPE_INTEGER);
	ac.SetDefault  (0.0, 0);
    ac.SetHint(hint_boolLimit);

    return LXe_OK;
}

/*
 * Attach to channel evaluations. This gets the indicies for the channels in
 * in attributes.
 */
LxResult aaOceanTexture::vtx_LinkChannels (ILxUnknownID eval, ILxUnknownID	item)
{
	CLxUser_Evaluation	 ev (eval);

	m_idx_outputType = ev.AddChan (item, "outputType");
	m_idx_resolution = ev.AddChan (item, "resolution");
	m_idx_oceanSize = ev.AddChan (item, "oceanSize");
	m_idx_waveHeight = ev.AddChan (item, "waveHeight");
	m_idx_waveSize = ev.AddChan (item, "waveSize");
	m_idx_surfaceTension = ev.AddChan (item, "surfaceTension");
	m_idx_waveAlign = ev.AddChan (item, "waveAlign");
	m_idx_waveSmooth = ev.AddChan (item, "waveSmooth");
	m_idx_waveDirection = ev.AddChan (item, "waveDirection");
	m_idx_waveReflection = ev.AddChan (item, "waveReflection");
	m_idx_waveSpeed = ev.AddChan (item, "waveSpeed");
	m_idx_waveChop = ev.AddChan (item, "waveChop");
	m_idx_oceanDepth = ev.AddChan (item, "oceanDepth");
    m_idx_seed = ev.AddChan(item, "seed");
    m_idx_repeatTime = ev.AddChan(item, "repeatTime");
    m_idx_doFoam = ev.AddChan(item, "doFoam");
    
	// m_idx_time = ev.AddChan (item, "time");

    m_idx_time = ev.AddTime ();
    
    tin_offset = pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_TEXTURE_INPUT);
	tinDsp_offset = pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_DISPLACE);
    nrm_offset  = pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_SURF_NORMAL);

    return LXe_OK;
}

/*
 * Read channel values which may have changed. These are stored in the allocated
 * data for later evaluation.
 */
LxResult aaOceanTexture::vtx_ReadChannels(ILxUnknownID attr, void  **ppvData)
{
	CLxUser_Attributes	 at (attr);

    std::unique_ptr<OceanData> newOceanData(new OceanData());

    newOceanData->m_outputType = at.Int(m_idx_outputType);
	if(newOceanData->m_outputType > 6)
    {
        newOceanData->m_outputType = 6;
    }
	if(newOceanData->m_outputType < 0)
    {
        newOceanData->m_outputType = 0;
    }

	newOceanData->m_resolution = at.Int(m_idx_resolution);
	if(newOceanData->m_resolution > 14)
    {
        newOceanData->m_resolution = 14;
    }
	if(newOceanData->m_resolution < 4)
    {
        newOceanData->m_resolution = 4;
    }

	newOceanData->m_oceanSize = at.Float(m_idx_oceanSize);
    if(newOceanData->m_oceanSize <= 0)
    {
        newOceanData->m_oceanSize = 0.01;
    }
	newOceanData->m_waveHeight = at.Float(m_idx_waveHeight);
	newOceanData->m_waveSize = at.Float(m_idx_waveSize);
	newOceanData->m_surfaceTension = at.Float(m_idx_surfaceTension);
	newOceanData->m_waveAlign = at.Int(m_idx_waveAlign);
	newOceanData->m_waveSmooth = at.Float(m_idx_waveSmooth);
	newOceanData->m_waveDirection = at.Float(m_idx_waveDirection);
	newOceanData->m_waveReflection = at.Float(m_idx_waveReflection);
	newOceanData->m_waveSpeed = at.Float(m_idx_waveSpeed);
    if(newOceanData->m_waveSpeed <= 0)
    {
        newOceanData->m_waveSpeed = 0.01;
    }
	newOceanData->m_waveChop = at.Float(m_idx_waveChop);
	newOceanData->m_oceanDepth = at.Float(m_idx_oceanDepth);
	newOceanData->m_seed = at.Int(m_idx_seed);
	newOceanData->m_repeatTime = at.Float(m_idx_repeatTime);
	newOceanData->m_doFoam = (bool) at.Int(m_idx_doFoam);
	newOceanData->m_doNormals = false; // disabled due to Vector issues (bool) at.Int(m_idx_doNormals);
    
    newOceanData->m_time = at.Float(m_idx_time);
    
    maybeResetOceanData(std::move(newOceanData));

	return LXe_OK;
}

// Unused in the aaOcean code.
/*
inline void mwnormalize(float *vec) 
{
	double magSq = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
	if (magSq > 0.0f) 
	{ // check for divide-by-zero
		double oneOverMag = 1.0 / sqrt(magSq);
		vec[0] *= oneOverMag;
		vec[1] *= oneOverMag;
		vec[2] *= oneOverMag;
	}
}
*/	

inline float remapValue(float value, float min, float max, float min2, float max2)
{
	return min2 + (value - min) * (max2 - min2) / (max - min);
}

void VNormalizeF(LXtFVector &v)
{
    double    x;
    
    x = LXx_VDOT (v, v);
    if (x > 0.0) {
        x = 1.0 / sqrt (x);
        LXx_VSCL (v, x);
    }
}

void VOrthogonalizeF(LXtFVector &v, LXtFVector &n)
{
    double    f;
    
    f = - LXx_VDOT (v, n);
    LXx_VADDS (v, n, f);
}

void NUVBasisF(LXtFVector &norm, LXtFVector &du, LXtFVector &dv)
{
    VOrthogonalizeF (du, norm);
    VNormalizeF (du);
    VOrthogonalizeF (dv, du);
    VOrthogonalizeF (dv, norm);
    VOrthogonalizeF (dv, du);
    VOrthogonalizeF (dv, norm);
    VNormalizeF (dv);
}

void aaOceanTexture::maybeResetOceanData(std::unique_ptr<OceanData> newOceanData) {
    if (oceanData_.get() == nullptr || *newOceanData != *oceanData_) {
        // Automatically unlocks the mutex when it goes out of scope.
        std::lock_guard<std::mutex> lock(myMutex_);
        // Check it again inside the mutex because another thread may have overridden it.
        if (oceanData_.get() == nullptr || *newOceanData != *oceanData_) {
            oceanData_ = std::move(newOceanData); // newOceanData is no longer valid, do not use it again in this function!
            mOcean_.input(  oceanData_->m_resolution,
                           (unsigned long)oceanData_->m_seed,
                           oceanData_->m_oceanSize,
                           oceanData_->m_oceanDepth,
                           oceanData_->m_surfaceTension,
                           oceanData_->m_waveSize,
                           oceanData_->m_waveSmooth,
                           oceanData_->m_waveDirection,
                           oceanData_->m_waveAlign,
                           oceanData_->m_waveReflection,
                           oceanData_->m_waveSpeed,
                           oceanData_->m_waveHeight * 100,
                           oceanData_->m_waveChop,
                           oceanData_->m_time,
                           oceanData_->m_repeatTime,
                           oceanData_->m_doFoam,
                           oceanData_->m_doNormals);
        }
    }
}


#ifdef MODO701
void aaOceanTexture::vtx_Evaluate (ILxUnknownID vector, LXpTextureOutput *tOut, void *data)
#else
void aaOceanTexture::vtx_Evaluate (ILxUnknownID etor, int *idx, ILxUnknownID vector, LXpTextureOutput *tOut, void *data)
#endif
{
    LXpTextureInput		*tInp;
	LXpDisplace *tInpDsp;

    tInp = (LXpTextureInput *) pkt_service.FastPacket (vector, tin_offset);
	tInpDsp = (LXpDisplace *) pkt_service.FastPacket (vector, tinDsp_offset);
    
    // Maya code for reference :
	// get height field
    /*
     worldSpaceVec[1] = pOcean->getOceanData(u[i], v[i], aaOcean::eHEIGHTFIELD);
     if(pOcean->isChoppy())
     {
     // get x and z displacement
     worldSpaceVec[0] = pOcean->getOceanData(u[i], v[i], aaOcean::eCHOPX);
     worldSpaceVec[2] = pOcean->getOceanData(u[i], v[i], aaOcean::eCHOPZ);
     
     if(foam)
     {
     if(foundEigenVector)
     {
     r = pOcean->getOceanData(u[i], v[i], aaOcean::eEIGENMINUSX);
     g = pOcean->getOceanData(u[i], v[i], aaOcean::eEIGENMINUSZ);
     b = pOcean->getOceanData(u[i], v[i], aaOcean::eEIGENPLUSX);
     a = pOcean->getOceanData(u[i], v[i], aaOcean::eEIGENPLUSZ);
     colArrayEigenVector.set(i, r, g, b, a);
     }
     if(foundEigenValue)
     {
     if(invert)
     r = 1.0f - pOcean->getOceanData(u[i], v[i], aaOcean::eFOAM);
     else
     r = pOcean->getOceanData(u[i], v[i], aaOcean::eFOAM);
     
     colArrayEigenValue.set(i, r, r, r);
     }
     }
     }
     
     localSpaceVec = worldSpaceVec * transform;
     verts[i] += localSpaceVec;
     */

    float result[3]; // vector for the color output.
    result[0] = result[1] = result[2] = 0.0;
    float value = 0.0; // value output
    float alpha = 1.0; // alpha output

    float x_pos = tInp->uvw[0]/oceanData_->m_oceanSize;
    float z_pos = tInp->uvw[2]/oceanData_->m_oceanSize;

    tOut->direct   = 1;
    // The intent of tInpDsp->enable isn't entirely clear. The docs, such as they are, indicate that the texture should set this when outputting displacement.
    tInpDsp->enable = true;

    // aaOceans works expecting 0-1 input range for UVs. To fit our ocean size into this 0-1 space, we need to divide it down. This is a first pass implementation.
    // The approach may change based on user feedback and subsequent refinement.
    
    if(oceanData_->m_outputType == 0) // normal displacement texture configuration
    {
        result[1] = mOcean_.getOceanData(x_pos, z_pos, aaOcean::eHEIGHTFIELD);
        if (mOcean_.isChoppy())
        {
            result[0] = mOcean_.getOceanData(x_pos, z_pos, aaOcean::eCHOPX);
            result[2] = mOcean_.getOceanData(x_pos, z_pos, aaOcean::eCHOPZ);
        } else {
            result[0] = 0.0;
            result[2] = 0.0;
        }

        /*if (LXx_VLEN(result) != 0)
        {
            LXx_VSCL(result,1/LXx_VLEN(result));
        }*/
        if(oceanData_->m_outputType == 0) // normal displacement texture configuration
        {
            // Fit to 0-1 range, with 0.5 being no displacement
            result[0] = (result[0]+1)/2;
            result[1] = (result[1]+1)/2;
            result[2] = (result[2]+1)/2;

            value = result[1];// * rd->m_waveHeight; // in case displacement is used rather than vector displacement.
        }

        if((oceanData_->m_outputType == 4) || (oceanData_->m_outputType == 5) || (oceanData_->m_outputType == 6))// tangent conversion v1
        {
            LXpSampleSurfNormal *sNrm = (LXpSampleSurfNormal*) pkt_service.FastPacket (vector, nrm_offset);
            LXtFVector surfaceNormal, cu, cv;
            LXx_VCPY (surfaceNormal, sNrm->wNorm0);

            LXtFVector objectSpaceVector;
            LXx_VCPY(objectSpaceVector, result);
            
            double objectSpaceVectorMag = LXx_VLEN(objectSpaceVector);
            
            if (oceanData_->m_outputType == 4)
            {
                LXtFVector projectedResult;
                
                // From http://www.euclideanspace.com/maths/geometry/elements/plane/lineOnPlane/
                LXx_VCROSS(projectedResult, objectSpaceVector,surfaceNormal);
                LXx_VSCL(projectedResult, 1/(objectSpaceVectorMag));
                LXx_VCROSS(projectedResult, objectSpaceVector, projectedResult);
                LXx_VSCL(projectedResult, 1/(objectSpaceVectorMag));
                
                LXtFVector projectedOnNormal;
                LXx_VCPY(projectedOnNormal, objectSpaceVector);
                LXx_VSCL(projectedOnNormal, 1/(objectSpaceVectorMag * objectSpaceVectorMag));
                LXx_VSCL(projectedOnNormal, LXx_VDOT(surfaceNormal, objectSpaceVector));

                result[0] = projectedResult[0];
                result[1] = projectedResult[2];
                result[2] = projectedResult[1];
                value = LXx_VLEN(projectedResult); // sqrt(LXx_VDOT(projectedOnNormal, projectedOnNormal));
            }
            if (oceanData_->m_outputType == 5)
            {
                LXtVector tan;
                tan[0] = LXx_VDOT (tInp->dpdu, objectSpaceVector);
                tan[1] = LXx_VDOT (tInp->dpdv, objectSpaceVector);
                tan[2] = LXx_VDOT (surfaceNormal, objectSpaceVector);
                
                result[0] = tan[0];
                result[2] = tan[1];
                result[1] = tan[2];
                value = LXx_VLEN(tan); // sqrt(LXx_VDOT(projectedOnNormal, projectedOnNormal));
            }
            if (oceanData_->m_outputType == 6)
            {
                // LXtFVector rgb;
                LXx_VCPY(cu, tInp->dpdu);
                LXx_VCPY(cv, tInp->dpdv);
                
                VNormalizeF(cu);
                VNormalizeF(cv);
                
                NUVBasisF(surfaceNormal, cu, cv);
                
                LXtFMatrix myMatrix;
                for (int i = 0; i < 3; i++)
                {
                    myMatrix[0][i] = cu[i];
                    myMatrix[1][i] = cv[i];
                    myMatrix[2][i] = surfaceNormal[i];
                }
                
                lx::MatrixTranspose(myMatrix);
                
                lx::MatrixMultiply(result, myMatrix, objectSpaceVector);
                
                // LXx_VCPY(result, rgb);
            }
        }
    }
    if(oceanData_->m_outputType == 1) // foam map requested
    {
        if(oceanData_->m_doFoam == true)
        {
            value = mOcean_.getOceanData(x_pos, z_pos, aaOcean::eFOAM);
        }
    }
    if(oceanData_->m_outputType == 2) // Eigenvalues - minus
    {
        if(oceanData_->m_doFoam == true)
        {
            result[0] = mOcean_.getOceanData(x_pos, z_pos, aaOcean::eEIGENMINUSX);
            result[2] = mOcean_.getOceanData(x_pos, z_pos, aaOcean::eEIGENMINUSZ);
        }
    }
    if(oceanData_->m_outputType == 3) // Eigenvalues - plus
    {
        if(oceanData_->m_doFoam == true)
        {
            result[0] = mOcean_.getOceanData(x_pos, z_pos, aaOcean::eEIGENPLUSX);
            result[2] = mOcean_.getOceanData(x_pos, z_pos, aaOcean::eEIGENPLUSZ);
        }
    }
    // Note that modo expects textures to output the right kind of data based on the context. This is the reason for checking against
    // LXi_TFX_COLOR in the context below. If we aren't driving a color, we output a value instead.
    if (LXi_TFX_COLOR == tInp->context)
    {
        tOut->color[0][0] = result[0];
        tOut->color[0][1] = result[1];
        tOut->color[0][2] = result[2];
    }
    tOut->value[0] = value;
    tOut->alpha[0] = alpha;
    bool debugMe = false;
    if (debugMe)
    {
        std::ofstream fout ("/Users/phil/aadebug_texture.csv", std::ios::app);
        std::string tmpString =
        std::to_string(x_pos) + "," + std::to_string(z_pos) + "," +
        std::to_string(result[0]) + "," + std::to_string(result[1]) + "," + std::to_string(result[2]) + "\n";
        fout << tmpString;
        fout.close();
    }
}

/*
 * Release the cached state after rendering is complete.
 */
void aaOceanTexture::vtx_Cleanup (void	*data)
{
}
