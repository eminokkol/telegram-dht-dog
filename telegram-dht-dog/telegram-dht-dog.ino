/* TELEGRAM BOT HUMIDITY AND TEMPERATURE WATCH DOG
This simple device can be used in incubators to follow humidity and temperature via Telegram.
Code uses:
  1-Universal Telegram Bot Library(https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot),
  2-Preferences library to save settings.
  3-NtpClient library to get time and date.
  4- DHT library to get data from DHT-22
Also, not a library but you can check emoji codes : https://github.com/ttacon/emoji/blob/master/emoji.go

Make your bot in Telegram via @BotFather and get your Bot Token. Use @myidbot to find your CHAT ID
CHAT ID is necessary so only you can use and control your bot, and through your bot; your esp-32 device.
If you want more than one user, you can find your group id and use it too.

CODE CONFIGURATION:
Enter your BOT-TOKEN, CHAT ID and wifi credentials. Define your local time in "configure_time" area.
Define "DEGREE" C for Celcius, F for Fahrenheit. Default is Celcius

NOTE: After resetting from your bot using /reset command; default values will be like this:
High temp : 39
Low temp: 34
High humidity : 100
Low humidity: 45
Notifications: closed
Notification interval: 10
Other values : 0

OPTIONAL:
For bot menu, you can go to BotFather->enter this command:  "/mybots" -> select bot-> edit bot -> edit commands 
and enter below without quotes:
"
start - Start bot
temperature - Current temp and humidity.   
timenow - Current date and time of device   
current_settings - Show calibration, low and high temp and humidity settings  
warn_temp_h - Enter high temp warning temperature
warn_temp_l - Enter low temp warning temperature
warn_humi_h - Enter high humidity warning value (%)  
warn_humi_l - Enter low humidity warning value (%)  
alarm_on - Switch on Notifications  
alarm_off - Switch off Notifications  
state - Notification status  
intervalt - Enter notification interval (minute) 
calibr_temp - Enter temp calibration value  
calibr_humi - Enter humidity calibration value 
reset - Reset all settings to default values. 
"

*/



#define BOTtoken "WRITE BOT-TOKEN"  // your Bot Token (Get from Botfather)
// Use @myidbot to find out the CHAT ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "write chat-id" 
#define configure_time() configTime(3 * 3600, 0, "tr.pool.ntp.org", "3.tr.pool.ntp.org"); /* get UTC time via NTP. Find your local time server from : www.ntppool.org */
#define DHT_SENSOR_PIN 21     // ESP32 pin GIOP21 connected to DHT22 sensor
#define DHT_SENSOR_TYPE DHT22
#define DEGREE "C"  // C for Celcius,  F for Fahrenheit.
#define check_interval 2500  // Check dht sensor interval (miliseconds)

#include <Preferences.h>
#include <NTPClient.h>
#include <DHT.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>  // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "wifi";
const char* password = "password";


Preferences preferences;
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
String humidity, temperature, timet, minutet, secondt;
bool alarmm = false;

bool bycc, bdc, byn, bdn;

unsigned long previousMillis = 0;
unsigned long interval = 30000;


#ifdef ESP8266
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
float humi, tempC;
int calib_temp;
int calib_humi;
int high_temp;
int low_temp;
int high_humi;
int intervalt;
int low_humi;
bool h_temp = false;
bool l_temp = false;
bool h_humi = false;
bool l_humi = false;
bool intrvl = false;
bool alarm_b = false;
bool resetting,reset_start = false;
bool calibrbool_tmp = false;
bool calibrbool_hm = false;
bool dhthealth = true;
String chat_id;
String text = "null";

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // Chat id of the requester
    chat_id = String(bot.messages[i].chat_id);
     // Check the sender of the message via chat_id. If it is not you, bot will not respond to commands, instead it will  answer as : "Unauthorized user"
    if (chat_id != CHAT_ID) 
    {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    text = bot.messages[i].text;
    Serial.println(text);
    preferences.begin("bot-memory", false);
    
    if (IsNumeric(text)) {


      if (h_temp) {
        high_temp = text.toInt();
        bot.sendMessage(chat_id, "High temp setting saved as : " + String(high_temp) + " \U00002705", "");
        h_temp = false;

        preferences.putUInt("high_temp", high_temp);
        delay(100);
      }
      if (h_humi) {
        high_humi = text.toInt();
        bot.sendMessage(chat_id, "High humidity setting saved as : " + String(high_humi) + " \U00002705", "");
        h_humi = false;

        preferences.putUInt("high_humi", high_humi);
        delay(100);
      }
      if (l_humi) {
        low_humi = text.toInt();
        bot.sendMessage(chat_id, "Low humidity setting saved as : " + String(low_humi) + " \U00002705", "");
        l_humi = false;

        preferences.putUInt("low_humi", low_humi);
        delay(100);
      }
      if (l_temp) {
        low_temp = text.toInt();
        bot.sendMessage(chat_id, "Low temp setting saved as : " + String(low_temp) + " \U00002705", "");
        l_temp = false;

        preferences.putUInt("low_temp", low_temp);
        delay(100);
      }
      if (intrvl) {
        intervalt = text.toInt();
        bot.sendMessage(chat_id, "Notification interval setting saved as : " + String(intervalt) + " minute. \U00002705", "");


        preferences.putUInt("intervalt", intervalt);
        delay(100);


        intrvl = false;
      }
    }
    
    if (calibrbool_tmp) {
      calib_temp = text.toInt();
      bot.sendMessage(chat_id, "Temp calibration setting saved as : " + String(calib_temp) + " \U00002705", "");
      calibrbool_tmp = false;
     
      preferences.putUInt("calib_temp", calib_temp);
      delay(100);
    }
    if (calibrbool_hm) {
      calib_humi = text.toInt();
      bot.sendMessage(chat_id, "Humidity calibration setting saved as : " + String(calib_humi) + " \U00002705", "");
      calibrbool_hm = false;

      preferences.putUInt("calib_humi", calib_humi);
      delay(100);
    }
    if (resetting) {
      if (text== "y")
      { bot.sendMessage(chat_id, " Resetting device to default values... \U000026a0", "");
      reset_start=true;
      }
      else
      {
      bot.sendMessage(chat_id, " Device reset cancelled.", "");
      reset_start=false;
      }
    }
  
    


    String from_name = bot.messages[i].from_name;
    // Bot Wellcome message, the answer of  "/start" command:
    if (text == "/start") {
      String welcome = "Wellcome, " + from_name + ".\n";
      welcome += "Use commands below to control the device:\n\n";
      welcome += "/temperature :Current temp and humidity.  \n";
      welcome += "/timenow : Current date and time of device  \n";
      welcome += "/current_settings : Show calibration, low and high temp and humidity settings \n";
      welcome += "/warn_temp_h : \U00002699 Enter high temp warning temperature  \n";
      welcome += "/warn_temp_l : \U00002699 Enter low temp warning temperature  \n";
      welcome += "/warn_humi_h : \U00002699 Enter high humidity warning value (%) \n";
      welcome += "/warn_humi_l : \U00002699 Enter low humidity warning value (%) \n";
      welcome += "/alarm_on : Switch on Notifications \n";
      welcome += "/alarm_off : Switch off Notifications \n";
      welcome += "/state : Notification status \n";
      welcome += "/intervalt : \U000023F3 Enter notification interval (minute)\n";
      welcome += "/calibr_temp : \U00002699 Enter temp calibration value \n";
      welcome += "/calibr_humi : \U00002699 Enter humidity calibration value \n";
      welcome += "/reset : \U0001f504 Reset all settings to default values. \n";

      bot.sendMessage(chat_id, welcome, "");
      if (!dhthealth)
        bot.sendMessage(chat_id, "\U000026a0 WARNING! Failed to read from sensor!  \U000026a0", "");
    }
    if (text == "/current_settings") {

      bot.sendMessage(chat_id, "High temp setting is: " + String(high_temp), "");
      bot.sendMessage(chat_id, "Low temp setting is: " + String(low_temp), "");
      bot.sendMessage(chat_id, "High humidity setting is : " + String(high_humi), "");
      bot.sendMessage(chat_id, "Low humidity setting is : " + String(low_humi), "");
      bot.sendMessage(chat_id, "Temp calibration setting is : " + String(calib_temp), "");
      bot.sendMessage(chat_id, "Humidity calibration setting is : " + String(calib_humi), "");
      if (alarmm) {
        bot.sendMessage(chat_id, "Notification status : Open ", "");

      } else {
        bot.sendMessage(chat_id, "Notification status : Closed ", "");
      }
      bot.sendMessage(chat_id, "In extreme conditions, notifications will repeat every " + String(intervalt) + " minutes.", "");
    }
    if (text == "/calibr_temp") {

      bot.sendMessage(chat_id, " \U00002699  Enter temp calibration value :", "");
      calibrbool_tmp = true;
    }
    if (text == "/calibr_humi") {

      bot.sendMessage(chat_id, " \U00002699 Enter humidity calibration value  :", "");
      calibrbool_hm = true;
    }
    if (text == "/intervalt") {

      bot.sendMessage(chat_id, " \U000023F3 Enter notification interval (minute) :", "");
      intrvl = true;
    }
    if (text == "/warn_temp_h") {

      bot.sendMessage(chat_id, "Enter high temp warning temperature :", "");
      h_temp = true;
    }
    if (text == "/warn_humi_h") {

      bot.sendMessage(chat_id, "Enter high humidity warning value (%):", "");
      h_humi = true;
    }
    if (text == "/warn_humi_l") {

      bot.sendMessage(chat_id, "Enter low humidity warning value (%):", "");
      l_humi = true;
    }
    if (text == "/warn_temp_l") {

      bot.sendMessage(chat_id, "Enter low temp warning temperature :", "");
      l_temp = true;
    }
    if (text == "/timenow") {
      timecheck();
      bot.sendMessage(chat_id, timet, "");
    }
    if (text == "/alarm_on") {
      bot.sendMessage(chat_id, "Notifications switched ON", "");
      alarmm = true;
      preferences.begin("bot-memory", false);
      preferences.putBool("alarm_b", alarmm);
      preferences.end();
      delay(100);
    }

    if (text == "/alarm_off") {
      bot.sendMessage(chat_id, "Notifications switched OFF", "");
      alarmm = false;
      preferences.begin("bot-memory", false);
      preferences.putBool("alarm_b", alarmm);
      preferences.end();
      delay(100);
    }

    if (text == "/state") {
      if (alarmm) {
        bot.sendMessage(chat_id, "Notifications : ON", "");
      } else {
        bot.sendMessage(chat_id, "Notifications : OFF", "");
      }
    }




    if (text == "/temperature") {
      timecheck();
      bot.sendMessage(chat_id, timet, "");
      readdht;
      if (DEGREE == "C")
      bot.sendMessage(chat_id, "Current Temperature : " + temperature+" °C", "");
      else
      bot.sendMessage(chat_id, "Current Temperature : " + temperature+" °F", "");
      bot.sendMessage(chat_id, "Current humidity : " + humidity+" %", "");
      checkall();
    }
    if (text == "/reset") {
    bot.sendMessage(chat_id, "\U000026a0 Are you sure to reset to default values? y/n ", "");
     resetting = true;
    }
  }
}

boolean IsNumeric(String str) {

  if (str.length()) {
    Serial.println(str);
    for (char i = 0; i < str.length(); i++) {
      if (!(isDigit(str.charAt(i)) || str.charAt(i) == '.')) {
        return false;
      }
    }
    return true;
  } else {
    return false;
  }
}
void reset_all()
{
  preferences.begin("bot-memory", false);
  delay(200);
  preferences.clear();
  delay(200);
  preferences.putUInt("low_temp", 34);
  delay(100);
	preferences.putUInt("low_humi", 45);
  delay(100);
	preferences.putUInt("high_humi", 100);
  delay(100);
	preferences.putUInt("high_temp", 39);
	delay(100);
  preferences.putUInt("intervalt", 10);
  delay(100);
  preferences.end();
  delay(300);
  Serial.println("Memory cleaned- resetting...");
  bot.sendMessage(chat_id, "Default values saved. Resetting device... \U0001f504", "");
   text="delete";
  delay(300);
  
  ESP.restart();
  

}

void readdht()  // Function to read sensor values.
{
  humidity = "0";
  temperature = "0";

  // read humidity
  humi = dht_sensor.readHumidity();
  if (DEGREE == "C")
  // read temperature in Celsius
  tempC = dht_sensor.readTemperature();
  else
  {
    // temperature in Fahrenheit
  tempC = dht_sensor.readTemperature();
  tempC = (tempC*9/5) + 32;
  }
  // Adjust temp and humidity values with calibration values
  tempC = tempC + calib_temp;
  humi = humi + calib_humi;

  // check whether the reading is successful or not
  if (isnan(tempC) || isnan(humi)) {
    Serial.println("Failed to read from DHT sensor!");
    dhthealth = false;
  } else {
    dhthealth = true;
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.print("%");

    Serial.print("  |  ");

    Serial.print("Temperature: ");
    Serial.print(tempC);
    if (DEGREE == "C")
    Serial.println("°C  ~  ");
    else
    Serial.println("°F  ~  ");
    humidity = humi;
    temperature = tempC;
  }
}

void setup() {
  Serial.begin(115200);
  dht_sensor.begin();  // initialize the DHT sensor
  humidity = "0";
  temperature = "0";
  bycc = true;
  bdc = true;
  byn = true;
  bdn = true;
 #ifdef ESP8266
  configure_time();
  client.setTrustAnchors(&cert);                                   /* Add root certificate for api.telegram.org */
 #endif

  preferences.begin("bot-memory", false);
  alarmm = preferences.getBool("alarm_b", false);

  high_temp = preferences.getUInt("high_temp", 0);
  delay(100);

  high_humi = preferences.getUInt("high_humi", 0);
  delay(100);

  low_humi = preferences.getUInt("low_humi", 0);
  delay(100);

  low_temp = preferences.getUInt("low_temp", 0);
  delay(100);

  intervalt = preferences.getUInt("intervalt", 0);
  delay(100);

  calib_temp = preferences.getUInt("calib_temp", 0);
  delay(100);

  calib_humi = preferences.getUInt("calib_humi", 0);
  delay(100);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 #ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org
 #endif
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
    configure_time();
  preferences.end();
  timecheck();
}
void timecheck()  //Get time, date, day of week 
{
  timet = " ";
  minutet = " ";
  secondt = " ";
  time_t now = time(nullptr);
  while (now < 24 * 3600) 
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  time_t rawtime;
  struct tm* timeinfo;
  char buffer[80], buffer2[40], buffer3[40], buffer4[40];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer2, 40, "%A", timeinfo);  //day of week
  strftime(buffer4, 40, "%S", timeinfo);  //seconds:  00-59
  strftime(buffer3, 40, "%M", timeinfo);  //minutes:  00-59
  String day_w(buffer2);
  strftime(buffer, 80, "\U0001f4c6 Date: %e/%m/%G , \U0001f564 Time: %T , Day :", timeinfo);

  minutet = buffer3;
  secondt = buffer4;

  timet = buffer + day_w;
}

void loop() 
{
    if (reset_start) {
      reset_all();
      delay(300);
      resetting=false;
      reset_start=false;
    }


  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) 
  {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }


  if (millis() > lastTimeBotRan + botRequestDelay) 
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) 
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  readdht();
  delay(check_interval);
  timecheck();

  if (intervalt <= 0) intervalt = 10;


  if (minutet.toInt() % intervalt == 0)  // If time is ok from notification interval settings
  {
    if (alarmm)  // If notification setting is ON
    {

      if (bycc || bdc || byn || bdn) {






        if (tempC >= high_temp) 
        {
          if (bycc) 
          {
            bot.sendMessage(chat_id, "\U0001F525  WARNING! High temp : " + temperature, "");
            bycc = false;
          }
        }
        if (tempC <= low_temp) 
        {
          if (bdc) 
          {
            bot.sendMessage(chat_id, "\U00002744 WARNING! Low temp: " + temperature, "");
            bdc = false;
          }
        }
        if (humi >= high_humi) 
        {
          if (byn) 
          {
            bot.sendMessage(chat_id, "\U0001F4A7 WARNING! High humidity: " + humidity, "");
            byn = false;
          }
        }
        if (humi <= low_humi) 
        {
          if (bdn) 
          {
            bot.sendMessage(chat_id, "\U00002600 WARNING! Low humidty: " + humidity, "");
            bdn = false;
          }
        }
      }
    }
  }
  else 
  {
    bycc = true;
    bdc = true;
    byn = true;
    bdn = true;
  }
}
void checkall()  //Check all conditions
{
  if (!dhthealth)
    bot.sendMessage(chat_id, "\U000026a0 WARNING! Failed to read from sensor!  \U000026a0", "");
  else {
    if (tempC >= high_temp)
      { if (DEGREE == "C")
      bot.sendMessage(chat_id, "\U0001F525 WARNING! High temp: " + temperature+" °C", "");
      else
      bot.sendMessage(chat_id, "\U0001F525 WARNING! High temp: " + temperature+" °F", "");
      }
    if (tempC <= low_temp)
    { if (DEGREE == "C")
      bot.sendMessage(chat_id, "\U00002744 WARNING! Low temp: " + temperature+" °C", "");
      else
      bot.sendMessage(chat_id, "\U00002744 WARNING! Low temp: " + temperature+" °F", "");
    }
    if (humi >= high_humi)
      bot.sendMessage(chat_id, "\U0001F4A7 WARNING! High humidity: " + humidity+" %", "");
    if (humi <= low_humi)
      bot.sendMessage(chat_id, "\U00002600 WARNING! Low humidty: " + humidity+" %", "");
    if (humi > low_humi && humi < high_humi && tempC > low_temp && tempC < high_temp)
      bot.sendMessage(chat_id, "\U0001f44d  All Values are NORMAL \U0001f44d ", "");
  }
}
