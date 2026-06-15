cmake_minimum_required(VERSION 3.16)
project(gnuscope C)

set(CMAKE_C_STANDARD 99)
#set(CMAKE_C_STANDARD 90)
set(CMAKE_C_EXTENSIONS ON)


set(BREW_PREFIX "/opt/homebrew")

add_executable(gnuscope
    getints.c
    commandfile.c
    configfile.c
    compress2.c
    filesel.c
    radwarecompat.c
    projectionbox.c
    gainshift.c
    pgammatrix.c
    pgamsort.c
    rwfiles.c
    gs92.c
    analysis.c
    displaywindow.c
    printing.c
    manipulate.c
    memmanage.c
    cursors.c
    progressbar.c
    peakfit.c
    ecallib.c
    spec-u-lator.c
    gtkgraph.c
    dialogboxes.c
    display.c
    output.c
    pgam.c
    gtktwodplot.c
    main.c
)

target_compile_options(gnuscope PRIVATE -Wall -O0 -g)

#target_compile_definitions(gnuscope PRIVATE
#    GDK_DISABLE_DEPRECATED
#    GTK_DISABLE_DEPRECATED
#    GTK_DISABLE_SINGLE_INCLUDES
#)

target_include_directories(gnuscope PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}

    ${BREW_PREFIX}/include/gtk-2.0
    ${BREW_PREFIX}/lib/gtk-2.0/include
    ${BREW_PREFIX}/include/glib-2.0
    ${BREW_PREFIX}/lib/glib-2.0/include
    ${BREW_PREFIX}/include/pango-1.0
    ${BREW_PREFIX}/include/harfbuzz
    ${BREW_PREFIX}/include/cairo
    ${BREW_PREFIX}/include/gdk-pixbuf-2.0
    ${BREW_PREFIX}/include/atk-1.0
)

target_link_directories(gnuscope PRIVATE
    ${BREW_PREFIX}/lib
)

target_link_libraries(gnuscope PRIVATE
    #gtk-x11-2.0
    #gdk-x11-2.0
gtk-quartz-2.0
gdk-quartz-2.0

gobject-2.0
    glib-2.0
    pango-1.0
    pangocairo-1.0
    cairo
    gdk_pixbuf-2.0
    atk-1.0
    m
)

set_source_files_properties(
    gtkgraph.c
    gtktwodplot.c
    pgammatrix.c
    PROPERTIES COMPILE_OPTIONS "-O3"
)
