INCLUDEPATH += $$(LIBS)/boost_1_67_0

SOURCES += \
    voronoi_visualizer.cpp

LIBS += -lOpenGL32

QT += widgets opengl
