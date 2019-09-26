#include <Arduino.h>
#include "../helper/Config.h"
#include "ActionAccessPoint.h"

#include "../helper/AutoShutdown.h"
#include <AZ3166WiFi.h>
#include "../web/HttpServer.h"
#include "ActionAccessPointCli.h"

bool ActionAccessPoint()
{
    Serial.println("ActionAccessPoint()");

	AutoShutdownUpdateStartTime();
	AutoShutdownSetTimeout(CONFIG_AUTO_SHUTDOWN_TIMEOUT_AP);

	if (WiFi.beginAP(Config.APmodeSSID, Config.APmodePassword) != WL_CONNECTED) return false;

	HttpServerStart();

	ActionAccessPointCliMain();

    return true;
}
