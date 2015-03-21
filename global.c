/*
 * File:   global.h
 * Author: pehladik
 *
 * Created on 21 avril 2011, 12:14
 */

#include "global.h"

RT_TASK tServeur;
RT_TASK tconnect;
RT_TASK tmove;
RT_TASK tenvoyer;
RT_TASK tBattery;       //<======= Thread battery
RT_TASK tMission;       //<======= Thread Mission


RT_MUTEX mutexEtat;
RT_MUTEX mutexMove;
RT_MUTEX mutexCptEchec; //*******
RT_MUTEX mutexStatus; //******* Mutex status

RT_SEM semConnecterRobot;
RT_SEM semBattery ;      //<======= Sem battery
RT_SEM semMission ;      //<======= Sem mission


RT_QUEUE queueMsgGUI;

int etatCommMoniteur = 1;
int etatCommRobot = 1;
int etatBattery = 0;         //<======= Etat battery
int cptEchec = 0;   //Compteur d'échec de com avec le robot, initialisé à 0 
DRobot *robot;
DMovement *move;
DServer *serveur;


int MSG_QUEUE_SIZE = 10;

int PRIORITY_TSERVEUR = 30;
int PRIORITY_TCONNECT = 20;
int PRIORITY_TMOVE = 10;
int PRIORITY_TENVOYER = 25;
int PRIORITY_TBAT=15;
int PRIORITY_TMission=9;