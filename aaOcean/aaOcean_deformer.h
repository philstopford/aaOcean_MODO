#ifndef H_NOT_MODO_DEFORMER
#define H_NOT_MODO_DEFORMER

#include <lx_deform.hpp>
#include <lx_package.hpp>
#include <lx_plugin.hpp>
#include <lxu_deform.hpp>
#include <lxu_modifier.hpp>
#include <lxu_math.hpp>
#include <lxidef.h>
#include <lx_select.hpp>
#include <string>
#include <math.h>
// #include <mutex>

#include "aaOceanClass.h"

namespace aaOceanDeformer {	// disambiguate everything with a namespace

// std::mutex myMutex; // global variable

#define SRVNAME_ITEMTYPE		"aaOceanDeform"
#define SRVNAME_MODIFIER		"aaOceanDeform"
#define SPWNAME_INSTANCE		"aaOcean.inst"
#define Cs_MORPH_MAPNAME		LXsICHAN_MORPHDEFORM_MAPNAME
#define Cs_WEIGHT_MAPNAME       LXsICHAN_WEIGHTCONTAINER_WEIGHTMAP
    
class CPackage;

class CInstance : public CLxImpl_PackageInstance
{
	public:
		CPackage		*src_pkg;
		CLxUser_Item	 m_item;

		LxResult	pins_Initialize (ILxUnknownID item, ILxUnknownID super)	LXx_OVERRIDE;
		void		pins_Cleanup (void) LXx_OVERRIDE;
};	


class CPackage : public CLxImpl_Package
{
	public:
		static LXtTagInfoDesc	 descInfo[];
		CLxSpawner<CInstance>	 inst_spawn;

		CPackage () : inst_spawn (SPWNAME_INSTANCE) {}

		LxResult	 pkg_SetupChannels (ILxUnknownID addChan)	LXx_OVERRIDE;
		LxResult	 pkg_TestInterface (const LXtGUID *guid)	LXx_OVERRIDE;
		LxResult	 pkg_Attach (void **ppvObj)					LXx_OVERRIDE;
};


/* 
 * Mesh Influence for a morph is dead simple. The CChanState holds values of
 * the channels and handles getting them from the evaluation state. It also
 * serves as the modifier cache so we can compare previous values.
 */
class CChanState : public CLxObject
{
	public:
		CChanState ()
		{

		}

		~CChanState()
		{

		}

		std::string		 name;
		double			 gain;
		LXtMatrix		 xfrm;
		bool			 enabled;
        bool             tone;
        int				 resolution; // default is 4
		float oceanSize; // default is 100.0
		float waveHeight; // default is 2.0
        float waveSize; // default is 4.0
        float surfaceTension; // default is 0.0
        int waveAlign; // default is 1
        float waveSmooth; // default is 0.0
        float waveDirection; // default is 45.0
        float waveReflection; // default is 0.0
        float waveSpeed; // default is 1.0
        float waveChop; // default is 2.0
		float oceanDepth; // default is 10000
		float seed; // default is 1.0
		float time;
        float repeatTime; // default is 1000.0
        bool doFoam; // default is FALSE for now.
        bool doNormals; // default is FALSE

        aaOcean *m_pOcean;

        void buildOcean();

		void setUpOceanPtrs(aaOcean *ocean);

		void Attach (CLxUser_Evaluation	&eval, ILxUnknownID item);

		void Read (CLxUser_Attributes &attr, unsigned index);

		LxResult Compare (CChanState &that);
};

/*
 * The influence itself selects the morph map by name and then extracts the
 * offsets for each point.
 */
class CInfluence : public CLxMeshInfluence
{
    public:
		CLxUser_SelectionService   selSrv;
		CChanState			cur;
		LXtMeshMapID		map_id, jacobianPos_id, jacobianMin_id, jacobianFoamX_id, jacobianFoamY_id, jacobianFoamZ_id, jacobianSprayX_id, jacobianSprayY_id, jacobianSprayZ_id, jacobianEigenpX_id, jacobianEigenpY_id, jacobianEigenpZ_id, jacobianEigenmX_id, jacobianEigenmY_id, jacobianEigenmZ_id;
		bool				gotUvs;

		bool SelectMap (CLxUser_Mesh &mesh, CLxUser_MeshMap &map) LXx_OVERRIDE;
		void Offset (CLxUser_Point &point, float weight, LXtFVector	offset)	LXx_OVERRIDE;
};
    
class CModifierElement : public CLxItemModifierElement
{
    public:
		unsigned	 index;
        aaOcean *pOcean;
        bool tone;
        int				 resolution; // default is 4
        float oceanSize; // default is 100.0
        float waveHeight; // default is 2.0
        float waveSize; // default is 4.0
        float surfaceTension; // default is 0.0
        int waveAlign; // default is 1
        float waveSmooth; // default is 0.0
        float waveDirection; // default is 45.0
        float waveReflection; // default is 0.0
        float waveSpeed; // default is 1.0
        float waveChop; // default is 2.0
        float oceanDepth; // default is 10000
        float seed; // default is 1.0
        float repeatTime; // default is 1000.0
        bool doFoam;
        bool doNormals;

		CModifierElement()
		{
			pOcean = new aaOcean();
            tone = true;
            resolution = 4;
            oceanSize =  100.0;
            waveHeight  = 2.0;
            waveSize = 4.0; // default is 4.0
            surfaceTension = 0.0; // default is 0.0
            waveAlign = 1; // default is 1
            waveSmooth = 0.0; // default is 0.0
            waveDirection = 45.0; // default is 45.0
            waveReflection = 0.0; // default is 0.0
            waveSpeed = 1.0; // default is 1.0
            waveChop = 2.0; // default is 2.0
            oceanDepth = 10000.0; // default is 10000
            seed = 1.0; // default is 1.0
            repeatTime = 1000.0; // default is 1000.0
            doFoam = bool(0);
            doNormals = bool(1);
		}

		~CModifierElement()
		{
			if (pOcean)
            {
                delete pOcean;
                pOcean = NULL;
            }
		}

		CLxObject *Cache ();

		LxResult	EvalCache (CLxUser_Evaluation &eval, CLxUser_Attributes &attr, CLxObject *cacheRaw, bool prev);
};

class CModifier : public CLxItemModifierServer
{
    public: 
		const char *ItemType ()
        {
			return SRVNAME_ITEMTYPE;
        }

		CLxItemModifierElement *Alloc (CLxUser_Evaluation &eval, ILxUnknownID item)
        {
            CModifierElement	*elt;
            CChanState		 tmp;

            elt = new CModifierElement;
            elt->index = eval.AddChan (item, LXsICHAN_MORPHDEFORM_MESHINF, LXfECHAN_WRITE);

            tmp.Attach (eval, item);

            return elt;
        }
};

/*
 * Export package server to define a new item type. Also create and destroy
 * the factories so they can persist while their objects are in use.
 */
void initialize ()
{
        CLxGenericPolymorph		*srv;

        srv = new CLxPolymorph<CPackage>;
        srv->AddInterface (new CLxIfc_Package   <CPackage>);
        srv->AddInterface (new CLxIfc_StaticDesc<CPackage>);
        lx::AddServer (SRVNAME_ITEMTYPE, srv);

        srv = new CLxPolymorph<CInstance>;
        srv->AddInterface (new CLxIfc_PackageInstance<CInstance>);
        lx::AddSpawner (SPWNAME_INSTANCE, srv);

		CLxExport_ItemModifierServer<CModifier> (SRVNAME_MODIFIER);
}



}; //end namespace

#endif //H_NOT_MODO_DEFORMER