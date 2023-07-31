# **TELEGRAM BOT HUMIDITY AND TEMPERATURE WATCH DOG**

Read dht sensor values through telegram bot from esp-32 device. Enter high and low values to get notification from bot. Also other settings to calibrate.

This simple device can be used in incubators to follow humidity and temperature via Telegram.



## Code uses:    

  1-Universal Telegram Bot Library(https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)
	
  2-Preferences library to save settings.
	
  3-NtpClient library to get time and date.
	
  4- DHT library to get data from DHT-22
	
  Also, not a library but you can check emoji codes : https://github.com/ttacon/emoji/blob/master/emoji.go


Make your bot in Telegram via @BotFather (You can find a lot of info from web how to make a bot) , and get your Bot Token. Use @myidbot to find your CHAT ID. To @myidbot , write "/getid" to get your chat-id.
CHAT ID is necessary; so only you can use and control your bot, and through your bot; your esp-32 device.
If you want more than one user, you can find your group id and use it too. (Add @myidbot to a group or channel you want to get the ID of, and send "/getgroupid" in the group)

##Diagram is quite simple:    


![diagram](https://github.com/eminokkol/telegram-dht-dog/assets/141066897/716decd4-eb99-416a-b369-4d8305f2826f)


## CODE CONFIGURATION:    
Enter your BOT-TOKEN, CHAT ID and wifi credentials. Define your local time in "configure_time" area.
Define "DEGREE" C for Celcius, F for Fahrenheit. Default is Celcius

## NOTE:    
There is a "reset" command to reset to default values. After resetting from your bot using /reset command; default values will be like this:
High temp : 39
Low temp: 34
High humidity : 100
Low humidity: 45
Notifications: closed
Notification interval: 10
Other values : 0

## OPTIONAL:        
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


![telegram1](https://github.com/eminokkol/telegram-dht-dog/assets/141066897/3e95ed52-b136-44a2-8c46-d1704e6c9a19)

![telegram2](https://github.com/eminokkol/telegram-dht-dog/assets/141066897/b200ec09-afb3-4066-acd7-3458763493d5)

## Optional Menu:    

![telegram3](https://github.com/eminokkol/telegram-dht-dog/assets/141066897/48cd8e84-5b6a-403d-b2b4-e00402b8d693)
