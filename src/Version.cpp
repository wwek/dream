/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2006
 *
 * Author(s):
 *	Andrea Russo, Julian Cable
 *
 * Description:
 *	Dream program version number
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
#include "Version.h"
#include "GlobalDefinitions.h"

const char dream_manufacturer[] = "drea";
#ifdef QT_CORE_LIB
# if QT_VERSION >= 0x050000
const char dream_implementation[] = "Q5";
# elif QT_VERSION >= 0x040000
const char dream_implementation[] = "Q4";
# else
const char dream_implementation[] = "QT";
# endif
#else
const char dream_implementation[] = "CL";
#endif
const int dream_version_major = 2;
const int dream_version_minor = 3;
const int dream_version_patch = 1;

/* Dynamic build info - injected via qmake preprocessor macros */
const char dream_version_build[] = "-" GIT_COMMIT " (" GIT_BRANCH ") " BUILD_TIMESTAMP;

/* Build info query functions implementation */
const char* GetBuildGitCommit()
{
    return GIT_COMMIT;
}

const char* GetBuildTimestamp()
{
    return BUILD_TIMESTAMP;
}

const char* GetBuildBranch()
{
    return GIT_BRANCH;
}

void PrintFullVersion()
{
    std::cout << "Dream DRM Software" << std::endl;
    std::cout << "Version " << dream_version_major << "."
              << dream_version_minor << "." << dream_version_patch
              << dream_version_build << std::endl;
    std::cout << "Manufacturer: " << dream_manufacturer << std::endl;
    std::cout << "Implementation: " << dream_implementation << std::endl;
    std::cout << "Build Information:" << std::endl;
    std::cout << "  Git Commit: " << GIT_COMMIT << std::endl;
    std::cout << "  Git Branch: " << GIT_BRANCH << std::endl;
    std::cout << "  Build Time: " << BUILD_TIMESTAMP << std::endl;
}

