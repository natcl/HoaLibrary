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
 *	- Redistributions may not be sold, nor may they be used in a commercial product or activity.
 *  - Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,Z
 *		this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *  - Neither the name of the CICM nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HOA_MAP_PLUGIN
#define HOA_MAP_PLUGIN

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../../Sources/HoaLibrary.h"
#include "HoaLookAndFeel.h"

class MapEditor : public Component
{
private:
    SourcesManager* m_sources_manager;
    AudioProcessor* m_processor;
    
    double          m_sources_size;
    long            m_souce_selected;
    long            m_group_selected;
    
    long             m_drag_mode;
    bool             m_selected;
    Rectangle<float> m_selection;;
    
    void draw_background(Graphics& g);
    void draw_sources(Graphics& g);
    void draw_groups(Graphics& g);
    void draw_selection(Graphics& g);
    
    HoaLookAndFeel          LookAndFeel;
public:
    MapEditor(SourcesManager* aSourceManager, AudioProcessor* aProcessor);
    ~MapEditor();
    
    void paint(Graphics& g);
    
    
    void mouseMove(const MouseEvent &event);
    void mouseDown(const MouseEvent &event);
    void mouseDrag(const MouseEvent &event);
    void mouseUp(const MouseEvent &event);
    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel);
};


#endif