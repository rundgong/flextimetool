/*
 * FlexConfiguration.h
 * $Revision: 1.0 $
 *
 * Copyright (c) 2016 Markus Eriksson
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef FLEX_CONFIGURATION_H__
#define FLEX_CONFIGURATION_H__


#include <vector>
#include <list>
#include <string>

int minuteParse( const std::string str );
std::string minuteDisplay( int minutes , bool forceSign = false );
std::string wdayToString( int wday );



class FlexConfiguration
{
public:    
    FlexConfiguration();
    static void init( std::string path = std::string() );
    static int getWorkingTime(unsigned int day);    // working time in minutes. day: sunday=0, saturday=6
    static int getLunchTime(unsigned int day);
    static std::string& getPath() { return getConfig().m_path; };
    static std::string& getInputEventPath() { return getConfig().m_inputEventPath; };
    
private:
    static FlexConfiguration& getConfig(){ static FlexConfiguration config; return config; }
    
    void initialize( std::string path );
    
    std::vector<int> m_workingTime;
    std::vector<int> m_lunchTime;
    std::string      m_path;
    std::string      m_inputEventPath;
    
    
};



#endif

