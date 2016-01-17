/*
 * xmain.cxx
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
 
/* Derived from xidle. Copyright and license terms as below */
 
/*    $OpenBSD: xidle.c,v 1.10 2005/10/07 19:47:26 fgsch Exp $    */
/*
 * Copyright (c) 2005 Federico G. Schwindt.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE OPENBSD PROJECT AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OPENBSD
 * PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <string>
#include <iostream>
#include "FlexTimeTracker.h"
#include "FlexConfiguration.h"

#ifndef PATH_PROG
#define PATH_PROG    "/usr/X11R6/bin/xlock"
#endif


void printHelp();

struct xinfo {
    Display        *dpy;
    Window         win;
    int         coord_x;
    int         coord_y;

    int         saver_event;    /* Only if Xss ext is available */

    int         saved_timeout;
    int         saved_interval;
    int         saved_pref_blank;
    int         saved_allow_exp;
};

struct xinfo x;
void    close_x(struct xinfo *);
void    init_x(const char *, struct xinfo *, int, int);
void    handler(int);


void
init_x(const char *display, struct xinfo *xi, int area, int timeout)
{
    XSetWindowAttributes attr;
    Display *dpy;
    Window win;
    int error, event;

    dpy = XOpenDisplay(display);
    if (!dpy) {
        errx(1, "Unable to open display %s", XDisplayName(display));
        /* NOTREACHED */
    }


    attr.override_redirect = True;
    win = XCreateWindow(dpy, DefaultRootWindow(dpy),
        xi->coord_x, xi->coord_y, area, area, 0, 0, InputOnly,
        CopyFromParent, CWOverrideRedirect, &attr);

    XMapWindow(dpy, win);

    /*
     * AFAICT, we need the event number for ScreenSaverNotify
     * _always_ since it's the only way to distinguish whether
     * we've been obscured by an external locking program or
     * by another window and react according.
     */
    if (XScreenSaverQueryExtension(dpy, &event, &error) == True) {
        xi->saver_event = event;

        XScreenSaverSelectInput(dpy, DefaultRootWindow(dpy),
            ScreenSaverNotifyMask);
    } else
        warnx("XScreenSaver extension not available.%s",
            timeout > 0 ? " Timeout disabled." : "");

    if (timeout > 0 && xi->saver_event) {
        XGetScreenSaver(dpy, &xi->saved_timeout, &xi->saved_interval,
            &xi->saved_pref_blank, &xi->saved_allow_exp);

        XSetScreenSaver(dpy, timeout, 0, DontPreferBlanking,
            DontAllowExposures);
    }

    xi->dpy = dpy;
    xi->win = win;
}


void
close_x(struct xinfo *xi)
{
    XSetScreenSaver(xi->dpy, xi->saved_timeout, xi->saved_interval,
        xi->saved_pref_blank, xi->saved_allow_exp);
    XDestroyWindow(xi->dpy, xi->win);
    XCloseDisplay(xi->dpy);
}



void
handler(int sig)
{
    XClientMessageEvent ev;

    ev.type = ClientMessage;
    ev.display = x.dpy;
    ev.window = x.win;
    ev.message_type = 0xdead;
    ev.format = 8;
    XSendEvent(x.dpy, x.win, False, 0L, (XEvent *)&ev);
    XFlush(x.dpy);
}

bool XNextEventOrTimeout(Display *display, XEvent *evt, int timeout_ms) 
{
    int pending = XPending(display);
    if (!pending) 
    {
        struct timeval tv;
        tv.tv_sec=0;
        tv.tv_usec=timeout_ms * 1000;
        int fd = XConnectionNumber(display);
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(fd, &readset);
        select( fd+1, &readset, 0, 0, &tv );
        pending = XPending(display);
    }
    if (pending) 
    {
        XNextEvent(display, evt);
        return true;
    }
    return false;
} 

int
main(int argc, char **argv)
{
    char *display = NULL;
    int area = 2;
    int timeout = 2;

    bzero(&x, sizeof(struct xinfo));

    init_x(display, &x, area, timeout);

    signal(SIGINT, handler);
    signal(SIGTERM, handler);




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
    



    for (;;) {
        XEvent ev;
        int tmo,inter;
        XGetScreenSaver(x.dpy, &tmo, &inter,
                &x.saved_pref_blank, &x.saved_allow_exp);
#ifdef DEBUG
        printf("loop: %d, %d\n", tmo,inter);
#endif
        if( tmo == 0 )
        {
#ifdef DEBUG
            printf("RESET screen saver\n");
#endif
            XSetScreenSaver(x.dpy, timeout, 0, DontPreferBlanking,
                    DontAllowExposures);
        }
        if(!XNextEventOrTimeout(x.dpy, &ev, 10000))
        {
            time( &rawtime );
            tracker.ping(false, rawtime);
            continue;
        }

#ifdef DEBUG
        printf("got event %d\n", ev.type);
#endif

        switch (ev.type) {

        case ClientMessage:
            if (ev.xclient.message_type != 0xdead)
            {
                break;
            }
            printf("Exiting...\n");
            close_x(&x);
            tracker.m_workingMonth.print(true);
            exit(0);
            /* NOTREACHED */

        default:
            if (ev.type != EnterNotify && ev.type != x.saver_event)
                break;

            if (ev.type == x.saver_event)
            {
                XScreenSaverNotifyEvent *se =(XScreenSaverNotifyEvent *)&ev;

                // Was for real or due to terminal switching or a locking program?
                if (se->forced != False)
                {
                    break;
                }
            }

            time( &rawtime );
            tracker.ping(true, rawtime);
#ifdef DEBUG
            struct tm tm_time;
            tm_time = *localtime( &rawtime );
            printf("action at time %s\n", asctime(&tm_time));
#endif
            break;
        }
    }

    /* NOTREACHED */
}
