# Free-Laundry-Manager
----------------------
ENG
This is a Management and access control system for laundry equipment in small apartment-buildings and shared-flats.
The aim of this repo is to develop a control and management system for a shared small-size laundry of one washing machine, and one dryer.
With this system, landlords and co-owners can provide fair and monitored access to a set of normal home appliances.
This system is not for commercial use, and as such does not provide any warranty what so ever and of any type.
The philosophy of this project is to improve the life of tenants by providing an optimized sharing of available resources like laundry equipment.
See it as a laundry-pooling solution! ;)

FR
Free-Laundry-Manager est un système de contrôle et de gestion pour petites buanderies d'immeubles locatifs ou de copropriétés.
L'objectif de ce Repo est de développer un  système capable de contrôler une petite structure constituée d'une lave-linge et d'un sèche-linge.
Avec ce dispositif, un propriétaire bailleur, ou les membres d'une colocation peuvent organiser et assurer le partage équitable d'équipements standards.
Ce système n'est pas destiné à un usage professionnel, et ne constitue donc aucune garantie d'aucune sorte.
La philosophie de ce projet est d'améliorer la vie des locataires, en leur fournissant un accès optimisé et équitable à des équipements partagés de lingerie.
Voyez-le comme un covoiturage de la lingerie ! ;)

Project main-features / fonctions principales :
----------------------------------------------
-ESP32 based,
 /basé sur un ESP32,
-washing machine + dryer (requires external relays and power relays),
 /lave-linge + sèche-linge (nécessite des relais externes et des relais de puissance),
-RFID tags + password access control,
 /contrôle d'accès par badges RFID et mot de passe,
-WIFI LAN/WAN admin extended control and local management via html interfacing,
 /gestion et contrôle étendu via interface html par Wifi LAN/WAN,
-SMS info’s and basic remote control,
 /informations et gestion de base par SMS,
-local basic management via system LCD and keypad,
 /gestion locale par écran et clavier.

Planned features / fonctions prévues:
-------------------------------------
-SD card logs and backups,
 /logs et sauvegardes sur carte SD.

CREDITS / REMERCIEMENTS:
-----------------------
ENG
This project is based on the original work of yohann74 and lesept, and is visible at https://forum.arduino.cc/t/gerer-un-lave-linge-en-co-propriete/630697 .
It's main contributors are hbachetti (http://riton-duino.blogspot.com/), J-M-L Jackson, Artouste, fdufnews, and all the great Arduino.cc community.

Great thanks to all of them for their continued support on this new/follow-up project!
This project live discussion is at https://forum.arduino.cc/t/gerer-un-lave-linge-en-co-propriete-realisation-du-parfait-noob/908070/1 .
Don't hesitate to join and contribute!

FR
Ce projet est basé sur le travail original de yohann74 et lesept, et est visible à https://forum.arduino.cc/t/gerer-un-lave-linge-en-co-propriete/630697 .
Ses principaux contributeurs sont hbachetti (http://riton-duino.blogspot.com/), J-M-L Jackson, Artouste, fdufnews, et la communauté Arduino.cc .

Mes sincères remerciements à eux pour la continuité de leur soutien dans ce projet en continuité.
Le fil de ce projet est à https://forum.arduino.cc/t/gerer-un-lave-linge-en-co-propriete-realisation-du-parfait-noob/908070/1 .
Venez nombreux nous rejoindre et contribuer !
 
-------------------------------------------------------------------------------------------------------
!!SPOILER ALERT!! ...the disclaimer! (ENG)
-------------------------------------------------------------------------------------------------------
DANGER!! The realisation of this project involves the cabling and routing of mains power high-voltages.
Electrical equipment should be installed, operated, serviced, and maintained only by qualified personnel.
No responsibility is assumed by the writer and co-writers for any consequences arising out of the use of this project and equipment.
A qualified person is one who has skills and knowledge related to the construction, installation, and operation of electrical equipment and has received safety training to recognize and avoid the hazards involved.
If you are 18 or less, or if you don't have the required experience, you cannot follow the instructions of this repository
as it may include a high risk of electrical shock.
-------------------------------------------------------------------------------------------------------
EXONERATION DE RESPONSABILITES (FR)
L’installation, l’utilisation, la réparation et la maintenance des équipements électriques doivent être assurées par du personnel qualifié uniquement.
L'auteur et ses co-auteurs déclinent toute responsabilité quant aux conséquences de l’utilisation de cet appareil.
Une personne qualifiée est une personne disposant de compétences et de connaissances dans le domaine de la construction, de l’installation et du fonctionnement des équipements électriques et ayant suivi une formation sur la sécurité lui permettant d’identifier et d’éviter les risques encourus.
Si vous êtes mineur, ou si vous n'avez pas l'expérience requise, vous ne pouvez pas suivre les instructions de ce repository, car il inclus un risque élevé d'électrocution.
-------------------------------------------------------------------------------------------------------
