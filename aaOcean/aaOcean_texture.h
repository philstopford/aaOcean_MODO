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
        unsigned		tin_offset,tinDsp_offset;
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

        class RendData {
			
            public:

                aaOcean *m_ocean = NULL;
                int m_outputType; // default is 0
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
                float m_seed; // default is 1.0
                float m_repeatTime; // default is 1000.0
                bool m_doFoam; // default is FALSE for now.
                bool m_doNormals; // default is FALSE
				float m_time;
        };
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