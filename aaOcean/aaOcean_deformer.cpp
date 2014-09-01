
#include "aaOcean_deformer.h"


using namespace aaOceanDeformer;


/*
* The instance is the implementation of the item, and there will be one
* allocated for each item in the scene. It can respond to a set of
* events.
*/

LxResult CInstance::pins_Initialize (ILxUnknownID item, ILxUnknownID super)
{
	m_item.set (item);
	return LXe_OK;
}

void CInstance::pins_Cleanup (void)
{
	m_item.clear ();
}



/*
 * ----------------------------------------------------------------
 * Package Class
 *
 * Packages implement item types, or simple item extensions. They are
 * like the metatype object for the item type. They define the common
 * set of channels for the item type and spawn new instances.
 */
/*
 * The package has a set of standard channels with default values. These
 * are setup at the start using the AddChannel interface.
 */

LXtTagInfoDesc CPackage::descInfo[] = {
        { LXsPKG_SUPERTYPE,		LXsITYPE_LOCDEFORM	},
        { LXsPKG_DEFORMER_CHANNEL,	LXsICHAN_MORPHDEFORM_MESHINF },
        { LXsPKG_DEFORMER_FLAGS,	"+WX"			},	// no weight, no xfrm
        { LXsPKG_DEFORMER_CREATECMD,	"aaOcean.create"	},
        { 0 }
};

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

LxResult CPackage::pkg_SetupChannels(ILxUnknownID addChan)
{
	CLxUser_AddChannel	 ac (addChan);

	ac.NewChannel  (LXsICHAN_MORPHDEFORM_MESHINF,		LXsTYPE_OBJREF);
	ac.SetInternal ();

	ac.NewChannel  ("enable",		LXsTYPE_BOOLEAN);
	ac.SetDefault  (0.0, 1);

	ac.NewChannel  (Cs_MORPH_MAPNAME,	LXsTYPE_VERTMAPNAME);

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
    
    ac.NewChannel  ("seed",	LXsTYPE_FLOAT);
	ac.SetDefault  (1.0f, 0);
    
    ac.NewChannel  ("repeatTime",	LXsTYPE_FLOAT);
	ac.SetDefault  (1000.0f, 0);
    
	ac.NewChannel  ("doFoam",	LXsTYPE_INTEGER);
	ac.SetDefault  (0.0, 0);
    
    ac.NewChannel  ("doNormals",	LXsTYPE_INTEGER);
	ac.SetDefault  (0.0, 0);

	return LXe_OK;
}

/*
 * TestInterface() is required so that nexus knows what interfaces instance
 * of this package support. Necessary to prevent query loops.
 */
LxResult CPackage::pkg_TestInterface (const LXtGUID		*guid)
{
	return inst_spawn.TestInterfaceRC (guid);
}

/*
 * Attach is called to create a new instance of this item. The returned
 * object implements a specific item of this type in the scene.
 */
LxResult CPackage::pkg_Attach (void **ppvObj)
{
	CInstance		*inst = inst_spawn.Alloc (ppvObj);

	inst->src_pkg = this;
	return LXe_OK;
}

void CChanState::buildOcean()
{
    m_pOcean->input(resolution,
                    (unsigned long)seed,
                    oceanSize,
                    oceanDepth,
                    surfaceTension,
                    waveSize,
                    waveSmooth,
                    waveDirection,
                    waveAlign,
                    waveReflection,
                    waveSpeed,
                    waveHeight,
                    waveChop,
                    time,
                    repeatTime,
                    doFoam,
                    doNormals);
}

void CChanState::setUpOceanPtrs(aaOcean *ocean)
{
    m_pOcean = ocean;
}

void CChanState::Attach (CLxUser_Evaluation	&eval, ILxUnknownID item)
{
    eval.AddChan (item, "enable");
    eval.AddChan (item, Cs_MORPH_MAPNAME);
    eval.AddChan (item, "resolution");
	eval.AddChan (item, "oceanSize");
	eval.AddChan (item, "waveHeight");
	eval.AddChan (item, "surfaceTension");
	eval.AddChan (item, "waveAlign");
	eval.AddChan (item, "waveSmooth");
	eval.AddChan (item, "waveDirection");
	eval.AddChan (item, "waveReflection");
	eval.AddChan (item, "waveSpeed");
	eval.AddChan (item, "waveChop");
	eval.AddChan (item, "oceanDepth");
	eval.AddChan (item, "waveSize");
	eval.AddChan (item, "seed");
	eval.AddChan (item, "repeatTime");
	eval.AddChan (item, "doFoam");
	eval.AddChan (item, "doNormals");

	eval.AddTime ();

	eval.AddChan (item, LXsICHAN_XFRMCORE_WORLDMATRIX);
}

void CChanState::Read (CLxUser_Attributes &attr, unsigned index)
{
    CLxUser_Matrix		 m4;

    enabled = attr.Bool (index++);
    if (enabled) 
	{
        attr.String (index++, name);
		
		resolution  = attr.Int  (index++);
		if(resolution > 12)
        {
            resolution = 12;
        }
        if(resolution < 4)
        {
            resolution = 4;
        }
		
        oceanSize = attr.Float (index++);
        if(oceanSize <= 0)
        {
            oceanSize = 0.01;
        }
		waveHeight = attr.Float (index++);
		surfaceTension = attr.Float (index++);
		waveAlign = attr.Int (index++);
		waveSmooth = attr.Float (index++);
		waveDirection = attr.Float (index++);
		waveReflection = attr.Float (index++);
		waveSpeed = attr.Float (index++);
		waveChop = attr.Float (index++);
		oceanDepth = attr.Float (index++);
		waveSize = attr.Float (index++);
		seed = attr.Float (index++);
		repeatTime = attr.Float (index++);
		doFoam  = attr.Bool(index++);
		doNormals  = attr.Bool  (index++);
		
		time = attr.Float (index++);

        attr.ObjectRO (index++, m4);
        m4.Get3 (xfrm);
    }
}


LxResult CChanState::Compare (CChanState &that)
{
	if (enabled != that.enabled || name.compare (that.name))
			return LXeEVAL_DIFFERENT;

	return LXeDEFORM_NEWOFFSET;
}

bool CInfluence::SelectMap (CLxUser_Mesh &mesh, CLxUser_MeshMap &map)
{
    map.SelectByName (LXi_VMAP_TEXTUREUV, "Texture");
    map_id = map.ID ();
    if (map_id) 
	{
        gotUvs = true;
    }


    return false;
}

void CInfluence::Offset (CLxUser_Point &point, float weight, LXtFVector	offset)
{
    LXtFVector		 posF;
	if(!cur.enabled)
        return;

	point.Pos (posF);

    float result[3];
    float p[2];
    p[0] = (float)posF[0];
    p[1] = (float)posF[2];

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
    
    // We need to scale our coordinates by the ocean size since aaOcean expects 0-1 ranges incoming (it was built as a texture)
    // Let's get the Y displacement first
    result[0] = result[1] = result[2] = 0.0;
    result[1] = cur.m_pOcean->getOceanData(p[0]/cur.m_pOcean->m_oceanScale,p[1]/cur.m_pOcean->m_oceanScale, aaOcean::eHEIGHTFIELD);
    if(cur.m_pOcean->isChoppy())
    {
        result[0] = cur.m_pOcean->getOceanData(p[0]/cur.m_pOcean->m_oceanScale, p[1]/cur.m_pOcean->m_oceanScale, aaOcean::eCHOPX);
        result[2] = cur.m_pOcean->getOceanData(p[0]/cur.m_pOcean->m_oceanScale, p[1]/cur.m_pOcean->m_oceanScale, aaOcean::eCHOPZ);
    } else {
        result[0] = 0.0;
        result[2] = 0.0;
    }
    LXtFVector	 tmp;

    lx::MatrixMultiply (tmp, cur.xfrm, result);
    LXx_VSCL3 (offset, tmp, weight);
}

CLxObject *CModifierElement::Cache ()
{
	return new CChanState;
}

LxResult CModifierElement::EvalCache (CLxUser_Evaluation &eval, CLxUser_Attributes &attr, CLxObject *cacheRaw, bool prev)
{
	CChanState		*cache = dynamic_cast<CChanState *> (cacheRaw);
	CInfluence		*infl;
	CLxUser_ValueReference	 ref;
	ILxUnknownID		 obj;
	LxResult		 rc;

    infl = new CInfluence;
    infl->Spawn ((void **) &obj);
    attr.ObjectRW (index, ref);
    ref.SetObject (obj);
    lx::UnkRelease (obj);

    infl->cur.Read (attr, index + 1);
	//early out
	if(!infl->cur.enabled)
        return LXe_OK;
    resolution = infl->cur.resolution;
    oceanSize = infl->cur.oceanSize;
    waveHeight = infl->cur.waveHeight;
    waveSize = infl->cur.waveSize;
    surfaceTension = infl->cur.surfaceTension;
    waveAlign = infl->cur.waveAlign;
    waveSmooth = infl->cur.waveSmooth;
    waveDirection = infl->cur.waveDirection;
    waveReflection = infl->cur.waveReflection;
    waveSpeed = infl->cur.waveSpeed;
    waveChop = infl->cur.waveChop;
    oceanDepth = infl->cur.oceanDepth;
    repeatTime = infl->cur.repeatTime;
    doFoam = infl->cur.doFoam;
    doNormals = infl->cur.doNormals;
    seed = infl->cur.seed;

    pOcean->input(	resolution,
                    (unsigned long)seed,
                    oceanSize,
                    oceanDepth,
                    surfaceTension,
                    waveSize,
                    waveSmooth,
                    waveDirection,
                    waveAlign,
                    waveReflection,
                    waveSpeed,
                    waveHeight,
                    waveChop,
                    infl->cur.time,
                    repeatTime,
                    doFoam,
                    doNormals);
    infl->cur.setUpOceanPtrs(pOcean);

    if (prev)
		rc = cache->Compare (infl->cur);
    else
		rc = LXe_OK;

    *cache = infl->cur;

    return rc;
}
