/******************************************************************************\
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *  David Flamand
 *
 * Description:
 *  Intended to provide a way to customize appearance of widget
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/

//#ifdef __ANDROID__
//# define USE_THEMECUSTOMIZER
//#else
# undef USE_THEMECUSTOMIZER
//#endif

#ifdef USE_THEMECUSTOMIZER
# define APPLY_CUSTOM_THEME() ApplyCustomTheme(this, NULL)
# define APPLY_CUSTOM_THEME_UI() ApplyCustomTheme(this, ui)
class QWidget;
void ApplyCustomTheme(QWidget* widget, void* pUi);
#else
# define APPLY_CUSTOM_THEME()
# define APPLY_CUSTOM_THEME_UI()
#endif

