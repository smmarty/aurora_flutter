/*******************************************************************************
** Copyright (c) 2023, Friflex LLC. Please see the AUTHORS file
** for details. Use of this source code is governed by a
** BSD-style license that can be found in the LICENSE file.
*******************************************************************************/

#include "encodable_helper.h"

#include <location_aurora/location_aurora_plugin.h>

#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>

#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPositionInfoSource>
#include <QtPositioning/QGeoPositionInfo>
#include <QtPositioning/QGeoSatelliteInfo>

#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>

typedef flutter::MethodCall<flutter::EncodableValue> MethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> MethodResult;

//******************************************************************************
//******************************************************************************
class LocationAuroraPlugin::impl
{
    friend class LocationAuroraPlugin;

private:
    LocationAuroraPlugin    * m_o;

    std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue> > m_methodChannel;

    std::shared_ptr<QGeoPositionInfoSource>  m_positionSource;
    // std::shared_ptrQGeoSatelliteInfoSource> m_satelliteSource;
    QGeoPositionInfo          m_positionInfo;
    // QMap<qint32, SatelliteInfo> m_satellites;
    bool                      m_active;

private:
    //
    impl(LocationAuroraPlugin * owner, flutter::PluginRegistrar * registrar);

    //
    void updatePosition();

    //
    void onMethodCall(const MethodCall & call,
                        std::unique_ptr<MethodResult> result);

    //
    void onGetLocation(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);
    
    //
    void onStartUpdates(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);

    //
    void unimplemented(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);
};

//******************************************************************************
//******************************************************************************
LocationAuroraPlugin::impl::impl(LocationAuroraPlugin * owner, flutter::PluginRegistrar * registrar)
    : m_o(owner)
    , m_methodChannel(new flutter::MethodChannel(registrar->messenger(), 
                                                    "location_aurora",
                                                    &flutter::StandardMethodCodec::GetInstance()))
    , m_positionSource(QGeoPositionInfoSource::createDefaultSource(nullptr))
    // , m_satelliteSource(QGeoSatelliteInfoSource::createDefaultSource(m_o))
    , m_active(false)
{
    m_methodChannel->SetMethodCallHandler([this](const MethodCall & call, std::unique_ptr<MethodResult> result)
    {
        onMethodCall(call, std::move(result));
    });


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
LocationAuroraPlugin::LocationAuroraPlugin(flutter::PluginRegistrar * registrar)
    : m_p(new impl(this, registrar))
{
    std::cout << "init OK" << std::endl;
}

//******************************************************************************
//******************************************************************************
// static
void LocationAuroraPlugin::RegisterWithRegistrar(flutter::PluginRegistrar * registrar)
{
    std::unique_ptr<LocationAuroraPlugin> plugin(new LocationAuroraPlugin(registrar));
    registrar->AddPlugin(std::move(plugin));
}

//******************************************************************************
//******************************************************************************
void LocationAuroraPlugin::impl::onMethodCall(const MethodCall & call,
                                                std::unique_ptr<MethodResult> result)
{
    const std::string & method = call.method_name();

    if (method == "Location#getLocation") 
    {
        onGetLocation(call, result);
        return;
    }

    else if (method == "Location#startUpdates") 
    {
        onStartUpdates(call, result);
        return;
    }

    unimplemented(call, result);
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
void LocationAuroraPlugin::impl::onGetLocation(const MethodCall & /*call*/,
                                                std::unique_ptr<MethodResult> & result)
{
    // std::cout << __func__ << std::endl;

    if (!m_positionSource) 
    {
        // TODO no geolocation
        std::cout << "no position source object" << std::endl;

        // call.SendErrorResponse(0, "No position source", nullptr);
        result->Success();
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
        
        result->Success();
        return;
    }

    if (!m_active)
    {
        m_positionSource->stopUpdates();
    }

    QGeoCoordinate coord = m_positionInfo.coordinate();
    
    EncodableMap locationParams;
    locationParams.emplace(std::make_pair(EncodableValue("altitude"),  
                        EncodableValue(dottedDouble(coord.altitude()))));
    locationParams.emplace(std::make_pair(EncodableValue("latitude"),  
                        EncodableValue(dottedDouble(coord.latitude()))));
    locationParams.emplace(std::make_pair(EncodableValue("longitude"), 
                        EncodableValue(dottedDouble(coord.longitude()))));
    locationParams.emplace(std::make_pair(EncodableValue("time"), 
                        EncodableValue(dottedDouble(m_positionInfo.timestamp().toMSecsSinceEpoch()))));
    locationParams.emplace(std::make_pair(EncodableValue("isMock"), 
                        EncodableValue("0")));

    if (m_positionInfo.hasAttribute(QGeoPositionInfo::GroundSpeed))
    {
      // dataMap['speed'] as double?,
        locationParams.emplace(std::make_pair(EncodableValue("speed"),  
                            EncodableValue(dottedDouble(m_positionInfo.attribute(QGeoPositionInfo::GroundSpeed)))));
    }

    if (m_positionInfo.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
    {
      // dataMap['accuracy'] as double?,
        locationParams.emplace(std::make_pair(EncodableValue("accuracy"),  
                            EncodableValue(dottedDouble(m_positionInfo.attribute(QGeoPositionInfo::HorizontalAccuracy)))));
    }

    if (m_positionInfo.hasAttribute(QGeoPositionInfo::VerticalAccuracy))
    {
      // dataMap['verticalAccuracy'] as double?,
        locationParams.emplace(std::make_pair(EncodableValue("verticalAccuracy"),  
                            EncodableValue(dottedDouble(m_positionInfo.attribute(QGeoPositionInfo::VerticalAccuracy)))));
    }

    // std::cout << "latitude:  " << coord.latitude() << std::endl
    //           << "longitude: " << coord.longitude() << std::endl;

    result->Success(EncodableValue(locationParams));
}

//******************************************************************************
//******************************************************************************
void LocationAuroraPlugin::impl::onStartUpdates(const MethodCall & /*call*/,
                                                 std::unique_ptr<MethodResult> & result)
{
    m_positionSource->startUpdates();
    result->Success();
}

//******************************************************************************
//******************************************************************************
void LocationAuroraPlugin::impl::unimplemented(const MethodCall & /*call*/,
                                                std::unique_ptr<MethodResult> & result)
{
    result->Success();
}
