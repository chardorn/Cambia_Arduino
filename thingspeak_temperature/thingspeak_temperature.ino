#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#define DATA_FIELD 1                                  // Data field to post the signal strength to.
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

const int nsensors = 2;


int16_t tempraw[nsensors];
unsigned long nextprint = 0;

OneWire  ds(2);  // on pin 2 (a 4.7K pullup is necessary)

//const char* ssid = "pink pants cricket";
const char* pass = "breezyshrub";
const char* server = "mqtt.thingspeak.com";
char mqttUserName[] = "";                // Use any name.
char mqttPass[] = "";                 // Change to your MQTT API key from Account > MyProfile.
long writeChannelID=1028998;
char writeAPIKey[]   = "6MCNQF4KPLG2D5O8";

int sensorPin = A0;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor

WiFiClient client;                                    // Initialize the Wi-Fi client library.
PubSubClient mqttClient( client );                    // Initialize the PuBSubClient library.
int changeFlag=0;                                     // Let the main loop know ther is new data to set.

//bool connectedToLocal = False;

//  
// Prototypes
//

// Handle messages from MQTT subscription.
int mqttSubscriptionCallback(char* topic, byte* payload, unsigned int length);  

// Generate a unique client ID and connect to MQTT broker.
void mqttConnect();  

// Subscribe to a field or feed from a ThingSpeak channel.
//int mqttSubscribe( long subChannelID,int field,char* readKey, int unSub);

// Publish messages to a channel feed.
// void mqttPublish(long pubChannelID, char* pubWriteAPIKey, float dataArray[], int fieldArray[]);

// Connect to a given Wi-Fi SSID
void connectWifi();

// Measure the Wi-Fi signal strength.
void updateRSSIValue();

// Build a random client ID for MQTT connection.
void getID(char clientID[], int idLength);

void setup() {

    Serial.begin( 9600 );
    Serial.println( "Start" );
    int status = WL_IDLE_STATUS; // Set temporary Wi-Fi status.

    // scan for existing networks:
    Serial.println("Scanning available networks...");
       
    connectWifi();  // Connect to Wi-Fi network.
    mqttClient.setServer( server, 1883 ); // Set the MQTT broker details.

}



void loop() {
    
    if (WiFi.status() != WL_CONNECTED) {
        connectWifi();
    }
    
    if (!mqttClient.connected())
    {
       mqttConnect(); // Connect if MQTT client is not connected.
    }


    float temp = sensors.getTempCByIndex(0);
  

    String dataString = "field5=" + String(temp);
    Serial.println(dataString);
 
    Serial.println(dataString);
    mqttClient.loop(); // Call the loop to maintain connection to the server.  
 
    String topicString ="channels/" + String(writeChannelID) + "/publish/"+String(writeAPIKey);
    mqttClient.publish( topicString.c_str(), dataString.c_str() );
    Serial.println( "to channel " + String(writeChannelID ) );

    //mqttPublish( writeChannelID, writeAPIKey, dataToPublish, fieldsToPublish );

    //CHANGE THIS FOR INTERVALS
    delay(20000);
}

/**
 * Process messages received from subscribed channel via MQTT broker.
 *   topic - Subscription topic for message.
 *   payload - Field to subscribe to. Value 0 means subscribe to all fields.
 *   mesLength - Message length.
 */

void mqttConnect()
{
    char clientID[ 9 ];
    
    // Loop until connected.
    while ( !mqttClient.connected() )
    {

        getID(clientID,8);
       
        // Connect to the MQTT broker.
        Serial.print( "Attempting MQTT connection..." );
        //if ( mqttClient.connect( clientID, mqttUserName, mqttPass ) )
        if ( mqttClient.connect( clientID) )

        {
            Serial.println( "Connected with Client ID:  " + String( clientID ) + " User "+ String( mqttUserName ) + " Pwd "+String( mqttPass ) );
           
        } else
        {
            Serial.print( "failed, rc = " );
            // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
            Serial.print( mqttClient.state() );
            Serial.println( " Will try again in 5 seconds" );
            delay( 5000 );
        }
    }
}

/**
 * Build a random client ID.
 *   clientID - Character array for output
 *   idLength - Length of clientID (actual length is one character longer for NULL)
 */

void getID(char clientID[], int idLength){
static const char alphanum[] ="0123456789"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";                        // For random generation of the client ID.

    // Generate client ID.
    for (int i = 0; i < idLength ; i++) {
        clientID[ i ] = alphanum[ random( 51 ) ];
    }
    clientID[ idLength ] = '\0';
    
}
void connectToLocal(){
    //connectedToLocal = True;
    WiFi.begin("NETGEAR98", pass);
}

void connectWifi()
{

      // scan for nearby networks:
    Serial.println("** Scan Networks **");
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1)
    { 
      Serial.println("Couldn't get a wifi connection");
      while(true);
    } 

      // print the list of networks seen:
    Serial.print("number of available networks:");
    Serial.println(numSsid);
    
    while (WiFi.status() != WL_CONNECTED ) {
      // print the network number and name for each network found:
      for (int thisNet = 0; thisNet<numSsid; thisNet++) {
        if(WiFi.SSID(thisNet) == "NETGEAR98"){
          if(numSsid == 1){
            connectToLocal();
          }
          
          continue;
        }
        WiFi.begin(WiFi.SSID(thisNet), pass);
        Serial.print(thisNet);
        Serial.print(") ");
        Serial.print(WiFi.SSID(thisNet));
        Serial.print("\tSignal: ");
        Serial.println(WiFi.RSSI(thisNet));
        for(int j = 0; j < 5; j ++){
          Serial.print("Status: ");
          Serial.println(WiFi.status());
          delay( 5000 );
          if (WiFi.status() == 3){
            Serial.println( "Connected" );
            //connectedToLocal = False;
            return;
          }
        }
        
      }
    }
}
