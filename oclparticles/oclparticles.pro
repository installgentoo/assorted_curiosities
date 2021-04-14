CONFIG -= qt
LIBS += -L/usr/lib -lrt -lXrandr -lOpenCL -lGLEW -lGL -lGLU -lglfw
INCLUDEPATH += /usr/include/CL
SOURCES += main.cpp \
    particleSystem_class.cpp \
    particle_system.cpp \
    tools.cpp

OTHER_FILES += \
    Particles.c

HEADERS += \
    particleSystem_class.h
