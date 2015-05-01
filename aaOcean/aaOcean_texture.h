#ifndef H_NOT_MODO_TEXTURE
#define H_NOT_MODO_TEXTURE

#include <lx_shade.hpp>
#include <lx_vector.hpp>
#include <lx_package.hpp>
#include <lx_channelui.hpp>
#include <lx_action.hpp>
#include <lx_value.hpp>
#include <lx_item.hpp>
#include <lx_log.hpp>
#include <lx_command.hpp>
#include <string>
#include <math.h>
#include <float.h>

#include "aaOceanClass.h"

namespace aaOceanTextureNamespace {	// disambiguate everything with a namespace

class OceanData {
    
    public:
        int m_outputType; // default is 0
        int     m_resolution; // default is 2
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
        float m_seed; // default is 1.0
        float m_repeatTime; // default is 1000.0
        bool m_doFoam; // default is FALSE for now.
        bool m_doNormals; // default is FALSE
        float m_time;

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
                    this->m_time == oceanData.m_time
                    ); // Check all the other values.
        }
        
        bool operator != (const OceanData &oceanData) const {
            return !(*this == oceanData);
        }
};

/*
 * ----------------------------------------------------------------
 * Value texture class
 *
 * This has the basic ValueTexture interface to support simple multi-effect
 * evaluations, plus the ChannelUI interface for enable states. The local
 * RendData struct is used for storing values used for a specific texture
 * evaluation.
 */
class aaOceanTexture : public CLxImpl_ValueTexture
{

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
        static LXtTagInfoDesc		 descInfo[];

		aaOceanTexture ();
		~aaOceanTexture ();

        LxResult		vtx_SetupChannels (ILxUnknownID addChan) LXx_OVERRIDE;
        LxResult		vtx_LinkChannels  (ILxUnknownID eval, ILxUnknownID item) LXx_OVERRIDE;
        LxResult		vtx_ReadChannels  (ILxUnknownID attr, void **ppvData) LXx_OVERRIDE;
#ifdef MODO701
        void			vtx_Evaluate      (ILxUnknownID vector, LXpTextureOutput *tOut, void *data) LXx_OVERRIDE;
#else
        void			vtx_Evaluate      (ILxUnknownID etor, int *idx, ILxUnknownID vector, LXpTextureOutput *tOut, void *data) LXx_OVERRIDE;
#endif
        void			vtx_Cleanup       (void *data) LXx_OVERRIDE;

		LXtItemType		MyType ();
        CLxUser_PacketService	pkt_service;
        unsigned		tin_offset,tinDsp_offset,nrm_offset;
        LXtItemType		my_type;

        unsigned m_idx_outputType;
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
        unsigned m_idx_seed;
        unsigned m_idx_repeatTime;
        unsigned m_idx_doFoam;
        unsigned m_idx_doNormals;
		unsigned m_idx_time;

        int m_outputTypeCache;
        int		m_resolutionCache;
        float m_oceanSizeCache;
        float m_waveHeightCache;
        float m_waveSizeCache;
        float m_surfaceTensionCache;
        int m_waveAlignCache;
        float m_waveSmoothCache;
        float m_waveDirectionCache;
        float m_waveReflectionCache;
        float m_waveSpeedCache;
        float m_waveChopCache;
        float m_oceanDepthCache;
        float m_seedCache;
        float m_repeatTimeCache;
        bool m_doFoamCache;
        bool m_doNormalsCache;
        float m_timeCache;

};
    void initialize ()
    {
        CLxGenericPolymorph		*srv;
        
        srv = new CLxPolymorph<aaOceanTexture>;
        srv->AddInterface (new CLxIfc_ValueTexture<aaOceanTexture>);
        //srv->AddInterface (new CLxIfc_ChannelUI   <aaOceanTexture>);
        srv->AddInterface (new CLxIfc_StaticDesc  <aaOceanTexture>);
        lx::AddServer ("aaOcean.texture", srv);
    }

}
#endif //H_NOT_MODO_TEXTURE