AM_CXXFLAGS = $(SSL_LIBS) $(LUA_LIBS) -std=c++11 -lpthread -static-libstdc++ -static-libgcc
AM_CPPFLAGS = $(SSL_CFLAGS) $(LUA_CFLAGS) -I$(top_srcdir)/include
bin_PROGRAMS = metabot
metabot_SOURCES = metabot.cpp bot.cpp net.cpp
