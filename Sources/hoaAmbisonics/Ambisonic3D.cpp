/**
 * HoaLibrary : A High Order Ambisonics Library
 * Copyright (c) 2012-2013 Julien Colafrancesco, Pierre Guillot, Eliott Paris, CICM, Universite Paris-8.
 * All rights reserved.re Guillot, CICM - Université Paris 8
 * All rights reserved.
 *
 * Website  : http://www.mshparisnord.fr/HoaLibrary/
 * Contacts : cicm.mshparisnord@gmail.com
 *
 * This file is part of HOA LIBRARY.
 *
 * HOA LIBRARY is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "Ambisonic3D.h"

Ambisonic3D::Ambisonic3D(long anOrder, long aVectorSize, long aSamplingRate)
{
	m_order					= Tools::clip_min(anOrder, (long)1);
	m_number_of_harmonics	= (m_order + 1) * (m_order + 1);
	m_number_of_inputs		= m_number_of_harmonics;
	m_number_of_outputs		= m_number_of_harmonics;
	
    m_harmonics_indices     = new long[m_number_of_harmonics];
    m_harmonics_orders      = new long[m_number_of_harmonics];
    for(int i = 0; i < m_number_of_harmonics; i++)
    {
        m_harmonics_orders[i] = sqrtf((float)i);
        int index = i - (m_harmonics_orders[i]) * (m_harmonics_orders[i]);
      
        int index2 = (index - 1) / 2. + 1.;
        if (index % 2 == 1)
            index2 = -index2;
        m_harmonics_indices[i] = index2;
    }
    
	setVectorSize(aVectorSize);
    setSamplingRate(aSamplingRate);
}

long Ambisonic3D::getOrder()
{
	return m_order;
}

long Ambisonic3D::getNumberOfHarmonics()
{
	return m_number_of_harmonics;
}

long Ambisonic3D::getNumberOfInputs()
{
	return m_number_of_inputs;
}

long Ambisonic3D::getNumberOfOutputs()
{
	return m_number_of_outputs;
}

long Ambisonic3D::getVectorSize()
{
	return m_vector_size;
}

long Ambisonic3D::getSamplingRate()
{
	return m_sampling_rate;
}

long Ambisonic3D::getHarmonicIndex(long anIndex)
{
    if(anIndex >= 0 && anIndex < m_number_of_harmonics)
    {
        return m_harmonics_indices[anIndex];
    }
    else
        return 0;
}

long Ambisonic3D::getHarmonicOrder(long anIndex)
{
    if(anIndex >= 0 && anIndex < m_number_of_harmonics)
    {
        return m_harmonics_orders[anIndex];
    }
    else
        return 0;
}

void Ambisonic3D::setVectorSize(long aVectorSize)
{
	m_vector_size = Tools::clip_power_of_two(aVectorSize);
}

void Ambisonic3D::setSamplingRate(long aSamplingRate)
{
	m_sampling_rate = Tools::clip_min(aSamplingRate, long(0));
}

std::string Ambisonic3D::getHarmonicsName(long anIndex)
{
    if(anIndex >= 0 && anIndex < m_number_of_harmonics)
        return "Harmonic " + Tools::intToString(getHarmonicOrder(anIndex)) + " " + Tools::intToString(getHarmonicIndex(anIndex));
    else
        return "No harmonic";
}


Ambisonic3D::~Ambisonic3D()
{
	cicm_free(m_harmonics_indices);
    cicm_free(m_harmonics_orders);
}

