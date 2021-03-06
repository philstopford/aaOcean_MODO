// aaOcean v2.5
// Author: Amaan Akram 
// www.amaanakram.com
// aaOcean is free software and can be redistributed and modified under the terms of the 
// GNU General Public License (Version 3) as provided by the Free Software Foundation.
// GNU General Public License http://www.gnu.org/licenses/gpl.html

#ifndef AAOCEANCLASS_CPP
#define AAOCEANCLASS_CPP

// Test for gcc
// gcc 4.2.0+ required for OpenMP
// 4.2.0 is ABI-compatible with 4.1.x.
// recommended compiler: gcc-4.2.4
#ifdef __GNUC__
    #if __GNUC_MINOR__ < 2 
        #warning [aaOcean] GNU C compiler 4.2.x, which is ABI-compatible with 4.1.0, is required
    #endif
#endif

#include <cmath>
#include <omp.h>
#include <climits>
#include <float.h>
#include <string.h>
#include "constants.h"
#include "functionLib.h"
#include "agnerFog/sfmt.cpp" 
#include "agnerFog/stocc.h"
#include "agnerFog/stoc1.cpp"
#include "agnerFog/userintf.cpp"
#include "aaOceanClass.h"

aaOcean::aaOcean() :
    // input variables
    m_resolution(-1),
    m_seed(1),
    m_windAlign(0),         
    m_velocity(15.0f),
    m_windDir(45.0f),
    m_cutoff(-1.0f),
    m_damp(0.985f),
    m_oceanScale(100.0f),
    m_oceanDepth(10000.f),
    m_surfaceTension(0.0f),
    m_chopAmount(1.0f),
    m_waveHeight(1.0f),
    m_waveSpeed(1.0f),
    m_time(1.0f),
    m_loopTime(10000.0f),
    m_foamBoundmax(1000.0f),
    m_foamBoundrange(1000.0f),

    // working arrays
    m_xCoord(0),
    m_zCoord(0),
    m_hokReal(0),
    m_hokImag(0),
    m_hktReal(0),
    m_hktImag(0),
    m_kX(0),
    m_kZ(0),
    m_omega(0),
    m_rand1(0),
    m_rand2(0),

    // bools to check ocean state
    m_isAllocated(0),
    m_isFoamAllocated(0),
    m_isNormalAllocated(0),
    m_doHoK(0),
    m_doSetup(0),
    m_doChop(0),
    m_doFoam(0),
    m_doNormals(0),

    // memory tracking
    m_memory(0),
    
    // fftw arrays
    m_fft_htField(0),
    m_fft_chopX(0),
    m_fft_chopZ(0),
    m_fft_jxx(0),
    m_fft_jzz(0),
    m_fft_jxz(0),
    m_normalsXY(0),
    m_normalsZ(0)
{
    strcpy (m_state, "");
}

aaOcean::aaOcean(const aaOcean &cpy)
{
    // empty copy constructor
    input(  cpy.m_resolution,
            cpy.m_seed,
            cpy.m_oceanScale,
            cpy.m_oceanDepth,
            cpy.m_surfaceTension,
            cpy.m_velocity,
            cpy.m_cutoff,
            cpy.m_windDir,
            cpy.m_windAlign,
            cpy.m_damp,
            cpy.m_waveSpeed,
            cpy.m_waveHeight,
            cpy.m_chopAmount,
            cpy.m_time,
            cpy.m_loopTime,
            cpy.m_doFoam,
            TRUE);
}

aaOcean::~aaOcean()
{
    clearArrays();
}

bool aaOcean::isChoppy()
{
    return m_doChop;
}

char* aaOcean::getState()
{
    return &m_state[0];
}

int aaOcean::getResolution()
{
    return m_resolution;
}

void aaOcean::input(int resolution, unsigned int seed, float oceanScale, float oceanDepth, float surfaceTension,
                    float velocity, float cutoff, float windDir, int windAlign, float damp, float waveSpeed, 
                    float waveHeight, float chopAmount, float time, float loopTime, bool doFoam, bool doNormals)
{
    // forcing to be power of two, setting minimum resolution of 2^4
    resolution  = (int)pow(2.0f, (4 + abs(resolution))); 
    reInit(resolution, seed);

    // scaled for better UI control
    m_waveHeight    = waveHeight * 0.01f;
    m_time          = time;
    m_waveSpeed     = waveSpeed;
    m_doFoam        = doFoam;

    if(chopAmount > 1.0e-6f)
    {
        // scaled for better UI control
        m_chopAmount = chopAmount * 0.01f;
        m_doChop = 1;
    }
    else
    {
        m_doChop = 0;
        m_chopAmount = 0.0f;
    }

    // clamping to minimum value
    oceanScale  = maximum<float>(oceanScale, 1.0e-6f);  
    velocity    = maximum<float>(velocity,   1.0e-6f); 
    oceanDepth  = maximum<float>(oceanDepth, 1.0e-6f); 
    // scaling for better UI control
    cutoff      = fabs(cutoff * 0.01f);
    // to radians
    windDir     = windDir * aa_PIBY180; 
    // forcing to even numbers
    windAlign   = maximum<int>(((windAlign + 1) * 2), 2); 
    // clamping to a maximum value of 1
    damp        = minimum<float>(damp, 1.0f); 

    if( m_oceanScale        != oceanScale       ||
        m_oceanDepth        != oceanDepth       ||
        m_surfaceTension    != surfaceTension   || 
        m_windDir           != windDir          ||
        m_cutoff            != cutoff           ||
        m_velocity          != velocity         ||
        m_windAlign         != windAlign        ||
        m_damp              != damp             ||
        m_loopTime          != loopTime)
    {
        m_oceanScale        = oceanScale;
        m_oceanDepth        = oceanDepth;
        m_surfaceTension    = surfaceTension;
        m_windDir           = windDir;
        m_cutoff            = cutoff;
        m_velocity          = velocity;
        m_windAlign         = windAlign;
        m_damp              = damp;
        m_loopTime          = loopTime;

        // re-evaluate HoK if any of these vars change
        m_doHoK = 1;
    }

    if(!m_doHoK || !m_doSetup)
        sprintf(m_state,"[aaOcean Core] Ocean base state unchanged. Re-evaluating ocean with cached data");
    
    // we have our inputs. start preparing ocean arrays
    prepareOcean();
}

void aaOcean::reInit(int resolution, int seed)
{
    // If ocean resolution has changed, or if we are creating an ocean from scratch
    // Flush any existing arrays and allocate them with the new array size (resolution)
    if(m_resolution != resolution || m_seed != seed)
    {
        m_seed = seed; // should be handled separately -- does not need mem reallocation
        m_resolution = resolution;
        allocateBaseArrays();           
        m_doHoK  = 1;
        m_doSetup = 1;
    }
}

void aaOcean::prepareOcean()
{
    if(m_doSetup)
        setupGrid();

    if(m_doHoK)
        evaluateHokData();

    evaluateHieghtField();

    if(m_doChop)
        evaluateChopField();

    if(m_doFoam)
    {
        if(!m_isFoamAllocated)
            allocateFoamArrays();
        evaluateJacobians();
    }
    sprintf(m_state,"%s\n[aaOcean Core] Working memory allocated: %.2f megabytes", m_state, float(m_memory)/1048576.f);
}

void aaOcean::allocateBaseArrays()
{
    if(m_isAllocated) 
    {
        sprintf(m_state,"[aaOcean Core] Reallocating memory for ocean data structures for resolution %dx%d", m_resolution, m_resolution);
        clearArrays();
    }
    else
        sprintf(m_state,"[aaOcean Core] Allocating memory for ocean data structures for resolution %dx%d", m_resolution, m_resolution);

    int size = m_resolution * m_resolution;
    m_memory = size * sizeof(int) * 2 + size * sizeof(float) * 9 + size * sizeof(fftwf_complex) * 3;

    m_xCoord    = (int*)   aligned_malloc(size * sizeof(int)); 
    m_zCoord    = (int*)   aligned_malloc(size * sizeof(int)); 
    
    m_hokReal   = (float*) aligned_malloc(size * sizeof(float)); 
    m_hokImag   = (float*) aligned_malloc(size * sizeof(float)); 
    m_hktReal   = (float*) aligned_malloc(size * sizeof(float)); 
    m_hktImag   = (float*) aligned_malloc(size * sizeof(float)); 
    m_kX        = (float*) aligned_malloc(size * sizeof(float)); 
    m_kZ        = (float*) aligned_malloc(size * sizeof(float)); 
    m_omega     = (float*) aligned_malloc(size * sizeof(float)); 
    m_rand1     = (float*) aligned_malloc(size * sizeof(float)); 
    m_rand2     = (float*) aligned_malloc(size * sizeof(float)); 
    
    if(m_resolution > 254)
    {
        int threads = omp_get_num_procs();
        fftwf_init_threads();
        fftwf_plan_with_nthreads(threads);
    }
    else
        fftwf_plan_with_nthreads(1);

    m_fft_htField = (fftwf_complex*) fftwf_malloc(size * sizeof(fftwf_complex));
    m_fft_chopX   = (fftwf_complex*) fftwf_malloc(size * sizeof(fftwf_complex));  
    m_fft_chopZ   = (fftwf_complex*) fftwf_malloc(size * sizeof(fftwf_complex)); 

    m_planHeightField   = fftwf_plan_dft_2d(m_resolution, m_resolution, m_fft_htField, m_fft_htField, 1, FFTW_ESTIMATE);
    m_planChopX         = fftwf_plan_dft_2d(m_resolution, m_resolution, m_fft_chopX, m_fft_chopX ,1, FFTW_ESTIMATE);
    m_planChopZ         = fftwf_plan_dft_2d(m_resolution, m_resolution, m_fft_chopZ, m_fft_chopZ ,1, FFTW_ESTIMATE);
    m_isAllocated       = TRUE;
}

 void aaOcean::allocateFoamArrays()
{
    int size = m_resolution * m_resolution;
    m_memory += size * sizeof(fftwf_complex) * 3;

    m_fft_jxx   = (fftwf_complex*) fftwf_malloc(size * sizeof(fftwf_complex));
    m_fft_jzz   = (fftwf_complex*) fftwf_malloc(size * sizeof(fftwf_complex)); 
    m_fft_jxz   = (fftwf_complex*) fftwf_malloc(size * sizeof(fftwf_complex)); 

    m_planJxx = fftwf_plan_dft_2d(m_resolution, m_resolution, m_fft_jxx, m_fft_jxx, 1, FFTW_ESTIMATE);
    m_planJxz = fftwf_plan_dft_2d(m_resolution, m_resolution, m_fft_jxz, m_fft_jxz, 1, FFTW_ESTIMATE);
    m_planJzz = fftwf_plan_dft_2d(m_resolution, m_resolution, m_fft_jzz, m_fft_jzz, 1, FFTW_ESTIMATE);
    m_isFoamAllocated = TRUE;
}

void aaOcean::allocateNormalArrays()
{
    int size = m_resolution * m_resolution;
    m_memory += size * sizeof(fftwf_complex) * 2;

    m_normalsXY = (fftwf_complex*) fftwf_malloc(size * sizeof(fftwf_complex));
    m_normalsZ  = (fftwf_complex*) fftwf_malloc(size * sizeof(fftwf_complex));

    m_isNormalAllocated = TRUE;
}

void aaOcean::clearResidualArrays()
{
    bool isResidualAllocated = TRUE;

    if(m_rand2)
    {
        aligned_free(m_rand2); 
        m_rand2 = FALSE;
    }
    else
        isResidualAllocated = FALSE;

    if(m_rand1)
    {
        aligned_free(m_rand1); 
        m_rand1 = FALSE;
    }
    if(m_omega)
    {
        aligned_free(m_omega); 
        m_omega = FALSE;
    }
    if(m_kZ)
    {
        aligned_free(m_kZ); 
        m_kZ = FALSE;
    }
    if(m_kX)
    {
        aligned_free(m_kX); 
        m_kX = FALSE;
    }
    if(m_hktImag)
    {
        aligned_free(m_hktImag); 
        m_hktImag = FALSE;
    }
    if(m_hktReal)
    {
        aligned_free(m_hktReal); 
        m_hktReal = FALSE;
    }
    if(m_hokImag)
    {
        aligned_free(m_hokImag); 
        m_hokImag = FALSE;
    }
    if(m_hokReal)
    {
        aligned_free(m_hokReal); 
        m_hokReal = FALSE;
    }
    if(m_zCoord)
    {
        aligned_free(m_zCoord); 
        m_zCoord = FALSE;
    }
    if(m_xCoord)
    {
        aligned_free(m_xCoord); 
        m_xCoord = FALSE;
    }

    if(isResidualAllocated)
    {
        int size = m_resolution * m_resolution;
        float cleared_memory = float(m_memory);
        m_memory = m_memory - (size * sizeof(float) * 9 + size * sizeof(int) * 2);
        cleared_memory = (cleared_memory - float(m_memory)) / 1048576.f;
        float working_memory = (float)m_memory/1048576.f;
        sprintf(m_state,"%s\n[aaOcean Core] Clearing %.2f megabytes of working memory. Current usage %.2f megabytes", \
            m_state, cleared_memory, working_memory);
    }
}

void aaOcean::clearArrays()
{
    if(m_isAllocated)
    {
        if(m_isNormalAllocated)
        {
            fftwf_free(m_normalsXY);
            fftwf_free(m_normalsZ);
            m_normalsXY = m_normalsZ = FALSE;
            m_isNormalAllocated = FALSE;
        }
        if(m_isFoamAllocated)
        {
            if(m_fft_jxx)
            {
                fftwf_destroy_plan(m_planJxx);
                fftwf_free(m_fft_jxx); 
                m_fft_jxx = FALSE;
            }
            if(m_fft_jzz)
            {
                fftwf_destroy_plan(m_planJzz);
                fftwf_free(m_fft_jzz);  
                m_fft_jzz = FALSE;
            }
            if(m_fft_jxz)
            {
                fftwf_destroy_plan(m_planJxz);
                fftwf_free(m_fft_jxz); 
                m_fft_jxz = FALSE;
            }
            m_isFoamAllocated = FALSE;
        }
        if(m_fft_chopZ)
        {
            fftwf_destroy_plan(m_planChopZ);
            fftwf_free(m_fft_chopZ); 
            m_fft_chopZ = FALSE;
        }
        if(m_fft_chopX)
        {
            fftwf_destroy_plan(m_planChopX);
            fftwf_free(m_fft_chopX); 
            m_fft_chopX = FALSE;
        }
        if(m_fft_htField)
        {
            fftwf_destroy_plan(m_planHeightField);
            fftwf_free(m_fft_htField); 
            m_fft_htField = FALSE;
        }
        m_isAllocated = FALSE;
    }
    
    clearResidualArrays();
    // call fftw cleanup routines
    if(m_resolution > 254)
        fftwf_cleanup_threads();
    fftwf_cleanup();
}

unsigned int aaOcean::generateUID(float xCoord, float zCoord)
{
    // a very simple hash function. should probably do a better one at some point
    float angle;
    float length;
    float coordSq;
    float id_out;
    unsigned int returnVal = 1;

    if (zCoord != 0.0f && xCoord != 0.0f)
    {
        coordSq = zCoord * zCoord + xCoord * xCoord;
        length = sqrt(coordSq);
        angle = xCoord / length; 
        angle = acos(angle);
        angle = RadsToDegs(angle);
        
        if (angle > 180.0f)
            angle = 360.0f - angle;
        
        id_out = coordSq + (length * angle) + 0.5f;
        
        if(angle == 0.0f)
            returnVal = (unsigned int)coordSq;
        else if (zCoord <= 0.0f)
            returnVal = (unsigned int)floor(id_out);
        else
            returnVal = INT_MAX - (unsigned int)floor(id_out) ;
    }
    return returnVal;
}

void aaOcean::setupGrid()
{
    if(!m_isAllocated)
        return;
    const int n = m_resolution;
    const int half_n = (-n / 2) - ((n-1) / 2);
    unsigned int index, uID; 

    #pragma omp parallel for private(index, uID)
    for(int i = 0; i < n; ++i)
    {
        for(int j = 0; j < n; ++j)
        {
            index = (i * n) + j;

            m_xCoord[index] = half_n + i * 2 ;
            m_zCoord[index] = half_n + j * 2 ;

            uID = (unsigned int)generateUID((float)m_xCoord[index], (float)m_zCoord[index]);

            StochasticLib1 sto(uID + (unsigned int)m_seed);
            m_rand1[index] = (float)sto.Normal(0.0, 1.0);
            m_rand2[index] = (float)sto.Normal(0.0, 1.0);
        }
    }
    m_doSetup = FALSE;
}

 void aaOcean::evaluateHokData()
{
    float k_sq, k_mag, k_dot_w, philips, x, z;

    const int      n        = m_resolution * m_resolution;
    const float    k_mult   = aa_TWOPI / m_oceanScale;
    const float    L        = m_velocity * m_velocity / aa_GRAVITY;
    const float    L_sq     = L * L;
    const float    windx    = cos(m_windDir);
    const float    windz    = sin(m_windDir);
    const float    omega0   = aa_TWOPI / m_loopTime;
    
    bool bDamp  = FALSE;
    if (m_damp > 0.0f)
        bDamp = true;

    #pragma omp parallel for private( k_sq, k_mag, k_dot_w, philips, x, z)  
    for(int index = 0; index < n; ++index)
    {
        // build Kx and Kz
        x = m_kX[index] =  m_xCoord[index] * k_mult; 
        z = m_kZ[index] =  m_zCoord[index] * k_mult;

        // philips spectrum vars
        k_sq        = (x * x) + (z * z);
        k_mag       = 1.0f / sqrt( k_sq );
        k_dot_w     = (x * k_mag) * windx + (z * k_mag) * windz;

        // calculate philips spectrum
        philips     = sqrt((( exp(-1.0f / ( L_sq * k_sq)) * pow(k_dot_w, m_windAlign)) / 
                      (k_sq * k_sq)) * exp(-k_sq * m_cutoff));

        // reduce reflected waves
        if(bDamp)
        {
            if(k_dot_w < 0.0f)
                philips *= (1.0f - m_damp);
        }       

        // build dispersion relationship with oceanDepth relationship
        m_omega[index]   = (aa_GRAVITY / k_mag) * tanh( sqrt( k_sq ) * m_oceanDepth);

        // modifying dispersion for capillary waves
        m_omega[index] = m_omega[index] * (1.0f + k_sq * m_surfaceTension * m_surfaceTension);

        m_omega[index] = sqrt(m_omega[index]);

        // add time looping support with OmegaNought
        m_omega[index]   = (int(m_omega[index] / omega0)) * omega0;

        m_hokReal[index] = (aa_INV_SQRTTWO) * (m_rand1[index]) * philips;
        m_hokImag[index] = (aa_INV_SQRTTWO) * (m_rand2[index]) * philips;
    }

    sprintf(m_state,"%s\n[aaOcean Core] Finished initializing all ocean data", m_state);
    m_doHoK = FALSE;
}

 void aaOcean::evaluateHieghtField()
{
    int  i,j,index, index_rev;
    float hokReal, hokImag, hokRealOpp, hokImagOpp, sinwt, coswt;

    const float wt  = m_waveSpeed * m_time;
    const int n = m_resolution;
    const int nn = n * n;
    const int n_sq = n * n - 1;
    const float signs[2] = { 1.0f, -1.0f };

    #pragma omp parallel for private(index, index_rev, hokReal, hokImag, hokRealOpp, hokImagOpp, sinwt, coswt)  
    for(index = 0; index < nn; ++index)
    {
        index_rev   = n_sq - index; //tail end 
        hokReal     = m_hokReal[index];
        hokImag     = m_hokImag[index];
        hokRealOpp  = m_hokReal[index_rev];
        hokImagOpp  = m_hokImag[index_rev];

        coswt = cos( m_omega[index] * wt);
        sinwt = sin( m_omega[index] * wt);

        m_hktReal[index]  = ( hokReal    *  coswt )  +  ( hokImag    *  sinwt )  + 
                            ( hokRealOpp *  coswt )  -  ( hokImagOpp *  sinwt )  ;  //complex conjugage
                
        m_hktImag[index]  = (-hokReal    *  sinwt )  +  ( hokImag    *  coswt )  +
                            ( hokRealOpp *  sinwt )  +  ( hokImagOpp *  coswt )  ;  //complex conjugage
        
        m_fft_htField[index][0] = m_hktReal[index];
        m_fft_htField[index][1] = m_hktImag[index];
    }

    fftwf_execute(m_planHeightField);

    for(i = 0; i < n; ++i)
    {
        #pragma omp parallel for private(j)
        for(j = 0; j < n; ++j)
            m_fft_htField[(i*n) + j][0] *= signs[(i + j) & 1]  * m_waveHeight;
    }
}

 void aaOcean::evaluateChopField()
{
    int  i, j, index;
    float  kX, kZ, kMag;
    int n = m_resolution * m_resolution;
    const float signs[2] = { 1.0f, -1.0f };
    float multiplier;

    #pragma omp parallel for private( index,  kX,  kZ, kMag)  
    for(index = 0; index < n; ++index)
    {           
        kMag = sqrt(m_kX[index] * m_kX[index] + m_kZ[index] * m_kZ[index]);
        kX = m_kX[index] / kMag;
        kZ = m_kZ[index] / kMag;
        
        m_fft_chopX[index][0] =  m_hktImag[index] * kX;
        m_fft_chopX[index][1] = -m_hktReal[index] * kX;

        m_fft_chopZ[index][0] =  m_hktImag[index] * kZ;
        m_fft_chopZ[index][1] = -m_hktReal[index] * kZ;
    }

    fftwf_execute(m_planChopX);
    fftwf_execute(m_planChopZ);

    n = m_resolution;
    for(i = 0; i < n; ++i)
    {
        #pragma omp parallel for private(multiplier, j, index)  
        for(j = 0; j < n; ++j)
        {
            index = (i*n) + j;
            multiplier = m_chopAmount * signs[(i + j) & 1] * -1.0f;
            m_fft_chopX[index][0] *= multiplier;
            m_fft_chopZ[index][0] *= multiplier;
        }
    }
}

void aaOcean::evaluateJacobians()
{
    int  i, j, index;
    float kX, kZ, kMag, kXZ, multiplier;
    const float signs[2] = { 1.0f, -1.0f };
    int n = m_resolution * m_resolution;

    #pragma omp parallel for private( index, kX, kZ, kXZ, kMag) 
    for(index = 0; index < n; ++index)
    {           
        kMag = 1.0f / sqrt(m_kX[index] * m_kX[index] + m_kZ[index] * m_kZ[index]);
        kX   = (m_kX[index] * m_kX[index]) * kMag;
        kZ   = (m_kZ[index] * m_kZ[index]) * kMag;
        kXZ  = (m_kX[index] * m_kZ[index]) * kMag;

        m_fft_jxx[index][0] =  m_hktReal[index] * kX;
        m_fft_jxx[index][1] =  m_hktImag[index] * kX;

        m_fft_jzz[index][0] =  m_hktReal[index] * kZ;
        m_fft_jzz[index][1] =  m_hktImag[index] * kZ;

        m_fft_jxz[index][0] =  m_hktReal[index] * kXZ;
        m_fft_jxz[index][1] =  m_hktImag[index] * kXZ;
    }

    fftwf_execute(m_planJxx);
    fftwf_execute(m_planJzz);
    fftwf_execute(m_planJxz);

    n = m_resolution;
    for(i = 0; i < n; ++i)
    {
        #pragma omp parallel for private(multiplier, j, index)  
        for(j = 0; j < n; ++j)
        {
            index = (i*n) + j;
            multiplier = -m_chopAmount * signs[(i + j) & 1];
            m_fft_jxx[index][0] = m_fft_jxx[index][0] * multiplier + 1.0f;
            m_fft_jzz[index][0] = m_fft_jzz[index][0] * multiplier + 1.0f;
            m_fft_jxz[index][0] = m_fft_jxz[index][0] * multiplier;
        }
    }

    float jPlus, jMinus, qPlus, qMinus, Jxx, Jzz, Jxz, temp;
    #pragma omp parallel for private(index, jPlus, jMinus, qPlus, qMinus, Jxx, Jzz, Jxz, temp)  
    for(index = 0; index < n*n; ++index)
    {
        Jxx = m_fft_jxx[index][0];
        Jzz = m_fft_jzz[index][0];
        Jxz = m_fft_jxz[index][0];

        temp = (0.5f * sqrt( ((Jxx - Jzz) * (Jxx - Jzz)) + 4.0f * (Jxz*Jxz)));
        jPlus   = (0.5f * (Jxx + Jzz))  +  temp;
        jMinus  = (0.5f * (Jxx + Jzz))  -  temp;
        qPlus   = (jPlus  - Jxx) / Jxz;
        qMinus  = (jMinus - Jxx) / Jxz;

        temp = sqrt( 1.0f + qPlus * qPlus);
        m_fft_jxx[index][0] = 1.0f  / temp;
        m_fft_jxx[index][1] = qPlus / temp;

        temp = sqrt( 1.0f + qMinus * qMinus);
        m_fft_jzz[index][0] = 1.0f   / temp;
        m_fft_jzz[index][1] = qMinus / temp;

        //store foam back in this array for convenience
        m_fft_jxz[index][0] =   (Jxx * Jzz) - (Jxz * Jxz); 
    }
}

// removed until vector class is gcc-4.2.x compliant
/*
void aaOcean::evaluateNormal()
{
    int index;
    int n = m_resolution;

    const int halfRes = n/2;    

    #pragma omp parallel for private(index)
    for(int i = 0; i < n; ++i)
    {
        // position vectors to surrounding points
        vector3 vCurrent, vNorth, vSouth, vEast, vWest, norm1, norm2, norm3, norm4;
        int ii, jj, xCoord, zCoord;
        float cX, cZ;
        for(int j = 0; j < n; ++j)
        {
            xCoord = i - n;
            zCoord = n - (n - j) + 1;

            if(isChoppy())
            {
                ii = wrap(i+1);
                index = (ii * n) + j;;
                cX = m_fft_chopX[index][0];
                cZ = m_fft_chopZ[index][0];
            }
            else
                cX = cZ = 0.0f;

            ii = wrap(i+1);
            index = (ii * n) + j;
            vNorth.x = xCoord + cX;
            vNorth.y = m_fft_htField[index][0];
            vNorth.z = zCoord + 1 + cZ;

            if(isChoppy())
            {
                ii = wrap(i-1);
                index = (ii * n) + j;
                cX = m_fft_chopX[index][0];
                cZ = m_fft_chopZ[index][0];
            }
            else
                cX = cZ = 0.0f;

            vSouth.x = xCoord + cX;
            vSouth.y = m_fft_htField[index][0];
            vSouth.z = zCoord - 1 + cZ;

            if(isChoppy())
            {
                jj = wrap(j-1);
                index = (i * n) + jj;
                cX = m_fft_chopX[index][0];
                cZ = m_fft_chopZ[index][0];
            }
            else
                cX = cZ = 0.0f;
            
            vEast.x = xCoord - 1 + cX;
            vEast.y = m_fft_htField[index][0];
            vEast.z = zCoord + cZ;

            if(isChoppy())
            {
                jj = wrap(j+1);
                index = (i * n) + jj;
                cX = m_fft_chopX[index][0];
                cZ = m_fft_chopZ[index][0];
            }
            else
                cX = cZ = 0.0f;
            
            vWest.x = xCoord + cX;
            vWest.y = m_fft_htField[index][0];
            vWest.z = zCoord + 1 + cZ;

            index = (j * n) + i;
            
            if(isChoppy())
            {
                cX = m_fft_chopX[index][0];
                cZ = m_fft_chopZ[index][0];
            }
            vCurrent.x = xCoord - cX;
            vCurrent.y = m_fft_htField[index][0];
            vCurrent.z = zCoord - cZ;

            vNorth  = vNorth - vCurrent;
            vSouth  = vSouth - vCurrent;
            vEast   = vEast - vCurrent; 
            vWest   = vWest - vCurrent;

            norm1 = vEast.cross(vNorth);
            norm2 = vWest.cross(vSouth);
            norm3 = vSouth.cross(vEast);
            norm4 = vNorth.cross(vWest);

            vector3 normal = (norm1.normalize() + norm2.normalize() + norm3.normalize() + norm4.normalize()) * 0.25f;
            if(vCurrent.length()==0.0f)
                norm1.x = norm1.y = norm1.z = 0.f;
            else
                norm1 = norm1.normalize();

            m_normalsXY[index][0] = norm1.x;
            m_normalsXY[index][1] = norm1.y;
            m_normalsZ[index][0]  = norm1.z;
        }
    }
}
*/

// Terribly slow for non-trivial cases.
void aaOcean::getFoamBounds(float& outBoundsMin, float& outBoundsMax)
{
    outBoundsMax = 0.0f;
    outBoundsMin =  0.0f;

    int index, n;
    n = m_resolution * m_resolution;
    for(index = 0; index < n; index++)
    {
        if(outBoundsMin < m_fft_jxz[index][0])
            outBoundsMin = m_fft_jxz[index][0];

        if(outBoundsMax > m_fft_jxz[index][0])
            outBoundsMax = m_fft_jxz[index][0];
    }
}

void aaOcean::getOceanArray(float *&outArray, aaOcean::arrayType type)
{
    fftwf_complex *arrayPointer;
    int arrayIndex;
    getArrayType(type, arrayPointer, arrayIndex);
    const int arraySize = m_resolution * m_resolution;

    #pragma omp parallel for 
    for(int i = 0; i < arraySize; ++i)
    {
        outArray[i] = arrayPointer[i][arrayIndex];
    }
}

float aaOcean::getOceanData(float uCoord, float vCoord, aaOcean::arrayType type) const
{
    float u, v, du, dv = 0;
    int xMinus1, yMinus1, x, y, xPlus1, yPlus1, xPlus2, yPlus2;
    fftwf_complex *arrayPointer;
    int arrayIndex;

    // maya and softimage V axis runs along negative z axis
    // aaOcean uses convention of V axis along positive z axis
    vCoord = -vCoord;

    // begin UV coordinate wrapping to [0-1] interval
    uCoord = fmod(uCoord, 1.0f);
    vCoord = fmod(vCoord, 1.0f);
    if(uCoord < 0.0f)
        uCoord = 1.0f + uCoord;
    if(vCoord < 0.0f)
        vCoord =  1.0f + vCoord;

    // use UV coordinates to work out ocean array indeces
    u = uCoord * float(m_resolution);
    v = vCoord * float(m_resolution);
    x = (int)floor(u);
    y = (int)floor(v);
    du = u - x; 
    dv = v - y; 

    // prepare catmul-rom end points for interpolation
    // wrap any indices that are outside the array boundaries
    xMinus1 = wrap(x - 1) * m_resolution;
    xPlus1  = wrap(x + 1) * m_resolution;
    xPlus2  = wrap(x + 2) * m_resolution;
    x       = wrap(x)   * m_resolution;
    y       = wrap(y);
    yMinus1 = wrap(y - 1);
    yPlus1  = wrap(y + 1);
    yPlus2  = wrap(y + 2);
    
    // get the pointer to the aaOcean array that we want to pull data from
    getArrayType(type, arrayPointer, arrayIndex);

    // prepare for catmul-rom interpolation
    const float a1 = catmullRom(du, 
                            arrayPointer[xMinus1    + yMinus1][arrayIndex],
                            arrayPointer[x          + yMinus1][arrayIndex],
                            arrayPointer[xPlus1     + yMinus1][arrayIndex],
                            arrayPointer[xPlus2     + yMinus1][arrayIndex]);

    const float b1 = catmullRom(du, 
                            arrayPointer[xMinus1    +   y][arrayIndex],
                            arrayPointer[x          +   y][arrayIndex],
                            arrayPointer[xPlus1     +   y][arrayIndex],
                            arrayPointer[xPlus2     +   y][arrayIndex]);

    const float c1 = catmullRom(du, 
                            arrayPointer[xMinus1    + yPlus1][arrayIndex],
                            arrayPointer[x          + yPlus1][arrayIndex],
                            arrayPointer[xPlus1     + yPlus1][arrayIndex],
                            arrayPointer[xPlus2     + yPlus1][arrayIndex]);

    const float d1 = catmullRom(du, 
                            arrayPointer[xMinus1    + yPlus2][arrayIndex],
                            arrayPointer[x          + yPlus2][arrayIndex],
                            arrayPointer[xPlus1     + yPlus2][arrayIndex],
                            arrayPointer[xPlus2     + yPlus2][arrayIndex]);

    float returnValue = catmullRom(dv, a1, b1, c1, d1);
    
    if (type == eFOAM)
    {
        float maxValue = m_foamBoundmax;
        float minValue = maxValue - m_foamBoundrange;
        
        float range = m_foamBoundrange;
        if (range == 0)
        {
            range = 1.0f;
        }
        
        // Clamp.
        if (returnValue < minValue)
        {
            returnValue = minValue;
        }
        if (returnValue > maxValue)
        {
            returnValue = maxValue;
        }
        
        // Find our delta in the range.
        
        float delta = std::abs(returnValue - minValue);
        
        returnValue = std::abs(delta / range);
    }
    
    return returnValue;
}

inline float aaOcean::catmullRom(const float t, const float a, const float b, const float c, const float d) const
{
    return  0.5f * ( ( 2.0f * b ) + ( -a + c ) * t + 
            ( 2.0f * a - 5.0f * b + 4.0f * c - d ) * t * t + 
            ( -a + 3.0f * b - 3.0f * c + d )* t * t * t );
}

inline int aaOcean::wrap(int x) const
{
    // fast modulo for power of 2 numbers
    x = x & (m_resolution - 1);
    return x;
}

void aaOcean::getArrayType(aaOcean::arrayType type, fftwf_complex*& outArray, int &arrayIndex) const
{
    // set pointer to the array that we need to interpolate data from
    arrayIndex = 0;
    
    if(type == eHEIGHTFIELD)
        outArray = m_fft_htField;
    else if(type == eCHOPX)
        outArray = m_fft_chopX;
    else if(type == eCHOPZ)
        outArray = m_fft_chopZ;
    else if(type == eFOAM)
        outArray = m_fft_jxz;
    else if(type == eEIGENPLUSX)
        outArray = m_fft_jxx;
    else if(type == eEIGENPLUSZ)
    {
        outArray = m_fft_jxx;
        arrayIndex = 1;
    }
    else if(type == eEIGENMINUSX)
        outArray = m_fft_jzz;
    else if(type == eEIGENMINUSZ)
    {
        outArray = m_fft_jzz;
        arrayIndex = 1;
    }
    else if (type == eNORMALSX)
        outArray = m_normalsXY;
    else if (type == eNORMALSY)
    {
        outArray = m_normalsXY;
        arrayIndex = 1;
    }
    else if (type == eNORMALSZ)
        outArray = m_normalsZ;
}
#endif  /* AAOCEANCLASS_CPP */
