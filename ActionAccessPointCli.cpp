#include <Arduino.h>
#include "Config.h"
#include "ActionAccessPointCli.h"

#include "AutoShutdown.h"
#include "SystemWiFi.h"

struct console_command 
{
    const char *name;
    const char *help;
    bool       isPrivacy;
    void (*function) (int argc, char **argv);
};

#define MAX_CMD_ARG     4

#define NULL_CHAR       '\0'
#define RET_CHAR        '\n'
#define END_CHAR        '\r'
#define TAB_CHAR        '\t'
#define SPACE_CHAR      ' '
#define BACKSPACE_CHAR  0x08
#define DEL_CHAR        0x7f
#define PROMPT          "\r\n# "

#define INBUF_SIZE      1024

static bool Shutdown = false;

////////////////////////////////////////////////////////////////////////////////////
// Commands table

static void help_command(int argc, char **argv);
static void reboot_and_exit_command(int argc, char **argv);
static void reset_factory_settings_command(int argc, char **argv);
static void wifi_scan(int argc, char **argv);
static void wifi_ssid_command(int argc, char **argv);
static void wifi_pwd_Command(int argc, char **argv);
static void time_server_Command(int argc, char **argv);
static void az_scopeid_command(int argc, char **argv);
static void az_deviceid_command(int argc, char **argv);
static void az_saskey_command(int argc, char **argv);
static void az_iothub_command(int argc, char **argv);
static void apmode_ssid_command(int argc, char **argv);
static void apmode_pwd_Command(int argc, char **argv);


static const struct console_command cmds[] = 
{
  {"help"                  , "Help document"                                , false, help_command                   },
  {"exit"                  , "Exit and shutdown"                            , false, reboot_and_exit_command        },
  {"reset_factory_settings", "Reset factory settings"                       , false, reset_factory_settings_command },
  {"scan"                  , "Scan Wi-Fi AP"                                , false, wifi_scan                      },
  {"set_wifissid"          , "Set Wi-Fi SSID"                               , false, wifi_ssid_command              },
  {"set_wifipwd"           , "Set Wi-Fi password"                           , true , wifi_pwd_Command               },
  {"set_timeserver"        , "Set time server"                              , false, time_server_Command            },
  {"set_az_scopeid"        , "Set scope id of Azure IoT Central"            , false, az_scopeid_command             },
  {"set_az_deviceid"       , "Set device id of Azure IoT Central"           , false, az_deviceid_command            },
  {"set_az_saskey"         , "Set SAS key of Azure IoT Central"             , true , az_saskey_command              },
  {"set_az_iothub"         , "Set the connection string of Azure IoT Hub"   , true , az_iothub_command              },
  {"set_apmodessid"        , "Set AP mode SSID"                             , false, apmode_ssid_command            },
  {"set_apmodepwd"         , "Set AP mode password"                         , true , apmode_pwd_Command             },
};

static const int cmd_count = sizeof(cmds) / sizeof(struct console_command);

////////////////////////////////////////////////////////////////////////////////////
// Command handlers

static void print_help()
{
    Serial.print("Configuration console:\r\n");
    
    for (int i = 0; i < cmd_count; i++)
    {
        Serial.printf(" - %s: %s.\r\n", cmds[i].name, cmds[i].help);
    }
}

static void help_command(int argc, char **argv)
{
    print_help();
}

static void reboot_and_exit_command(int argc, char **argv)
{
    Serial.printf("Shutdown\r\n");
    Shutdown = true;
}

static void reset_factory_settings_command(int argc, char **argv)
{
	ConfigResetFactorySettings();
    ConfigWrite();
    Serial.printf("INFO: Reset factory settings successfully.\r\n");
}

static void wifi_scan(int argc, char **argv)
{
    WiFiAccessPoint aps[10];
    memset(aps, 0, sizeof(aps));
    int count = WiFiScan(aps, 10);
    if (count > 0)
    {
        Serial.printf("Available networks:\r\n");
        for (int i =0; i < count; i++)
        {
            Serial.printf("  %s\r\n", aps[i].get_ssid());
        }
    }
    else
    {
        Serial.printf("No available network.\r\n");
    }
}

static void wifi_ssid_command(int argc, char **argv)
{
    if (argc == 1 || argv[1] == NULL) 
    {
        Serial.printf("Usage: set_wifissid <SSID>. Please provide the SSID of the Wi-Fi.\r\n");
        return;
    }
    int len = strlen(argv[1]);
    if (len == 0 || len > CONFIG_WIFI_SSID_MAX_LEN)
    {
        Serial.printf("Invalid Wi-Fi SSID.\r\n");
        return;
    }
    
    strncpy(Config.WiFiSSID, argv[1], CONFIG_WIFI_SSID_MAX_LEN + 1);

    ConfigWrite();
    Serial.printf("INFO: Set Wi-Fi SSID successfully.\r\n");
}

static void wifi_pwd_Command(int argc, char **argv)
{
    char* pwd = NULL;
    if (argc == 1)
    {
		pwd = "";
    }
    else
    {
        if (argv[1] == NULL) 
        {
            Serial.printf("Usage: set_wifipwd [password]. Please provide the password of the Wi-Fi.\r\n");
            return;
        }
        int len = strlen(argv[1]);
        if (len > CONFIG_WIFI_PASSWORD_MAX_LEN)
        {
            Serial.printf("Invalid Wi-Fi password.\r\n");
        }
        pwd = argv[1];
    }

    strncpy(Config.WiFiPassword, pwd, CONFIG_WIFI_PASSWORD_MAX_LEN + 1);

    ConfigWrite();
    Serial.printf("INFO: Set Wi-Fi password successfully.\r\n");
}

static void time_server_Command(int argc, char **argv)
{
	if (argc == 1 || argv[1] == NULL)
	{
		Serial.printf("Usage: set_timeserver <time server>. Please provide the time server.\r\n");
		return;
	}
	int len = strlen(argv[1]);
	if (len == 0 || len > CONFIG_TIME_SERVER_MAX_LEN)
	{
		Serial.printf("Invalid time server.\r\n");
		return;
	}

	strncpy(Config.TimeServer, argv[1], CONFIG_TIME_SERVER_MAX_LEN + 1);

	ConfigWrite();
	Serial.printf("INFO: Set time server successfully.\r\n");
}

static void az_scopeid_command(int argc, char **argv)
{
	if (argc == 1 || argv[1] == NULL)
	{
		Serial.printf("Usage: set_az_scopeid <scope id>. Please provide the scope id of the Azure IoT Central.\r\n");
		return;
	}
	int len = strlen(argv[1]);
	if (len == 0 || len > CONFIG_SCOPE_ID_MAX_LEN)
	{
		Serial.printf("Invalid scope id.\r\n");
		return;
	}

	strncpy(Config.ScopeId, argv[1], CONFIG_SCOPE_ID_MAX_LEN + 1);
	strncpy_w_zero(Config.IoTHubConnectionString, "", sizeof(Config.IoTHubConnectionString));

	ConfigWrite();
	Serial.printf("INFO: Set scope id successfully.\r\n");
}

static void az_deviceid_command(int argc, char **argv)
{
	if (argc == 1 || argv[1] == NULL)
	{
		Serial.printf("Usage: set_az_deviceid <device id>. Please provide the device id of the Azure IoT Central.\r\n");
		return;
	}
	int len = strlen(argv[1]);
	if (len == 0 || len > CONFIG_DEVICE_ID_MAX_LEN)
	{
		Serial.printf("Invalid device id.\r\n");
		return;
	}

	strncpy(Config.DeviceId, argv[1], CONFIG_DEVICE_ID_MAX_LEN + 1);
	strncpy_w_zero(Config.IoTHubConnectionString, "", sizeof(Config.IoTHubConnectionString));

	ConfigWrite();
	Serial.printf("INFO: Set device id successfully.\r\n");
}

static void az_saskey_command(int argc, char **argv)
{
	if (argc == 1 || argv[1] == NULL)
	{
		Serial.printf("Usage: set_az_saskey <SAS key>. Please provide the SAS key of the Azure IoT Central.\r\n");
		return;
	}
	int len = strlen(argv[1]);
	if (len == 0 || len > CONFIG_SAS_KEY_MAX_LEN)
	{
		Serial.printf("Invalid SAS key.\r\n");
		return;
	}

	strncpy(Config.SasKey, argv[1], CONFIG_SAS_KEY_MAX_LEN + 1);
	strncpy_w_zero(Config.IoTHubConnectionString, "", sizeof(Config.IoTHubConnectionString));

	ConfigWrite();
	Serial.printf("INFO: Set SAS key successfully.\r\n");
}

static void az_iothub_command(int argc, char **argv)
{
    if (argc == 1 || argv[1] == NULL) 
    {
        Serial.printf("Usage: set_az_iothub <connection string>. Please provide the connection string of the Azure IoT hub.\r\n");
        return;
    }
    int len = strlen(argv[1]);
    if (len == 0 || len > CONFIG_IOTHUB_CONNECTION_STRING_MAX_LEN)
    {
        Serial.printf("Invalid Azure IoT hub connection string.\r\n");
        return;
    }

    strncpy(Config.IoTHubConnectionString, argv[1], CONFIG_IOTHUB_CONNECTION_STRING_MAX_LEN + 1);
	strncpy_w_zero(Config.ScopeId, "", sizeof(Config.ScopeId));
	strncpy_w_zero(Config.DeviceId, "", sizeof(Config.DeviceId));
	strncpy_w_zero(Config.SasKey, "", sizeof(Config.SasKey));

    ConfigWrite();
    Serial.printf("INFO: Set Azure Iot hub connection string successfully.\r\n");
}

static void apmode_ssid_command(int argc, char **argv)
{
	if (argc == 1 || argv[1] == NULL)
	{
		Serial.printf("Usage: set_apmodessid <SSID>. Please provide the SSID of the Access Point.\r\n");
		return;
	}
	int len = strlen(argv[1]);
	if (len == 0 || len > CONFIG_APMODE_SSID_MAX_LEN)
	{
		Serial.printf("Invalid AP mode SSID.\r\n");
		return;
	}

	strncpy(Config.APmodeSSID, argv[1], CONFIG_APMODE_SSID_MAX_LEN + 1);

	ConfigWrite();
	Serial.printf("INFO: Set AP mode SSID successfully.\r\n");
}

static void apmode_pwd_Command(int argc, char **argv)
{
	char* pwd = NULL;
	if (argc == 1)
	{
		pwd = "";
	}
	else
	{
		if (argv[1] == NULL)
		{
			Serial.printf("Usage: set_apmodepwd [password]. Please provide the password of the Access Point.\r\n");
			return;
		}
		int len = strlen(argv[1]);
		if (len > CONFIG_APMODE_PASSWORD_MAX_LEN)
		{
			Serial.printf("Invalid AP mode password.\r\n");
		}
		pwd = argv[1];
	}

	strncpy(Config.APmodePassword, pwd, CONFIG_APMODE_PASSWORD_MAX_LEN + 1);

	ConfigWrite();
	Serial.printf("INFO: Set AP mode password successfully.\r\n");
}

////////////////////////////////////////////////////////////////////////////////////
// Console app

static bool is_privacy_cmd(char *inbuf, unsigned int bp)
{
    // Check privacy mode
    char cmdName[INBUF_SIZE];
    for(int j = 0; j < bp; j++)
    {
        if (inbuf[j] == SPACE_CHAR)
        {
            // Check the table
            cmdName[j] = 0;
            for(int i = 0; i < cmd_count; i++)
            {
                if(strcmp(cmds[i].name, cmdName) == 0)
                {
                    // It's privacy command
                    return cmds[i].isPrivacy;
                }
            }
            break;
        }
        else
        {
            cmdName[j] = inbuf[j];
        }
    }
    
    return false;
}

static bool get_input(char *inbuf, unsigned int *bp)
{
    if (inbuf == NULL) 
    {
        return false;
    }
    
    while (true) 
    {
        while (!Serial.available())
        {
            wait_ms(50);
        }
        inbuf[*bp] = (char)Serial.read();
        
        if (inbuf[*bp] == END_CHAR) 
        {
            /* end of input line */
            inbuf[*bp] = NULL_CHAR;
            *bp = 0;
            return true;
        }
        else if (inbuf[*bp] == TAB_CHAR) 
        {
            inbuf[*bp] = SPACE_CHAR;
        }
        else if (inbuf[*bp] == BACKSPACE_CHAR || inbuf[*bp] == DEL_CHAR)
        {
            // Delete
            if (*bp > 0) 
            {
                (*bp)--;
                Serial.write(BACKSPACE_CHAR);
                Serial.write(SPACE_CHAR);
                Serial.write(BACKSPACE_CHAR);
            }
            continue;
        }
        else if (inbuf[*bp] < SPACE_CHAR)
        {
            continue;
        }

        // Echo
        if (!is_privacy_cmd(inbuf, *bp))
        {
            Serial.write(inbuf[*bp]);
        }
        else
        {
            Serial.write('*');
        }
        (*bp)++;
        
        if (*bp >= INBUF_SIZE) 
        {
            Serial.printf("\r\nError: input buffer overflow\r\n");
            Serial.printf(PROMPT);
            *bp = 0;
            break;
        }
    }
    
    return false;
}

static int handle_input(char* inbuf)
{
    struct
    {
        unsigned inArg:1;
        unsigned inQuote:1;
        unsigned done:1;
    } stat;
  
    static char* argv[MAX_CMD_ARG];
    int argc = 0;

    int i = 0;
        
    memset((void*)&argv, 0, sizeof(argv));
    memset(&stat, 0, sizeof(stat));
  
    do 
    {
        switch (inbuf[i]) 
        {
        case '\0':
            if (stat.inQuote)
            {
                return 1;
            }
            stat.done = 1;
            break;
  
        case '"':
            if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) 
            {
                memcpy(&inbuf[i - 1], &inbuf[i], strlen(&inbuf[i]) + 1);
                
                --i;
                break;
            }
            if (!stat.inQuote && stat.inArg)
            {
                break;
            }
            if (stat.inQuote && !stat.inArg)
            {
                return 1;
            }
            
            if (!stat.inQuote && !stat.inArg) 
            {
                stat.inArg = 1;
                stat.inQuote = 1;
                argc++;
                argv[argc - 1] = &inbuf[i + 1];
            } 
            else if (stat.inQuote && stat.inArg) 
            {
                stat.inArg = 0;
                stat.inQuote = 0;
                inbuf[i] = '\0';
            }
            break;
      
        case ' ':
            if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) 
            {
                memcpy(&inbuf[i - 1], &inbuf[i], strlen(&inbuf[i]) + 1);
                --i;
                break;
            }
            if (!stat.inQuote && stat.inArg)
            {
                stat.inArg = 0;
                inbuf[i] = '\0';
            }
            break;
        default:
            if (!stat.inArg) 
            {
                stat.inArg = 1;
                argc++;
                argv[argc - 1] = &inbuf[i];
            }
            break;
        }
    } while (!stat.done && ++i < INBUF_SIZE && argc <= MAX_CMD_ARG);
  
    if (stat.inQuote)
    {
        return 1;
    }
    if (argc < 1)
    {
        return 0;
    }
    
    Serial.printf("\r\n");
    
    for(int i = 0; i < cmd_count; i++)
    {
        if(strcmp(cmds[i].name, argv[0]) == 0)
        {
            cmds[i].function(argc, argv);
            return 0;
        }
    }
    
    Serial.printf("Error:Invalid command: %s\r\n", argv[0]);
    return 0;
}

void ActionAccessPointCliMain()
{
    char inbuf[INBUF_SIZE];
    unsigned int bp = 0;
    
    print_help();
    Serial.print(PROMPT);
    
    while (true) 
    {
        if (!get_input(inbuf, &bp))
        {
            continue;
        }
                
        int ret = handle_input(inbuf);
        if (ret == 1)
        {
            Serial.print("Error:Syntax error\r\n");
        }
        if (Shutdown) break;
        
        Serial.print(PROMPT);
		AutoShutdownUpdateStartTime();
    }
}
