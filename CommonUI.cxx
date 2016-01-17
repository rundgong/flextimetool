/*
 * common_ui.cxx
 * $Revision: 1.0 $
 *
 * Copyright (c) 2009-2010 Markus Eriksson
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

#include "FlexTimeTracker.h"
#include <iostream>

std::string wdayToString( int wday );

void printHelp()
{
    std::cerr << "\n   FlexTimeTool (c) Markus Eriksson 2010\n\n"
              << "Log your working hours and calculate +/- flex time.\n"
              << "Your working hours are calculated by sensing your presence"
                 " at the computer by tracking mouse movements.\n\n"
              << "Usage: FlexTimeTool [-h | -d dir ]\n\n"
              << "Options:\n\n"
              << "   -h: Display this help text\n\n"
              << "   -d dir: Use dir as your working directory, where settings are read\n"
                 "           and log files stored.\n"
              << "           Default value is ~/.flextimetool\n\n"
              << "Settings:\n"
              << "   The file \"flextimetool.conf\" in the working directory will be read\n"
                 "   at start up. It can be used to override the default settings.\n\n";
}





void ui_log( const FlexTimeTracker::Day& day, std::string totalFlex )
{
    static time_t prevTime = 0;
    static std::string prevString;
    struct tm tm_time1;
    struct tm tm_time2;
    
    if(day.m_workHours.size() == 0) return; // safety check

    if( prevTime == 0 ) prevTime = day.m_workHours.back().second;
    
    tm_time1 = *localtime( &prevTime );   
    tm_time2 = *localtime( &day.m_workHours.back().second );
    
    if( tm_time1.tm_year != tm_time2.tm_year || tm_time1.tm_yday != tm_time2.tm_yday )  // new day
    {
        std::cout << std::endl;
        prevString = "";   
    }
    
    if( tm_time2.tm_wday < tm_time1.tm_wday || tm_time2.tm_yday - tm_time1.tm_yday > 6 )  // new week
    {
        std::cout << "=======================================" << std::endl;
    }
    
    if( tm_time1.tm_mon != tm_time2.tm_mon  )  // new month
    {
        std::cout << "---------------------------------------" << std::endl;
    }

    if( tm_time1.tm_year != tm_time2.tm_year  )  // new year
    {
        std::cout << "***************************************" << std::endl;
    }

    char backspace = 8;
    for( unsigned int i=0; i<prevString.size(); i++ )
    {
        std::cout << backspace;
    }
    prevString = wdayToString(tm_time2.tm_wday) + " ";
    prevString += day.toString();
    prevString.erase(prevString.size()-1,1);   // remove new line at the end
    prevString += " Accumulated flex: " + totalFlex + " ";
    std::cout << prevString;
    std::cout.flush();
    prevTime = day.m_workHours.back().second;
}
