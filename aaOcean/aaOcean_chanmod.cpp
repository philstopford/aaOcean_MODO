#include "aaOcean_chanmod.h"
#include <lx_plugin.hpp>
#include <string>
#include <fstream>
#include <mutex>
#include <memory>

std::mutex myMutex;

using namespace aaOceanChanModNameSpace;

aaOceanChanMod::aaOceanChanMod ()
{
}

aaOceanChanMod::~aaOceanChanMod ()
{
}


LxResult
aaOceanChanMod::pins_Initialize (
        ILxUnknownID		 item,
        ILxUnknownID		 super)
{
        log.Info ("Initialize");
        
        m_item.set (item);
        
        return LXe_OK;
}

        void
aaOceanChanMod::pins_Cleanup (void)
{
        m_item.clear ();
}

        LxResult
aaOceanChanMod::pins_SynthName (
        char			*buf,
        unsigned		 len)
{
        std::string name ("aaOceanChanMod");
        size_t count = name.size () + 1;
        if (count > len) {
                count = len;
        }
        memcpy (buf, &name[0], count);

        return LXe_OK;
}


// Allocate is called prior to evaluation. We can't use the indices here to, for example, 'optimize' the Flags method.
        LxResult
aaOceanChanMod::cmod_Allocate (
        ILxUnknownID		 cmod,		// ILxChannelModifierID
        ILxUnknownID		 eval,		// ILxEvaluationID
        ILxUnknownID		 item,
        void		       **ppvData)
{
        CLxLoc_ChannelModifier	 chanMod (cmod);
        CLxUser_Item		 modItem (item);
    
        //	log.Info ("cmod_Allocate Method");
        int index = 0;
        // Lookup the index of the 'input X' channel and add as an input.
        // m_idx_ variables used to identify input/output channels in the Flags method for the schematic.
        // cm_idx_ variables used to find each channel in the channel modifier later (to read/write values).
        //   Channel modifier uses separate 0-index counters for inputs and outputs, based on order of addition so we just use the index counter.
        modItem.ChannelLookup ("inputX", &m_idx_x);
        chanMod.AddInput (item, m_idx_x);
        cm_idx_x = index; 
        index++;
        // Lookup the index of the 'inputZ' channel and add as an input.
        modItem.ChannelLookup ("inputZ", &m_idx_z);
        chanMod.AddInput (item, m_idx_z);
        cm_idx_z = index;
        index++;
    
        // Lookup the index of the 'tone' channel and add as an input.
        modItem.ChannelLookup ("tone", &m_idx_tone);
        chanMod.AddInput (item, m_idx_tone);
        cm_idx_tone = index;
        index++;

        // Lookup the index of the 'resolution' channel and add as an input.
        modItem.ChannelLookup ("resolution", &m_idx_resolution);
        chanMod.AddInput (item, m_idx_resolution);
        cm_idx_resolution = index;
        index++;

        // Lookup the index of the 'oceanSize' channel and add as an input.
        modItem.ChannelLookup ("oceanSize", &m_idx_oceanSize);
        chanMod.AddInput (item, m_idx_oceanSize);
        cm_idx_oceanSize = index;
        index++;

        // Lookup the index of the 'waveHeight' channel and add as an input.
        modItem.ChannelLookup ("waveHeight", &m_idx_waveHeight);
        chanMod.AddInput (item, m_idx_waveHeight);
        cm_idx_waveHeight = index;
        index++;

        // Lookup the index of the 'waveSize' channel and add as an input.
        modItem.ChannelLookup ("waveSize", &m_idx_waveSize);
        chanMod.AddInput (item, m_idx_waveSize);
        cm_idx_waveSize = index;
        index++;
    
        // Lookup the index of the 'surfaceTension' channel and add as an input.
        modItem.ChannelLookup ("surfaceTension", &m_idx_surfaceTension);
        chanMod.AddInput (item, m_idx_surfaceTension);
        cm_idx_surfaceTension = index;
        index++;

        // Lookup the index of the 'waveAlign' channel and add as an input.
        modItem.ChannelLookup ("waveAlign", &m_idx_waveAlign);
        chanMod.AddInput (item, m_idx_waveAlign);
        cm_idx_waveAlign = index;
        index++;

        // Lookup the index of the 'waveSmooth' channel and add as an input.
        modItem.ChannelLookup ("waveSmooth", &m_idx_waveSmooth);
        chanMod.AddInput (item, m_idx_waveSmooth);
        cm_idx_waveSmooth = index;
        index++;

        // Lookup the index of the 'waveDirection' channel and add as an input.
        modItem.ChannelLookup ("waveDirection", &m_idx_waveDirection);
        chanMod.AddInput (item, m_idx_waveDirection);
        cm_idx_waveDirection = index;
        index++;

        // Lookup the index of the 'waveReflection' channel and add as an input.
        modItem.ChannelLookup ("waveReflection", &m_idx_waveReflection);
        chanMod.AddInput (item, m_idx_waveReflection);
        cm_idx_waveReflection = index;
        index++;

        // Lookup the index of the 'waveSpeed' channel and add as an input.
        modItem.ChannelLookup ("waveSpeed", &m_idx_waveSpeed);
        chanMod.AddInput (item, m_idx_waveSpeed);
        cm_idx_waveSpeed = index;
        index++;

        // Lookup the index of the 'waveChop' channel and add as an input.
        modItem.ChannelLookup ("waveChop", &m_idx_waveChop);
        chanMod.AddInput (item, m_idx_waveChop);
        cm_idx_waveChop = index;
        index++;

        // Lookup the index of the 'oceanDepth' channel and add as an input.
        modItem.ChannelLookup ("oceanDepth", &m_idx_oceanDepth);
        chanMod.AddInput (item, m_idx_oceanDepth);
        cm_idx_oceanDepth = index;
        index++;

        // Lookup the index of the 'seed' channel and add as an input.
        modItem.ChannelLookup ("seed", &m_idx_seed);
        chanMod.AddInput (item, m_idx_seed);
        cm_idx_seed = index;
        index++;

        // Lookup the index of the 'repeatTime' channel and add as an input.
        modItem.ChannelLookup ("repeatTime", &m_idx_repeatTime);
        chanMod.AddInput (item, m_idx_repeatTime);
        cm_idx_repeatTime = index;
        index++;

        // Lookup the index of the 'doFoam' channel and add as an input.
        modItem.ChannelLookup ("doFoam", &m_idx_doFoam);
        chanMod.AddInput (item, m_idx_doFoam);
        cm_idx_doFoam = index;
        index++;

        chanMod.AddTime ();
        cm_idx_time = index;

        index = 0; // reset counter for output channel tracking.
        // Lookup the index of the 'displacement' channel and add it as an output.
        modItem.ChannelLookup ("displacement.X", &m_idx_displacementX);
        chanMod.AddOutput (item, m_idx_displacementX);
        cm_idx_displacementX = index;
        index++;
        modItem.ChannelLookup ("displacement.Y", &m_idx_displacementY);
        chanMod.AddOutput (item, m_idx_displacementY);
        cm_idx_displacementY = index;
        index++;
        modItem.ChannelLookup ("displacement.Z", &m_idx_displacementZ);
        chanMod.AddOutput (item, m_idx_displacementZ);
        cm_idx_displacementZ = index;
        index++;

        // Lookup the index of the 'foam' channel and add it as an output.
        modItem.ChannelLookup ("foam", &m_idx_foam);
        chanMod.AddOutput (item, m_idx_foam);
        cm_idx_foam = index;
        index++;
    
        // Lookup the index of the 'Eigenplus' channel and add it as an output.
        modItem.ChannelLookup ("Eigenplus.X", &m_idx_eigenplusX);
        chanMod.AddOutput (item, m_idx_eigenplusX);
        cm_idx_eigenplusX = index;
        index++;
        modItem.ChannelLookup ("Eigenplus.Y", &m_idx_eigenplusY);
        chanMod.AddOutput (item, m_idx_eigenplusY);
        cm_idx_eigenplusY = index;
        index++;
        modItem.ChannelLookup ("Eigenplus.Z", &m_idx_eigenplusZ);
        chanMod.AddOutput (item, m_idx_eigenplusZ);
        cm_idx_eigenplusZ = index;
        index++;

        // Lookup the index of the 'Eigenminus' channel and add it as an output.
        modItem.ChannelLookup ("Eigenminus.X", &m_idx_eigenminusX);
        chanMod.AddOutput (item, m_idx_eigenminusX);
        cm_idx_eigenminusX = index;
        index++;
        modItem.ChannelLookup ("Eigenminus.Y", &m_idx_eigenminusY);
        chanMod.AddOutput (item, m_idx_eigenminusY);
        cm_idx_eigenminusY = index;
        index++;
        modItem.ChannelLookup ("Eigenminus.Z", &m_idx_eigenminusZ);
        chanMod.AddOutput (item, m_idx_eigenminusZ);
        cm_idx_eigenminusZ = index;
    
        return LXe_OK;
}

        void
aaOceanChanMod::cmod_Cleanup (
        void			*data)
{
}

// Flags defines schematic rules for the channels. This applies when the modifier is added to the schematic.
// Note that you can't simplify this to use the indices from Allocate because Allocate is used only prior to evaluation.
        unsigned int
aaOceanChanMod::cmod_Flags (
        ILxUnknownID		 item,
        unsigned int		 index)
{
        CLxUser_Item		 modItem (item);
        unsigned int         chanIdx;
    
        //	log.Info ("cmod_Flags Method");

        if (LXx_OK (modItem.ChannelLookup ("inputX", &chanIdx))) {
                if (index == chanIdx)
                        return LXfCHMOD_INPUT;
        }
        if (LXx_OK (modItem.ChannelLookup ("inputZ", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }
    
        if (LXx_OK (modItem.ChannelLookup ("tone", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("resolution", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("oceanSize", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("waveHeight", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("waveSize", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("surfaceTension", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("waveAlign", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("waveSmooth", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("waveDirection", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("waveReflection", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("waveSpeed", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("waveChop", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("oceanDepth", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("seed", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("doFoam", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("repeatTime", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_INPUT;
        }
    
        if (LXx_OK (modItem.ChannelLookup ("displacement.X", &chanIdx))) {
                if (index == chanIdx)
                        return LXfCHMOD_OUTPUT;
        }
        if (LXx_OK (modItem.ChannelLookup ("displacement.Y", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_OUTPUT;
        }
        if (LXx_OK (modItem.ChannelLookup ("displacement.Z", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_OUTPUT;
        }
    
        if (LXx_OK (modItem.ChannelLookup ("foam", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_OUTPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("Eigenplus.X", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_OUTPUT;
        }
        if (LXx_OK (modItem.ChannelLookup ("Eigenplus.Y", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_OUTPUT;
        }
        if (LXx_OK (modItem.ChannelLookup ("Eigenplus.Z", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_OUTPUT;
        }

        if (LXx_OK (modItem.ChannelLookup ("Eigenminus.X", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_OUTPUT;
        }
        if (LXx_OK (modItem.ChannelLookup ("Eigenminus.Y", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_OUTPUT;
        }
        if (LXx_OK (modItem.ChannelLookup ("Eigenminus.Z", &chanIdx))) {
            if (index == chanIdx)
                return LXfCHMOD_OUTPUT;
        }

        return 0;
}

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
        LxResult
aaOceanChanMod::cmod_Evaluate (
        ILxUnknownID		 cmod,		// ILxChannelModifierID
        ILxUnknownID		 attr,		// ILxAttributesID
        void			*data)
{
    CLxLoc_ChannelModifier	 chanMod (cmod);

    CLxUser_Attributes	 at (attr);
    std::unique_ptr<OceanData> newOceanData(new OceanData());
    
    // Variables used to handle returned values from chanMod.ReadInput* methods.
    double dTemp; // used for 'ReadFloat' where modo's SDK returns a double.
    int iTemp;

    //	log.Info ("cmod_Evaluate Method");
    // ReadInputFloat() returns a double, so we use dTemp as an intermediate to cast to a float later in each case.
    chanMod.ReadInputFloat (attr, cm_idx_x, &dTemp);
    float m_x = (float) dTemp;
    chanMod.ReadInputFloat (attr, cm_idx_z, &dTemp);
    float m_z = (float) dTemp;

    chanMod.ReadInputInt (attr, cm_idx_tone, &iTemp);
    tone = (bool) iTemp;

    chanMod.ReadInputInt (attr, cm_idx_resolution, &iTemp);
    newOceanData->m_resolution = iTemp;
	if(newOceanData->m_resolution > 14)
    {
        newOceanData->m_resolution = 14;
    }
	if(newOceanData->m_resolution < 4)
    {
        newOceanData->m_resolution = 4;
    }

    chanMod.ReadInputFloat (attr, cm_idx_oceanSize, &dTemp);
    newOceanData->m_oceanSize = (float) dTemp;
    if(newOceanData->m_oceanSize <= 0)
    {
        newOceanData->m_oceanSize = 0.01; // safety value.
    }

    // Let's scale our values - experiment.
    m_x = m_x/newOceanData->m_oceanSize;
    // Note the flip of V sign as aaOcean modifies the sign again internally due to SI/Maya coordinate system.
    // This approach appears to result in a better ocean characteristic.
    m_z = -m_z/newOceanData->m_oceanSize;
    
    chanMod.ReadInputFloat (attr, cm_idx_waveHeight, &dTemp);
    newOceanData->m_waveHeight = (float) dTemp;

    chanMod.ReadInputFloat (attr, cm_idx_waveSize, &dTemp);
    newOceanData->m_waveSize = (float) dTemp;

    chanMod.ReadInputFloat (attr, cm_idx_surfaceTension, &dTemp);
    newOceanData->m_surfaceTension = (float) dTemp;

    chanMod.ReadInputInt (attr, cm_idx_waveAlign, &iTemp);
    newOceanData->m_waveAlign = (int) iTemp;

    chanMod.ReadInputFloat (attr, cm_idx_waveSmooth, &dTemp);
    newOceanData->m_waveSmooth = (float) dTemp;

    chanMod.ReadInputFloat (attr, cm_idx_waveDirection, &dTemp);
    newOceanData->m_waveDirection = (float) dTemp;

    chanMod.ReadInputFloat (attr, cm_idx_waveReflection, &dTemp);
    newOceanData->m_waveReflection = (float) dTemp;

    chanMod.ReadInputFloat (attr, cm_idx_waveSpeed, &dTemp);
    newOceanData->m_waveSpeed = (float) dTemp;
    if(newOceanData->m_waveSpeed <= 0)
    {
        newOceanData->m_waveSpeed = 0.01; // safety value.
    }

    chanMod.ReadInputFloat (attr, cm_idx_waveChop, &dTemp);
    newOceanData->m_waveChop = (float) dTemp;

    chanMod.ReadInputFloat (attr, cm_idx_oceanDepth, &dTemp);
    newOceanData->m_oceanDepth = (float) dTemp;

    chanMod.ReadInputInt (attr, cm_idx_seed, &iTemp);
    newOceanData->m_seed = iTemp;

    chanMod.ReadInputFloat (attr, cm_idx_repeatTime, &dTemp);
    newOceanData->m_repeatTime = (float) dTemp;

    chanMod.ReadInputInt (attr, cm_idx_doFoam, &iTemp);
    newOceanData->m_doFoam = (bool) iTemp;

    newOceanData->m_doNormals = false;

    chanMod.ReadInputFloat (attr, cm_idx_time, &dTemp);
    newOceanData->m_time = (float) dTemp;

    maybeResetOceanData(std::move(newOceanData)); // newOceanData is now invalid, do not use it again in this function!
    
    float result[3];
    float foam, eigenminus[3], eigenplus[3];
    bool foamy = false;
    
    result[1] = mOcean_.getOceanData(m_x, m_z, aaOcean::eHEIGHTFIELD);
    if (mOcean_.isChoppy())
    {
        result[0] = mOcean_.getOceanData(m_x, m_z, aaOcean::eCHOPX);
        result[2] = mOcean_.getOceanData(m_x, m_z, aaOcean::eCHOPZ);
        if(oceanData_->m_doFoam)
        {
            foamy = true;
            foam = mOcean_.getOceanData(m_x, m_z, aaOcean::eFOAM);
            eigenminus[0] = mOcean_.getOceanData(m_x, m_z, aaOcean::eEIGENMINUSX);
            eigenminus[2] = mOcean_.getOceanData(m_x, m_z, aaOcean::eEIGENMINUSZ);
            eigenplus[0] = mOcean_.getOceanData(m_x, m_z, aaOcean::eEIGENPLUSX);
            eigenplus[2] = mOcean_.getOceanData(m_x, m_z, aaOcean::eEIGENPLUSZ);
        }
    } else {
        result[0] = 0.0f;
        result[2] = 0.0f;
    }
    if (foamy == false)
    {
        foam = 0.0f;
        eigenminus[0] = eigenminus[1] = eigenminus[2] = 0.0f;
        eigenplus[0] = eigenplus[1] = eigenplus[2] = 0.0f;
    }
    
    if (tone)
    {
        LXx_VSCL(result, -1.0f);
    }

    chanMod.WriteOutputFloat (attr, cm_idx_displacementX, result[0]); // vector, normalize to 0-1, 0.5 is no displacement
    chanMod.WriteOutputFloat (attr, cm_idx_displacementY, result[1]); // vector, normalize to 0-1, 0.5 is no displacement
    chanMod.WriteOutputFloat (attr, cm_idx_displacementZ, result[2]); // vector, normalize to 0-1, 0.5 is no displacement
    chanMod.WriteOutputFloat (attr, cm_idx_foam, foam);
    chanMod.WriteOutputFloat (attr, cm_idx_eigenminusX, eigenminus[0]); // vector
    chanMod.WriteOutputFloat (attr, cm_idx_eigenminusY, eigenminus[1]); // vector
    chanMod.WriteOutputFloat (attr, cm_idx_eigenminusZ, eigenminus[2]); // vector
    chanMod.WriteOutputFloat (attr, cm_idx_eigenplusX, eigenplus[0]); // vector
    chanMod.WriteOutputFloat (attr, cm_idx_eigenplusY, eigenplus[1]); // vector
    chanMod.WriteOutputFloat (attr, cm_idx_eigenplusZ, eigenplus[2]); // vector
    
    return LXe_OK;
}

void aaOceanChanMod::maybeResetOceanData(std::unique_ptr<OceanData> newOceanData) {
    if (oceanData_.get() == nullptr || *newOceanData != *oceanData_) {
        // Automatically unlocks the mutex when it goes out of scope.
        std::lock_guard<std::mutex> lock(myMutex_);
        // Check it again inside the mutex because another thread may have overridden it.
        if (oceanData_.get() == nullptr || *newOceanData != *oceanData_) {
            oceanData_ = std::move(newOceanData); // newOceanData is no longer valid, do not use it again in this function!
            mOcean_.input(	oceanData_->m_resolution,
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
// Package class

LXtTagInfoDesc	 aaOceanChanModPackage::descInfo[] = {
        { LXsPKG_SUPERTYPE,	"chanModify"	},
        { LXsSRV_LOGSUBSYSTEM,	"aaOceanChanMod"	},
        { 0 }
};

// Constructor

aaOceanChanModPackage::aaOceanChanModPackage ()
{
        chanmod_factory.AddInterface (new CLxIfc_PackageInstance<aaOceanChanMod>);
        chanmod_factory.AddInterface (new CLxIfc_ChannelModItem<aaOceanChanMod>);
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

        LxResult
aaOceanChanModPackage::pkg_SetupChannels (
        ILxUnknownID		 addChan)
{
        CLxUser_AddChannel	 ac (addChan);
        LXtVector            displacement, eigenplus, eigenminus;

        // Order here is not so important since we look up by name and set indices later. Yay for freedom!

        // Input channels below, this being defined in Flags and Allocate.

        ac.NewChannel ("inputX", LXsTYPE_FLOAT);
        ac.SetDefault (0.0, 0);

        ac.NewChannel ("inputZ", LXsTYPE_FLOAT);
        ac.SetDefault (0.0, 0);
    
        ac.NewChannel ("tone", LXsTYPE_BOOLEAN);
        ac.SetDefault (0.0, 1);

        ac.NewChannel  ("resolution",	LXsTYPE_INTEGER);
        ac.SetDefault  (0.0, 4);
        ac.SetHint(hint_resolution);
        
        ac.NewChannel  ("oceanSize",			LXsTYPE_FLOAT);
        ac.SetDefault  (100.0f, 0);
        
        ac.NewChannel  ("waveHeight",	LXsTYPE_FLOAT);
        ac.SetDefault  (1.0f, 0);
    
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
    
        // Output channels below, this being defined in Flags and Allocate.

        // Note that these vectors end up having three channels (.X, .Y, .Z) elsewhere, leading to checks for displacement.X, etc. Don't get confused.
        ac.NewChannel ("displacement", LXsTYPE_FLOAT);
        ac.SetVector(LXsCHANVEC_XYZ);
        LXx_VCLR (displacement);
        ac.SetDefaultVec (displacement);

        ac.NewChannel ("foam", LXsTYPE_FLOAT);
        ac.SetDefault (0.0f, 0);

        // Note that these vectors end up having three channels (.X, .Y, .Z) elsewhere, leading to checks for displacement.X, etc. Don't get confused.
        ac.NewChannel ("Eigenplus", LXsTYPE_FLOAT);
        ac.SetVector(LXsCHANVEC_XYZ);
        LXx_VCLR (eigenplus);
        ac.SetDefaultVec (eigenplus);

        ac.NewChannel ("Eigenminus", LXsTYPE_FLOAT);
        ac.SetVector(LXsCHANVEC_XYZ);
        LXx_VCLR (eigenminus);
        ac.SetDefaultVec (eigenminus);

        return LXe_OK;
}

        LxResult
aaOceanChanModPackage::pkg_TestInterface (
        const LXtGUID		*guid)
{
        return (chanmod_factory.TestInterface (guid) ? LXe_TRUE : LXe_FALSE);
}

        LxResult
aaOceanChanModPackage::pkg_Attach (
        void		       **ppvObj)
{
        aaOceanChanMod	*chanmod = chanmod_factory.Alloc (ppvObj);

        chanmod->src_pkg  = this;
        chanmod->inst_ifc = (ILxUnknownID) ppvObj[0];

        return LXe_OK;
}
