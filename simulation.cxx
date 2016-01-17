/*
 * FlexTimeTool.cxx
 * $Revision: 1.0 $
 *
 * Copyright (c) 2009-2016 Markus Eriksson
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
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "FlexTimeTracker.h"
#include "FlexConfiguration.h"

static void printHelpSIM()
{
    std::cout << "Usage:" << std::endl
              << "simulation -s START_TIME" << std::endl;
}

int main(int argc, char** argv)
{
    std::string dir("");
    int simulationTime(0);
    
    if(argc > 1)
    {
        std::string arg1( argv[1] );

        if( arg1.find("-h") == 0 )
        {
             printHelpSIM();
             return 0;
        }
        else if( argc > 2 && arg1.find("-s") == 0 )
        {
            simulationTime = strtol(argv[2],0,10);
            std::cout<< "Running FlexTimeTool in simulation mode " << std::endl;
            dir = ".";
        }
        else
        {
            std::cerr << "Error: Unknown option. Use \"" << argv[0] << " -h\" to print help\n";
            return 0;
        }
    }
    else
    {
        printHelpSIM();
        return 0;
    }
    
    FlexConfiguration::init(dir);
    
    FlexTimeTracker tracker;
    
    time_t rawtime;
    struct tm timeinfo;
    
    time( &rawtime );
    
    rawtime = simulationTime;
    timeinfo = *localtime( &rawtime );
  
    tracker.initialize(rawtime);
    std::cout << "Previous total flex: " << minuteDisplay(tracker.m_workingMonth.calcFlex(true), true) << std::endl;
    
    tracker.ping(true, rawtime);
    
    
    for(int j=0; j<5; j++)
    {
        for(int i=0; i<5; i++)
        {
            rawtime += 60*60; usleep(100000);
            tracker.ping(true, rawtime);
            rawtime += 60*60; usleep(100000);
            tracker.ping(true, rawtime);
            rawtime += 60*60; usleep(100000);
            tracker.ping(true, rawtime);
            rawtime += 60*60; usleep(100000);
            tracker.ping(true, rawtime);
            rawtime += 60*60; usleep(100000);
            tracker.ping(true, rawtime);
            rawtime += 60*60; usleep(100000);
            tracker.ping(true, rawtime);
            rawtime += 60*60; usleep(100000);
            tracker.ping(true, rawtime);
            rawtime += 60*60; usleep(100000);
            tracker.ping(true, rawtime);
            rawtime += 60*60; usleep(100000);
            tracker.ping(true, rawtime);
            if( i%2 )
            {
                rawtime += 60*60; usleep(100000);
                tracker.ping(true, rawtime);
                rawtime -= 60*60; usleep(100000);
            }


            rawtime += 15*60*60;
        }
        rawtime += 2*24*60*60;
    }
    
    std::cout << std::endl;
    return 0;
}

