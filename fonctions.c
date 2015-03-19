#include "fonctions.h"

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size);

void envoyer(void * arg) {
    DMessage *msg;
    int err;

    while (1) {
        rt_printf("tenvoyer : Attente d'un message\n");
        if ((err = rt_queue_read(&queueMsgGUI, &msg, sizeof (DMessage), TM_INFINITE)) >= 0) {
            rt_printf("tenvoyer : envoi d'un message au moniteur\n");
            serveur->send(serveur, msg);
            msg->free(msg);
        } else {
            rt_printf("Error msg queue write: %s\n", strerror(-err));
        }
    }
}

void connecter(void * arg) {
    int status;
    DMessage *message;

    rt_printf("tconnect : Debut de l'exécution de tconnect\n");

    while (1) {
        rt_printf("tconnect : Attente du sémarphore semConnecterRobot\n");
        rt_sem_p(&semConnecterRobot, TM_INFINITE);
        rt_printf("tconnect : Ouverture de la communication avec le robot\n");
        status = robot->open_device(robot);

        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        etatCommRobot = status;
        rt_mutex_release(&mutexEtat);

        if (status == STATUS_OK) {
            status = robot->start_insecurely(robot);
            if (status == STATUS_OK){
                rt_printf("tconnect : Robot démarrer\n");
                rt_sem_v(&semBattery);  
            }
        }

        message = d_new_message();
        message->put_state(message, status);

        rt_printf("tconnecter : Envoi message\n");
        message->print(message, 100);

        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
            message->free(message);
        }
    }
}

void communiquer(void *arg) {
    DMessage *msg = d_new_message();
    int var1 = 1;
    int num_msg = 0;

    rt_printf("tserver : Début de l'exécution de serveur\n");
    serveur->open(serveur, "8000");
    rt_printf("tserver : Connexion\n");

    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    etatCommMoniteur = 0;
    rt_mutex_release(&mutexEtat);

    while (var1 > 0) {
        rt_printf("tserver : Attente d'un message\n");
        var1 = serveur->receive(serveur, msg);
        num_msg++;
        if (var1 > 0) {
            switch (msg->get_type(msg)) {
                case MESSAGE_TYPE_ACTION:
                    rt_printf("tserver : Le message %d reçu est une action\n",
                            num_msg);
                    DAction *action = d_new_action();
                    action->from_message(action, msg);
                    switch (action->get_order(action)) {
                        case ACTION_CONNECT_ROBOT:
                            rt_printf("tserver : Action connecter robot\n");
                            rt_sem_v(&semConnecterRobot);
                            break;
                    }
                    break;
                case MESSAGE_TYPE_MOVEMENT:
                    rt_printf("tserver : Le message reçu %d est un mouvement\n",
                            num_msg);
                    rt_mutex_acquire(&mutexMove, TM_INFINITE);
                    move->from_message(move, msg);
                    move->print(move);
                    rt_mutex_release(&mutexMove);
                    break;
            }
        }
    }
}

void deplacer(void *arg) {
    int status = 1;
    int gauche;
    int droite;
    DMessage *message;

    rt_printf("tmove : Debut de l'éxecution de periodique à 1s\n");
    rt_task_set_periodic(NULL, TM_NOW, 1000000000);

    while (1) {
        /* Attente de l'activation périodique */
        rt_task_wait_period(NULL);
        rt_printf("tmove : Activation périodique\n");

        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        status = etatCommRobot;
        rt_mutex_release(&mutexEtat);

        if (status == STATUS_OK) {
            rt_mutex_acquire(&mutexMove, TM_INFINITE);
            switch (move->get_direction(move)) {
                case DIRECTION_FORWARD:
                    gauche = MOTEUR_ARRIERE_LENT;
                    droite = MOTEUR_ARRIERE_LENT;
                    break;
                case DIRECTION_LEFT:
                    gauche = MOTEUR_ARRIERE_LENT;
                    droite = MOTEUR_AVANT_LENT;
                    break;
                case DIRECTION_RIGHT:
                    gauche = MOTEUR_AVANT_LENT;
                    droite = MOTEUR_ARRIERE_LENT;
                    break;
                case DIRECTION_STOP:
                    gauche = MOTEUR_STOP;
                    droite = MOTEUR_STOP;
                    break;
                case DIRECTION_STRAIGHT:
                    gauche = MOTEUR_AVANT_LENT;
                    droite = MOTEUR_AVANT_LENT;
                    break;
            }
            rt_mutex_release(&mutexMove);

            status = robot->set_motors(robot, gauche, droite);

            if (status != STATUS_OK) {
                rt_mutex_acquire(&mutexEtat, TM_INFINITE);
                etatCommRobot = status;
                rt_mutex_release(&mutexEtat);

                message = d_new_message();
                message->put_state(message, status);

                rt_printf("tmove : Envoi message\n");
                if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                    message->free(message);
                }
            }
        }
    }
}

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size) {
    void *msg;
    int err;

    msg = rt_queue_alloc(msgQueue, size);
    memcpy(msg, &data, size);

    if ((err = rt_queue_send(msgQueue, msg, sizeof (DMessage), Q_NORMAL)) < 0) {
        rt_printf("Error msg queue send: %s\n", strerror(-err));
    }
    rt_queue_free(&queueMsgGUI, msg);

    return err;
}

/**********Fonction qui permet de connaitre le statut de communication avec le robot *************/

int verifier_com_robot(int status) {
    
    rt_printf("Vérification de l'état de communication avec le robot. \n");
    int var=0; //Compteur d'échec pour la vérification de la perte de com avec le robot
    DMessage *message;

    rt_printf("STATUS_OK = %d \n", STATUS_OK);
    //rt_printf("STATUS = %d \n", status);

    if (status != STATUS_OK) 
    {
        rt_printf("La communication avec le robot n'est pas OK!\n");
        /* On prend le mutex du compteur d'échec avant d'incrémenter le compteur */
        rt_mutex_acquire(&mutexCptEchec, TM_INFINITE);
        cptEchec += cptEchec;
        var = cptEchec;
        /* On libère le mutex du compteur d'échec */
        rt_mutex_release(&mutexCptEchec);
        rt_printf("Status com avec le robot, compteur = %d \n", var);

        /* 
         * On fera 3 tentatives successives de communication avant de déclarer un échec de communication total.
         * Pour cela, on vérifiera si le compteur d'échec dépasse 3 et donc déclarer l'échec de com au moniteur
         *  */
        if (var >= 3) {

            /*  MAJ de l'état de com entre robot et superviseur */
            rt_mutex_acquire(&mutexEtat, TM_INFINITE);
            etatCommRobot = status;
            rt_printf("Etat comm robot : %d \n", etatCommRobot);
            rt_mutex_release(&mutexEtat);

            /* Envoi d'un message de perte de communication au moniteur */
            message = d_new_message();
            message->put_state(message, status);

            rt_printf("Fonction verifier_robot : Envoi message\n");
            message->print(message, 100);

            if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                message->free(message);
            }

            /*  On revient dans l'état initial --> On stoppe toutes les com avec le robot */
            robot->close_com(robot);

            /*  
             * Appel de la fonction de réinitialisation ( disconnect_robot) via un thread qui s'autodétruira à la fin
             */
            //rt_task_spawn(&tDeconnectRobot, NULL, 0, PRIORITY_TDECONNECTROBOT, 0, &disconnect_robot, NULL);
        }
    } else if (status == STATUS_OK) {

        // On modifie l'état des com du robot avec la bonne valeur
        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        etatCommRobot = status;
        rt_printf("Etat comm robot : %d \n", etatCommRobot);
        rt_mutex_release(&mutexEtat);

        // On remet à zéro la valeur du compteur d'échec
        rt_mutex_acquire(&mutexCptEchec, TM_INFINITE);
        cptEchec = 0;
        var = cptEchec;
        rt_mutex_release(&mutexCptEchec);
        
        rt_printf("Status com avec le robot, compteur = %d \n", var);
    }
    return status ;

}


void get_etat_battery(void *arg)
{
    int status,i,j;
    DMessage *messBat;
    DBattery *battery;
	

    rt_printf("tBattery : Debut de l'execution du thread\n");
    rt_printf("tBattery : Debut de l'éxecution de periodique à 250 ms\n");
    rt_task_set_periodic (&tBattery,TM_NOW,250000000);

    while (1) 
	{
			
			rt_printf("******************* tBattery : Attente du sémarphore semtBattery ****\n"); 
			rt_sem_p(&semBattery, TM_INFINITE);
			j=1;
			rt_mutex_acquire(&mutexEtat, TM_INFINITE);
			status = robot->open_device(robot); //Permet d'ouvrir le port de communication pour permettre la communication avec le robot.
			rt_mutex_release(&mutexEtat);
		
			rt_printf("******************* tBattery : Verification de l'etat de communication avec le robot **********\n");
			
			rt_mutex_acquire(&mutexStatus, TM_INFINITE);
			i=verifier_com_robot(status);
			rt_mutex_release(&mutexStatus);
			

			
			if (i == STATUS_OK)
			{
					rt_printf("******************* tBattery : Etat Communication Superviseur-Robot = OK *****\n");   
					rt_printf("*************** tBattery : Récupération de l'etat Batterie du robot ****\n");          
					
					while (j!=0) 
					{
						rt_printf("************** tBattery : Activation periodique *****\n");
						rt_task_wait_period(NULL);
						rt_mutex_acquire(&mutexStatus, TM_INFINITE);
						status=robot->get_vbat(robot,&etatBattery);   
						i=verifier_com_robot(status);
						rt_mutex_release(&mutexStatus);
						j=0;
					}
					if (i == STATUS_OK) 
					{
							rt_printf("***************** tBattery : STATUS-Batterie = OK *****\n");
							battery = d_new_battery();                
							battery->set_level(battery, etatBattery);
							
							//----------------------------------------------------------------------//
							if(etatBattery==0)
							{
									rt_printf("======= Etat Battery = BATTERY_OFF ========\n");	
							}
							else if(etatBattery==1)
							{
									rt_printf("======= Etat Battery = BATTERY_LOW ========\n");	
							}
							else if(etatBattery==2)
							{
									rt_printf("======= Etat Battery = BATTERY_OK ========\n");	
							}					
						
							//-------------------------------------------------------------------------//
							messBat = d_new_message();
							messBat->put_battery_level(messBat, battery);   
							
							//-------------------------------------------------------------------------//
							rt_printf("***************** tBattery : Envoi du niveau de la batterie au moniteur *****\n");				
							if(serveur->active==0)
							{
									rt_printf("***************** tBattery : Reconnexion avec le moniteur *****\n");
									serveur->open(serveur, "8000");
							}				
							serveur->send(serveur, messBat);				
					}
					
					else
					{
							rt_printf("***************** tBattery : STATUS-Batterie = NOK *****\n");
					}
			}
		
			else
			{
					rt_printf("******************* tBattery : Etat Communication Superviseur-Robot = NOK *****\n");  
			}        		
				 
			rt_printf("***************** Mise a jour de la vue connexion Robot Affichage********\n");     
			battery->print(battery);
			messBat->print(messBat);

			if (write_in_queue(&queueMsgGUI, messBat, sizeof (DMessage)) < 0) 
			{
					messBat->free(messBat);
			}
				
		
    }
}
