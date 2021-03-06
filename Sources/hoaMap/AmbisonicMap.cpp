/**
 * HoaLibrary : A High Order Ambisonics Library
 * Copyright (c) 2012-2013 Julien Colafrancesco, Pierre Guillot, Eliott Paris, CICM, Universite Paris-8.
 * All rights reserved.
 *
 * Website  : http://www.mshparisnord.fr/hoalibrary/
 * Contacts : cicm.mshparisnord@gmail.com
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions may not be sold, nor may they be used in a commercial product or activity.
 *  - Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *  - Neither the name of the CICM nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "AmbisonicMap.h"

AmbisonicMap::AmbisonicMap(long anOrder, long aRampSample, long aVectorSize, long aSamplingRate) : Ambisonic(anOrder, aVectorSize, aSamplingRate)
{
	m_number_of_inputs		= 1;

    m_encoder   = new AmbisonicEncoder(m_order);
    m_wider     = new AmbisonicWider(m_order);
    m_line_one  = new CicmLine(aRampSample);
    m_line_two  = new CicmLine(aRampSample);
    m_line_thr  = new CicmLine(aRampSample);
    m_line_fou  = new CicmLine(aRampSample);
    
    cicm_malloc_vec_f(m_harmonics_float, m_number_of_harmonics);
    cicm_malloc_vec_d(m_harmonics_double, m_number_of_harmonics);
    
    m_harmonics_matrix_float  = new float*[m_number_of_harmonics];
    m_harmonics_matrix_double = new double*[m_number_of_harmonics];
    for (int i = 0; i < m_number_of_harmonics; i++)
    {
        cicm_malloc_vec_f(m_harmonics_matrix_float[i], 1);
        cicm_malloc_vec_d(m_harmonics_matrix_double[i], 1);
    }
    
    m_vector_float = NULL;
    m_vector_double = NULL;
    
    cicm_malloc_vec_f(m_gains_float, 1);
    cicm_malloc_vec_f(m_radius_float, 1);
    cicm_malloc_vec_f(m_azimuth_float, 1);
    cicm_malloc_vec_f(m_abscissa_float, 1);
    cicm_malloc_vec_f(m_ordinate_float, 1);
    
    cicm_malloc_vec_d(m_gains_double, 1);
    cicm_malloc_vec_d(m_radius_double, 1);
    cicm_malloc_vec_d(m_azimuth_double, 1);
    cicm_malloc_vec_d(m_abscissa_double, 1);
    cicm_malloc_vec_d(m_ordinate_double, 1);
    
    setVectorSize(aVectorSize);
    
    m_line_one->setCoefficientDirect(1.);
    m_line_two->setCoefficientAngleDirect(0.);
    m_line_thr->setCoefficientDirect(0.);
    m_line_fou->setCoefficientDirect(1.);
    setCoordinatesCartesianLine(0., 1.);
}


void AmbisonicMap::setVectorSize(long aVectorSize)
{
    m_vector_size = Tools::clip_power_of_two(aVectorSize);
    m_encoder->setVectorSize(m_vector_size);
    m_wider->setVectorSize(m_vector_size);
    m_line_one->setVectorSize(m_vector_size);
    m_line_two->setVectorSize(m_vector_size);
    m_line_thr->setVectorSize(m_vector_size);
    m_line_fou->setVectorSize(m_vector_size);
    
    cicm_free(m_gains_double);
    cicm_free(m_gains_float);
    cicm_free(m_radius_float);
    cicm_free(m_radius_double);
    cicm_free(m_azimuth_float);
    cicm_free(m_azimuth_double);
    cicm_free(m_abscissa_double);
    cicm_free(m_abscissa_float);
    cicm_free(m_ordinate_float);
    cicm_free(m_ordinate_double);
    cicm_malloc_vec_f(m_gains_float, m_vector_size);
    cicm_malloc_vec_f(m_radius_float, m_vector_size);
    cicm_malloc_vec_f(m_azimuth_float, m_vector_size);
    cicm_malloc_vec_d(m_gains_double, m_vector_size);
    cicm_malloc_vec_d(m_radius_double, m_vector_size);
    cicm_malloc_vec_d(m_azimuth_double, m_vector_size);
    cicm_malloc_vec_d(m_abscissa_double, m_vector_size);
    cicm_malloc_vec_f(m_abscissa_float, m_vector_size);
    cicm_malloc_vec_d(m_ordinate_double, m_vector_size);
    cicm_malloc_vec_f(m_ordinate_float, m_vector_size);
    for (int i = 0; i < m_number_of_harmonics; i++)
    {
        cicm_free(m_harmonics_matrix_float[i]);
        cicm_free(m_harmonics_matrix_double[i]);
        cicm_malloc_vec_f(m_harmonics_matrix_float[i], m_vector_size);
        cicm_malloc_vec_d(m_harmonics_matrix_double[i], m_vector_size);
    }
    
    if(m_vector_double)
		cicm_free(m_vector_double);
	if(m_vector_float)
		cicm_free(m_vector_float);
	cicm_malloc_vec_f(m_vector_float, m_vector_size);
	cicm_malloc_vec_d(m_vector_double, m_vector_size);
}

void AmbisonicMap::setSamplingRate(long aSamplingRate)
{
    m_sampling_rate = Tools::clip_min(aSamplingRate, long(0));
}

void AmbisonicMap::setCoordinatesPolar(double aRadius, double anAzimuth)
{
    m_radius = Tools::clip_min(aRadius, 0.);
    if(m_radius <= 1.)
    {
        m_gain = 1.;
        m_wide = m_radius;
    }
    else
    {
        m_gain = 1. / (m_radius * m_radius);
        m_wide = 1.;
    }
   
    m_azimuth = anAzimuth;
    
}

void AmbisonicMap::setCoordinatesRadius(double aRadius)
{
    setCoordinatesPolar(m_azimuth, aRadius);
}

void AmbisonicMap::setCoordinatesAzimuth(double anAzimuth)
{
    setCoordinatesPolar(anAzimuth, m_radius);
}

void AmbisonicMap::setCoordinatesCartesian(double anAbscissa, double anOrdinate)
{
    m_abscissa = anAbscissa;
    m_ordinate = anOrdinate;
    setCoordinatesPolar(Tools::radius(m_abscissa, m_ordinate), Tools::angle(m_abscissa, m_ordinate) - CICM_PI2);
}

void AmbisonicMap::setCoordinatesAbscissa(double anAbscissa)
{
    m_abscissa = anAbscissa;
    setCoordinatesPolar(Tools::radius(m_abscissa, m_ordinate), Tools::angle(m_abscissa, m_ordinate) - CICM_PI2);
}

void AmbisonicMap::setCoordinatesOrdinate(double anOrdinate)
{
    m_ordinate = anOrdinate;
    setCoordinatesPolar(Tools::radius(m_abscissa, m_ordinate), Tools::angle(m_abscissa, m_ordinate) - CICM_PI2);
}

void AmbisonicMap::setCoordinatesPolarLine(double aRadius, double anAzimuth)
{
    aRadius = Tools::clip_min(aRadius, 0.);
    m_line_one->setCoefficient(aRadius);
    m_line_two->setCoefficientAngle(anAzimuth);
}

void AmbisonicMap::setCoordinatesRadiusLine(double aRadius)
{
    aRadius = Tools::clip_min(aRadius, 0.);
    m_line_one->setCoefficient(aRadius);
}

void AmbisonicMap::setCoordinatesAzimuthLine(double anAzimuth)
{
    m_line_two->setCoefficientAngle(anAzimuth);
}

void AmbisonicMap::setCoordinatesCartesianLine(double anAbscissa, double anOrdinate)
{
    m_line_thr->setCoefficient(anAbscissa);
    m_line_fou->setCoefficient(anOrdinate);
    m_abscissa = anAbscissa;
    m_ordinate = anOrdinate;
    setCoordinatesPolarLine(Tools::radius(m_abscissa, m_ordinate), Tools::angle(m_abscissa, m_ordinate) - CICM_PI2);
}

void AmbisonicMap::setCoordinatesAbscissaLine(double anAbscissa)
{
    m_line_thr->setCoefficient(anAbscissa);
    m_abscissa = anAbscissa;
    setCoordinatesPolarLine(Tools::radius(m_abscissa, m_ordinate), Tools::angle(m_abscissa, m_ordinate) - CICM_PI2);
}

void AmbisonicMap::setCoordinatesOrdinateLine(double anOrdinate)
{
    m_line_fou->setCoefficient(anOrdinate);
    m_ordinate = anOrdinate;
    setCoordinatesPolarLine(Tools::radius(m_abscissa, m_ordinate), Tools::angle(m_abscissa, m_ordinate) - CICM_PI2);
}


void AmbisonicMap::setRamp(long aNumberOfSample)
{
    m_line_one->setRampInSample(aNumberOfSample);
    m_line_two->setRampInSample(aNumberOfSample);
}

long AmbisonicMap::getRamp()
{
    return m_line_one->getRampInSample();
}

AmbisonicMap::~AmbisonicMap()
{
    delete m_encoder;
    delete m_wider;
    delete m_line_one;
    delete m_line_two;
    delete m_line_thr;
    delete m_line_fou;
    
    cicm_free(m_abscissa_double);
    cicm_free(m_abscissa_float);
    cicm_free(m_ordinate_float);
    cicm_free(m_ordinate_double);
    cicm_free(m_gains_float);
    cicm_free(m_gains_double);
    cicm_free(m_harmonics_float);
    cicm_free(m_harmonics_double);
    cicm_free(m_radius_float);
    cicm_free(m_radius_double);
    cicm_free(m_azimuth_float);
    cicm_free(m_azimuth_double);
    for (int i = 0; i < m_number_of_harmonics; i++)
    {
        cicm_free(m_harmonics_matrix_float[i]);
        cicm_free(m_harmonics_matrix_double[i]);
    }
    cicm_free(m_harmonics_matrix_float);
    cicm_free(m_harmonics_matrix_double);
}

