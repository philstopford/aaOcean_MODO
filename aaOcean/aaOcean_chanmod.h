#ifndef CMAAOCEANMODO_H
#define CMAAOCEANMODO_H

#include <lx_item.hpp>
#include <lx_package.hpp>
#include <lx_value.hpp>
#include <lxlog.h>
#include <lxu_log.hpp>
#include <lx_chanmod.hpp>
#include <memory>
#include <iostream>
#include <mutex>
#include "aaOceanClass.h"

namespace aaOceanChanModNameSpace {

class aaOceanChanModLog : public CLxLuxologyLogMessage
{
    public:
        aaOceanChanModLog () : CLxLuxologyLogMessage ("aaOceanChanMod") { }

        const char *	 GetFormat  () { return "aaOcean for Modo Channel modifier"; }
};

class aaOceanChanModPackage;

class OceanData {
        
    public:
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
        float foamMax;
        float foamRange;
    
        bool operator == (const OceanData &oceanData) const {
            return (this->m_resolution == oceanData.m_resolution &&
                    this->m_oceanSize == oceanData.m_oceanSize &&
                    this->m_waveHeight == oceanData.m_waveHeight &&
                    this->m_waveSize == oceanData.m_waveSize &&
                    this->m_surfaceTension == oceanData.m_surfaceTension &&
                    this->m_waveAlign == oceanData.m_waveAlign &&
                    this->m_waveSmooth == oceanData.m_waveSmooth &&
                    this->m_waveDirection == oceanData.m_waveDirection &&
                    this->m_waveReflection == oceanData.m_waveReflection &&
                    this->m_waveSpeed == oceanData.m_waveSpeed &&
                    this->m_waveChop == oceanData.m_waveChop &&
                    this->m_oceanDepth == oceanData.m_oceanDepth &&
                    this->m_seed == oceanData.m_seed &&
                    this->m_doFoam == oceanData.m_doFoam &&
                    this->m_doNormals == oceanData.m_doNormals &&
                    this->m_time == oceanData.m_time &&
                    this->foamMax == oceanData.foamMax &&
                    this->foamRange == oceanData.foamRange
                    ); // Check all the other values.
        }
        
        bool operator != (const OceanData &oceanData) const {
            return !(*this == oceanData);
        }
        
};
    
class aaOceanChanMod
        :
        public CLxImpl_PackageInstance,
        public CLxImpl_ChannelModItem
{
        aaOceanChanModLog		 log;

    private:
        // This will reinitialize the ocean data and aaOcean object if the values have changed, or
        // they haven't been initialized at all yet.
        void maybeResetOceanData(std::unique_ptr<OceanData> newOceanData);
        
        // A single oceanData for this object.
        // We use a unique pointer because it guarantees that an uninitialized object will crash,
        // and makes it easy to check for initialization without another variable.
        // And we don't ever need to copy OceanData values, even when we've set them all.
        std::unique_ptr<OceanData> oceanData_;
        // A single aaOcean for this object.
        aaOcean mOcean_;
        // One Mutex per object.
        std::mutex myMutex_;
    public:
    
        aaOceanChanModPackage	*src_pkg;
        CLxUser_Item		 m_item;
        ILxUnknownID		 inst_ifc;
        
        aaOceanChanMod ();
        ~aaOceanChanMod ();

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

        bool tone;
        // Indices for ChannelLookup, Flags, etc.
        unsigned m_idx_x;
        unsigned m_idx_z;
        unsigned m_idx_tone;
        unsigned m_idx_resolution;
        unsigned m_idx_oceanSize;
        unsigned m_idx_waveHeight;
        unsigned m_idx_waveSize;
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
        unsigned m_idx_foamRange;
        unsigned m_idx_foamMax;

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
        unsigned cm_idx_tone;
        unsigned cm_idx_resolution;
        unsigned cm_idx_oceanSize;
        unsigned cm_idx_waveHeight;
        unsigned cm_idx_waveSize;
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
        unsigned cm_idx_foamRange;
        unsigned cm_idx_foamMax;

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
