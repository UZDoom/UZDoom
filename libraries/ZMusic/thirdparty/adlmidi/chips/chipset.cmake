#
# Interfaces over Yamaha OPL2 (YM3812) and Yamaha OPL3 (YMF262) chip emulators
#
# Copyright (c) 2017-2026 Vitaly Novichkov (Wohlstand)
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

set(CHIPS_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/opl_chip_base.h"
    "${CMAKE_CURRENT_LIST_DIR}/opl_chip_base.tcc"
)

if(OPL_CHIPSET_DOS_HARDWARE_MODE)
    list(APPEND CHIPS_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/dos_hw_opl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dos_hw_opl.h"
    )
    # Disable all the software emulators in this way
    set(OPL_CHIPSET_ENABLE_SW_EMULATORS OFF)
    set(OPL_CHIPSET_ENABLE_LLE_OPL2 OFF)
    set(OPL_CHIPSET_ENABLE_LLE_OPL3 OFF)
    set(OPL_CHIPSET_ENABLE_VPC OFF)
    set(ENABLE_OPL3_PROXY OFF)
    set(ENABLE_SERIAL_PORT OFF)
else()
    if(NOT DEFINED OPL_CHIPSET_ENABLE_SW_EMULATORS)
        set(OPL_CHIPSET_ENABLE_SW_EMULATORS ON)
    endif()
endif()

if(OPL_CHIPSET_ENABLE_SW_EMULATORS)
    list(APPEND CHIPS_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/dosbox_opl3.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dosbox_opl3.h"
        "${CMAKE_CURRENT_LIST_DIR}/esfmu_opl3.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/esfmu_opl3.h"
        "${CMAKE_CURRENT_LIST_DIR}/java_opl3.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/java_opl3.h"
        "${CMAKE_CURRENT_LIST_DIR}/mame_opl2.h"
        "${CMAKE_CURRENT_LIST_DIR}/mame_opl2.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_opl2.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_opl2.h"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_opl3.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_opl3.h"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_cqm.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_cqm.h"
        "${CMAKE_CURRENT_LIST_DIR}/opal_opl3.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/opal_opl3.h"
        "${CMAKE_CURRENT_LIST_DIR}/opal/opal.c"
        "${CMAKE_CURRENT_LIST_DIR}/opal/opal.h"
        "${CMAKE_CURRENT_LIST_DIR}/esfmu/esfm.c"
        "${CMAKE_CURRENT_LIST_DIR}/esfmu/esfm.h"
        "${CMAKE_CURRENT_LIST_DIR}/esfmu/esfm_registers.c"
        "${CMAKE_CURRENT_LIST_DIR}/nuked/nukedopl2.c"
        "${CMAKE_CURRENT_LIST_DIR}/nuked/nukedopl2.h"
        "${CMAKE_CURRENT_LIST_DIR}/nuked/nukedopl3.c"
        "${CMAKE_CURRENT_LIST_DIR}/nuked/nukedopl3.h"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_cqm/cqm.c"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_cqm/cqm.h"
        "${CMAKE_CURRENT_LIST_DIR}/mame/opl.h"
        "${CMAKE_CURRENT_LIST_DIR}/mame/mame_fmopl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dosbox/dbopl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dosbox/dbopl.h"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_opl3_fast.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_opl3_fast.h"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_fast/nukedopl3_fast.c"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_fast/nukedopl3_fast.h"
        "${CMAKE_CURRENT_LIST_DIR}/nuked_fast/wf_rom.h"
    )
endif()

if(OPL_CHIPSET_ENABLE_LLE_OPL2)
    list(APPEND CHIPS_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/ym3812_lle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ym3812_lle.h"
        "${CMAKE_CURRENT_LIST_DIR}/ym3812_lle/nuked_fmopl2.c"
        "${CMAKE_CURRENT_LIST_DIR}/ym3812_lle/nuked_fmopl2.h"
        "${CMAKE_CURRENT_LIST_DIR}/ym3812_lle/nopl2.c"
        "${CMAKE_CURRENT_LIST_DIR}/ym3812_lle/nopl2.h"
    )
endif()

if(OPL_CHIPSET_ENABLE_LLE_OPL3)
    list(APPEND CHIPS_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/ymf262_lle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ymf262_lle.h"
        "${CMAKE_CURRENT_LIST_DIR}/ymf262_lle/nuked_fmopl3.c"
        "${CMAKE_CURRENT_LIST_DIR}/ymf262_lle/nuked_fmopl3.h"
        "${CMAKE_CURRENT_LIST_DIR}/ymf262_lle/nopl3.c"
        "${CMAKE_CURRENT_LIST_DIR}/ymf262_lle/nopl3.h"
    )
endif()

if(OPL_CHIPSET_ENABLE_VPC)
    list(APPEND CHIPS_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/vpc_opl3_emu.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/vpc_opl3_emu.h"
        "${CMAKE_CURRENT_LIST_DIR}/vpc_opl3/vpc_opl3.c"
        "${CMAKE_CURRENT_LIST_DIR}/vpc_opl3/vpc_opl3.h"
    )
endif()

if(COMPILER_SUPPORTS_CXX14 AND OPL_CHIPSET_ENABLE_SW_EMULATORS) # YMFM can be built in only condition when C++14 and newer were available
    set(YMFM_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/ymfm_opl2.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm_opl2.h"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm_opl3.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm_opl3.h"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm/ymfm.h"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm/ymfm_opl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm/ymfm_opl.h"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm/ymfm_misc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm/ymfm_misc.h"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm/ymfm_pcm.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm/ymfm_pcm.h"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm/ymfm_adpcm.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm/ymfm_adpcm.h"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm/ymfm_ssg.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ymfm/ymfm_ssg.h"
    )
    if(DEFINED FLAG_CPP14)
        set_source_files_properties(${YMFM_SOURCES} COMPILE_FLAGS ${FLAG_CPP14})
    endif()
    list(APPEND CHIPS_SOURCES ${YMFM_SOURCES})
endif()

if(ENABLE_OPL3_PROXY)
    list(APPEND CHIPS_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/win9x_opl_proxy.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/win9x_opl_proxy.h"
    )
endif()

if(ENABLE_SERIAL_PORT)
    list(APPEND CHIPS_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/opl_serial_port.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/opl_serial_port.h"
    )
endif()
