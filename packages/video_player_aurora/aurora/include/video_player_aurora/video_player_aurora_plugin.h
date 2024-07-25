//******************************************************************************
//******************************************************************************

#ifndef __VIDEO_PLAYER_AURORA_PLUGIN_H__
#define __VIDEO_PLAYER_AURORA_PLUGIN_H__

//******************************************************************************
//******************************************************************************
#include <flutter/plugin_registrar.h>
#include <video_player_aurora/globals.h>

#include <memory>

//******************************************************************************
//******************************************************************************
class PLUGIN_EXPORT VideoPlayerAuroraPlugin final : public flutter::Plugin 
{
private:
    VideoPlayerAuroraPlugin(flutter::PluginRegistrar * registrar);
    
public:
    static void RegisterWithRegistrar(flutter::PluginRegistrar * registrar);
    ~VideoPlayerAuroraPlugin();
    
private:
    class impl;
    std::shared_ptr<impl> m_p;
};

#endif //__VIDEO_PLAYER_AURORA_PLUGIN_H__