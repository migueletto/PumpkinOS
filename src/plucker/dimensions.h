/*
 * $Id: dimensions.h,v 1.8 2003/07/03 03:13:14 prussar Exp $
 *
 * Viewer - a part of Plucker, the free off-line HTML viewer for PalmOS
 * Copyright (c) 1998-2002, Mark Ian Lillywhite and Michael Nordstrom
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#define TOOLBAR_HEIGHT                       15
#define TOOLBAR_HEIGHT_HIRES_SONY            30
#define TOOLBAR_HEIGHT_HIRES_HANDERA         15
#define TOOLBAR_HEIGHT_HIRES_PALM            30

#define TITLEBAR_HEIGHT                      15
#define TITLEBAR_HEIGHT_SONY                 30
#define TITLEBAR_HEIGHT_HANDERA              22
#define TITLEBAR_HEIGHT_PALM                 30

#define SCROLLBAR_WIDTH                       7
#define SCROLLBAR_WIDTH_HIRES_SONY           14
#define SCROLLBAR_WIDTH_HIRES_HANDERA         7
#define SCROLLBAR_WIDTH_HIRES_PALM           14

#define LEFT_MARGIN                           1
#define LEFT_MARGIN_SCROLLBAR                 (LEFT_MARGIN + SCROLLBAR_WIDTH)
#define RIGHT_MARGIN                          1
#define RIGHT_MARGIN_SCROLLBAR                (RIGHT_MARGIN + SCROLLBAR_WIDTH)

#define LEFT_MARGIN_HIRES_SONY                2
#define LEFT_MARGIN_SCROLLBAR_HIRES_SONY      (LEFT_MARGIN_HIRES_SONY + \
                                              SCROLLBAR_WIDTH_HIRES_SONY)
#define RIGHT_MARGIN_HIRES_SONY               2
#define RIGHT_MARGIN_SCROLLBAR_HIRES_SONY     (RIGHT_MARGIN_HIRES_SONY + \
                                              SCROLLBAR_WIDTH_HIRES_SONY)

#define LEFT_MARGIN_HIRES_HANDERA             1
#define LEFT_MARGIN_SCROLLBAR_HIRES_HANDERA   (LEFT_MARGIN_HIRES_HANDERA + \
                                              SCROLLBAR_WIDTH_HIRES_HANDERA)
#define RIGHT_MARGIN_HIRES_HANDERA            1
#define RIGHT_MARGIN_SCROLLBAR_HIRES_HANDERA  (RIGHT_MARGIN_HIRES_HANDERA + \
                                              SCROLLBAR_WIDTH_HIRES_HANDERA)

#define LEFT_MARGIN_HIRES_PALM                1
#define LEFT_MARGIN_SCROLLBAR_HIRES_PALM      (LEFT_MARGIN_HIRES_PALM + \
                                              SCROLLBAR_WIDTH_HIRES_PALM)
#define RIGHT_MARGIN_HIRES_PALM               1
#define RIGHT_MARGIN_SCROLLBAR_HIRES_PALM     (RIGHT_MARGIN_HIRES_PALM + \
                                              SCROLLBAR_WIDTH_HIRES_PALM)

#define TOP_MARGIN                            0
#define TOP_MARGIN_TOOLBAR                    (TOP_MARGIN + TOOLBAR_HEIGHT)
#define BOTTOM_MARGIN                         0
#define BOTTOM_MARGIN_TOOLBAR                 (BOTTOM_MARGIN + TOOLBAR_HEIGHT)

#define TOP_MARGIN_HIRES_SONY                 0
#define TOP_MARGIN_TOOLBAR_HIRES_SONY         (TOP_MARGIN_HIRES_SONY + \
                                              TOOLBAR_HEIGHT_HIRES_SONY)
#define BOTTOM_MARGIN_HIRES_SONY              0
#define BOTTOM_MARGIN_TOOLBAR_HIRES_SONY      (BOTTOM_MARGIN_HIRES_SONY + \
                                              TOOLBAR_HEIGHT_HIRES_SONY)

#define TOP_MARGIN_HIRES_HANDERA              0
#define TOP_MARGIN_TOOLBAR_HIRES_HANDERA      (TOP_MARGIN_HIRES_HANDERA + \
                                              TOOLBAR_HEIGHT_HIRES_HANDERA)
#define BOTTOM_MARGIN_HIRES_HANDERA           0
#define BOTTOM_MARGIN_TOOLBAR_HIRES_HANDERA   (BOTTOM_MARGIN_HIRES_HANDERA + \
                                              TOOLBAR_HEIGHT_HIRES_HANDERA)

#define TOP_MARGIN_HIRES_PALM                 0
#define TOP_MARGIN_TOOLBAR_HIRES_PALM         (TOP_MARGIN_HIRES_PALM + \
                                              TOOLBAR_HEIGHT_HIRES_PALM)
#define BOTTOM_MARGIN_HIRES_PALM              0
#define BOTTOM_MARGIN_TOOLBAR_HIRES_PALM      (BOTTOM_MARGIN_HIRES_PALM + \
                                              TOOLBAR_HEIGHT_HIRES_PALM)

/* Length (in pixels) to scroll for one page  */
#define SCROLL_ONE_PAGE                       160
#define SCROLL_ONE_PAGE_HIRES_SONY            320
#define SCROLL_ONE_PAGE_HIRES_HANDERA         240
#define SCROLL_ONE_PAGE_HIRES_PALM            320

#define SILKSCREEN_HEIGHT_HIRES_HANDERA       65
#define SILKSCREEN_HEIGHT_HIRES_SONY          130
#define STATUSBAR_HEIGHT_HIRES_SONY           30

#define NORMAL_SCREEN_HEIGHT                  160
#define NORMAL_SCREEN_WIDTH                   160
#define PALM_SCREEN_HEIGHT                    320
#define PALM_SCREEN_WIDTH                     320
#define SONY_SCREEN_HEIGHT                    320
#define SONY_SCREEN_WIDTH                     320
#define HANDERA_SCREEN_HEIGHT                 320
#define HANDERA_SCREEN_WIDTH                  240
