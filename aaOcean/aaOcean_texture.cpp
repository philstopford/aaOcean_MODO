#include "aaOcean_texture.h"
#include <lxu_math.hpp>

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
#ifndef TEXRENDDATA
	m_ocean = NULL;
#endif
}

aaOceanTexture::~aaOceanTexture ()
{
#ifndef TEXRENDDATA
	if(m_ocean != NULL) delete m_ocean;
#endif
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
    4,			"&min",		// int min 4
    12,			"&max",		// int max 12
    -1,			NULL
};

static LXtTextValueHint hint_outputType[] = {
    0,			"&min",		// int min 0
    3,			"&max",		// int max 3
    -1,			NULL
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

    // ac.NewChannel  ("doNormals",	LXsTYPE_INTEGER);
	// ac.SetDefault  (0.0, 0);

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
    // m_idx_doNormals = ev.AddChan(item, "doNormals");
    
	// m_idx_time = ev.AddChan (item, "time");

    m_idx_time = ev.AddTime ();
    
    tin_offset = pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_TEXTURE_INPUT);
	tinDsp_offset = pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_DISPLACE);

    return LXe_OK;
}

/*
 * Read channel values which may have changed. These are stored in the allocated
 * data for later evaluation.
 */
LxResult aaOceanTexture::vtx_ReadChannels(ILxUnknownID attr, void  **ppvData)
{
	CLxUser_Attributes	 at (attr);
	RendData		*rd = new RendData;

    rd->m_outputType = at.Int(m_idx_outputType);
	if(rd->m_outputType > 3)
    {
        rd->m_outputType = 3;
    }
	if(rd->m_outputType < 0)
    {
        rd->m_outputType = 0;
    }

	rd->m_resolution = at.Int(m_idx_resolution);
	if(rd->m_resolution > 12)
    {
        rd->m_resolution = 12;
    }
	if(rd->m_resolution < 4)
    {
        rd->m_resolution = 4;
    }

	rd->m_oceanSize = at.Float(m_idx_oceanSize);
    if(rd->m_oceanSize <= 0)
    {
        rd->m_oceanSize = 0.01;
    }
	rd->m_waveHeight = at.Float(m_idx_waveHeight);
	rd->m_waveSize = at.Float(m_idx_waveSize);
	rd->m_surfaceTension = at.Float(m_idx_surfaceTension);
	rd->m_waveAlign = at.Int(m_idx_waveAlign);
	rd->m_waveSmooth = at.Float(m_idx_waveSmooth);
	rd->m_waveDirection = at.Float(m_idx_waveDirection);
	rd->m_waveReflection = at.Float(m_idx_waveReflection);
	rd->m_waveSpeed = at.Float(m_idx_waveSpeed);
    if(rd->m_waveSpeed <= 0)
    {
        rd->m_waveSpeed = 0.01;
    }
	rd->m_waveChop = at.Float(m_idx_waveChop);
	rd->m_oceanDepth = at.Float(m_idx_oceanDepth);
	rd->m_seed = at.Int(m_idx_seed);
	rd->m_repeatTime = at.Float(m_idx_repeatTime);
	rd->m_doFoam = (bool) at.Int(m_idx_doFoam);
	rd->m_doNormals = false; // disabled due to Vector issues (bool) at.Int(m_idx_doNormals);
    
    rd->m_time = at.Float(m_idx_time);
	
#ifdef TEXRENDDATA
    rd->m_ocean->input( rd->m_resolution,
                   (unsigned long)rd->m_seed,
                   rd->m_oceanSize,
                   rd->m_oceanDepth,
                   rd->m_surfaceTension,
                   rd->m_waveSize,
                   rd->m_waveSmooth,
                   rd->m_waveDirection,
                   rd->m_waveAlign,
                   rd->m_waveReflection,
                   rd->m_waveSpeed,
                   rd->m_waveHeight * 100,
                   rd->m_waveChop,
                   rd->m_time,
                   rd->m_repeatTime,
                   rd->m_doFoam,
                   rd->m_doNormals);
#else
    if (m_ocean == NULL)
    {
        m_ocean = new aaOcean();
    }
    // We scale the height because aaOcean scales it down again internally.
    m_ocean->input( rd->m_resolution,
                        (unsigned long)rd->m_seed,
                        rd->m_oceanSize,
                        rd->m_oceanDepth,
                        rd->m_surfaceTension,
                        rd->m_waveSize,
                        rd->m_waveSmooth,
                        rd->m_waveDirection,
                        rd->m_waveAlign,
                        rd->m_waveReflection,
                        rd->m_waveSpeed,
                        rd->m_waveHeight * 100,
                        rd->m_waveChop,
                        rd->m_time,
                        rd->m_repeatTime,
                        rd->m_doFoam,
                        rd->m_doNormals);
#endif

	ppvData[0] = rd;

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

#ifdef MODO701
void aaOceanTexture::vtx_Evaluate (ILxUnknownID vector, LXpTextureOutput *tOut, void *data)
#else
void aaOceanTexture::vtx_Evaluate (ILxUnknownID etor, int *idx, ILxUnknownID vector, LXpTextureOutput *tOut, void *data)
#endif
{
    RendData		*rd = (RendData *) data;
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

    float x_pos = tInp->uvw[0]/rd->m_oceanSize;
    float z_pos = tInp->uvw[2]/rd->m_oceanSize;

#ifdef TEXRENDDATA
	if(rd->m_ocean != NULL)
#else
    if(m_ocean != NULL)
#endif
    {
        tOut->direct   = 1;
        // The intent of tInpDsp->enable isn't entirely clear. The docs, such as they are, indicate that the texture should set this when outputting displacement.
        tInpDsp->enable = true;

        // aaOceans works expecting 0-1 input range for UVs. To fit our ocean size into this 0-1 space, we need to divide it down. This is a first pass implementation.
        // The approach may change based on user feedback and subsequent refinement.
        
        if(rd->m_outputType == 0) // normal displacement texture configuration
        {
#ifdef TEXRENDDATA
            result[1] = rd->m_ocean->getOceanData(x_pos, z_pos, aaOcean::eHEIGHTFIELD);
            if (rd->m_ocean->isChoppy())
            {
                result[0] = rd->m_ocean->getOceanData(x_pos, z_pos, aaOcean::eCHOPX);
                result[2] = rd->m_ocean->getOceanData(x_pos, z_pos, aaOcean::eCHOPZ);
            } else {
                result[0] = 0.0;
                result[2] = 0.0;
            }
#else
            result[1] = m_ocean->getOceanData(x_pos, z_pos, aaOcean::eHEIGHTFIELD);
            if (m_ocean->isChoppy())
            {
                result[0] = m_ocean->getOceanData(x_pos, z_pos, aaOcean::eCHOPX);
                result[2] = m_ocean->getOceanData(x_pos, z_pos, aaOcean::eCHOPZ);
            } else {
                result[0] = 0.0;
                result[2] = 0.0;
            }
#endif
            // Fit to 0-1 range, with 0.5 being no displacement
            result[0] = (result[0]+1)/2;
            result[1] = (result[1]+1)/2;
            result[2] = (result[2]+1)/2;

            value = result[1];// * rd->m_waveHeight; // in case displacement is used rather than vector displacement.
        }
        if(rd->m_outputType == 1) // foam map requested
        {
            if(rd->m_doFoam == true)
            {
#ifdef TEXRENDDATA
                value = rd->m_ocean->getOceanData(x_pos, z_pos, aaOcean::eFOAM);
#else
                value = m_ocean->getOceanData(x_pos, z_pos, aaOcean::eFOAM);
#endif
            }
        }
        if(rd->m_outputType == 2) // Eigenvalues - minus
        {
            if(rd->m_doFoam == true)
            {
#ifdef TEXRENDDATA
                result[0] = rd->m_ocean->getOceanData(x_pos, z_pos, aaOcean::eEIGENMINUSX);
                result[2] = rd->m_ocean->getOceanData(x_pos, z_pos, aaOcean::eEIGENMINUSZ);
#else
                result[0] = m_ocean->getOceanData(x_pos, z_pos, aaOcean::eEIGENMINUSX);
                result[2] = m_ocean->getOceanData(x_pos, z_pos, aaOcean::eEIGENMINUSZ);
#endif
            }
        }
        if(rd->m_outputType == 3) // Eigenvalues - plus
        {
            if(rd->m_doFoam == true)
            {
#ifdef TEXRENDDATA
                result[0] = rd->m_ocean->getOceanData(x_pos, z_pos, aaOcean::eEIGENPLUSX);
                result[2] = rd->m_ocean->getOceanData(x_pos, z_pos, aaOcean::eEIGENPLUSZ);
#else
                result[0] = m_ocean->getOceanData(x_pos, z_pos, aaOcean::eEIGENPLUSX);
                result[2] = m_ocean->getOceanData(x_pos, z_pos, aaOcean::eEIGENPLUSZ);
#endif
            }
        }
        // Note that modo expects textures to output the right kind of data based on the context. This is the reason for checking against
        // LXi_TFX_COLOR in the context below. If we aren't driving a color, we output a value instead.
	}
    if (LXi_TFX_COLOR == tInp->context)
    {
        tOut->color[0][0] = result[0];
        tOut->color[0][1] = result[1];
        tOut->color[0][2] = result[2];
    }
    tOut->value[0] = value;
    tOut->alpha[0] = alpha;

}

/*
 * Release the cached state after rendering is complete.
 */
void aaOceanTexture::vtx_Cleanup (void	*data)
{
	RendData		*rd = (RendData *) data;

	if(rd != NULL) delete rd;

}
