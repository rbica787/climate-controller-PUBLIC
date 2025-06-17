#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <ESP_Mail_Client.h>

#define WIFI_SSID "REPLACE_WITH_YOUR_SSID"
#define WIFI_PASSWORD "REPLACE_WITH_YOUR_PASSWORD"

/** The smtp host name e.g. smtp.gmail.com for GMail or smtp.office365.com for Outlook or smtp.mail.yahoo.com */
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "YOUR_EMAIL@XXXX.com"
#define AUTHOR_PASSWORD "YOUR_EMAIL_APP_PASS"

/* Recipient's email*/
#define RECIPIENT_EMAIL "RECIPIENTE_EMAIL@XXXX.com"

/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

//#define DEBUG1

#define D1_pin 5
#define HH_pin 4

int D1_fault_signal = 0;
int HH_fault_signal = 0;

boolean D1_one_shot = 0;
boolean HH_one_shot = 0;

boolean HH_delay_met = 0;
boolean D1_delay_met = 0;

unsigned long currentmillis;
unsigned long last_HumidityEmail_Time = 0;
unsigned long last_D1_Email_Time = 0;
unsigned long HighHumidity_emailDelay = 1;    //delay time in hours
unsigned long DehumidifierFault_emailDelay = 4;    //delay time in hours


void setup(){
  HighHumidity_emailDelay = HighHumidity_emailDelay * 3600000;  // this converts from hours to milliseconds
  DehumidifierFault_emailDelay = DehumidifierFault_emailDelay * 3600000;  // this converts from hours to milliseconds
  Serial.begin(115200);
  Serial.println();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  pinMode(D1_pin, INPUT);
  pinMode(HH_pin, INPUT);
}






void loop(){
  currentmillis = millis();
  D1_fault_signal = digitalRead(D1_pin);
  HH_fault_signal = digitalRead(HH_pin);
  HH_delay_met = (currentmillis - last_HumidityEmail_Time) >= HighHumidity_emailDelay;
  D1_delay_met = (currentmillis - last_D1_Email_Time) >= DehumidifierFault_emailDelay;
  
  #ifdef DEBUG1
    Serial.println("");
    Serial.println("");
    Serial.println("________________________________________________________________________________________________________________________________________________");
    Serial.println("");
    Serial.println("");
    Serial.println("D1_fault_signal && (!D1_one_shot || D1_delay_met)");
    Serial.print(D1_fault_signal);
    Serial.print(" && (");
    Serial.print(!D1_one_shot);
    Serial.print(" || ");
    Serial.print(D1_delay_met);
    Serial.println(")");
    Serial.println("");

    Serial.print("D1_delay_met = (currentmillis - last_HumidityEmail_Time) >= HighHumidity_emailDelay ------>   ");
    Serial.print(currentmillis - last_D1_Email_Time);
    Serial.print(" >= ");
    Serial.print(DehumidifierFault_emailDelay);
    Serial.println("");
    Serial.println("");
    Serial.println("");

    
    Serial.println("HH_fault_signal && (!HH_one_shot || HH_delay_met)");
    Serial.print(HH_fault_signal);
    Serial.print(" && (");
    Serial.print(!HH_one_shot);
    Serial.print(" || ");
    Serial.print(HH_delay_met);
    Serial.println(")");
    Serial.println("");
    
    Serial.print("HH_delay_met = (currentmillis - last_HumidityEmail_Time) >= HighHumidity_emailDelay ------>   ");
    Serial.print(currentmillis - last_HumidityEmail_Time);
    Serial.print(" >= ");
    Serial.print(HighHumidity_emailDelay);
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("________________________________________________________________________________________________________________________________________________");
    Serial.println("");
    Serial.println("");
    delay(2000);
  #endif

if(D1_fault_signal && (!D1_one_shot || D1_delay_met)){
  sendEmail("SMALL ROOM ALERT - DEHUMIDIFIER FULL", "Dehumidifier 1 full - Empty Reservoir");
  D1_one_shot = 1;
  last_D1_Email_Time = currentmillis;
}

if(HH_fault_signal && (!HH_one_shot || HH_delay_met)){
  sendEmail("SMALL ROOM ALERT - HIGH HUMIDITY", "HUMIDITY LEVEL HIGH - Humidity level above threshold deadband");
  HH_one_shot = 1;
  last_HumidityEmail_Time = currentmillis;
}


  delay(2000);  //for stability
}









/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)
      
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}







void sendEmail(String subject, String body){
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  MailClient.networkReconnect(true);
  smtp.debug(1);
  smtp.callback(smtpCallback);

  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  SMTP_Message message;
  message.sender.name = F("ESP");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = subject;
  message.addRecipient(F("Rick"), RECIPIENT_EMAIL);

  message.text.content = body.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  if (!smtp.connect(&config)){
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s",
                    smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()){
    Serial.println("\nNot yet logged in.");
  } else {
    Serial.println(smtp.isAuthenticated() ? "\nSuccessfully logged in." : "\nConnected with no Auth.");
  }

  if (!MailClient.sendMail(&smtp, &message))
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s",
                    smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
}
