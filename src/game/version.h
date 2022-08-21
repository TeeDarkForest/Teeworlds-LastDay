/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

/*#ifndef GAME_VERSION_H
#define GAME_VERSION_H
#include "generated/nethash.cpp"
#define GAME_VERSION "0.6.1"
#define GAME_NETVERSION "0.6 " GAME_NETVERSION_HASH
#endif*/

#ifndef GAME_VERSION_H
#define GAME_VERSION_H
#ifndef NON_HASED_VERSION
#include "generated/nethash.cpp"
#define GAME_VERSION "0.6.4"
#define GAME_NETVERSION "0.6 626fce9a778df4d4" //the std game version
static const char GAME_RELEASE_VERSION[8] = "0.6.4";

#define MOD_NAME "Last Day"
#define MOD_VERSION "0.3"
#define MOD_AUTHORS "ErrorDreemur, FlowerFell-Sans"
#define MOD_CREDITS "ErrorDreemur, FlowerFell-Sans"
#define MOD_THANKS "Ninecloud, SushiTee, StarOnTheSky, Necropotame, Mettaton, Spamton G. Spamton"
#define MOD_SOURCES ""
#endif
#endif
