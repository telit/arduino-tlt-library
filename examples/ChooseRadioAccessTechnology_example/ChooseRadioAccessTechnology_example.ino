/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    - TLTMDM.h

  @brief
    Sample test of the use of Radio Access Technology selection for Arduino with TLTMDM.\n

  @details

    This sketch shows how to select a network operator and register the module via the ME310 library.\n
    You can choose among CAT-M1, NB-IoT or a combination of both.\n
    LTE technology supported:
      - CAT-M1 (0)
      - NB-IoT (1)
      - CAT-M1 (preferred) and NB-IoT (2)
      - CAT-M1 and NB-IoT (preferred) (3)

  @version
    1.1.0

  @note

  @author
    Cristina Desogus

  @date
    09/23/2021
 */

#include <TLTMDM.h>

ME310 myME310;

void setup() {
  Serial.begin(115200);
  myME310.begin(115200);
  delay(1000);
  myME310.powerOn(ON_OFF);
  delay(5000);
  for (int i = 0; i < 80; i++) Serial.print("*");
  Serial.println();
  Serial.println("This sketch allows you to select your preferred");
  Serial.println("4G Narrowband Radio Access Technology (RAT).");
  Serial.println();
  Serial.println("You can choose among CAT-M1, NB-IoT or a combination of both.");
  Serial.println();
  Serial.println("Selecting JUST ONE technology will speed up the modem's network registration A LOT!");
  Serial.println();
  Serial.println("The chosen configuration will be saved to modem's internal memory");
  Serial.println("and will be preserved through MKR NB 1500 sketch uploads.");
  Serial.println();
  Serial.println("In order to change the RAT, you will need to run this sketch again.");
  for (int i = 0; i < 80; i++) Serial.print("*");

  Serial.println();
  Serial.println();
  Serial.println("Please choose your Radio Access Technology:");
  Serial.println();
  Serial.println("    0 - CAT M1 only");
  Serial.println("    1 - NB IoT only");
  Serial.println("    2 - CAT M1 preferred, NB IoT as failover (default)");
  Serial.println("    3 - NB IoT preferred, CAT M1 as failover");
  Serial.println();
}

void loop() {
  String uratChoice;

  Serial.print("> ");

  Serial.setTimeout(0);
  while (Serial.available() == 0);
  String uratInput = Serial.readStringUntil('\n');
  uratInput.trim();
  int urat = uratInput.toInt();
  Serial.println(urat);


  switch (urat) {
    case 0:
      uratChoice = "0";
      break;
    case 1:
      uratChoice = "1";
      break;
    case 2:
      uratChoice = "2";
      break;
    case 3:
      uratChoice = "3";
      break;
    default:
      Serial.println("Invalid input. Please, retry.");
      return;
  }

  setRAT(uratChoice);
  apply();

  Serial.println();
  Serial.println("Radio Access Technology selected.");
  Serial.println("Now you can upload your 4G application sketch.");
  exit(0);
}

bool setRAT(String choice)
{
  String response;

  Serial.print("Disconnecting from network: ");
  myME310.operator_selection(2);
  Serial.println("done.");

  Serial.print("Setting Radio Access Technology: ");
  myME310.select_iot_technology(choice.toInt());
  Serial.println("done.");

  return true;
}

bool apply()
{
  ME310::return_t rc;
  Serial.print("Applying changes and saving configuration: ");
  myME310.set_phone_functionality(1);
  delay(5000);

  do
  {
    delay(1000);
    rc = myME310.attention();
  } while (rc != ME310::RETURN_VALID);

  Serial.println("done.");

  return true;
}
