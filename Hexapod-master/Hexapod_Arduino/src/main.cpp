/* 
 * GRO 400 - Conception d'un robot articulé
 * Auteurs: MANUS
 * date: 25 janvier 2022
*/

/*------------------------------ Librairies ---------------------------------*/
#include <LibS3GRO.h>
#include <ArduinoJson.h>
/*------------------------------ Constantes ---------------------------------*/
using namespace std;

#define BAUD                            115200    // Frequence de transmission serielle
#define UPDATE_PERIODE                  100     // Periode (ms) d'envoie d'etat general

//-----------------------------PIN SERVO-----------------------------------
#define MAGPIN                          32        // Port numerique pour electroaimant J-18

/*---------------------------- variables globales ---------------------------*/

MegaServo servo_;                                 // objet servomoteur
//IMU9DOF imu_;                                     // objet imu central inertielle

volatile bool shouldSend_ =             false;    // drapeau prêt à envoyer un message
volatile bool shouldRead_ =             false;    // drapeau prêt à lire un message
volatile bool shouldPulse_ =            false;    // drapeau pour effectuer un pulse
volatile bool isInPulse_ =              false;    // drapeau pour effectuer un pulse

SoftTimer timerSendMsg_;                          // chronometre d'envoie de messages
SoftTimer timerPulse_;                            // chronometre pour la duree d'un pulse

uint16_t pulseTime_ =                   0;        // temps dun pulse en ms

int time =                              0;        //timer pour la loop
int32_t compteur_encodeur =             0;        //Encodeur du moteur

//------------------------------ VARIABLES-----------------------------------------------

// double fonction =                       0.0;      //fonction de tests dans la loop
// double goal_voulu_angle =               0.0;      //Permet de dire l'angle voulue
// double position_depart =                0.03;      //Permet de savoir la position initial du robot
// double position_obstacle =              0.75;      //Permet de savoir la position de l'obstacle
// double position_arrive =                 1.2;      //Permet de savoir la position du dépot du sapin
// double distance_ins =                   0.0;      //Permet de savoir la distance instantanné du véhicule pour calculer la vitesse
// double hauteur_obstacle =               0.0;      //Permet de savoir la hauteur de l'obstacle
// double distance_old =                   0.0;      //Permet de savoir la distance précédente pour le calcul de la vitesse
// double temps_ins =                      0.0;      //Permet de savoir le temps instantanné du véhicule pour calculer la vitesse
// double temps_old =                      0.0;      //Permet de savoir le temps précédente pour le calcul de la vitesse

double temps1 =                         0.0;
double temps2 =                         0.0;
float pulsePWM_ =                       0.1;      // Amplitude de la tension au moteur pour la position[-1,1]
float pulsePWM_angle =                  0.1;      //Amplitude de la tension au moteur pour l'angle [-1,1]
float Axyz[3];                                    // tableau pour accelerometre
float Gxyz[3];                                    // tableau pour giroscope
float Mxyz[3];                                    // tableau pour magnetometre
float Potentio_zero =                   0.0;      //permet de savoir la valeur initiale du pendule
float angle_pendule =                   0.0;      //Permet de savoir l'agle actuelle du pendule
float cur_pos =                         0.0;      //Permet de savoir la position en temps réelle du pendule
float cur_vel =                         0.0;      //Permet de savoir la vitesse en temps réelle du pendule
float cur_angle =                       0.0;      //Permet de savoir l'angle en temps réelle du pendule
float pwm_correction  =                 0.0;
int i =                                   0;      


/*------------------------- Prototypes de fonctions -------------------------*/
void timerCallback();
void startPulse();
void endPulse();
void sendMsg(); 
void readMsg();
void serialEvent();
void digitalWrite(uint8_t pin, uint8_t val);


// Caller les fonctions ICI 
//ex:
// double Calculangle();
// float reduce_angle();
// double vitesse();



/*---------------------------- fonctions "Main" -----------------------------*/

void setup() {
  Serial.begin(BAUD);               // initialisation de la communication serielle
  //imu_.init();                      // initialisation de la centrale inertielle
  
  // Chronometre envoie message
  timerSendMsg_.setDelay(UPDATE_PERIODE);
  timerSendMsg_.setCallback(timerCallback);
  timerSendMsg_.enable();

  // Chronometre duration pulse
  timerPulse_.setCallback(endPulse);

}


// Boucle principale (infinie) 
void loop() {

  if(shouldRead_){
    readMsg();
  }
  if(shouldSend_){
    sendMsg();
  }
  if(shouldPulse_){
    startPulse();
  }

//----------------------FAIRE SWITCH CASE ICI-------------------------------


  // Mise à jour des chronometres
  timerSendMsg_.update();
  timerPulse_.update();
}

/*---------------------------Definition de fonctions ------------------------*/

void serialEvent(){shouldRead_ = true;}

void timerCallback(){shouldSend_ = true;}

void startPulse(){
  /* Demarrage d'un pulse */
  timerPulse_.setDelay(pulseTime_);
  timerPulse_.enable();
  timerPulse_.setRepetition(1);
  shouldPulse_ = false;
  isInPulse_ = true;
}

void endPulse(){
  /* Rappel du chronometre */
  timerPulse_.disable();
  isInPulse_ = false;
}

void sendMsg(){
  /* Envoit du message Json sur le port seriel */
  StaticJsonDocument<500> doc;
  // Elements du message

  doc["time"]      = (millis()/1000.0);
  doc["pulsePWM"]  = pulsePWM_;
  doc["cur_vel"]   = cur_vel;
  doc["cur_pos"]   = cur_pos;
  doc["cur_angle"] = cur_angle;

  // Serialisation
  serializeJson(doc, Serial);
  // Envoit
  Serial.println();
  shouldSend_ = false;
}

void readMsg(){
  // Lecture du message Json
  StaticJsonDocument<500> doc;
  JsonVariant parse_msg;

  // Lecture sur le port Seriel
  DeserializationError error = deserializeJson(doc, Serial);
  shouldRead_ = false;

  // Si erreur dans le message
  if (error) {
    //Serial.print("deserialize() failed: ");
    //Serial.println(error.c_str());
    return;
  }
  
  //À LAISSER DANS LA PREMIÈRE FENETRE QT------------------------------------------------------------À FAIRE-------------------------------------

  // Analyse des éléments du message message
  parse_msg = doc["pulsePWM"];
  if(!parse_msg.isNull()){
     pulsePWM_ = doc["pulsePWM"].as<float>();
  }

  parse_msg = doc["pulseTime"];
  if(!parse_msg.isNull()){
     pulseTime_ = doc["pulseTime"].as<float>();
  }

  parse_msg = doc["pulse"];
  if(!parse_msg.isNull()){
     shouldPulse_ = doc["pulse"];
  }

  //-----------------------AJOUTER FONCTIONS DE MOUVEMENT ICI-----------------------------------------------------
}