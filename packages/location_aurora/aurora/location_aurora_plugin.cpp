/*******************************************************************************
** Copyright (c) 2023, Friflex LLC. Please see the AUTHORS file
** for details. Use of this source code is governed by a
** BSD-style license that can be found in the LICENSE file.
*******************************************************************************/

#include <location_aurora/location_aurora_plugin.h>
#include <flutter/method-channel.h>

#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPositionInfoSource>
#include <QtPositioning/QGeoPositionInfo>
#include <QtPositioning/QGeoSatelliteInfo>

#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>

//******************************************************************************
//******************************************************************************
class LocationAuroraPlugin::impl
{
    friend class LocationAuroraPlugin;

private:
    LocationAuroraPlugin    * m_o;

    std::shared_ptr<QGeoPositionInfoSource>  m_positionSource;
    // std::shared_ptrQGeoSatelliteInfoSource> m_satelliteSource;
    QGeoPositionInfo          m_positionInfo;
    // QMap<qint32, SatelliteInfo> m_satellites;
    bool                      m_active;

private:
    //
    impl(LocationAuroraPlugin * owner);

    //
    void init(const MethodCall &call);

    //
    void updatePosition();

    //
    void onGetLocation(const MethodCall &call);
    
    //
    void onStartUpdates(const MethodCall &call);
};

//******************************************************************************
//******************************************************************************
LocationAuroraPlugin::impl::impl(LocationAuroraPlugin * owner)
    : m_o(owner)
    , m_positionSource(QGeoPositionInfoSource::createDefaultSource(nullptr))
    // , m_satelliteSource(QGeoSatelliteInfoSource::createDefaultSource(m_o))
    , m_active(false)
{
    QStringList sources = QGeoPositionInfoSource::availableSources();
    if (!sources.size())
    {
        std::cout << "No sources" << std::endl;
    }
    else
    {
        for (const QString & s : sources)
        {
            std::cout << s.toStdString() << std::endl;
        }
    }

    QObject::connect(m_positionSource.get(), 
            &QGeoPositionInfoSource::positionUpdated,
            [this](const QGeoPositionInfo & info)
            {
                // std::cout << __func__ << std::endl;

                m_positionInfo = info;

                QGeoCoordinate coord = m_positionInfo.coordinate();

                std::cout << "latitude:  " << coord.latitude() << std::endl
                          << "longitude: " << coord.longitude() << std::endl;
            });


    // TODO error handling
    // QObject::connect(m_positionSource.get(), 
    //         &QGeoPositionInfoSource::errorOccurred,
    //         [this](const QGeoPositionInfoSource::Error positioningError)
    //         {
    //         });

    m_positionSource->startUpdates();
}

//******************************************************************************
//******************************************************************************
LocationAuroraPlugin::LocationAuroraPlugin()
    : PluginInterface()
    , m_p(new impl(this))
{
    std::cout << "init OK" << std::endl;
}

//******************************************************************************
//******************************************************************************
void LocationAuroraPlugin::RegisterWithRegistrar(PluginRegistrar &registrar)
{
    registrar.RegisterMethodChannel("location_aurora",
                                    MethodCodecType::Standard,
                                    [this](const MethodCall &call) 
                                    { 
                                        this->onMethodCall(call); 
                                    });
}

//******************************************************************************
//******************************************************************************
void LocationAuroraPlugin::onMethodCall(const MethodCall &call)
{
    const auto &method = call.GetMethod();

    if (method == "Location#getLocation") 
    {
        m_p->onGetLocation(call);
        return;
    }

    else if (method == "Location#startUpdates") 
    {
        m_p->onStartUpdates(call);
        return;
    }

    unimplemented(call);
}

//******************************************************************************
//******************************************************************************
void LocationAuroraPlugin::impl::updatePosition()
{
    // std::cout << __func__ << std::endl;

    m_positionInfo = m_positionSource->lastKnownPosition();

    if (!m_positionInfo.isValid())
    {
        // TODO no geolocation
        std::cout << "no valid position" << std::endl;
    }
}

//******************************************************************************
//******************************************************************************
std::string dottedDouble(const double & d)
{
    std::string str = std::to_string(d);

    for (char & ch : str)
    {
        if (ch == ',')
        {
            ch = '.';
        }
    }

    return str;
}

//******************************************************************************
//******************************************************************************
void LocationAuroraPlugin::impl::onGetLocation(const MethodCall & call)
{
    // std::cout << __func__ << std::endl;

    if (!m_positionSource) 
    {
        // TODO no geolocation
        std::cout << "no position source object" << std::endl;

        // call.SendErrorResponse(0, "No position source", nullptr);
        call.SendSuccessResponse(nullptr);
        return;
    }

    // if (!m_p->m_positionSource->lastKnownPosition().isValid())
    // {
    //     // TODO no geolocation
    //     std::cout << "no valid position" << std::endl;

    //     // call.SendErrorResponse(0, "No position source", nullptr);
    //     call.SendSuccessResponse(nullptr);
    //     return;
    // }

    updatePosition();

    if (!m_positionInfo.isValid())
    {
        m_positionSource->startUpdates();
        
        call.SendSuccessResponse(nullptr);
        return;
    }

    if (!m_active)
    {
        m_positionSource->stopUpdates();
    }

    QGeoCoordinate coord = m_positionInfo.coordinate();
    
    Encodable::Map locationParams;
    locationParams.emplace(std::make_pair(Encodable("altitude"),  
                        Encodable(dottedDouble(coord.altitude()))));
    locationParams.emplace(std::make_pair(Encodable("latitude"),  
                        Encodable(dottedDouble(coord.latitude()))));
    locationParams.emplace(std::make_pair(Encodable("longitude"), 
                        Encodable(dottedDouble(coord.longitude()))));
    locationParams.emplace(std::make_pair(Encodable("time"), 
                        Encodable(dottedDouble(m_positionInfo.timestamp().toMSecsSinceEpoch()))));
    locationParams.emplace(std::make_pair(Encodable("isMock"), 
                        Encodable("0")));

    if (m_positionInfo.hasAttribute(QGeoPositionInfo::GroundSpeed))
    {
      // dataMap['speed'] as double?,
        locationParams.emplace(std::make_pair(Encodable("speed"),  
                            Encodable(dottedDouble(m_positionInfo.attribute(QGeoPositionInfo::GroundSpeed)))));
    }

    if (m_positionInfo.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
    {
      // dataMap['accuracy'] as double?,
        locationParams.emplace(std::make_pair(Encodable("accuracy"),  
                            Encodable(dottedDouble(m_positionInfo.attribute(QGeoPositionInfo::HorizontalAccuracy)))));
    }

    if (m_positionInfo.hasAttribute(QGeoPositionInfo::VerticalAccuracy))
    {
      // dataMap['verticalAccuracy'] as double?,
        locationParams.emplace(std::make_pair(Encodable("verticalAccuracy"),  
                            Encodable(dottedDouble(m_positionInfo.attribute(QGeoPositionInfo::VerticalAccuracy)))));
    }

    // std::cout << "latitude:  " << coord.latitude() << std::endl
    //           << "longitude: " << coord.longitude() << std::endl;

    call.SendSuccessResponse(locationParams);
}

//******************************************************************************
//******************************************************************************
void LocationAuroraPlugin::impl::onStartUpdates(const MethodCall & call)
{
    m_positionSource->startUpdates();
    call.SendSuccessResponse(nullptr);
}

//******************************************************************************
//******************************************************************************
void LocationAuroraPlugin::unimplemented(const MethodCall & call)
{
    call.SendSuccessResponse(nullptr);
}
