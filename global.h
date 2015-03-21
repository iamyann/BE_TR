/* 
 * File:   global.h
 * Author: pehladik
 *
 * Created on 12 janvier 2012, 10:11
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#include "includes.h"

/* @descripteurs des tâches */
extern RT_TASK tServeur;
extern RT_TASK tconnect;
extern RT_TASK tmove;
extern RT_TASK tenvoyer;
extern RT_TASK tBattery;       //<======= Thread battery
extern RT_TASK tMission;       //<======= Thread Mission

/* @descripteurs des mutex */
extern RT_MUTEX mutexEtat;
extern RT_MUTEX mutexMove;
extern RT_MUTEX mutexCptEchec;
extern RT_MUTEX mutexStatus; //******* Mutex status


/* @descripteurs des sempahore */
extern RT_SEM semConnecterRobot;
extern RT_SEM semBattery ;      //<======= Sem battery
extern RT_SEM semMission ;      //<======= Sem mission

/* @descripteurs des files de messages */
extern RT_QUEUE queueMsgGUI;

/* @variables partagées */
extern int etatCommMoniteur;
extern int etatCommRobot;
extern int etatBattery;         //<======= Etat battery
extern int cptEchec; 
extern DRobot *robot;
extern DMovement *move;
extern DServer *serveur;


/* @constantes */
extern int MSG_QUEUE_SIZE;
extern int PRIORITY_TSERVEUR;
extern int PRIORITY_TCONNECT;
extern int PRIORITY_TMOVE;
extern int PRIORITY_TENVOYER;
extern int PRIORITY_TBAT;
extern int PRIORITY_TMission;


#endif	/* GLOBAL_H */