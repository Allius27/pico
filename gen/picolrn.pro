TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += picolrn.c

LIBS += -lrt -lm -fopenmp
