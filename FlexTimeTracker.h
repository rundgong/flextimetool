/*
 * FlexTimeTracker.h
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

#ifndef __FLEXTIMETRACKER_H__
#define __FLEXTIMETRACKER_H__


#include <vector>
#include <list>
#include <string>


//class FlexConfiguration
//{
//public:
//    FlexConfiguration();
//    static void init( std::string path = std::string() );
//    static int getWorkingTime(unsigned int day);    // working time in minutes. day: sunday=0, saturday=6
//    static int getLunchTime(unsigned int day);
//    static std::string& getPath() { return getConfig().m_path; };
//
//private:
//    static FlexConfiguration& getConfig(){ static FlexConfiguration config; return config; }
//
//    void initialize( std::string path );
//
//    std::vector<int> m_workingTime;
//    std::vector<int> m_lunchTime;
//    std::string      m_path;
//
//
//};

class FlexTimeTracker
{
public:
    class Day
    {
    public:
        Day(): m_lunch(40), m_target(480){}; // 40 min lunch, 8h work day
        std::list< std::pair<time_t,time_t> > m_workHours; // set of work hours
        int m_lunch;    //lunch break in minutes
        int m_target;   // target working time in minutes
        std::string m_comment;
        //int m_day;
        
        std::string toString() const;
        //fromString( std::string& buffer );
        int parse( const std::string& str, int year, int month );
        int calcFlex() const ; // return daily flex in minutes
    };
    
    class Month
    {
    public:
        Month(): m_previousFlex(0){};//, m_currentFlex(0){};
        void addDay( time_t initTime ); //adds day at the end of the list if last day is not same day
        void print( bool toFile = false );
        bool loadFile( std::string fileName );
        int calcFlex( bool skipToday = false );     // return month's total flex in minutes

        std::list<Day> m_days;
        int m_month;
        int m_year;
        int m_previousFlex; // flex balance in minutes from previous month
        //int m_currentFlex;  // current flex balance
    };
    
    FlexTimeTracker(): m_initialized(false){}
    void initialize( time_t initTime = 0);
    void ping(bool activity, time_t pingTime = 0);
    
    Month m_workingMonth;
    Day m_workingDay;
    bool m_initialized;
    
    time_t timeOfLastFlush;
    bool activitySinceLastFlush;
};




#endif

