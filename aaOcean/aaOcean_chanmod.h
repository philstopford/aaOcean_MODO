#ifndef CMAAOCEANMODO_H
#define CMAAOCEANMODO_H

#include <lx_item.hpp>
#include <lx_package.hpp>
#include <lx_value.hpp>
#include <lxlog.h>
#include <lxu_log.hpp>
#include <lx_chanmod.hpp>

#include <iostream>

#include "aaOceanClass.h"

namespace aaOceanChanModNameSpace {

class aaOceanChanModLog : public CLxLuxologyLogMessage
{
    public:
        aaOceanChanModLog () : CLxLuxologyLogMessage ("aaOceanChanMod") { }

        const char *	 GetFormat  () { return "aaOcean for Modo Channel modifier"; }
};

class aaOceanChanModPackage;

class aaOceanChanMod
        :
        public CLxImpl_PackageInstance,
        public CLxImpl_ChannelModItem
{
        aaOceanChanModLog		 log;
        
    public:
    
        aaOceanChanModPackage	*src_pkg;
        CLxUser_Item		 m_item;
        ILxUnknownID		 inst_ifc;
        
        aaOceanChanMod ();
        ~aaOceanChanMod ();
        aaOcean *m_ocean;

        LxResult		 pins_Initialize (ILxUnknownID item, ILxUnknownID super) LXx_OVERRIDE;
        void			 pins_Cleanup (void) LXx_OVERRIDE;
        LxResult		 pins_SynthName (char *buf, unsigned len) LXx_OVERRIDE;

        unsigned int		 cmod_Flags (ILxUnknownID item, unsigned int index) LXx_OVERRIDE;
        LxResult		 cmod_Allocate (
                                        ILxUnknownID cmod,
                                        ILxUnknownID eval,
                                        ILxUnknownID item,
                                        void **ppvData) LXx_OVERRIDE;
        void			 cmod_Cleanup (void *data) LXx_OVERRIDE;
        LxResult		 cmod_Evaluate (ILxUnknownID cmod, ILxUnknownID attr, void *data) LXx_OVERRIDE;

        // Indices for ChannelLookup, Flags, etc.
        unsigned m_idx_x;
        unsigned m_idx_z;
        unsigned m_idx_resolution;
        unsigned m_idx_oceanSize;
        unsigned m_idx_waveHeight;
        unsigned m_idx_surfaceTension;
        unsigned m_idx_waveAlign;
        unsigned m_idx_waveSmooth;
        unsigned m_idx_waveDirection;
        unsigned m_idx_waveReflection;
        unsigned m_idx_waveSpeed;
        unsigned m_idx_waveChop;
        unsigned m_idx_oceanDepth;
        unsigned m_idx_repeatTime;
        unsigned m_idx_doFoam;

        unsigned m_idx_seed;
        unsigned m_idx_time;

        unsigned m_idx_displacementX;
        unsigned m_idx_displacementY;
        unsigned m_idx_displacementZ;
        unsigned m_idx_foam;
        unsigned m_idx_eigenplusX;
        unsigned m_idx_eigenplusY;
        unsigned m_idx_eigenplusZ;
        unsigned m_idx_eigenminusX;
        unsigned m_idx_eigenminusY;
        unsigned m_idx_eigenminusZ;
    
        // Indices for ChanMod
        unsigned cm_idx_x;
        unsigned cm_idx_z;
        unsigned cm_idx_resolution;
        unsigned cm_idx_oceanSize;
        unsigned cm_idx_waveHeight;
        unsigned cm_idx_surfaceTension;
        unsigned cm_idx_waveAlign;
        unsigned cm_idx_waveSmooth;
        unsigned cm_idx_waveDirection;
        unsigned cm_idx_waveReflection;
        unsigned cm_idx_waveSpeed;
        unsigned cm_idx_waveChop;
        unsigned cm_idx_oceanDepth;
        unsigned cm_idx_repeatTime;
        unsigned cm_idx_doFoam;

        unsigned cm_idx_seed;
        unsigned cm_idx_time;
    
        unsigned cm_idx_displacementX;
        unsigned cm_idx_displacementY;
        unsigned cm_idx_displacementZ;
        unsigned cm_idx_foam;
        unsigned cm_idx_eigenplusX;
        unsigned cm_idx_eigenplusY;
        unsigned cm_idx_eigenplusZ;
        unsigned cm_idx_eigenminusX;
        unsigned cm_idx_eigenminusY;
        unsigned cm_idx_eigenminusZ;

        class OceanData {
            
            public:
                float m_x;
                float m_z;
                int		m_resolution; // default is 2
                float m_oceanSize; // default is 100.0
                float m_waveHeight; // default is 2.0
                float m_waveSize; // default is 4.0
                float m_surfaceTension; // default is 0.0
                int m_waveAlign; // default is 1
                float m_waveSmooth; // default is 0.0
                float m_waveDirection; // default is 45.0
                float m_waveReflection; // default is 0.0
                float m_waveSpeed; // default is 1.0
                float m_waveChop; // default is 2.0
                float m_oceanDepth; // default is 10000
                int m_seed; // default is 1.0
                float m_repeatTime; // default is 1000.0
                bool m_doFoam; // default is FALSE for now.
                bool m_doNormals; // default is FALSE
                float m_time;

        };

};

class aaOceanChanModPackage : public CLxImpl_Package
{
    public:
        static LXtTagInfoDesc			descInfo[];
        CLxPolymorph<aaOceanChanMod>	chanmod_factory;
        
        aaOceanChanModPackage ();

        LxResult		pkg_SetupChannels (ILxUnknownID addChan);
        LxResult		pkg_TestInterface (const LXtGUID *guid);
        LxResult		pkg_Attach (void **ppvObj);
};

    void
    
    initialize ()
    {
        CLxGenericPolymorph		*srv;
        
        srv = new CLxPolymorph<aaOceanChanModPackage>;
        srv->AddInterface (new CLxIfc_Package          <aaOceanChanModPackage>);
        srv->AddInterface (new CLxIfc_StaticDesc       <aaOceanChanModPackage>);
        lx::AddServer ("aaOceanChanMod", srv);
    }

}
#endif // CMAAOCEANMODO_H
