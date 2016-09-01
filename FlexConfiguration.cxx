/*
 * FlexConfiguration.cxx
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

#include <iostream>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "FlexConfiguration.h"


std::string 
minuteDisplay( int minutes , bool forceSign )
{
    std::ostringstream oss;
    if( minutes<0 )
    { 
        oss << "-"; 
        minutes *= -1;
    }
    else if( forceSign )
    {
        oss << "+"; 
    }
    /*if( minutes>59 )*/{ oss << minutes/60 << "."; }
    int rest = minutes%60;
    if( rest<10 ){ oss << "0"; }
    oss << rest;
    
    return oss.str();
}

int
minuteParse( const std::string str )
{
    std::string tmpStr;
    int hrs(0),mins(0);
    std::string::size_type pos;
    pos = str.find(".");
    if( pos != std::string::npos )
    {
        tmpStr = str.substr(0,pos);
        hrs = strtol(tmpStr.c_str(),0,10);
        tmpStr = str.substr(pos+1);
    }
    else
    {
        tmpStr = str;
    }
    
    mins = strtol(tmpStr.c_str(),0,10);
    int sign = 1;
    if(hrs<0){ sign = -1; } // if hour part has a '-' in front, minutes also need to be negative
    
    return 60*hrs+sign*mins;
}


std::string
wdayToString( int wday )
{
    if( wday == 0 ) return std::string("Sun");
    if( wday == 1 ) return std::string("Mon");
    if( wday == 2 ) return std::string("Tue");
    if( wday == 3 ) return std::string("Wed");
    if( wday == 4 ) return std::string("Thu");
    if( wday == 5 ) return std::string("Fri");
    if( wday == 6 ) return std::string("Sat");

    return std::string("Illegal day number");
}

FlexConfiguration::FlexConfiguration()
{
    // Setup default config. mon-fri 8hrs, 40min lunch, sat+sun, no work

    m_workingTime.resize(7,0);
    m_lunchTime.resize(7,0);

    m_workingTime[1] = 480;
    m_workingTime[2] = 480;
    m_workingTime[3] = 480;
    m_workingTime[4] = 480;
    m_workingTime[5] = 480;

    m_lunchTime[1] = 40;
    m_lunchTime[2] = 40;
    m_lunchTime[3] = 40;
    m_lunchTime[4] = 40;
    m_lunchTime[5] = 40;
    m_path = ".";
    m_inputEventPath = "/dev/input/event0";
}

void 
FlexConfiguration::init( std::string path )
{
    getConfig().initialize( path );
}

void
FlexConfiguration::initialize( std::string path )
{
    std::string configPath;
    if( path == "" )
    {
        configPath = std::string( getenv("HOME") );
        configPath += "/.flextimetool/";
    }
    else
    {
        configPath = path;
    }
    m_path = configPath;
    std::cout<< "Using " << configPath << " for saving data" << std::endl;
    
    struct stat stat_buf;
    
    if( stat( configPath.c_str(), &stat_buf) != 0 )
    {
        if( errno == ENOENT )
        {
            
            mkdir( configPath.c_str(), S_IRWXU );   // read/write/exec by owner only
        }
        else
        {
            std::cerr << "Failed to stat path";
        }
    }
    
    std::string configFilePath = configPath += "/flextimetool.conf";
    
    std::ifstream conf( configFilePath.c_str() );
    
    while( conf.good() )
    {
        std::string confLine;
        std::getline(conf,confLine);
        if( confLine[0] == '#' ) continue;  // comment line, ignore and move to next line
        
        size_t pos = confLine.find(": ");
        if( pos != std::string::npos )
        {
            std::string key = confLine.substr(0,pos);
            std::string val = confLine.substr(pos+2);
            
            if( key ==  "Sunday" )                  m_workingTime[0] = minuteParse(val);
            else if( key ==  "Monday" )             m_workingTime[1] = minuteParse(val);
            else if( key ==  "Tuesday" )            m_workingTime[2] = minuteParse(val);
            else if( key ==  "Wednesday" )          m_workingTime[3] = minuteParse(val);
            else if( key ==  "Thursday" )           m_workingTime[4] = minuteParse(val);
            else if( key ==  "Friday" )             m_workingTime[5] = minuteParse(val);
            else if( key ==  "Saturday" )           m_workingTime[6] = minuteParse(val);
            
            else if( key ==  "Sunday Lunch" )       m_lunchTime[0] = minuteParse(val);
            else if( key ==  "Monday Lunch" )       m_lunchTime[1] = minuteParse(val);
            else if( key ==  "Tuesday Lunch" )      m_lunchTime[2] = minuteParse(val);
            else if( key ==  "Wednesday Lunch" )    m_lunchTime[3] = minuteParse(val);
            else if( key ==  "Thursday Lunch" )     m_lunchTime[4] = minuteParse(val);
            else if( key ==  "Friday Lunch" )       m_lunchTime[5] = minuteParse(val);
            else if( key ==  "Saturday Lunch" )     m_lunchTime[6] = minuteParse(val);

            else if( key ==  "Input Event Path" )   m_inputEventPath = val;
            
            else std::cerr << "Error. Unknown option: " << key << std::endl;
            
        }
    }
    
    std::cout << "Using settings:" << std::endl;
    for( int i=0; i<7; i++ )
    {
        std::cout << wdayToString(i) << ": " << minuteDisplay(m_workingTime[i]) << ", " << minuteDisplay(m_lunchTime[i]) << std::endl;
    }
}

int 
FlexConfiguration::getWorkingTime(unsigned int day)
{
    if( day>6 ) return 0;
    
    return getConfig().m_workingTime[day];
}    

int 
FlexConfiguration::getLunchTime(unsigned int day)
{
    if( day>6 ) return 0;
    
    return getConfig().m_lunchTime[day];
}    

