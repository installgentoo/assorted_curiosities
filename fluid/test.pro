CONFIG -= qt
LIBS += ../test/glfw3.dll -lopengl32 -lWs2_32 -lole32 -lcomctl32 -lgdi32 -lcomdlg32 -luuid
SOURCES += \
    support.cpp \
    shader.cpp \
    fluid.cpp \
    _main.cpp \
    glew.c \
    mapbuffer.cpp

HEADERS += \
    support.h \
    shader.h \
    fluid.h \
    _main.h \
    mapbuffer.h

OTHER_FILES += \
    ps_border.c \
    ps_move.c \
    vs_basic.c \
    ps_render.c \
    ps_iterate.c \
    ps_intake.c \
    ps_substract.c \
    ps_decompose.c \
    ps_forces.c
