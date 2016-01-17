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

void printHelp();

int main(int argc, char** argv)
{
    std::string dir("");

    if(argc > 1)
    {
        std::string arg1( argv[1] );

        if( arg1.find("-h") == 0 )
        {
             printHelp();
             return 0;
        }
        else if( argc > 2 && arg1.find("-d") == 0 )
        {
            dir = argv[2];
        }
        else
        {
            std::cerr << "Error: Unknown option. Use \"" << argv[0] << " -h\" to print help\n";
            return 0;
        }
    }

    FlexConfiguration::init(dir);

    FlexTimeTracker tracker;

    time_t rawtime;
    struct tm timeinfo;

    time( &rawtime );

    timeinfo = *localtime( &rawtime );

    tracker.initialize(rawtime);
    std::cout << "Previous total flex: " << minuteDisplay(tracker.m_workingMonth.calcFlex(true), true) << std::endl;

    tracker.ping(true, rawtime);

    std::string fileName(FlexConfiguration::getInputEventPath());

    int fd;
    fd = open(fileName.c_str(),O_RDONLY);

    if( fd<0 )
    {
        std::cerr << "\n\n*** Error opening file " << fileName << " ***" << std::endl;
        return -1;
    }
    fcntl(fd, F_SETFL, O_NONBLOCK);
    char buf[256];
    ssize_t size;
    bool activity;
    while( true )
    {
        activity = true;
        size = read(fd,buf,256);
        if(size<0)
        {
            activity = false;    // nothing to read => no activity on mouse/keyboard
        }
        else
        {
#ifdef DEBUG
            std::cout << "read=" << size << std::endl;
#endif
        }

        time( &rawtime );
        tracker.ping(activity, rawtime);
        usleep(1000000);
    }
    close(fd);

    return 0;
}

