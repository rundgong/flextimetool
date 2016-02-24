/*
 * FlexTimeTracker.cxx
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

#include <iostream>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "FlexTimeTracker.h"
#include "FlexConfiguration.h"


extern void ui_log( const FlexTimeTracker::Day& day, std::string totalFlex );

std::string 
zeroPad( int x )
{
    std::ostringstream oss;
    if(x<10){ oss <<"0"; }
    oss << x;

    return oss.str();
}

void
setPreviousMonth(int& year, int& month)
{
    if( month==0 )
    {
        year -= 1;
        month = 11;
    }
    else
    {
        month -= 1;
    }
}

std::string 
getFileName( int year, int month)
{
    std::ostringstream fname;
    fname << year+1900 << "-";
    if(month<9){ fname << "0"; }  // month is in range 0-11. convert to 01-12
    fname <<  month+1 << ".flexlog";
    return fname.str();
}

void
FlexTimeTracker::initialize(time_t initTime)
{
    time_t rawtime;
    struct tm tm_time;
    
    if( initTime!=0 )
    {
        rawtime = initTime;
    }
    else
    {
        time( &rawtime );
    }
    tm_time = *localtime( &rawtime );
    std::cout << "Initialize FlexTimeTracker with time: " << asctime(&tm_time) << std::endl;

    m_workingMonth.m_month = tm_time.tm_mon;
    m_workingMonth.m_year = tm_time.tm_year;
    std::string fname = FlexConfiguration::getPath() + "/";
    fname += getFileName( m_workingMonth.m_year, m_workingMonth.m_month);
    if( !m_workingMonth.loadFile(fname) )
    {
        setPreviousMonth( m_workingMonth.m_year, m_workingMonth.m_month);
        std::string prevMonthfname = FlexConfiguration::getPath() + "/";
        prevMonthfname += getFileName( m_workingMonth.m_year, m_workingMonth.m_month);

        if( m_workingMonth.loadFile(prevMonthfname) )
        {
            int tempFlex = m_workingMonth.calcFlex();
            m_workingMonth.print(true);
            m_workingMonth = Month();
            m_workingMonth.m_previousFlex = tempFlex;
        }
        
        m_workingMonth.m_year = tm_time.tm_year;    // restore to current month
        m_workingMonth.m_month = tm_time.tm_mon;

    }
    m_workingMonth.addDay(rawtime);

    timeOfLastFlush = rawtime;
    activitySinceLastFlush = true;
    
    
}

void
FlexTimeTracker::ping(bool activity, time_t pingTime )
{
    time_t rawTime;
    struct tm tm_time;
    struct tm tm_prevTime;
    
    if( pingTime!=0 )
    {
        rawTime = pingTime;
    }
    else
    {
        time( &rawTime );
    }

    if( activitySinceLastFlush && rawTime > timeOfLastFlush+5*60 )    // write to file every 5 minutes if something has happened
    {
        m_workingMonth.print(true);
        activitySinceLastFlush = false;
        timeOfLastFlush = rawTime;
    }
    
    if( !activity )
    {
        // no new action - TODO handle maintenace?
        return;
    }
 
    activitySinceLastFlush = true;
   
    
        
    tm_time = *localtime( &rawTime );
    tm_prevTime = *localtime( &m_workingMonth.m_days.back().m_workHours.back().second );
    
    if( tm_prevTime.tm_mon != tm_time.tm_mon )
    {
        int tempFlex = m_workingMonth.calcFlex();
        m_workingMonth.print(true);
        m_workingMonth = Month();
        m_workingMonth.m_previousFlex = tempFlex;
        m_workingMonth.m_year = tm_time.tm_year;
        m_workingMonth.m_month = tm_time.tm_mon;
    }
    
    if( tm_prevTime.tm_mday != tm_time.tm_mday )
    {
        m_workingMonth.addDay(rawTime);
        return; 
    }
    
    m_workingMonth.m_days.back().m_workHours.back().second = rawTime;
    ui_log( m_workingMonth.m_days.back(), minuteDisplay(m_workingMonth.calcFlex(), true) );

}

void 
FlexTimeTracker::Month::addDay( time_t initTime )
{
    struct tm tm_initTime;
    tm_initTime = *localtime( &initTime );
    if( m_days.size() > 0 )
    {
        time_t lastDayRaw = m_days.back().m_workHours.front().first;
        struct tm tm_lastDay;
        tm_lastDay = *localtime( &lastDayRaw );
        if(tm_lastDay.tm_mday == tm_initTime.tm_mday ){ return; }
    }
    
    m_days.push_back( Day() );
    m_days.back().m_workHours.push_back( std::make_pair<time_t,time_t>(0,0) );
    m_days.back().m_workHours.back().first = initTime;
    m_days.back().m_workHours.back().second = initTime;
    m_days.back().m_lunch = FlexConfiguration::getLunchTime(tm_initTime.tm_wday);
    m_days.back().m_target = FlexConfiguration::getWorkingTime(tm_initTime.tm_wday);
    
}

void 
FlexTimeTracker::Month::print( bool toFile )
{
    std::ofstream outFile;
    std::ostream *out;
    if( toFile )
    {
        std::string fname = FlexConfiguration::getPath() + "/";
        fname += getFileName(m_year, m_month);
        outFile.open( fname.c_str() );
        out = &outFile;
    }
    else
    {
        out = &std::cout;
    }
    *out << "previous flex: " << minuteDisplay(m_previousFlex) << std::endl;

    std::list<Day>::iterator dayIt = m_days.begin();
    while( dayIt != m_days.end())
    {
        *out << dayIt->toString();
        dayIt++;
    }

    int currentFlex = calcFlex();;
    *out << "total flex: " << minuteDisplay(currentFlex) << std::endl;
    
    if( outFile.is_open() )
    {
        outFile.close();
    }
}

bool
FlexTimeTracker::Month::loadFile( std::string fileName )
{
    std::ifstream inFile;
    std::string buffer, tmpBuf;

    inFile.open( fileName.c_str() );
    
    if( !inFile.is_open() ) return false;
    
    std::getline(inFile,buffer);
    
    if(buffer.find("previous flex: ") == 0)
    {
        tmpBuf = buffer.substr(15);
        m_previousFlex = minuteParse(tmpBuf);
    }

    std::getline(inFile,buffer);
    while( buffer[0]>='0' && buffer[0]<='3' )   // rows start with date [01,31]
    {
        Day day;
        day.parse(buffer, m_year,m_month);
        m_days.push_back(day);
        std::getline(inFile,buffer);
    }
    
    if( inFile.is_open() )
    {
        inFile.close();
    }
    
    return true;
}

int 
FlexTimeTracker::Month::calcFlex( bool skipToday )
{
    std::list<Day>::iterator dayIt;
    int totalFlex = m_previousFlex;
    int lastFlex = 0;

    dayIt = m_days.begin();
    while( dayIt != m_days.end())
    {
        lastFlex = dayIt->calcFlex();
        totalFlex += lastFlex;
        dayIt++;
    }
    
    if(skipToday) 
    {
        totalFlex -= lastFlex;
    }
    
    return totalFlex;
}


std::string 
FlexTimeTracker::Day::toString() const
{
    struct tm time;
    std::list< std::pair<time_t,time_t> >::const_iterator it;
    it = m_workHours.begin();
    if(it == m_workHours.end())
    { 
        static std::string temp;
        return temp; 
    }
    
    time = *localtime( &(it->first) );
    std::ostringstream oss;
    oss << zeroPad(time.tm_mday) << ": " << zeroPad(time.tm_hour) << "." << zeroPad(time.tm_min) << "-";
    time = *localtime( &(it->second) );
    oss << zeroPad(time.tm_hour) << "." << zeroPad(time.tm_min);
    it++;

    while( it != m_workHours.end() )
    {
        time = *localtime( &(it->first) );
        oss << ", " << zeroPad(time.tm_hour) << "." << zeroPad(time.tm_min) << "-";
        time = *localtime( &(it->second) );
        oss << zeroPad(time.tm_hour) << "." << zeroPad(time.tm_min);
        it++;
    }
    int dailyFlex = calcFlex();

    oss << "; " << minuteDisplay(m_target) << "; " << minuteDisplay(m_lunch) << "; " << minuteDisplay(dailyFlex, true) << "#"<< m_comment << std::endl;
    return oss.str();
}


std::pair<time_t,time_t>
readPair(std::string buffer)
{
    std::pair<time_t,time_t> ret;
    std::string::size_type pos;

    pos = buffer.find("-");
    ret.first = minuteParse(buffer.substr(0,pos));
    ret.second = minuteParse(buffer.substr(pos+1));

    return ret;
}

int
FlexTimeTracker::Day::parse( const std::string& buffer, int year, int month )
{
    std::string tmpBuf;
    std::string tmpBuf2;
    std::string::size_type pos, oldpos, tmppos;
    struct tm timeInfo;
    timeInfo.tm_year = year;
    timeInfo.tm_mon = month;
    timeInfo.tm_sec = 0;
    timeInfo.tm_isdst = -1;
    timeInfo.tm_min = 0;
    timeInfo.tm_hour = 0;
    timeInfo.tm_wday = 0;

    pos = buffer.find(":");     // end of date
    tmpBuf = buffer.substr(0,pos);
    int date = minuteParse(tmpBuf);
    timeInfo.tm_mday = date;
    oldpos = pos;
    pos = buffer.find(";",oldpos+1);  // end of hours
    tmpBuf = buffer.substr(oldpos+1,pos-oldpos-1);
    tmppos = tmpBuf.find(",");
    while( tmppos != std::string::npos )
    {
        tmpBuf2 = tmpBuf.substr(0,tmppos);
        tmpBuf = tmpBuf.substr(tmppos+1);
        std::pair<time_t,time_t> hours = readPair(tmpBuf2);
        timeInfo.tm_hour = 0;
        timeInfo.tm_min = hours.first;
        hours.first = mktime( &timeInfo );
        timeInfo.tm_hour = 0;
        timeInfo.tm_min = hours.second;
        hours.second = mktime( &timeInfo );
        m_workHours.push_back(hours);
        tmppos = tmpBuf.find(",");
    }
    std::pair<time_t,time_t> hours = readPair(tmpBuf);
    timeInfo.tm_hour = 0;
    timeInfo.tm_min = hours.first;
    hours.first = mktime( &timeInfo );
    timeInfo.tm_hour = 0;
    timeInfo.tm_min = hours.second;
    hours.second = mktime( &timeInfo );
    m_workHours.push_back(hours);
    
    oldpos = pos;
    pos = buffer.find(";",oldpos+1);      // end of target time
    tmpBuf = buffer.substr(oldpos+1,pos-oldpos-1);
    m_target = minuteParse(tmpBuf);
    
    oldpos = pos;
    pos = buffer.find(";",oldpos+1);      // end of lunch time
    tmpBuf = buffer.substr(oldpos+1,pos-oldpos-1);
    m_lunch = minuteParse(tmpBuf);

    oldpos = pos;
    pos = buffer.find("#",oldpos+1);      // start of comment
    m_comment = buffer.substr(pos+1);
    
    return date;
    
}


int 
FlexTimeTracker::Day::calcFlex()  const
{
    int rawSum = 0;
    std::list< std::pair<time_t,time_t> >::const_iterator it;
    it = m_workHours.begin();
    while( it != m_workHours.end() )
    {
        int tempTime = it->second - it->first;
        // deduct lunch if working hours exceed 5hrs and you only did one shift
        if( m_workHours.size()==1 && tempTime > 5*60*60 ){ tempTime -= 60*m_lunch; } 
        rawSum += tempTime;
        it++;
    }
    
    return rawSum/60 - m_target;
}

