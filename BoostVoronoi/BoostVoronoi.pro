INCLUDEPATH += $$(LIBS)/boost_1_67_0

SOURCES += \
    voronoi_visualizer.cpp

HEADERS += \
    voronoi_visual_utils.hpp

LIBS += -lOpenGL32

QT += widgets opengl
