/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "stm32746g_discovery_audio.h"
#include "stdio.h"
#include "math.h"
#include "fatfs_storage.h"
#include <string.h>
#include <stdlib.h>
#include "evoli2.h"
#include "evoliD.h"
#include "roucool.h"
#include "roucoolC.h"
#include "roucoolD.h"
#include "evoliC.h"
#include "salameche.h"
#include "salamecheC.h"
#include "salamecheD.h"
#include "carapuce.h"
#include "carapuceC.h"
#include "carapuceD.h"
#include "bulbizarre.h"
#include "bulbizarreC.h"
#include "bulbizarreD.h"
#include "choix.h"
#include <math.h>
#include "pokeball.h"
#include "pokeballO.h"
#include "mustebouee.h"
#include "musteboueeC.h"
#include "musteboueeD.h"
#include "roselia.h"
#include "roseliaD.h"
#include "roseliaC.h"
#include "droite.h"
#include "gauche.h"
#include "haut.h"
#include "bas.h"
#include "ponyta.h"
#include "ponytaC.h"
#include "ponytaD.h"
#include "ganon.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ARBG8888_BYTE_PER_PIXEL   4

/**
 * @brief  SDRAM Write read buffer start address after CAM Frame buffer
 * Assuming Camera frame buffer is of size 640x480 and format RGB565 (16 bits per pixel).
 */
#define SDRAM_WRITE_READ_ADDR        ((uint32_t)(LCD_FB_START_ADDRESS + (RK043FN48H_WIDTH * RK043FN48H_HEIGHT * ARBG8888_BYTE_PER_PIXEL)))

#define SDRAM_WRITE_READ_ADDR_OFFSET ((uint32_t)0x0800)
#define SRAM_WRITE_READ_ADDR_OFFSET  SDRAM_WRITE_READ_ADDR_OFFSET

#define AUDIO_REC_START_ADDR         SDRAM_WRITE_READ_ADDR

#define AUDIO_BLOCK_SIZE   	((uint32_t)512)
#define AUDIO_BUFFER_IN    	AUDIO_REC_START_ADDR     /* In SDRAM */
#define AUDIO_BUFFER_OUT   	(AUDIO_REC_START_ADDR + (AUDIO_BLOCK_SIZE*2)) /* In SDRAM */
#define AUDIO_BUFFER_READ  	(AUDIO_REC_START_ADDR + (AUDIO_BLOCK_SIZE*4))
#define AUDIO_BUFFER_POST  	(AUDIO_REC_START_ADDR + (AUDIO_BLOCK_SIZE*6))

#define Audio_freq 			48000
#define Audio_bit_res 		DEFAULT_AUDIO_IN_BIT_RESOLUTION	//16
#define Audio_chan 			DEFAULT_AUDIO_IN_CHANNEL_NBR	//2
#define BytePerBloc			((uint16_t)Audio_bit_res*Audio_chan/8)
#define BytePerSec			((uint32_t)BytePerBloc*Audio_freq)

#define MASK_32_TO_8_0		0x000000FF
#define MASK_32_TO_8_1		0x0000FF00
#define MASK_32_TO_8_2		0x00FF0000
#define MASK_32_TO_8_3		0xFF000000

#define MAX_MUSIQUE 		6



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc3;

DMA2D_HandleTypeDef hdma2d;

LTDC_HandleTypeDef hltdc;

SAI_HandleTypeDef hsai_BlockA2;
SAI_HandleTypeDef hsai_BlockB2;
DMA_HandleTypeDef hdma_sai2_a;
DMA_HandleTypeDef hdma_sai2_b;

SD_HandleTypeDef hsd1;
DMA_HandleTypeDef hdma_sdmmc1_rx;
DMA_HandleTypeDef hdma_sdmmc1_tx;

UART_HandleTypeDef huart1;

SDRAM_HandleTypeDef hsdram1;
SDRAM_HandleTypeDef hsdram2;

osThreadId Play_wavHandle;
osThreadId task_Affich_PicHandle;
osThreadId task_deplacementHandle;
osThreadId task_chgZoneHandle;
osThreadId task_DemarrageHandle;
osThreadId task_combatHandle;
osThreadId task_deplac_pokHandle;
osMessageQId WakeUpHandle;
osMessageQId QueueP1toAHandle;
osMessageQId QueueP2toAHandle;
osMessageQId QueueP3toAHandle;
osMessageQId QueueAtoPHandle;
osMessageQId QueueOtoAHandle;
osMessageQId QueueAtoCHandle;
osMessageQId QueueCtoAHandle;
osMessageQId QueueC1toAHandle;
/* USER CODE BEGIN PV */

uint8_t  enable = 0, fin_record = 0,etat = 0;
uint32_t NB_Bloc=0,Bloc_Cursor=0,freq_audio,Nb_octets_seconde=1;
int8_t indice=-1;
unsigned char text[32];
unsigned char duree_total[5];
unsigned char duree_actuelle[5];
char musique[MAX_MUSIQUE][15]={"Mus1.WAV","Mus2.WAV","Mus3.WAV","Musique4.WAV","Musique5.WAV","Musique6.WAV"};
const unsigned char* spriteJoueur[] = {bas_bmp, droite_bmp, gauche_bmp, haut_bmp};
const unsigned char* spritePoke[] = {salameche_bmp, salamecheC_bmp, salamecheD_bmp, carapuce_bmp, carapuceC_bmp, carapuceD_bmp, bulbizarre_bmp, bulbizarreC_bmp, bulbizarreD_bmp, roucool_bmp, roucoolC_bmp, roucoolD_bmp, evoli2_bmp, evoliC_bmp, evoliD_bmp, mustebouee_bmp, musteboueeC_bmp, musteboueeD_bmp, roselia_bmp, roseliaC_bmp, roseliaD_bmp, ponyta_bmp, ponytaC_bmp, ponytaD_bmp, ganon_bmp};
const unsigned char* spriteObjet[] = {pokeball_bmp, pokeballO_bmp};
uint8_t objetTrouves[] = {0,0,0,0,0,0,0,0,0,0};
int State_demarrage = 0, transparency = 0;;
char *pDirectoryFiles[MAX_BMP_FILES];
char *pDirectoryWaveFiles[MAX_WAVE_FILES];
uint8_t ubNumberOfFiles = 0;
uint32_t uwBmplen = 0;
uint8_t carte[30][17];
uint8_t tailleTuile = 16;
uint8_t *uwInternelBuffer; //Buffer pour la mémoire SDRAM
uint8_t *uwInternelBuffer2;
FIL F1;
uint8_t str[30];
uint8_t initZ1=0, initZ2=0, initZ3 = 0, initZ4 = 0, initZ5 = 0;
uint8_t sector1[512];
uint32_t Debut = 0;
uint8_t efficace = 0;
uint32_t joystick_h, joystick_v;
/*definition des structures*/
typedef struct{
	char nom[15];
	uint8_t idx; //2 = potion

}Objet;

typedef struct  {

    char nom[15];
    uint8_t niveau;
    uint8_t type ; //1 : normal, 2 : feu, 3 : eau, 4 : plante
    uint8_t faiblesse ;
    uint8_t resistance ;
    uint8_t degats;
    uint8_t defense;
    uint8_t PVMAX;
    int8_t PV;
    uint8_t EXP;
    uint16_t x;
    uint16_t y;
    uint8_t largeur;
    uint8_t hauteur;
    uint8_t sprite;
    uint8_t spriteC;
    uint8_t spriteD;
    uint8_t vitesse;
    uint8_t idx;
    char attaque[3][15];
    char attaqueSelect[15];


}Pokemon;

typedef struct{
	char nom[15];
    uint16_t x;
    uint16_t y;
    Pokemon pokemon1;
    Pokemon pokemon2;
    Pokemon pokemon3;
    Pokemon pokemon4;
    Pokemon *pokemonSelect;
    uint8_t zone;
    uint8_t zonePred;
    uint8_t sprite;
    Objet objet[10];

}Joueur;

typedef struct{
    uint16_t x;
    uint16_t y;
    uint8_t rad;
}Curseur;
//definition de fonctions
Pokemon pokeInit(uint8_t idxPoke, uint8_t niveau){
	Pokemon pokemon;

	if(idxPoke == 0){//salameche
		strcpy(pokemon.nom , "Salameche");
		pokemon.niveau = niveau;
		pokemon.PVMAX = 19 + pokemon.niveau*2;
		pokemon.PV = pokemon.PVMAX;
		pokemon.type = 2;
		pokemon.degats = 10;
		pokemon.EXP = 0;
		pokemon.defense = 2;
		pokemon.sprite = 0;
		pokemon.spriteC = 1;
		pokemon.spriteD = 2;
		strcpy(pokemon.attaque[0] , "Flameche");
		strcpy(pokemon.attaque[1] , "Charge");
		strcpy(pokemon.attaque[2] , "Concentration");
		pokemon.vitesse = 9*pokemon.niveau;
		pokemon.faiblesse = 3;
		pokemon.resistance = 4;
		pokemon.idx = 0;
		pokemon.largeur = 32;
		pokemon.hauteur = 38;
	}

	if(idxPoke == 1){//carapuce
		strcpy(pokemon.nom , "Carapuce");
		pokemon.niveau = niveau;
		pokemon.PVMAX = 22 + pokemon.niveau*2;
		pokemon.PV = pokemon.PVMAX;
		pokemon.type = 3;
		pokemon.degats = 9;
		pokemon.EXP = 0;
		pokemon.defense = 3;
		pokemon.sprite = 3;
		pokemon.spriteC = 4;
		pokemon.spriteD = 5;
		strcpy(pokemon.attaque[0] , "Bulle d'eau");
		strcpy(pokemon.attaque[1] , "Charge");
		strcpy(pokemon.attaque[2] , "Soin");
		pokemon.vitesse = 8*pokemon.niveau;
		pokemon.faiblesse = 4;
		pokemon.resistance = 2;
		pokemon.idx = 1;
		pokemon.largeur = 32;
		pokemon.hauteur = 33;
	}

	if(idxPoke == 2){//Bulbizarre
		strcpy(pokemon.nom , "Bubizarre");
		pokemon.niveau = niveau;
		pokemon.PVMAX = 22 + pokemon.niveau*2;
		pokemon.PV = pokemon.PVMAX;
		pokemon.type = 4;
		pokemon.degats = 8;
		pokemon.EXP = 0;
		pokemon.defense = 4;
		pokemon.sprite = 6;
		pokemon.spriteC = 7;
		pokemon.spriteD = 8;
		strcpy(pokemon.attaque[0] , "Tranche herbe");
		strcpy(pokemon.attaque[1] , "Charge");
		strcpy(pokemon.attaque[2] , "Rugissement");
		pokemon.vitesse = 7*pokemon.niveau;
		pokemon.faiblesse = 2;
		pokemon.resistance = 3;
		pokemon.idx = 2;
		pokemon.largeur = 34;
		pokemon.hauteur = 32;
	}

	if(idxPoke == 3){//Roucool
		strcpy(pokemon.nom , "Roucool");
		pokemon.niveau = niveau;
		pokemon.PVMAX = 17 + pokemon.niveau*2;
		pokemon.PV = pokemon.PVMAX;
		pokemon.type = 1;
		pokemon.EXP = 0;
		pokemon.degats = 7;
		pokemon.defense = 1;
		pokemon.largeur = 26;
		pokemon.hauteur = 28;
		pokemon.sprite = 9;
		pokemon.spriteC = 10;
		pokemon.spriteD = 11;
		strcpy(pokemon.attaque[0] , "Charge");
		strcpy(pokemon.attaque[1] , "Concentration");
		strcpy(pokemon.attaque[2] , "Soin");
		pokemon.vitesse = 5*pokemon.niveau;
		pokemon.faiblesse = 0;
		pokemon.resistance = 0;
		pokemon.idx = 3;
	}

	if(idxPoke == 4){//Evoli
		strcpy(pokemon.nom , "Evoli");
		pokemon.niveau = niveau;
		pokemon.PVMAX = 20 + pokemon.niveau*2;
		pokemon.PV = pokemon.PVMAX;
		pokemon.type = 0;
		pokemon.EXP = 0;
		pokemon.degats = 8;
		pokemon.defense = 2;
		pokemon.largeur = 24;
		pokemon.hauteur = 25;
		pokemon.sprite = 12;
		pokemon.spriteC = 13;
		pokemon.spriteD = 14;
		strcpy(pokemon.attaque[0] , "Charge");
		strcpy(pokemon.attaque[1] , "Rugissement");
		strcpy(pokemon.attaque[2] , "Soin");
		pokemon.vitesse = 10*pokemon.niveau;
		pokemon.faiblesse = 0;
		pokemon.resistance = 0;
		pokemon.idx = 4;
	}

	if(idxPoke == 5){//Mustebouee
		strcpy(pokemon.nom , "Mustebouee");
		pokemon.niveau = niveau;
		pokemon.PVMAX = 20 + pokemon.niveau*2;
		pokemon.PV = pokemon.PVMAX;
		pokemon.type = 3;
		pokemon.EXP = 0;
		pokemon.degats = 10;
		pokemon.defense = 1;
		pokemon.largeur = 24;
		pokemon.hauteur = 30;
		pokemon.sprite = 15;
		pokemon.spriteC = 16;
		pokemon.spriteD = 17;
		strcpy(pokemon.attaque[0] , "Bulle d'eau");
		strcpy(pokemon.attaque[1] , "Rugissement");
		strcpy(pokemon.attaque[2] , "Concentration");
		pokemon.vitesse = 12*pokemon.niveau;
		pokemon.faiblesse = 4;
		pokemon.resistance = 2;
		pokemon.idx = 5;
	}

	if(idxPoke == 6){//Roselia
		strcpy(pokemon.nom , "Roselia");
		pokemon.niveau = niveau;
		pokemon.PVMAX = 22 + pokemon.niveau*2;
		pokemon.PV = pokemon.PVMAX;
		pokemon.type = 4;
		pokemon.EXP = 0;
		pokemon.degats = 11;
		pokemon.defense = 1;
		pokemon.largeur = 18;
		pokemon.hauteur = 16;
		pokemon.sprite = 18;
		pokemon.spriteC = 19;
		pokemon.spriteD = 20;
		strcpy(pokemon.attaque[0] , "Tranche herbe");
		strcpy(pokemon.attaque[1] , "Soin");
		strcpy(pokemon.attaque[2] , "Concentration");
		pokemon.vitesse = 8*pokemon.niveau;
		pokemon.faiblesse = 2;
		pokemon.resistance = 3;
		pokemon.idx = 6;
	}

	if(idxPoke == 7){//Ponyta
		strcpy(pokemon.nom , "Ponyta");
		pokemon.niveau = niveau;
		pokemon.PVMAX = 18 + pokemon.niveau*2;
		pokemon.PV = pokemon.PVMAX;
		pokemon.type = 2;
		pokemon.EXP = 0;
		pokemon.degats = 10;
		pokemon.defense = 3;
		pokemon.largeur = 32;
		pokemon.hauteur = 31;
		pokemon.sprite = 21;
		pokemon.spriteC = 22;
		pokemon.spriteD = 23;
		strcpy(pokemon.attaque[0] , "Flameche");
		strcpy(pokemon.attaque[1] , "Rugissement");
		strcpy(pokemon.attaque[2] , "Concentration");
		pokemon.vitesse = 10*pokemon.niveau;
		pokemon.faiblesse = 3;
		pokemon.resistance = 4;
		pokemon.idx = 7;
	}

	if(idxPoke == 8){//Ganondorf
		strcpy(pokemon.nom , "Ganondorf");
		pokemon.niveau = niveau;
		pokemon.PVMAX = 26 + pokemon.niveau*2;
		pokemon.PV = pokemon.PVMAX;
		pokemon.type = 5;
		pokemon.EXP = 0;
		pokemon.degats = 8;
		pokemon.defense = 2;
		pokemon.spriteC = 24;
		strcpy(pokemon.attaque[0] , "Flameche");
		strcpy(pokemon.attaque[1] , "Tranche herbe");
		strcpy(pokemon.attaque[2] , "Bulle d'eau");
		pokemon.vitesse = 10*pokemon.niveau;
		pokemon.faiblesse = 0;
		pokemon.resistance = 0;
		pokemon.idx = 8;
	}
	return pokemon;

}

void CarteInitZ1(){

	int i,j;
	for(i = 0;i<30;i++){
		for(j = 0;j<17;j++){
			if((i<9 && j<11) || (j<2) || (i>25) || (j>11)){
				carte[i][j] = 0;
			}
			else if( (i>19 && i<24 && j>5 && j<11) || (i>10 && i<15 && j>2 && j<8)){
				carte[i][j]  = 0;
			}
			else{
				carte[i][j]  = 1;
			}

		}
	}
	//position des objets
	carte[25][2] = 2;

	//position sortie de carte
	carte[0][11] = 12;

}
void CarteInitZ2(){

	int i,j;
	for(i = 0;i<30;i++){
		for(j = 0;j<17;j++){
			if((i<4 && j<5) || (i<2) || (j>13) || (i>26 && j<11) || (i>25 && j>11) || (j<2 && i<26) ){
				carte[i][j] = 0;
			}
			else if( (i>7 && i<14 && j>6 && j<13) ){
				carte[i][j]  = 0;
			}
			else{
				carte[i][j]  = 1;
			}

		}
	}
	//position des objets
	carte[3][5] = 2; //potion
	carte[11][13] = 3; //fragment triforce
	//position sortie de carte
	carte[29][11] = 21;
	carte[26][0] = 23;

}
void CarteInitZ3(){

	int i,j;
	for(i = 0;i<30;i++){
		for(j = 0;j<17;j++){
			if((i<6 && j<3) || (i<2) || i>26 || (i>6 && j<3) || (j>13 && i<26) ){
				carte[i][j] = 0;
			}
			else if( (i>15 && i<19 && j>6 && j<10) || (i>6 && i<10 && j>3 && j<7) || (i>8 && i<12 && j>8 && j<13)){
				carte[i][j]  = 0;
			}
			else{
				carte[i][j]  = 1;
			}

		}
	}
	//position des objets
	carte[8][7] = 2; //potion
	carte[17][10] = 3; //fragment triforce
	//position sortie de carte
	carte[26][16] = 32;
	carte[6][0] = 34;

}
void CarteInitZ4(){


	int i,j;
	for(i = 0;i<30;i++){
		for(j = 0;j<17;j++){
			if((i<6 && j>12) || (i<2) ||(i<4 && j>3 && j<11) || (i == 2 && j<5) || j<2 || (i>26 && j>2)  || (j>12 && i> 6) ){
				carte[i][j] = 0;
			}
			else if( (i==4 && j>3 && j<6) || (i>6 && i<10 && j>3 && j<6) || (i>7 && i<10 && j>4 && j<10) || (i>7  && j>9 && j<12) || (i>13 && i<16  && j<9) || (i>17 && j<9  && j>6) || (i>19 && i<24 && j<7  && j>3)){
				carte[i][j]  = 0;
			}
			else{
				carte[i][j]  = 1;
			}

		}
	}
	//position des objets
	carte[26][12] = 2; //potion
	carte[13][2] = 3; //fragment triforce
	//position sortie de carte
	carte[6][16] = 43;
	carte[29][2] = 45;
}
void CarteInitZ5(){

	int i,j;
	for(i = 0;i<30;i++){
		for(j = 0;j<17;j++){
			if(j<2 || (j>14) ||(i<1 && j>2) || i>25 || (i>18 && j<13) || (j>5 && j<13 && i>16) || ( j<3 && i>1) || (i>1 && i<13 && j<13) || (i>12 && i<15 && j>5 &&j<13) || (i>1 && i<6 && j<14) ){
				carte[i][j] = 0;
			}
			else if(i>14 && i<17 && j<5){
				carte[i][j] = 0;
			}
			else{
				carte[i][j]  = 1;
			}

		}
	}
	//position des objets
	carte[6][13] = 2; //potion
	carte[26][13] = 2; //potion
	//position sortie de carte
	carte[0][2] = 54;
	//position placement frag tri
	carte[16][5] = 4;
}
uint8_t collision(Joueur joueur, Pokemon pokemon) {
	 uint16_t gauche1 = joueur.x*tailleTuile;
     uint16_t droite1 = joueur.x*tailleTuile + 30;
     uint16_t haut1 = joueur.y*tailleTuile;
     uint16_t bas1 = joueur.y*tailleTuile + 32;

     uint16_t gauche2 = pokemon.x*tailleTuile;
     uint16_t droite2 = pokemon.x*tailleTuile + pokemon.largeur;
     uint16_t haut2 = pokemon.y*tailleTuile;
     uint16_t bas2 = pokemon.y*tailleTuile + pokemon.hauteur;
     if (gauche1 < droite2 &&
             droite1 > gauche2 &&
             haut1 < bas2 &&
             bas1 > haut2) {

        return 1; // Collision détectée
    }
    return 0; // Pas de collision
}
/*
uint8_t collision(Joueur joueur, Pokemon pokemon) {
    if (joueur.x == pokemon.x && joueur.y == pokemon.y ) {
        return 1; // Collision détectée
    }
    return 0; // Pas de collision
}*/

Joueur joueur;
Pokemon pokemonAdverse;
Curseur curseur;
//A TESTER (attaque super efficace, soin)
uint8_t degatAttaque(Pokemon* p1, Pokemon* p2){
	uint8_t efficace = 0;
	float facteur_niveau = (float)(2*p1->niveau +10)/12;
	float stat = (float)(p1->degats-p2->defense);
	uint8_t deg = 2+round(facteur_niveau*stat);

	if(strcmp(p1->attaqueSelect,"Concentration") == 0){
		p1->degats *= 1.5;
	}
	else if(strcmp(p1->attaqueSelect,"Rugissement") == 0){
		p2->degats *= 0.8;
	}
	else if(strcmp(p1->attaqueSelect,"Soin") == 0){
		if(p1->PV+0.3*p1->PVMAX>p1->PVMAX){
			p1->PV = p1->PVMAX;
		}
		else{
			p1->PV += 0.3*p1->PVMAX;
		}

	}
	else if(strcmp(p1->attaqueSelect,"Charge") == 0){
		if(p1->type == 1){
			deg *= 1.4;
		}
		p2->PV -= deg;
	}
	else if(strcmp(p1->attaqueSelect,"Flameche") == 0 || strcmp(p1->attaqueSelect,"Tranche herbe") == 0 || strcmp(p1->attaqueSelect,"Bulle d'eau") == 0){
		deg *= 1.1;
		if(p1->type == p2->faiblesse){
			p2->PV -= deg * 1.8;
			efficace = 2;
		}
		else if(p1->type == p2->resistance){
			p2->PV -= deg * 0.5;
			efficace = 1;
		}
		else if(p1->type == 5 && p2->type != 1){
			p2->PV -= deg * 1.8;
			efficace = 2;
		}
		else{
			p2->PV -= deg;
		}
	}
	return efficace;

}
void chgEcran(uint8_t ecran){
	f_open(&F1, (TCHAR const*) str, FA_READ);
	f_read(&F1, sector1, 512, (UINT*) &Debut);
	/* Format the string */
	sprintf((char*) str, "Media/%-11.11s",
			pDirectoryFiles[ecran]);

	if (Storage_CheckBitmapFile((const char*) str, &uwBmplen) == 0)
			 {
		/* Connect the Output Buffer to LCD Background Layer  */
		BSP_LCD_SelectLayer(0);

		/* Format the string */
		sprintf((char*) str, "Media/%-11.11s",
				pDirectoryFiles[ecran]);

		/* Open a file and copy its content to an internal buffer */
		Storage_OpenReadFile(uwInternelBuffer, (const char*) str);

		/* Write bmp file on LCD frame buffer */
		BSP_LCD_DrawBitmap(0, 0, uwInternelBuffer);

		/* Configure the transparency for background layer : decrease the transparency */
		for (transparency = 0; transparency < 255;
				(transparency++)) {
			BSP_LCD_SetTransparency(0, transparency);

			/* Insert a delay of display */
			HAL_Delay(5);
		}
			 }
	f_close(&F1);

}



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SDMMC1_SD_Init(void);
static void MX_FMC_Init(void);
static void MX_LTDC_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_DMA2D_Init(void);
static void MX_SAI2_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC3_Init(void);
void Play_Wave(void const * argument);
void Affichage_Pic(void const * argument);
void deplac_pok(void const * argument);
void Demarrage(void const * argument);
void deplacement(void const * argument);
void chgZone(void const * argument);
void combat(void const * argument);


/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void SD_Init() {
	if (f_mount(&SDFatFS, (TCHAR const*) SDPath, 0) != FR_OK) {
		Error_Handler();
	} else {
		BSP_LCD_DisplayStringAt(0, 40, (uint8_t*) "SD - Mount Ok", CENTER_MODE);
	}
}

void Audio_Init(uint32_t freq) {
	static int init=0;
	if (BSP_AUDIO_IN_OUT_Init(INPUT_DEVICE_INPUT_LINE_1,
	OUTPUT_DEVICE_HEADPHONE, freq,
	Audio_bit_res,
	Audio_chan) == AUDIO_OK) {
		if (init==0){
		//BSP_LCD_DisplayStringAt(0, 20, (uint8_t*) "Init Audio - OK",CENTER_MODE);
		}
	}

	/* Initialize SDRAM buffers */
//	memset((uint16_t*) AUDIO_BUFFER_IN, 0, AUDIO_BLOCK_SIZE * 2);
	memset((uint16_t*) AUDIO_BUFFER_OUT, 0, AUDIO_BLOCK_SIZE * 2);
//	memset((uint16_t*) AUDIO_BUFFER_READ, 0, AUDIO_BLOCK_SIZE * 2);

	/* Start Recording */
	BSP_AUDIO_OUT_SetVolume(70);
	/* Start Playback */
	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
	if (BSP_AUDIO_OUT_Play((uint16_t*) AUDIO_BUFFER_OUT,
	AUDIO_BLOCK_SIZE * 2) == AUDIO_OK) {

		if (init==0){
		//sprintf(text,"Aud_freq= %u",(int)freq);
		init=1;
		}
	}

}

void LCD_Init() {
	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
	BSP_LCD_LayerDefaultInit(1,
	LCD_FB_START_ADDRESS + BSP_LCD_GetXSize() * BSP_LCD_GetYSize() * 4);
	BSP_LCD_DisplayOn();
	BSP_LCD_SelectLayer(1);
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	BSP_LCD_SetFont(&Font12);
	BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
	BSP_LCD_SetTextColor(LCD_COLOR_LIGHTBLUE);
	if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) != TS_OK) {
		BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 95, (uint8_t*) "ERROR",
				CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 80,
				(uint8_t*) "Touchscreen cannot be initialized", CENTER_MODE);
	} else {
		BSP_LCD_DisplayStringAt(0, 10, (uint8_t*) "Init Ecran - OK",
				CENTER_MODE);
	}
}

void read_header(){

	uint32_t data=0;
	uint32_t nb_bl;
	uint32_t bytesread;
	uint32_t taille_octet;


	//Lecture du nombre d'octets
	f_lseek(&SDFile,04);
	f_read(&SDFile, &data, 4, (void*) &bytesread);
//	taille_fichier=((data|MASK_32_TO_8_0)<<24)|((data|MASK_32_TO_8_1)<<8)|((data|MASK_32_TO_8_2)>>8)|((data|MASK_32_TO_8_3)>>24);
	taille_octet=data;
	nb_bl=data/512;
	NB_Bloc=(uint32_t)nb_bl;
	data=0;

	//Lecture de la fréquence d'échantillonnage
	f_lseek(&SDFile,24);
	f_read(&SDFile, &data, 4 , (void*) &bytesread);
//	freq=((data2|MASK_32_TO_8_0)<<24)|((data2|MASK_32_TO_8_1)<<8)|((data2|MASK_32_TO_8_2)>>8)|((data2|MASK_32_TO_8_3)>>24);
	freq_audio=data;

	//Nombre d'octets par secondes
	f_lseek(&SDFile,28);
	f_read(&SDFile, (uint8_t*)&data, 4, (void*) &bytesread);
	Nb_octets_seconde=data;


	}

void Charge_Wave(uint8_t indice){

	f_close(&SDFile);
	f_open(&SDFile, musique[indice], FA_READ);
	read_header();
	Audio_Init(freq_audio);
	f_lseek(&SDFile, 44);
	Bloc_Cursor=0;

}

void ClearEcran(){
	BSP_LCD_SelectLayer(0);
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	BSP_LCD_SelectLayer(1);
	BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
}

void TextIntro(){
	char text1[50];
	BSP_LCD_SelectLayer(0);
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	sprintf(text1, "Le monde est en danger");
	BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
	vTaskDelay(2000);
	//mettre un fondu?
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	sprintf(text1, "Toi seul peux le sauver");
	BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
	vTaskDelay(2000);
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	sprintf(text1, "Pour cela tu dois rassembler la triforce");
	BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
	vTaskDelay(2000);
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	sprintf(text1, "Ainsi tu pourras retablir" );
	BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
	vTaskDelay(2000);
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	sprintf(text1, "l'equilibre dans la force" );
	BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
	vTaskDelay(2000);
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	sprintf(text1, "Pour t'aider dans ta quete,");
	BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
	vTaskDelay(2000);
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	sprintf(text1, "choisis un pokemon.");
	BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
	vTaskDelay(2000);
}

void AfficheRect(){
	//affichage rectangle de texte
	BSP_LCD_DrawRect(1*tailleTuile, 12*tailleTuile-5, 28*tailleTuile, 5*tailleTuile);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(1*tailleTuile+1, 12*tailleTuile+1-5, 28*tailleTuile-1, 5*tailleTuile-1);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
}

void ValJoystick(){
	//config de la récupération des valeurs du joystick
	ADC_ChannelConfTypeDef sConfig = { 0 };
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	//recuperation valeur joystick
	sConfig.Channel = ADC_CHANNEL_8;
	HAL_ADC_ConfigChannel(&hadc3, &sConfig);
	HAL_ADC_Start(&hadc3);
	while (HAL_ADC_PollForConversion(&hadc3, 100) != HAL_OK)
		;
	joystick_v = HAL_ADC_GetValue(&hadc3);

	HAL_ADC_Start(&hadc1);
	while (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK)
		;
	joystick_h = HAL_ADC_GetValue(&hadc1);
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  MX_FMC_Init();
  MX_LTDC_Init();
  MX_USART1_UART_Init();
  MX_DMA2D_Init();
  MX_SAI2_Init();
  MX_ADC1_Init();
  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */


	// Initialisation de l'écran LCD
	       BSP_LCD_Init();
	       BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
	       BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS + BSP_LCD_GetXSize()*BSP_LCD_GetYSize()*4);
	       BSP_LCD_SelectLayer(0);
	       BSP_LCD_Clear(LCD_COLOR_WHITE);
	       BSP_LCD_SelectLayer(1);
	       BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
	       BSP_LCD_SetFont(&Font16);

	       // Activer les deux couches
	       BSP_LCD_SetLayerVisible(0, ENABLE);
	       BSP_LCD_SetLayerVisible(1, ENABLE);

	       // Activer le mode multi-couches
	       BSP_LCD_SetTransparency(0, 255); // Couche inférieure opaque
	       BSP_LCD_SetTransparency(1, 0); // Couche supérieure transparente
		BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
		Audio_Init(Audio_freq);
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of WakeUp */
  osMessageQDef(WakeUp, 1, uint8_t);
  WakeUpHandle = osMessageCreate(osMessageQ(WakeUp), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* definition and creation of QueueActivation */

  osMessageQDef(QueueAtoC, 1, Pokemon);
  QueueAtoCHandle = osMessageCreate(osMessageQ(QueueAtoC), NULL);
  osMessageQDef(QueueCtoA, 1, uint8_t);
  QueueCtoAHandle = osMessageCreate(osMessageQ(QueueCtoA), NULL);
  osMessageQDef(QueueC1toA, 1, uint8_t);
  QueueC1toAHandle = osMessageCreate(osMessageQ(QueueC1toA), NULL);
  osMessageQDef(QueueAtoP, 1, Pokemon);
  QueueAtoPHandle = osMessageCreate(osMessageQ(QueueAtoP), NULL);

  osMessageQDef(QueueOtoA, 1, uint8_t);
  QueueOtoAHandle = osMessageCreate(osMessageQ(QueueOtoA), NULL);

  /* definition and creation of PtoA */
  osMessageQDef(QueueP1toA, 1, Pokemon);
  QueueP1toAHandle = osMessageCreate(osMessageQ(QueueP1toA), NULL);
  osMessageQDef(QueueP2toA, 1, Pokemon);
  QueueP2toAHandle = osMessageCreate(osMessageQ(QueueP2toA), NULL);
  osMessageQDef(QueueP3toA, 1, Pokemon);
  QueueP3toAHandle = osMessageCreate(osMessageQ(QueueP3toA), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */

  /* definition and creation of Play_wav */
  osThreadDef(Play_wav, Play_Wave, osPriorityHigh, 0, 256);
  Play_wavHandle = osThreadCreate(osThread(Play_wav), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* definition and creation of task_Affich_Pic */
   osThreadDef(task_Affich_Pic, Affichage_Pic, osPriorityNormal, 0, 1024);
   task_Affich_PicHandle = osThreadCreate(osThread(task_Affich_Pic), NULL);

   /* definition and creation of task_Demarrage */
   osThreadDef(task_Demarrage, Demarrage, osPriorityHigh, 0, 1024);
   task_DemarrageHandle = osThreadCreate(osThread(task_Demarrage), NULL);

   /* USER CODE BEGIN RTOS_THREADS */
   osThreadDef(task_chgZone, chgZone, osPriorityHigh, 0, 256);
   task_chgZoneHandle = osThreadCreate(osThread(task_chgZone), NULL);
   /* definition and creation of deplacement */
   osThreadDef(task_deplacement, deplacement, osPriorityHigh, 0, 512);
   task_deplacementHandle = osThreadCreate(osThread(task_deplacement), NULL);
   /* definition and creation of deplac_pok */
   osThreadDef(task_deplac_pok, deplac_pok, osPriorityNormal, 0, 1024);
   task_deplac_pokHandle = osThreadCreate(osThread(task_deplac_pok), NULL);
   /* definition and creation of task_combat */
   osThreadDef(task_combat, combat, osPriorityHigh, 0, 1024);
   task_combatHandle = osThreadCreate(osThread(task_combat), NULL);
   vTaskSuspend(task_combatHandle);
   vTaskSuspend(Play_wavHandle);
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();
  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_SAI2
                              |RCC_PERIPHCLK_SDMMC1|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV8;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
  PeriphClkInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLSAI;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
  PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 40;
  hltdc.Init.VerticalSync = 9;
  hltdc.Init.AccumulatedHBP = 53;
  hltdc.Init.AccumulatedVBP = 11;
  hltdc.Init.AccumulatedActiveW = 533;
  hltdc.Init.AccumulatedActiveH = 283;
  hltdc.Init.TotalWidth = 565;
  hltdc.Init.TotalHeigh = 285;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 480;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 272;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  pLayerCfg.FBStartAdress = 0xC0000000;
  pLayerCfg.ImageWidth = 480;
  pLayerCfg.ImageHeight = 272;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief SAI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SAI2_Init(void)
{

  /* USER CODE BEGIN SAI2_Init 0 */

  /* USER CODE END SAI2_Init 0 */

  /* USER CODE BEGIN SAI2_Init 1 */

  /* USER CODE END SAI2_Init 1 */
  hsai_BlockA2.Instance = SAI2_Block_A;
  hsai_BlockA2.Init.Protocol = SAI_FREE_PROTOCOL;
  hsai_BlockA2.Init.AudioMode = SAI_MODEMASTER_TX;
  hsai_BlockA2.Init.DataSize = SAI_DATASIZE_8;
  hsai_BlockA2.Init.FirstBit = SAI_FIRSTBIT_MSB;
  hsai_BlockA2.Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;
  hsai_BlockA2.Init.Synchro = SAI_ASYNCHRONOUS;
  hsai_BlockA2.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  hsai_BlockA2.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
  hsai_BlockA2.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
  hsai_BlockA2.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
  hsai_BlockA2.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockA2.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai_BlockA2.Init.CompandingMode = SAI_NOCOMPANDING;
  hsai_BlockA2.Init.TriState = SAI_OUTPUT_NOTRELEASED;
  hsai_BlockA2.FrameInit.FrameLength = 8;
  hsai_BlockA2.FrameInit.ActiveFrameLength = 1;
  hsai_BlockA2.FrameInit.FSDefinition = SAI_FS_STARTFRAME;
  hsai_BlockA2.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
  hsai_BlockA2.FrameInit.FSOffset = SAI_FS_FIRSTBIT;
  hsai_BlockA2.SlotInit.FirstBitOffset = 0;
  hsai_BlockA2.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
  hsai_BlockA2.SlotInit.SlotNumber = 1;
  hsai_BlockA2.SlotInit.SlotActive = 0x00000000;
  if (HAL_SAI_Init(&hsai_BlockA2) != HAL_OK)
  {
    Error_Handler();
  }
  hsai_BlockB2.Instance = SAI2_Block_B;
  hsai_BlockB2.Init.Protocol = SAI_FREE_PROTOCOL;
  hsai_BlockB2.Init.AudioMode = SAI_MODESLAVE_RX;
  hsai_BlockB2.Init.DataSize = SAI_DATASIZE_8;
  hsai_BlockB2.Init.FirstBit = SAI_FIRSTBIT_MSB;
  hsai_BlockB2.Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;
  hsai_BlockB2.Init.Synchro = SAI_SYNCHRONOUS;
  hsai_BlockB2.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  hsai_BlockB2.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
  hsai_BlockB2.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockB2.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai_BlockB2.Init.CompandingMode = SAI_NOCOMPANDING;
  hsai_BlockB2.Init.TriState = SAI_OUTPUT_NOTRELEASED;
  hsai_BlockB2.FrameInit.FrameLength = 8;
  hsai_BlockB2.FrameInit.ActiveFrameLength = 1;
  hsai_BlockB2.FrameInit.FSDefinition = SAI_FS_STARTFRAME;
  hsai_BlockB2.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
  hsai_BlockB2.FrameInit.FSOffset = SAI_FS_FIRSTBIT;
  hsai_BlockB2.SlotInit.FirstBitOffset = 0;
  hsai_BlockB2.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
  hsai_BlockB2.SlotInit.SlotNumber = 1;
  hsai_BlockB2.SlotInit.SlotActive = 0x00000000;
  if (HAL_SAI_Init(&hsai_BlockB2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SAI2_Init 2 */

  /* USER CODE END SAI2_Init 2 */

}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDMMC1_Init 2 */

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
  /* DMA2_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_1;
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_DISABLE;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_DISABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 16;
  SdramTiming.ExitSelfRefreshDelay = 16;
  SdramTiming.SelfRefreshTime = 16;
  SdramTiming.RowCycleDelay = 16;
  SdramTiming.WriteRecoveryTime = 16;
  SdramTiming.RPDelay = 16;
  SdramTiming.RCDDelay = 16;

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /** Perform the SDRAM2 memory initialization sequence
  */
  hsdram2.Instance = FMC_SDRAM_DEVICE;
  /* hsdram2.Init */
  hsdram2.Init.SDBank = FMC_SDRAM_BANK2;
  hsdram2.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram2.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram2.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
  hsdram2.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram2.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_1;
  hsdram2.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram2.Init.SDClockPeriod = FMC_SDRAM_CLOCK_DISABLE;
  hsdram2.Init.ReadBurst = FMC_SDRAM_RBURST_DISABLE;
  hsdram2.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 16;
  SdramTiming.ExitSelfRefreshDelay = 16;
  SdramTiming.SelfRefreshTime = 16;
  SdramTiming.RowCycleDelay = 16;
  SdramTiming.WriteRecoveryTime = 16;
  SdramTiming.RPDelay = 16;
  SdramTiming.RCDDelay = 16;

  if (HAL_SDRAM_Init(&hsdram2, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOI, ARDUINO_D7_Pin|ARDUINO_D8_Pin|LCD_DISP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_BL_CTRL_GPIO_Port, LCD_BL_CTRL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, DCMI_PWR_EN_Pin|LED2_Pin|LED1_Pin|LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, ARDUINO_D4_Pin|ARDUINO_D2_Pin|EXT_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : OTG_HS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_HS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_HS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : QSPI_D2_Pin */
  GPIO_InitStruct.Pin = QSPI_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(QSPI_D2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_TXD1_Pin RMII_TXD0_Pin RMII_TX_EN_Pin */
  GPIO_InitStruct.Pin = RMII_TXD1_Pin|RMII_TXD0_Pin|RMII_TX_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_SCL_D15_Pin ARDUINO_SDA_D14_Pin */
  GPIO_InitStruct.Pin = ARDUINO_SCL_D15_Pin|ARDUINO_SDA_D14_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_PWM_D3_Pin */
  GPIO_InitStruct.Pin = ARDUINO_PWM_D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(ARDUINO_PWM_D3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPDIF_RX0_Pin */
  GPIO_InitStruct.Pin = SPDIF_RX0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF8_SPDIFRX;
  HAL_GPIO_Init(SPDIF_RX0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_PWM_D9_Pin */
  GPIO_InitStruct.Pin = ARDUINO_PWM_D9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(ARDUINO_PWM_D9_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DCMI_D6_Pin DCMI_D7_Pin */
  GPIO_InitStruct.Pin = DCMI_D6_Pin|DCMI_D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_VBUS_Pin */
  GPIO_InitStruct.Pin = OTG_FS_VBUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_VBUS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Audio_INT_Pin */
  GPIO_InitStruct.Pin = Audio_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Audio_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OTG_FS_P_Pin OTG_FS_N_Pin OTG_FS_ID_Pin */
  GPIO_InitStruct.Pin = OTG_FS_P_Pin|OTG_FS_N_Pin|OTG_FS_ID_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DCMI_D5_Pin */
  GPIO_InitStruct.Pin = DCMI_D5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(DCMI_D5_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_D7_Pin ARDUINO_D8_Pin LCD_DISP_Pin */
  GPIO_InitStruct.Pin = ARDUINO_D7_Pin|ARDUINO_D8_Pin|LCD_DISP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : uSD_Detect_Pin */
  GPIO_InitStruct.Pin = uSD_Detect_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(uSD_Detect_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_BL_CTRL_Pin */
  GPIO_InitStruct.Pin = LCD_BL_CTRL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BL_CTRL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DCMI_VSYNC_Pin */
  GPIO_InitStruct.Pin = DCMI_VSYNC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(DCMI_VSYNC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TP3_Pin NC2_Pin */
  GPIO_InitStruct.Pin = TP3_Pin|NC2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_SCK_D13_Pin */
  GPIO_InitStruct.Pin = ARDUINO_SCK_D13_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(ARDUINO_SCK_D13_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DCMI_PWR_EN_Pin LED2_Pin LED1_Pin LED3_Pin */
  GPIO_InitStruct.Pin = DCMI_PWR_EN_Pin|LED2_Pin|LED1_Pin|LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : DCMI_D4_Pin DCMI_D0_Pin */
  GPIO_InitStruct.Pin = DCMI_D4_Pin|DCMI_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_PWM_CS_D5_Pin */
  GPIO_InitStruct.Pin = ARDUINO_PWM_CS_D5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
  HAL_GPIO_Init(ARDUINO_PWM_CS_D5_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_PWM_D10_Pin */
  GPIO_InitStruct.Pin = ARDUINO_PWM_D10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(ARDUINO_PWM_D10_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_INT_Pin */
  GPIO_InitStruct.Pin = LCD_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LCD_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_RX_D0_Pin ARDUINO_TX_D1_Pin */
  GPIO_InitStruct.Pin = ARDUINO_RX_D0_Pin|ARDUINO_TX_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : ULPI_NXT_Pin */
  GPIO_InitStruct.Pin = ULPI_NXT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(ULPI_NXT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_D4_Pin ARDUINO_D2_Pin EXT_RST_Pin */
  GPIO_InitStruct.Pin = ARDUINO_D4_Pin|ARDUINO_D2_Pin|EXT_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : ULPI_D6_Pin ULPI_D5_Pin ULPI_D3_Pin ULPI_D2_Pin
                           ULPI_D1_Pin ULPI_D4_Pin */
  GPIO_InitStruct.Pin = ULPI_D6_Pin|ULPI_D5_Pin|ULPI_D3_Pin|ULPI_D2_Pin
                          |ULPI_D1_Pin|ULPI_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : ULPI_STP_Pin ULPI_DIR_Pin */
  GPIO_InitStruct.Pin = ULPI_STP_Pin|ULPI_DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_MDC_Pin RMII_RXD0_Pin RMII_RXD1_Pin */
  GPIO_InitStruct.Pin = RMII_MDC_Pin|RMII_RXD0_Pin|RMII_RXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : QSPI_D1_Pin QSPI_D3_Pin QSPI_D0_Pin */
  GPIO_InitStruct.Pin = QSPI_D1_Pin|QSPI_D3_Pin|QSPI_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : RMII_RXER_Pin */
  GPIO_InitStruct.Pin = RMII_RXER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RMII_RXER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_REF_CLK_Pin RMII_MDIO_Pin RMII_CRS_DV_Pin */
  GPIO_InitStruct.Pin = RMII_REF_CLK_Pin|RMII_MDIO_Pin|RMII_CRS_DV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : DCMI_HSYNC_Pin PA6 */
  GPIO_InitStruct.Pin = DCMI_HSYNC_Pin|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : ULPI_CLK_Pin ULPI_D0_Pin */
  GPIO_InitStruct.Pin = ULPI_CLK_Pin|ULPI_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_SDA_Pin */
  GPIO_InitStruct.Pin = LCD_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
  HAL_GPIO_Init(LCD_SDA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_MISO_D12_Pin ARDUINO_MOSI_PWM_D11_Pin */
  GPIO_InitStruct.Pin = ARDUINO_MISO_D12_Pin|ARDUINO_MOSI_PWM_D11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void BSP_AUDIO_OUT_TransferComplete_CallBack(void){
	char a=1;
	xQueueSendFromISR(WakeUpHandle, &a,0);
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void){
	char a=0;
	xQueueSendFromISR(WakeUpHandle, &a,0);
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_Play_Wave */
/**
* @brief Function implementing the Play_wav thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Play_Wave */
void Play_Wave(void const * argument)
{
  /* USER CODE BEGIN Play_Wave */
	char i;
	uint32_t bytesread;
	uint32_t taille_octet;
	  /* Infinite loop */
	  for(;;)
	  {
		  xQueueReceive(WakeUpHandle, &i, portMAX_DELAY);
		  if (i==0){
			  if (Bloc_Cursor++==NB_Bloc-1){//fin de musique
				  f_close(&SDFile);
					f_open(&SDFile, musique[0], FA_READ);
					read_header();
					Audio_Init(freq_audio);
					f_lseek(&SDFile, 44);
					Bloc_Cursor=0;
			  }
			 f_read(&SDFile, ((uint8_t*)AUDIO_BUFFER_OUT), AUDIO_BLOCK_SIZE,(void*) &bytesread);

			  taille_octet=512*Bloc_Cursor;

		  }
		  else{
			  if (Bloc_Cursor++==NB_Bloc-1){
				  f_close(&SDFile);
				f_open(&SDFile, musique[0], FA_READ);
				read_header();
				Audio_Init(freq_audio);
				f_lseek(&SDFile, 44);
				Bloc_Cursor=0;
			  }
			 f_read(&SDFile, ((uint8_t*)AUDIO_BUFFER_OUT+AUDIO_BLOCK_SIZE), AUDIO_BLOCK_SIZE,(void*) &bytesread);

			  taille_octet=512*Bloc_Cursor;


		  }
	  }
  /* USER CODE END Play_Wave */
}
/* USER CODE BEGIN Header_Affichage_Pic */
/**
 * @brief Function implementing the task_Affich_Pic thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Affichage_Pic */
void Affichage_Pic(void const * argument)
{
  /* USER CODE BEGIN Affichage_Pic */
	//action a faire au debut (eventuellement a deplacer dans fct demarrage)
	//spritePoke[0] = evoli2_bmp[];
	ClearEcran();

	char text1[50] = { };
	uint8_t phaseCombat = 0;

	curseur.x = 2;
	curseur.y = 13;
	curseur.rad = 3;

	Pokemon p[3];
	/* Infinite loop */
	for (;;) {

		//affichage du fond
		 // Effacer la couche inférieure
		  BSP_LCD_SelectLayer(0);
		  //BSP_LCD_Clear(LCD_COLOR_WHITE);

		  // Dessiner le fond sur la couche inférieure
		  BSP_LCD_DrawBitmap(0,0, uwInternelBuffer);


		  /*  Pokemon evoli1;
			Pokemon roucool1;
			Pokemon roucool2;

		  xQueueReceive(QueueP1toAHandle, &evoli1, 0);
		  BSP_LCD_DrawBitmap(evoli1.x*tailleTuile, evoli1.y*tailleTuile, (uint8_t*)spritePoke[evoli1.sprite]);
		  xQueueReceive(QueueP2toAHandle, &roucool1, 0);
		  BSP_LCD_DrawBitmap(roucool1.x*tailleTuile, roucool1.y*tailleTuile, (uint8_t*)spritePoke[roucool1.sprite]);
		  xQueueReceive(QueueP3toAHandle, &roucool2, 0);
		  BSP_LCD_DrawBitmap(roucool2.x*tailleTuile, roucool2.y*tailleTuile, (uint8_t*)spritePoke[roucool2.sprite]);
*/

		  if (joueur.zone == 1){
			  if(objetTrouves[0] == 0){
				  BSP_LCD_DrawBitmap(25*tailleTuile, 1*tailleTuile, (uint8_t*)spriteObjet[0]);
			  }
			  else{
				  BSP_LCD_DrawBitmap(25*tailleTuile, 1*tailleTuile, (uint8_t*)spriteObjet[1]);
			  }
		  }

		  if(joueur.zone == 2){
			  if(objetTrouves[1] == 0){
				  BSP_LCD_DrawBitmap(3*tailleTuile, 4*tailleTuile, (uint8_t*)spriteObjet[0]);
			  }
			  else{
				  BSP_LCD_DrawBitmap(3*tailleTuile, 4*tailleTuile, (uint8_t*)spriteObjet[1]);
			  }
			  if(objetTrouves[2] == 0){
				  BSP_LCD_DrawBitmap(11*tailleTuile, 12*tailleTuile, (uint8_t*)spriteObjet[0]);
			  }
			  else{
				  BSP_LCD_DrawBitmap(11*tailleTuile, 12*tailleTuile, (uint8_t*)spriteObjet[1]);
			  }
		  }

		  if(joueur.zone == 3){
			  if(objetTrouves[3] == 0){
				  BSP_LCD_DrawBitmap(8*tailleTuile, 6*tailleTuile, (uint8_t*)spriteObjet[0]);
			  }
			  else{
				  BSP_LCD_DrawBitmap(8*tailleTuile, 6*tailleTuile, (uint8_t*)spriteObjet[1]);
			  }
			  if(objetTrouves[4] == 0){
				  BSP_LCD_DrawBitmap(17*tailleTuile, 9*tailleTuile, (uint8_t*)spriteObjet[0]);
			  }
			  else{
				  BSP_LCD_DrawBitmap(17*tailleTuile, 9*tailleTuile, (uint8_t*)spriteObjet[1]);
			  }
		  }
		  if(joueur.zone == 4){
			  if(objetTrouves[5] == 0){
				  BSP_LCD_DrawBitmap(26*tailleTuile, 11*tailleTuile, (uint8_t*)spriteObjet[0]);
			  }
			  else{
				  BSP_LCD_DrawBitmap(26*tailleTuile, 11*tailleTuile, (uint8_t*)spriteObjet[1]);
			  }
			  if(objetTrouves[6] == 0){
				  BSP_LCD_DrawBitmap(13*tailleTuile, 1*tailleTuile, (uint8_t*)spriteObjet[0]);
			  }
			  else{
				  BSP_LCD_DrawBitmap(13*tailleTuile, 1*tailleTuile, (uint8_t*)spriteObjet[1]);
			  }
		  }
		  if(joueur.zone == 5){
			  if(objetTrouves[7] == 0){
				  BSP_LCD_DrawBitmap(6*tailleTuile, 12*tailleTuile, (uint8_t*)spriteObjet[0]);
			  }
			  else{
				  BSP_LCD_DrawBitmap(6*tailleTuile, 12*tailleTuile, (uint8_t*)spriteObjet[1]);
			  }
			  if(objetTrouves[8] == 0){
				  BSP_LCD_DrawBitmap(26*tailleTuile, 12*tailleTuile, (uint8_t*)spriteObjet[0]);
			  }
			  else{
				  BSP_LCD_DrawBitmap(26*tailleTuile, 12*tailleTuile, (uint8_t*)spriteObjet[1]);
			  }
		  }

		if(joueur.zone != 0){
			//affichage personnage

			BSP_LCD_SelectLayer(1);
			BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
			BSP_LCD_SetTransparency(1,255);
			BSP_LCD_DrawBitmap(joueur.x*tailleTuile,joueur.y*tailleTuile, (uint8_t*)spriteJoueur[joueur.sprite]);

			// Sélectionner la couche inferieur pour dessiner les objets / pokemons
			BSP_LCD_SelectLayer(0);
			if(joueur.zone != 5){
				  xQueueReceive(QueueP1toAHandle, &p[0], 0);
				  BSP_LCD_DrawBitmap(p[0].x*tailleTuile, p[0].y*tailleTuile, (uint8_t*)spritePoke[p[0].sprite]);
				  xQueueReceive(QueueP2toAHandle, &p[1], 0);
				  BSP_LCD_DrawBitmap(p[1].x*tailleTuile, p[1].y*tailleTuile, (uint8_t*)spritePoke[p[1].sprite]);
				  xQueueReceive(QueueP3toAHandle, &p[2], 0);
				  BSP_LCD_DrawBitmap(p[2].x*tailleTuile, p[2].y*tailleTuile, (uint8_t*)spritePoke[p[2].sprite]);
			}

		}

		if(joueur.zone == 0){
			BSP_LCD_SelectLayer(0);
			BSP_LCD_DrawBitmap(0,0, uwInternelBuffer);
			//affichage sprite
			BSP_LCD_DrawBitmap(20*tailleTuile, 2*tailleTuile, (uint8_t*)spritePoke[pokemonAdverse.spriteC]);
			BSP_LCD_DrawBitmap(6*tailleTuile, 8*tailleTuile, (uint8_t*)spritePoke[joueur.pokemonSelect->spriteD]);

			//affichage PV
			BSP_LCD_SetFont(&Font16);
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			sprintf(text1, "PV : %d/%d", joueur.pokemonSelect->PV, joueur.pokemonSelect->PVMAX );
			BSP_LCD_DisplayStringAt(1*tailleTuile,6*tailleTuile, (uint8_t*) text1, LEFT_MODE);
			sprintf(text1, "PV : %d/%d", pokemonAdverse.PV, pokemonAdverse.PVMAX );
			BSP_LCD_DisplayStringAt(17*tailleTuile,8*tailleTuile, (uint8_t*) text1, LEFT_MODE);

			//affichage nom + niveau
			sprintf(text1, "%s niveau : %d", joueur.pokemonSelect->nom, joueur.pokemonSelect->niveau);
			BSP_LCD_DisplayStringAt(1*tailleTuile,5*tailleTuile, (uint8_t*) text1, LEFT_MODE);
			sprintf(text1, "%s niveau : %d", pokemonAdverse.nom, pokemonAdverse.niveau);
			BSP_LCD_DisplayStringAt(16*tailleTuile,7*tailleTuile, (uint8_t*) text1, LEFT_MODE);

			AfficheRect();

			xQueueReceive(QueueCtoAHandle, &phaseCombat, 0);
			if(phaseCombat == 0){
				//affichage choix attaque
				sprintf(text1, "%s",joueur.pokemonSelect->attaque[0]);
				BSP_LCD_DisplayStringAt(2*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "%s", joueur.pokemonSelect->attaque[1]);
				BSP_LCD_DisplayStringAt(15*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "%s", joueur.pokemonSelect->attaque[2]);
				BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "Changer Poke");
				BSP_LCD_DisplayStringAt(15*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				BSP_LCD_FillCircle(curseur.x*tailleTuile-8,curseur.y*tailleTuile+8, curseur.rad);

			}
			if(phaseCombat == 1){
				sprintf(text1, "%s attaque %s",joueur.pokemonSelect->nom,joueur.pokemonSelect->attaqueSelect);
				BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				if(efficace == 1){
					sprintf(text1, "Ce n'est pas tres efficace...");
					BSP_LCD_DisplayStringAt(2*tailleTuile,15*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				}
				else if(efficace == 2){
					sprintf(text1, "C'est tres efficace !");
					BSP_LCD_DisplayStringAt(2*tailleTuile,15*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				}


			}
			if(phaseCombat == 2){
				sprintf(text1, "%s ennemi attaque %s",pokemonAdverse.nom,pokemonAdverse.attaqueSelect);
				BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				if(efficace == 1){
					sprintf(text1, "Ce n'est pas tres efficace...");
					BSP_LCD_DisplayStringAt(2*tailleTuile,15*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				}
				else if(efficace == 2){
					sprintf(text1, "C'est tres efficace !");
					BSP_LCD_DisplayStringAt(2*tailleTuile,15*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				}


			}
			if(phaseCombat == 3){
				sprintf(text1, "%s ennemi a ete mis KO",pokemonAdverse.nom);
				BSP_LCD_DisplayStringAt(2*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "%s a gagne %d point d'XP",joueur.pokemonSelect->nom, pokemonAdverse.niveau);
				BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
			}
			if(phaseCombat == 4){
				sprintf(text1, "%s a ete mis KO",joueur.pokemonSelect->nom);
				BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);

			}
			if(phaseCombat == 5){//monté de niveau
				sprintf(text1, "%s monte de niveau",joueur.pokemonSelect->nom);
				BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
			}
			if(phaseCombat == 6){//choix changement poke
				sprintf(text1, "%s", joueur.pokemon1.nom);
				BSP_LCD_DisplayStringAt(2*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "%s", joueur.pokemon2.nom);
				BSP_LCD_DisplayStringAt(15*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "%s", joueur.pokemon3.nom);
				BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "%s", joueur.pokemon4.nom);
				BSP_LCD_DisplayStringAt(15*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				BSP_LCD_FillCircle(curseur.x*tailleTuile-8,curseur.y*tailleTuile+8, curseur.rad);
			}
			if(phaseCombat == 7){//Capturer le poke?
				sprintf(text1, "Voulez vous capturer %s?", pokemonAdverse.nom);
				BSP_LCD_DisplayStringAt(2*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "oui");
				BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "non");
				BSP_LCD_DisplayStringAt(15*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				BSP_LCD_FillCircle(curseur.x*tailleTuile-8,curseur.y*tailleTuile+8, curseur.rad);
			}
			if(phaseCombat == 8){//sur quelle slot mettre le poke
				sprintf(text1, "Quel pokemon remplacer?");
				BSP_LCD_DisplayStringAt(2*tailleTuile,12*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "%s", joueur.pokemon1.nom);
				BSP_LCD_DisplayStringAt(2*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "%s", joueur.pokemon2.nom);
				BSP_LCD_DisplayStringAt(15*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "%s", joueur.pokemon3.nom);
				BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				sprintf(text1, "%s", joueur.pokemon4.nom);
				BSP_LCD_DisplayStringAt(15*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				BSP_LCD_FillCircle(curseur.x*tailleTuile-8,curseur.y*tailleTuile+8, curseur.rad);
			}
			if(phaseCombat == 9){ //fin du jeu
				vTaskDelete(task_combatHandle);
				ClearEcran();
				BSP_LCD_SelectLayer(0);
				BSP_LCD_SetFont(&Font16);
				BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
				vTaskDelay(2000);
				sprintf(text1, "Felicitation");
				BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
				vTaskDelay(2000);
				BSP_LCD_Clear(LCD_COLOR_WHITE);
				sprintf(text1, "Vous aver reussi a trouver..");
				BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
				vTaskDelay(2000);
				BSP_LCD_Clear(LCD_COLOR_WHITE);
				sprintf(text1, "...les 3 fragments de la triforce.");
				BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
				vTaskDelay(2000);
				BSP_LCD_Clear(LCD_COLOR_WHITE);
				sprintf(text1, "Et vaincu le terrible Ganondorf." );
				BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
				vTaskDelay(2000);
				BSP_LCD_Clear(LCD_COLOR_WHITE);
				sprintf(text1, "Grace a vous ..." );
				BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
				vTaskDelay(2000);
				BSP_LCD_Clear(LCD_COLOR_WHITE);
				sprintf(text1, "...le monde est sauf");
				BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
				vTaskDelay(2000);
				BSP_LCD_Clear(LCD_COLOR_BLACK);
				vTaskDelete(task_Affich_PicHandle);

			}


		}
		osDelay(100);

		}

  /* USER CODE END Affichage_Pic */
}

/* USER CODE BEGIN Header_deplac_pok */
/**
 * @brief Function implementing the task_deplac_pok thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_deplac_pok */
void deplac_pok(void const * argument)
{
  /* USER CODE BEGIN deplac_pok */

	const TickType_t Delai_Attente = 5000; //5s d'attente maximun
	Pokemon p[3];

	for (;;) {
		if(joueur.zone == 1){
			if(initZ1 == 0){
				p[0] = pokeInit(4,joueur.pokemon1.niveau + rand() % 2);
				p[0].x = 15; p[0].y = 2;
				p[1] = pokeInit(3,2 + rand() % 2);
				p[1].x = 18; p[1].y = 10;
				p[2] = pokeInit(3,2 + rand() % 2);
				p[2].x = 11; p[2].y = 10;
				initZ1 = 1;
			}
			//deplacement des pokemons
			if((p[0].x + 1) < 25 && (p[0].x - 1) > 10){
				p[0].x += (rand() % 3 - 1);
			}
			else if((p[0].x + 1)==25){
				p[0].x -= 1;
			}
			else if((p[0].x - 1)==10){
				p[0].x += 1;
			}
			xQueueSend(QueueP1toAHandle, &p[0], Delai_Attente);
			if((p[1].x + 1) < 20 && (p[1].x - 1) > 16){
				p[1].x += (rand() % 3 - 1);
			}
			else if((p[1].x + 1)==20){
				p[1].x -= 1;
			}
			else if((p[1].x - 1)==16){
				p[1].x += 1;
			}
			if((p[1].y + 1) < 12 && (p[1].y - 1) > 8){
				p[1].y += (rand() % 3 - 1);
			}
			else if((p[1].y + 1)==12){
				p[1].y -= 1;
			}
			else if((p[1].y - 1)==8){
				p[1].y += 1;
			}
			xQueueSend(QueueP2toAHandle, &p[1], Delai_Attente);
			if((p[2].x + 1) < 13 && (p[2].x - 1) > 9){
				p[2].x += (rand() % 3 - 1);
			}
			else if((p[2].x + 1)==13){
				p[2].x -= 1;
			}
			else if((p[2].x - 1)==9){
				p[2].x += 1;
			}
			if((p[2].y + 1) < 12 && (p[2].y - 1) > 8){
				p[2].y += (rand() % 3 - 1);
			}
			else if((p[2].y + 1)==12){
				p[2].y -= 1;
			}
			else if((p[2].y - 1)==8){
				p[2].y += 1;
			}
			xQueueSend(QueueP3toAHandle, &p[2], Delai_Attente);
		}
		else if(joueur.zone == 2){
			if(initZ2 == 0){
				p[0] = pokeInit(1,joueur.pokemon1.niveau + rand() % 2);
				p[0].x = 8; p[0].y = 13;
				p[1] = pokeInit(5,3 + rand() % 2);
				p[1].x = 5; p[1].y = 7;
				p[2] = pokeInit(5,3 + rand() % 2);
				p[2].x = 20; p[2].y = 6;
				initZ2 = 1;
			}
			//deplacement des pokemons
			if((p[0].x + 1) < 10 && (p[0].x - 1) > 2){
				p[0].x += (rand() % 3 - 1);
			}
			else if((p[0].x + 1)==10){
				p[0].x -= 1;
			}
			else if((p[0].x - 1)==2){
				p[0].x += 1;
			}
			xQueueSend(QueueP1toAHandle, &p[0], Delai_Attente);
			if((p[1].x + 1) < 7 && (p[1].x - 1) > 2){
				p[1].x += (rand() % 3 - 1);
			}
			else if((p[1].x + 1)==7){
				p[1].x -= 1;
			}
			else if((p[1].x - 1)==2){
				p[1].x += 1;
			}
			if((p[1].y + 1) < 12 && (p[1].y - 1) > 5){
				p[1].y += (rand() % 3 - 1);
			}
			else if((p[1].y + 1)==12){
				p[1].y -= 1;
			}
			else if((p[1].y - 1)==5){
				p[1].y += 1;
			}
			xQueueSend(QueueP2toAHandle, &p[1], Delai_Attente);
			if((p[2].x + 1) < 23 && (p[2].x - 1) > 14){
				p[2].x += (rand() % 3 - 1);
			}
			else if((p[2].x + 1)==23){
				p[2].x -= 1;
			}
			else if((p[2].x - 1)==14){
				p[2].x += 1;
			}
			if((p[2].y + 1) < 9 && (p[2].y - 1) > 3){
				p[2].y += (rand() % 3 - 1);
			}
			else if((p[2].y + 1)==9){
				p[2].y -= 1;
			}
			else if((p[2].y - 1)==3){
				p[2].y += 1;
			}
			xQueueSend(QueueP3toAHandle, &p[2], Delai_Attente);
		}

		else if(joueur.zone == 3){
			if(initZ3 == 0){
				p[0] = pokeInit(2,joueur.pokemon1.niveau + 2 + rand() % 2);
				p[0].x = 8; p[0].y = 7;
				p[1] = pokeInit(6,5 + rand() % 2);
				p[1].x = 12; p[1].y = 11;
				p[2] = pokeInit(6,5 + rand() % 2);
				p[2].x = 22; p[2].y = 5;
				initZ3 = 1;
			}
			//deplacement des pokemons
			if((p[0].x + 1) < 14 && (p[0].x - 1) > 6){
				p[0].x += (rand() % 3 - 1);
			}
			else if((p[0].x + 1)==14){
				p[0].x -= 1;
			}
			else if((p[0].x - 1)==6){
				p[0].x += 1;
			}
			xQueueSend(QueueP1toAHandle, &p[0], Delai_Attente);
			if((p[1].x + 1) < 17 && (p[1].x - 1) > 12){
				p[1].x += (rand() % 3 - 1);
			}
			else if((p[1].x + 1)==17){
				p[1].x -= 1;
			}
			else if((p[1].x - 1)==12){
				p[1].x += 1;
			}
			if((p[1].y + 1) < 12 && (p[1].y - 1) > 9){
				p[1].y += (rand() % 3 - 1);
			}
			else if((p[1].y + 1)==12){
				p[1].y -= 1;
			}
			else if((p[1].y - 1)==9){
				p[1].y += 1;
			}
			xQueueSend(QueueP2toAHandle, &p[1], Delai_Attente);
			if((p[2].x + 1) < 26 && (p[2].x - 1) > 20){
				p[2].x += (rand() % 3 - 1);
			}
			else if((p[2].x + 1)==26){
				p[2].x -= 1;
			}
			else if((p[2].x - 1)==20){
				p[2].x += 1;
			}
			if((p[2].y + 1) < 9 && (p[2].y - 1) > 3){
				p[2].y += (rand() % 3 - 1);
			}
			else if((p[2].y + 1)==9){
				p[2].y -= 1;
			}
			else if((p[2].y - 1)==3){
				p[2].y += 1;
			}
			xQueueSend(QueueP3toAHandle, &p[2], Delai_Attente);

		}

		else if(joueur.zone == 4){
			if(initZ4 == 0){
				p[0] = pokeInit(7,9 + rand() % 2);
				p[0].x = 14; p[0].y = 12;
				p[1] = pokeInit(0,joueur.pokemon1.niveau + 2 + rand() % 2);
				p[1].x = 11; p[1].y = 5;
				p[2] = pokeInit(7,9 + rand() % 2);
				p[2].x = 17; p[2].y = 2;
				initZ4 = 1;
			}
			//deplacement des pokemons
			if((p[0].x + 1) < 20 && (p[0].x - 1) > 12){
				p[0].x += (rand() % 3 - 1);
			}
			else if((p[0].x + 1)==20){
				p[0].x -= 1;
			}
			else if((p[0].x - 1)==12){
				p[0].x += 1;
			}
			xQueueSend(QueueP1toAHandle, &p[0], Delai_Attente);
			if((p[1].x + 1) < 14 && (p[1].x - 1) > 9){
				p[1].x += (rand() % 3 - 1);
			}
			else if((p[1].x + 1)==14){
				p[1].x -= 1;
			}
			else if((p[1].x - 1)==9){
				p[1].x += 1;
			}
			if((p[1].y + 1) < 7 && (p[1].y - 1) > 3){
				p[1].y += (rand() % 3 - 1);
			}
			else if((p[1].y + 1)==7){
				p[1].y -= 1;
			}
			else if((p[1].y - 1)==3){
				p[1].y += 1;
			}
			xQueueSend(QueueP2toAHandle, &p[1], Delai_Attente);
			if((p[2].x + 1) < 26 && (p[2].x - 1) > 16){
				p[2].x += (rand() % 3 - 1);
			}
			else if((p[2].x + 1)==16){
				p[2].x -= 1;
			}
			else if((p[2].x - 1)==26){
				p[2].x += 1;
			}
			if((p[2].y + 1) < 4 && (p[2].y - 1) > 1){
				p[2].y += (rand() % 3 - 1);
			}
			else if((p[2].y + 1)==4){
				p[2].y -= 1;
			}
			else if((p[2].y - 1)==1){
				p[2].y += 1;
			}
			xQueueSend(QueueP3toAHandle, &p[2], Delai_Attente);

		}
		else if (joueur.zone == 5){
			p[0].x=-1;
			p[0].y=-1;
			xQueueSend(QueueP3toAHandle, &p[0], Delai_Attente);
			p[1].x=-1;
			p[1].y=-1;
			xQueueSend(QueueP3toAHandle, &p[1], Delai_Attente);
			p[2].x=-1;
			p[2].y=-1;
			xQueueSend(QueueP3toAHandle, &p[2], Delai_Attente);


		}

		///////////////Gestion des collisions et landcement des combats///////////////////////
		if (collision(joueur, p[0])) {
			//disparition du poke
			p[0].x = -1; p[0].y  = -1;
			vTaskSuspend(task_Affich_PicHandle);
			//pokemon adverse pour le combat
			pokemonAdverse = p[0];
			joueur.zonePred = joueur.zone;
			joueur.zone = 0; //zone de combat
			xTaskCreate(combat, "task_combat", 1024, NULL, osPriorityHigh, &task_combatHandle);

		}
		if (collision(joueur, p[1])) {
			p[1].x = -1; p[1].y  = -1;
			//pokemon adverse pour le combat
			pokemonAdverse = p[1];
			vTaskSuspend(task_Affich_PicHandle);
			joueur.zonePred = joueur.zone;
			joueur.zone = 0; //zone de combat
			xTaskCreate(combat, "task_combat", 1024, NULL, osPriorityHigh, &task_combatHandle);

		}

	    if (collision(joueur, p[2])) {
	    	p[2].x = -1; p[2].y  = -1;
			pokemonAdverse = p[2];
			vTaskSuspend(task_Affich_PicHandle);
			joueur.zonePred = joueur.zone;
			joueur.zone = 0; //zone de combat
			xTaskCreate(combat, "task_combat", 1024, NULL, osPriorityHigh, &task_combatHandle);

	    }
		osDelay(150);

		}

  /* USER CODE END deplac_pok */
}

/* USER CODE BEGIN Header_deplacement */
/**
 * @brief Function implementing the task_deplacement thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_deplacement */
void deplacement(void const * argument)
{
  /* USER CODE BEGIN deplacement */
	Pokemon *p;
	uint8_t sac = 0;
	char text1[50] = { };
	Curseur c;
	c.x = 2;
	c.y = 2;
	c.rad = 3;
	uint8_t nbfrag = 0;

	//fonction pour selectionner le pokemon dans l'équipe
	void selectPoke(){
		c.x = 2;
		c.y = 13;
		vTaskDelay(500);
		while(HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) != 0){
			vTaskDelay(50);
			AfficheRect();
			sprintf(text1, "Ou le placer?");
			BSP_LCD_DisplayStringAt(8*tailleTuile,12*tailleTuile, (uint8_t*) text1, LEFT_MODE);
			sprintf(text1, "%s %d/%d", joueur.pokemon1.nom, joueur.pokemon1.PV, joueur.pokemon1.PVMAX );
			BSP_LCD_DisplayStringAt(2*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
			sprintf(text1, "%s %d/%d", joueur.pokemon2.nom, joueur.pokemon2.PV, joueur.pokemon2.PVMAX);
			BSP_LCD_DisplayStringAt(15*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
			sprintf(text1, "%s %d/%d", joueur.pokemon3.nom, joueur.pokemon3.PV, joueur.pokemon3.PVMAX);
			BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
			sprintf(text1, "%s %d/%d", joueur.pokemon4.nom, joueur.pokemon4.PV, joueur.pokemon4.PVMAX);
			BSP_LCD_DisplayStringAt(15*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
			BSP_LCD_FillCircle(c.x*tailleTuile-8,c.y*tailleTuile+8, curseur.rad);

			ValJoystick();

			//deplacement du curseur
			if(joystick_h>3500 && joystick_v<3500 && joystick_v>1500 && c.x == 15){
				c.x = 2; //gauche
			}
			else if(joystick_h<1500 && joystick_v<3500 && joystick_v>1500 && c.x == 2	){
				c.x = 15; //droite
			}
			else if(joystick_v<1500 && joystick_h<3500 && joystick_h>1500 &&  c.y == 13){
				c.y = 14; //bas
			}
			else if(joystick_v>3500 && joystick_h<3500 && joystick_h>1500 &&  c.y == 14){
				c.y = 13; //haut
			}
		}
	}

	//fonction pour intervertir deux pokemons
	void swap(Pokemon *ptr1, Pokemon *ptr2) {
	    Pokemon temp = *ptr1; // Stocker la valeur pointée par ptr1 dans une variable temporaire
	    *ptr1 = *ptr2; // Copier la valeur pointée par ptr2 dans ptr1
	    *ptr2 = temp; // Copier la valeur temporaire dans ptr2
	}

	int i;

	/* Infinite loop */
	for (;;) {

		ValJoystick();

		if(HAL_GPIO_ReadPin(BP2_GPIO_Port, BP2_Pin) == 0 || sac == 1){//ouverture du sac

			sac = 1;
			vTaskSuspend(task_deplac_pokHandle);
			vTaskSuspend(task_Affich_PicHandle);

			ValJoystick();

			//affichage rectangle de texte
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_DrawRect(1*tailleTuile, 2*tailleTuile-5, 10*tailleTuile, 3*tailleTuile);
			BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
			BSP_LCD_FillRect(1*tailleTuile+1, 2*tailleTuile+1-5, 10*tailleTuile-1, 3*tailleTuile-1);
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_FillCircle(c.x*tailleTuile-8,c.y*tailleTuile+8, curseur.rad);
			sprintf(text1, "Objets");
			BSP_LCD_DisplayStringAt(2*tailleTuile,2*tailleTuile, (uint8_t*) text1, LEFT_MODE);
			sprintf(text1, "Pokemons");
			BSP_LCD_DisplayStringAt(2*tailleTuile,3*tailleTuile, (uint8_t*) text1, LEFT_MODE);

			if(joystick_v<1500 && joystick_h<3500 && joystick_h>1500 &&  c.y == 2){
				c.y +=1; //bas
			}
			else if(joystick_v>3500 && joystick_h<3500 && joystick_h>1500 &&  c.y == 3){
				c.y -=1; //haut
			}

			if(HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) == 0){
				if (c.y == 2){//Selection objets
					vTaskDelay(500);
					while(HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) != 0){

						ValJoystick();
						vTaskDelay(50);
						//affichage rectangle de texte
						BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
						BSP_LCD_DrawRect(1*tailleTuile, 1*tailleTuile-5, 5*tailleTuile, 12*tailleTuile);
						BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
						BSP_LCD_FillRect(1*tailleTuile+1, 1*tailleTuile+1-5, 5*tailleTuile-1, 12*tailleTuile-1);
						BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
						sprintf(text1, "Objets");
						BSP_LCD_DisplayStringAt(1*tailleTuile,1*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						//affichage des objets
						for(i = 0;i<10;i++){
							sprintf(text1, "%s", joueur.objet[i].nom);
							BSP_LCD_DisplayStringAt(2*tailleTuile,(i+2)*tailleTuile, (uint8_t*) text1, LEFT_MODE);

						}
						sprintf(text1, "Sortir");
						BSP_LCD_DisplayStringAt(2*tailleTuile,12*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						BSP_LCD_FillCircle(c.x*tailleTuile-8,c.y*tailleTuile+8, c.rad);
						if(joystick_v<1500 && joystick_h<3500 && joystick_h>1500 &&  c.y < 12){
							c.y +=1; //bas
						}
						else if(joystick_v>3500 && joystick_h<3500 && joystick_h>1500 &&  c.y > 2){
							c.y -=1; //haut
						}
					}
						vTaskDelay(500);
						if(c.y == 12){
							sac = 0;
							c.x = 2;
							c.y = 2;
							vTaskResume(task_deplac_pokHandle);
							vTaskResume(task_Affich_PicHandle);
						}

						else if(strcmp(joueur.objet[c.y-2].nom,"Potion") == 0){
							strcpy(joueur.objet[c.y-2].nom, "/0");
							c.x = 2;
							c.y = 13;
							while(HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) != 0){
								vTaskDelay(50);
								AfficheRect();
								sprintf(text1, "Quel pokemon soigner?");
								BSP_LCD_DisplayStringAt(8*tailleTuile,12*tailleTuile, (uint8_t*) text1, LEFT_MODE);
								sprintf(text1, "%s %d/%d", joueur.pokemon1.nom, joueur.pokemon1.PV, joueur.pokemon1.PVMAX );
								BSP_LCD_DisplayStringAt(2*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
								sprintf(text1, "%s %d/%d", joueur.pokemon2.nom, joueur.pokemon2.PV, joueur.pokemon2.PVMAX);
								BSP_LCD_DisplayStringAt(15*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
								sprintf(text1, "%s %d/%d", joueur.pokemon3.nom, joueur.pokemon3.PV, joueur.pokemon3.PVMAX);
								BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
								sprintf(text1, "%s %d/%d", joueur.pokemon4.nom, joueur.pokemon4.PV, joueur.pokemon4.PVMAX);
								BSP_LCD_DisplayStringAt(15*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
								BSP_LCD_FillCircle(c.x*tailleTuile-8,c.y*tailleTuile+8, curseur.rad);

								ValJoystick();

								//deplacement du curseur
								if(joystick_h>3500 && joystick_v<3500 && joystick_v>1500 && c.x == 15){
									c.x = 2; //gauche
								}
								else if(joystick_h<1500 && joystick_v<3500 && joystick_v>1500 && c.x == 2	){
									c.x = 15; //droite
								}
								else if(joystick_v<1500 && joystick_h<3500 && joystick_h>1500 &&  c.y == 13){
									c.y = 14; //bas
								}
								else if(joystick_v>3500 && joystick_h<3500 && joystick_h>1500 &&  c.y == 14){
									c.y = 13; //haut
								}

							}

							//pokemon a soigner
							if(c.x ==2 && c.y == 13){
								joueur.pokemon1.PV = joueur.pokemon1.PVMAX;
							}
							if(c.x ==15 && c.y == 13){
								joueur.pokemon2.PV = joueur.pokemon2.PVMAX;
							}
							if(c.x ==2 && c.y == 14){
								joueur.pokemon3.PV = joueur.pokemon3.PVMAX;
							}
							if(c.x ==15 && c.y == 14){
								joueur.pokemon4.PV = joueur.pokemon4.PVMAX;
							}

							AfficheRect();
							sprintf(text1, "%s %d/%d", joueur.pokemon1.nom, joueur.pokemon1.PV, joueur.pokemon1.PVMAX );
							BSP_LCD_DisplayStringAt(2*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
							sprintf(text1, "%s %d/%d", joueur.pokemon2.nom, joueur.pokemon2.PV, joueur.pokemon2.PVMAX);
							BSP_LCD_DisplayStringAt(15*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
							sprintf(text1, "%s %d/%d", joueur.pokemon3.nom, joueur.pokemon3.PV, joueur.pokemon3.PVMAX);
							BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
							sprintf(text1, "%s %d/%d", joueur.pokemon4.nom, joueur.pokemon4.PV, joueur.pokemon4.PVMAX);
							BSP_LCD_DisplayStringAt(15*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
							vTaskDelay(500);
							sac = 0;
							vTaskResume(task_deplac_pokHandle);
							vTaskResume(task_Affich_PicHandle);
							c.x = 2;
							c.y = 2;
						}
						else{
							sac = 0;
							vTaskResume(task_deplac_pokHandle);
							vTaskResume(task_Affich_PicHandle);
							c.x = 2;
							c.y = 2;
						}
					}


				else{
					c.x = 2;
					c.y = 13;
					vTaskDelay(500);
					while(HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) != 0){
						vTaskDelay(50);
						AfficheRect();
						sprintf(text1, "Quel pokemon deplacer?");
						BSP_LCD_DisplayStringAt(8*tailleTuile,12*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						sprintf(text1, "%s %d/%d", joueur.pokemon1.nom, joueur.pokemon1.PV, joueur.pokemon1.PVMAX );
						BSP_LCD_DisplayStringAt(2*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						sprintf(text1, "%s %d/%d", joueur.pokemon2.nom, joueur.pokemon2.PV, joueur.pokemon2.PVMAX);
						BSP_LCD_DisplayStringAt(15*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						sprintf(text1, "%s %d/%d", joueur.pokemon3.nom, joueur.pokemon3.PV, joueur.pokemon3.PVMAX);
						BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						sprintf(text1, "%s %d/%d", joueur.pokemon4.nom, joueur.pokemon4.PV, joueur.pokemon4.PVMAX);
						BSP_LCD_DisplayStringAt(15*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						BSP_LCD_FillCircle(c.x*tailleTuile-8,c.y*tailleTuile+8, curseur.rad);

						ValJoystick();

						//deplacement du curseur
						if(joystick_h>3500 && joystick_v<3500 && joystick_v>1500 && c.x == 15){
							c.x = 2; //gauche
						}
						else if(joystick_h<1500 && joystick_v<3500 && joystick_v>1500 && c.x == 2	){
							c.x = 15; //droite
						}
						else if(joystick_v<1500 && joystick_h<3500 && joystick_h>1500 &&  c.y == 13){
							c.y = 14; //bas
						}
						else if(joystick_v>3500 && joystick_h<3500 && joystick_h>1500 &&  c.y == 14){
							c.y = 13; //haut
						}

					}

						if(c.x ==2 && c.y == 13){
							p = &joueur.pokemon1;
						}
						if(c.x ==15 && c.y == 13){
							p = &joueur.pokemon2;
						}
						if(c.x ==2 && c.y == 14){
							p = &joueur.pokemon3;
						}
						if(c.x ==15 && c.y == 14){
							p = &joueur.pokemon4;
						}
						selectPoke();
						if(c.x ==2 && c.y == 13){
							swap(p, &joueur.pokemon1);
						}
						if(c.x ==15 && c.y == 13){
							swap(p, &joueur.pokemon2);
						}
						if(c.x ==2 && c.y == 14){
							swap(p, &joueur.pokemon3);
						}
						if(c.x ==15 && c.y == 14){
							swap(p, &joueur.pokemon4);
						}

						AfficheRect();
						sprintf(text1, "Ou le placer?");
						BSP_LCD_DisplayStringAt(8*tailleTuile,12*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						sprintf(text1, "%s %d/%d", joueur.pokemon1.nom, joueur.pokemon1.PV, joueur.pokemon1.PVMAX );
						BSP_LCD_DisplayStringAt(2*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						sprintf(text1, "%s %d/%d", joueur.pokemon2.nom, joueur.pokemon2.PV, joueur.pokemon2.PVMAX);
						BSP_LCD_DisplayStringAt(15*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						sprintf(text1, "%s %d/%d", joueur.pokemon3.nom, joueur.pokemon3.PV, joueur.pokemon3.PVMAX);
						BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						sprintf(text1, "%s %d/%d", joueur.pokemon4.nom, joueur.pokemon4.PV, joueur.pokemon4.PVMAX);
						BSP_LCD_DisplayStringAt(15*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						vTaskDelay(500);
						sac = 0;
						vTaskResume(task_deplac_pokHandle);
						vTaskResume(task_Affich_PicHandle);
						c.x = 2;
						c.y = 2;

					}

				}



		}
		else{
			//mouvement en fonction des données (envoie dans une file d'attente vers affichage)

			if(joystick_h>3500 && joystick_v<3500 && joystick_v>1500){
				joueur.x -= 1; //gauche
				joueur.sprite = 2;
			}
			else if(joystick_h<1500 && joystick_v<3500 && joystick_v>1500){
				joueur.x += 1; //droite
				joueur.sprite = 1;
			}
			else if(joystick_v<1500 && joystick_h<3500 && joystick_h>1500){
				joueur.y += 1; //bas
				joueur.sprite = 0;
			}
			else if(joystick_v>3500 && joystick_h<3500 && joystick_h>1500){
				joueur.y -= 1; //haut
				joueur.sprite = 3;
			}
			//limitation des murs
			if(carte[joueur.x][joueur.y] == 0){
				switch(joueur.sprite){
				case 0 :
					joueur.y -= 1;
					break;
				case 1 :
					joueur.x -= 1;
					break;
				case 2 :
					joueur.x += 1;
					break;
				case 3 :
					joueur.y += 1;
					break;
				}

			}
			//changement de zone
			if(carte[joueur.x][joueur.y] == 12){
				joueur.zonePred = joueur.zone;
				joueur.zone = 2;

				vTaskSuspend(task_deplac_pokHandle);
				vTaskSuspend(task_Affich_PicHandle);
				ClearEcran();
				vTaskDelay(500);
				vTaskResume(task_chgZoneHandle);
				vTaskSuspend(task_deplacementHandle);
			}
			if(carte[joueur.x][joueur.y] == 21){
				joueur.zonePred = joueur.zone;
				joueur.zone = 1;

				vTaskSuspend(task_deplac_pokHandle);
				vTaskSuspend(task_Affich_PicHandle);
				ClearEcran();
				vTaskDelay(500);
				vTaskResume(task_chgZoneHandle);
				vTaskSuspend(task_deplacementHandle);
			}
			if(carte[joueur.x][joueur.y] == 32){
				joueur.zonePred = joueur.zone;
				joueur.zone = 2;
				vTaskSuspend(task_deplac_pokHandle);
				vTaskSuspend(task_Affich_PicHandle);
				ClearEcran();
				vTaskDelay(500);
				vTaskResume(task_chgZoneHandle);
				vTaskSuspend(task_deplacementHandle);
			}
			if(carte[joueur.x][joueur.y] == 23){
				joueur.zonePred = joueur.zone;
				joueur.zone = 3;
				vTaskSuspend(task_deplac_pokHandle);
				vTaskSuspend(task_Affich_PicHandle);
				ClearEcran();
				vTaskDelay(500);
				vTaskResume(task_chgZoneHandle);
				vTaskSuspend(task_deplacementHandle);
			}
			if(carte[joueur.x][joueur.y] == 34){
				joueur.zonePred = joueur.zone;
				joueur.zone = 4;
				vTaskSuspend(task_deplac_pokHandle);
				vTaskSuspend(task_Affich_PicHandle);
				ClearEcran();
				vTaskDelay(500);
				vTaskResume(task_chgZoneHandle);
				vTaskSuspend(task_deplacementHandle);
			}
			if(carte[joueur.x][joueur.y] == 43){
				joueur.zonePred = joueur.zone;
				joueur.zone = 3;
				vTaskSuspend(task_deplac_pokHandle);
				vTaskSuspend(task_Affich_PicHandle);
				ClearEcran();
				vTaskDelay(500);
				vTaskResume(task_chgZoneHandle);
				vTaskSuspend(task_deplacementHandle);
			}
			if(carte[joueur.x][joueur.y] == 45){
				joueur.zonePred = joueur.zone;
				joueur.zone = 5;
				vTaskSuspend(task_deplac_pokHandle);
				vTaskSuspend(task_Affich_PicHandle);
				ClearEcran();
				vTaskDelay(500);
				vTaskResume(task_chgZoneHandle);
				vTaskSuspend(task_deplacementHandle);
			}
			if(carte[joueur.x][joueur.y] == 54){
				joueur.zonePred = joueur.zone;
				joueur.zone = 4;
				vTaskSuspend(task_deplac_pokHandle);
				vTaskSuspend(task_Affich_PicHandle);
				ClearEcran();
				vTaskDelay(500);
				vTaskResume(task_chgZoneHandle);
				vTaskSuspend(task_deplacementHandle);
			}
			//objet trouvé?
			if(carte[joueur.x][joueur.y]!=4 && carte[joueur.x][joueur.y]!=1 && carte[joueur.x][joueur.y]!=0 && HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) == 0 && joueur.sprite == 3){

				vTaskSuspend(task_deplac_pokHandle);
				vTaskSuspend(task_Affich_PicHandle);
				BSP_LCD_SelectLayer(0);
				AfficheRect();
				Objet objet;
				if(carte[joueur.x][joueur.y]==2){
					strcpy(objet.nom, "Potion");
					sprintf(text1, "Vous avez trouve %s", objet.nom);
					BSP_LCD_DisplayStringAt(4*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				}
				else if(carte[joueur.x][joueur.y]==3){
					strcpy(objet.nom, "Frag Triforce");
					sprintf(text1, "Vous avez trouve %s", objet.nom);
					BSP_LCD_DisplayStringAt(1*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
				}
				vTaskDelay(1000);
				vTaskResume(task_deplac_pokHandle);
				vTaskResume(task_Affich_PicHandle);
				//autre objet
				int i = 0;
				while(strcmp(joueur.objet[i].nom,"\0")!=0){
					i++;
				}
				if(i>9){
					vTaskSuspend(task_deplac_pokHandle);
					vTaskSuspend(task_Affich_PicHandle);
					BSP_LCD_SelectLayer(0);
					AfficheRect();
					sprintf(text1, "Sac plein");
					BSP_LCD_DisplayStringAt(8*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
					vTaskDelay(1000);
					vTaskResume(task_deplac_pokHandle);
					vTaskResume(task_Affich_PicHandle);
				}
				else{
					if(joueur.zone == 1){
						objetTrouves[0] = 1;
					}
					if(joueur.zone == 2 && joueur.x ==3 && joueur.y ==5){
						objetTrouves[1] = 1;
					}
					if(joueur.zone == 2 && joueur.x ==11 && joueur.y ==13){
						objetTrouves[2] = 1;
					}
					if(joueur.zone == 3 && joueur.x ==8 && joueur.y ==7){
						objetTrouves[3] = 1;
					}
					if(joueur.zone == 3 && joueur.x ==17 && joueur.y ==10){
						objetTrouves[4] = 1;
					}
					if(joueur.zone == 4 && joueur.x ==26 && joueur.y ==12){
						objetTrouves[5] = 1;
					}
					if(joueur.zone == 4 && joueur.x ==13 && joueur.y ==2){
						objetTrouves[6] = 1;
					}
					if(joueur.zone == 5 && joueur.x ==6 && joueur.y ==13){
						objetTrouves[7] = 1;
					}
					if(joueur.zone == 5 && joueur.x ==26 && joueur.y ==13){
						objetTrouves[8] = 1;
					}
					joueur.objet[i] = objet;
					carte[joueur.x][joueur.y] = 1; //retirer l'objet de la carte
				}


			}
			//panneau zone 5
			if(joueur.zone == 5 && carte[joueur.x][joueur.y] == 4 && HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) == 0){

				nbfrag = 0;
				for(i=0;i<10;i++){
					if(strcmp(joueur.objet[i].nom,"Frag Triforce")==0){
						nbfrag++;
					}
				}
				vTaskSuspend(task_deplac_pokHandle);
				vTaskSuspend(task_Affich_PicHandle);
				BSP_LCD_SelectLayer(0);
				vTaskDelay(250);
				if(nbfrag == 3){
					c.x = 2;
					c.y = 15;
					while(HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) != 0){
						ValJoystick();
						vTaskDelay(50);
						AfficheRect();
						sprintf(text1, "Voulez vous placez les fragaments?");
						BSP_LCD_DisplayStringAt(1*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						sprintf(text1, "Oui");
						BSP_LCD_DisplayStringAt(2*tailleTuile,15*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						sprintf(text1, "Non?");
						BSP_LCD_DisplayStringAt(15*tailleTuile,15*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						BSP_LCD_FillCircle(c.x*tailleTuile-8,c.y*tailleTuile+8, curseur.rad);

						//deplacement du curseur
						if(joystick_h>3500 && joystick_v<3500 && joystick_v>1500 && c.x == 15){
							c.x = 2; //gauche
						}
						else if(joystick_h<1500 && joystick_v<3500 && joystick_v>1500 && c.x == 2	){
							c.x = 15; //droite
						}
					}
					if(c.x == 2){//lancement du combat de boss
						AfficheRect();
						sprintf(text1, "Vous sentez une presence...");
						BSP_LCD_DisplayStringAt(1*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						vTaskDelay(2000);
						AfficheRect();
						sprintf(text1, "...malefique");
						BSP_LCD_DisplayStringAt(1*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
						vTaskDelay(2000);
						Pokemon boss = pokeInit(8,12);
						pokemonAdverse = boss;
						joueur.zonePred = joueur.zone;
						joueur.zone = 0; //zone de combat
						xTaskCreate(combat, "task_combat", 1024, NULL, osPriorityHigh, &task_combatHandle);
					}

				}
				else{
					AfficheRect();
					sprintf(text1, "Vous n'avez pas trouve tous les fragments");
					BSP_LCD_DisplayStringAt(1*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
					vTaskDelay(2000);
				}
				vTaskDelay(250);
				vTaskResume(task_deplac_pokHandle);
				vTaskResume(task_Affich_PicHandle);
			}
		}

		osDelay(100);
		}
  /* USER CODE END deplacement */
}

/* USER CODE BEGIN Header_combat */
/**
 * @brief Function implementing the task_combat thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_combat */
void combat(void const * argument)
{
  /* USER CODE BEGIN combat */
	uint8_t phaseCombat=0;
	char text1[50] = { };
	ClearEcran();
	if(joueur.pokemon1.PV > 0){
		joueur.pokemonSelect = &joueur.pokemon1;
	}
	else if(joueur.pokemon2.PV > 0){
		joueur.pokemonSelect = &joueur.pokemon2;
	}
	else if(joueur.pokemon3.PV > 0){
		joueur.pokemonSelect = &joueur.pokemon3;
	}
	else{
		joueur.pokemonSelect = &joueur.pokemon4;
	}
	vTaskSuspend(Play_wavHandle);
	BSP_AUDIO_OUT_Pause();
	chgEcran(5);
	Charge_Wave(2);
	BSP_AUDIO_OUT_Resume();
	vTaskResume(Play_wavHandle);
	vTaskResume(task_Affich_PicHandle);
	vTaskSuspend(task_deplacementHandle);
	vTaskSuspend(task_deplac_pokHandle);
	Pokemon p;

	xQueueSend(QueueCtoAHandle, &phaseCombat, 0);

	uint8_t phase1_fini = 0;
	uint8_t phase2_fini = 0;
	curseur.x = 2;
	curseur.y = 13;

	/* Infinite loop */
	for (;;) {

		//PHASE 0 : Choix de l'attaque
		if(phaseCombat == 0){
			phase1_fini = 0;
			phase2_fini = 0;

			ValJoystick();

			//deplacement du curseur
			if(joystick_h>3500 && joystick_v<3500 && joystick_v>1500 && curseur.x == 15){
				curseur.x = 2; //gauche
			}
			else if(joystick_h<1500 && joystick_v<3500 && joystick_v>1500 && curseur.x == 2	){
				curseur.x = 15; //droite
			}
			else if(joystick_v<1500 && joystick_h<3500 && joystick_h>1500 &&  curseur.y == 13){
				curseur.y = 14; //bas
			}
			else if(joystick_v>3500 && joystick_h<3500 && joystick_h>1500 &&  curseur.y == 14){
				curseur.y = 13; //haut
			}

			//validation du choix de l'attaque
			if(HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) == 0){
				//choix de l'attaque adverse (a completer)
				if(pokemonAdverse.type == 5){
					if(joueur.pokemonSelect->faiblesse == 3){
						strcpy(pokemonAdverse.attaqueSelect , pokemonAdverse.attaque[2]); //bulle d'eau
					}
					if(joueur.pokemonSelect->faiblesse == 4){
						strcpy(pokemonAdverse.attaqueSelect , pokemonAdverse.attaque[1]); //tranche herbe
					}
					if(joueur.pokemonSelect->faiblesse == 2){
						strcpy(pokemonAdverse.attaqueSelect , pokemonAdverse.attaque[0]); //flameche
					}
				}
				else{
					if(joueur.pokemonSelect->faiblesse == pokemonAdverse.type){
						strcpy(pokemonAdverse.attaqueSelect , pokemonAdverse.attaque[0]);
					}
					else{
						strcpy(pokemonAdverse.attaqueSelect , pokemonAdverse.attaque[rand()%3]);
					}
				}
				//choix joueur chg poke
				if(curseur.x == 15 && curseur.y == 14){
					phaseCombat = 6;
					xQueueSend(QueueCtoAHandle, &phaseCombat, 0);
					curseur.x = 2;
					curseur.y = 13;
					vTaskDelay(500);
				}
				//choix joueur attaque
				else{
					if(curseur.x ==2 && curseur.y == 13){
						strcpy(joueur.pokemonSelect->attaqueSelect , joueur.pokemonSelect->attaque[0]);
					}
					if(curseur.x ==15 && curseur.y == 13){
						strcpy(joueur.pokemonSelect->attaqueSelect , joueur.pokemonSelect->attaque[1]);
					}
					if(curseur.x ==2 && curseur.y == 14){
						strcpy(joueur.pokemonSelect->attaqueSelect , joueur.pokemonSelect->attaque[2]);
					}

					if(joueur.pokemonSelect->vitesse > pokemonAdverse.vitesse){
						phaseCombat = 1;
					}
					else{
						phaseCombat = 2;
					}

				}

				xQueueSend(QueueCtoAHandle, &phaseCombat, 0);

			}
		}

		//PHASE 1 (Joueur attaque adverse)
		if(phaseCombat == 1){
			xQueueSend(QueueCtoAHandle, &phaseCombat, 0);
			vTaskDelay(2000);
			efficace = degatAttaque(joueur.pokemonSelect,&pokemonAdverse); //p1 attaque p2

			if(pokemonAdverse.PV <=0){ //si le poke adverse a ete mis KO => fin de combat
				phaseCombat = 3;
				pokemonAdverse.PV = 0;
			}
			else if(phase2_fini == 1){
				phaseCombat = 0;
			}
			else{
				phaseCombat = 2;
			}

			vTaskDelay(1000);
			efficace = 0;
			phase1_fini = 1;
			xQueueSend(QueueCtoAHandle, &phaseCombat, 0);
		}

		//PHASE 2 (adverse attaque joueur)
		if(phaseCombat == 2){
			xQueueSend(QueueCtoAHandle, &phaseCombat, 0);
			//HAL_Delay(2000);
			vTaskDelay(2000);
			//modification de stats en fonction de l'attaque
			efficace = degatAttaque(&pokemonAdverse,joueur.pokemonSelect);

			if(joueur.pokemonSelect->PV <=0){
				joueur.pokemonSelect->PV = 0;
				phaseCombat = 4;
			}
			else if(phase1_fini == 1){
				phaseCombat = 0;
			}
			else{
				phaseCombat = 1;
			}
			phase2_fini = 1;
			vTaskDelay(1000);
			efficace = 0;
			xQueueSend(QueueCtoAHandle, &phaseCombat, 0);
		}
		//PHASE 3 defaite du pokemon adverse
		if(phaseCombat == 3){

			//attribution de l'XP
			joueur.pokemonSelect->EXP += pokemonAdverse.niveau;
			vTaskDelay(2000);
			while(joueur.pokemonSelect->EXP>joueur.pokemonSelect->niveau){
				phaseCombat = 5;
				xQueueSend(QueueCtoAHandle, &phaseCombat, 0);
				joueur.pokemonSelect->niveau += 1;
				joueur.pokemonSelect->EXP = joueur.pokemonSelect->EXP - joueur.pokemonSelect->niveau;
				vTaskDelay(2000);
			}

			p = pokeInit(joueur.pokemonSelect->idx, joueur.pokemonSelect->niveau);
			//reset des statistiques de combat
			joueur.pokemonSelect->degats = p.degats;
			joueur.pokemonSelect->defense = p.defense;
			joueur.pokemonSelect->vitesse = p.vitesse;

			if(joueur.zonePred == 5){
				phaseCombat = 9;
				ClearEcran();
				vTaskDelay(200);
				xQueueSend(QueueCtoAHandle, &phaseCombat, 0);
			}
			else{
				phaseCombat = 7;
				curseur.x = 2;
				curseur.y = 14;
				xQueueSend(QueueCtoAHandle, &phaseCombat, 0);
			}
		}

		// PHASE 4 defaite du pokemon
		if(phaseCombat == 4){
			vTaskDelay(2000);
			if(joueur.pokemon1.PV > 0){
				joueur.pokemonSelect = &joueur.pokemon1;
			}
			else if(joueur.pokemon2.PV > 0){
				joueur.pokemonSelect = &joueur.pokemon2;
			}
			else if(joueur.pokemon3.PV > 0){
				joueur.pokemonSelect = &joueur.pokemon3;
			}
			else if(joueur.pokemon4.PV > 0){
				joueur.pokemonSelect = &joueur.pokemon4;
			}
			else{
				//GAME OVER
				xQueueSend(QueueCtoAHandle, &phaseCombat, 0);
				vTaskDelay(2000);
				vTaskSuspend(Play_wavHandle);
				BSP_AUDIO_OUT_Pause();
				vTaskSuspend(task_deplac_pokHandle);
				vTaskSuspend(task_deplacementHandle);
				vTaskSuspend(task_Affich_PicHandle);

				ClearEcran();
				BSP_LCD_SelectLayer(0);
				vTaskDelay(1000);
				BSP_LCD_SetFont(&Font24);
				BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
				sprintf(text1, "GAME OVER");
				BSP_LCD_DisplayStringAt(0*tailleTuile,7*tailleTuile, (uint8_t*) text1, CENTER_MODE);
				vTaskDelay(5000);
				xTaskCreate(Demarrage, "demarrage", 1024, NULL, osPriorityHigh, &task_DemarrageHandle);
			}
			phaseCombat = 0;
			xQueueSend(QueueCtoAHandle, &phaseCombat, 0);
		}

		//PHASE 6 changement de poke
		if(phaseCombat == 6){

			ValJoystick();

			//deplacement du curseur
			if(joystick_h>3500 && joystick_v<3500 && joystick_v>1500 && curseur.x == 15){
				curseur.x = 2; //gauche
			}
			else if(joystick_h<1500 && joystick_v<3500 && joystick_v>1500 && curseur.x == 2	){
				curseur.x = 15; //droite
			}
			else if(joystick_v<1500 && joystick_h<3500 && joystick_h>1500 &&  curseur.y == 13){
				curseur.y = 14; //bas
			}
			else if(joystick_v>3500 && joystick_h<3500 && joystick_h>1500 &&  curseur.y == 14){
				curseur.y = 13; //haut
			}

			if(HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) == 0){
				if(curseur.x ==2 && curseur.y == 13){
					joueur.pokemonSelect = &joueur.pokemon1;
				}
				if(curseur.x ==15 && curseur.y == 13){
					joueur.pokemonSelect = &joueur.pokemon2;
				}
				if(curseur.x ==2 && curseur.y == 14){
					joueur.pokemonSelect = &joueur.pokemon3;
				}
				if(curseur.x ==15 && curseur.y == 14){
					joueur.pokemonSelect = &joueur.pokemon4;
				}

				//choix de l'attaque adverse (a completer)
				if(joueur.pokemonSelect->faiblesse == pokemonAdverse.type){
					strcpy(pokemonAdverse.attaqueSelect , pokemonAdverse.attaque[1]);
				}
				else{
					strcpy(pokemonAdverse.attaqueSelect , pokemonAdverse.attaque[0]);
				}
				phaseCombat = 2;
				phase1_fini = 1;
				xQueueSend(QueueCtoAHandle, &phaseCombat, 0);

			}
			xQueueSend(QueueCtoAHandle, &phaseCombat, 0);
		}

		//PHASE 7 capturer le poke?
		if(phaseCombat == 7){

			ValJoystick();

			//deplacement du curseur
			if(joystick_h>3500 && joystick_v<3500 && joystick_v>1500 && curseur.x == 15){
				curseur.x = 2; //gauche
			}
			else if(joystick_h<1500 && joystick_v<3500 && joystick_v>1500 && curseur.x == 2	){
				curseur.x = 15; //droite
			}

			if(HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) == 0){
				vTaskDelay(500);
				if(curseur.x == 2){
					phaseCombat = 8;
					xQueueSend(QueueCtoAHandle, &phaseCombat, 0);
					curseur.x = 2;
					curseur.y = 13;
				}
				else{
					joueur.zone = joueur.zonePred;
					vTaskSuspend(task_Affich_PicHandle);
					ClearEcran();
					BSP_LCD_SelectLayer(0);
					vTaskDelay(500);

					vTaskSuspend(Play_wavHandle);
					BSP_AUDIO_OUT_Pause();
					if(joueur.zone == 1){
						chgEcran(0);
					}
					if(joueur.zone == 2){
						chgEcran(6);
					}
					if(joueur.zone == 3){
						chgEcran(7);
					}
					if(joueur.zone == 4){
						chgEcran(8);
					}
					Charge_Wave(1);
					BSP_AUDIO_OUT_Resume();
					vTaskResume(Play_wavHandle);
					vTaskResume(task_Affich_PicHandle);
					vTaskResume(task_deplac_pokHandle);
					vTaskResume(task_deplacementHandle);
					vTaskDelete(task_combatHandle);
				}
			}
		}

		// PHASE 8 choisir le poke a remplacer
		if(phaseCombat == 8){
			ValJoystick();

			//deplacement du curseur
			if(joystick_h>3500 && joystick_v<3500 && joystick_v>1500 && curseur.x == 15){
				curseur.x = 2; //gauche
			}
			else if(joystick_h<1500 && joystick_v<3500 && joystick_v>1500 && curseur.x == 2	){
				curseur.x = 15; //droite
			}
			else if(joystick_v<1500 && joystick_h<3500 && joystick_h>1500 &&  curseur.y == 13){
				curseur.y = 14; //bas
			}
			else if(joystick_v>3500 && joystick_h<3500 && joystick_h>1500 &&  curseur.y == 14){
				curseur.y = 13; //haut
			}

			if(HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) == 0){
				if(curseur.x ==2 && curseur.y == 13){
					joueur.pokemon1 = pokemonAdverse;
					joueur.pokemon1.PV = joueur.pokemon1.PVMAX;
				}
				if(curseur.x ==15 && curseur.y == 13){
					joueur.pokemon2 = pokemonAdverse;
					joueur.pokemon2.PV = joueur.pokemon2.PVMAX;
				}
				if(curseur.x ==2 && curseur.y == 14){
					joueur.pokemon3 = pokemonAdverse;
					joueur.pokemon3.PV = joueur.pokemon3.PVMAX;
				}
				if(curseur.x ==15 && curseur.y == 14){
					joueur.pokemon4 = pokemonAdverse;
					joueur.pokemon4.PV = joueur.pokemon4.PVMAX;
				}
				joueur.zone = joueur.zonePred;
				vTaskSuspend(task_Affich_PicHandle);
				ClearEcran();
				BSP_LCD_SelectLayer(0);
				vTaskDelay(500);

				vTaskSuspend(Play_wavHandle);
				BSP_AUDIO_OUT_Pause();
				if(joueur.zone == 1){
					chgEcran(0);
				}
				if(joueur.zone == 2){
					chgEcran(6);
				}
				if(joueur.zone == 3){
					chgEcran(7);
				}
				if(joueur.zone == 4){
					chgEcran(8);
				}
				Charge_Wave(1);
				BSP_AUDIO_OUT_Resume();
				vTaskResume(Play_wavHandle);
				vTaskResume(task_Affich_PicHandle);
				vTaskResume(task_deplacementHandle);
				vTaskResume(task_deplac_pokHandle);
				vTaskDelete(task_combatHandle);
			}

		}

		osDelay(20);
		}
  /* USER CODE END combat */
}

/* USER CODE BEGIN Header_Demarrage */
/**
 * @brief Function implementing the task_Demarrage thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Demarrage */
void Demarrage(void const * argument)
{
  /* USER CODE BEGIN Demarrage */
	/* USER CODE BEGIN Demarrage */
	ClearEcran();
	vTaskSuspend(task_deplac_pokHandle);
	vTaskSuspend(task_deplacementHandle);
	vTaskSuspend(task_Affich_PicHandle);
	vTaskDelete(task_combatHandle);
	vTaskSuspend(task_chgZoneHandle);
	vTaskDelay(100);

	uint8_t str[30];

	//Reset des objets du joueur
	int i;
	for (i=0; i<9; i++){
		objetTrouves[i] = 0;
		strcpy(joueur.objet[i].nom , "\0");
	}

	uwInternelBuffer = (uint8_t*) 0xC0260000;
	uwInternelBuffer2 = (uint8_t*) 0xC0360000;


	uint8_t counter;

	/*##- Initialize the Directory Files pointers (heap) ###################*/
	for (counter = 0; counter < MAX_BMP_FILES; counter++) {
		pDirectoryFiles[counter] = malloc(MAX_BMP_FILE_NAME);
		if (pDirectoryFiles[counter] == NULL) {
			/* Set the Text Color */
			BSP_LCD_SetTextColor(LCD_COLOR_RED);

			BSP_LCD_DisplayStringAtLine(8,
					(uint8_t*) "  Cannot allocate memory ");

			while (1) {
			}
		}

	}

	/* Get the BMP file names on root directory */
	ubNumberOfFiles = Storage_GetDirectoryBitmapFiles("/Media",
			pDirectoryFiles);

    //Chargement de la premiere Zone dans le buffer
	f_open(&F1, (TCHAR const*) str, FA_READ);
	f_read(&F1, sector1, 512, (UINT*) &Debut);
	sprintf((char*) str, "Media/%-11.11s", pDirectoryFiles[0]);
	Storage_OpenReadFile(uwInternelBuffer, (const char*) str);
	f_close(&F1);

	//init des variables
	Pokemon p;
	Curseur c;
	c.x = 2;
	c.y = 12;
	c.rad = 3;
	char text1[50] = { };

	//init des coordonnées de joueur sur la Map 1
	joueur.x = 2;
	joueur.y = 11;
	joueur.sprite = 1;
	joueur.zone = 1;
	initZ1 = 0;
	joueur.zonePred = 1;


	//initialisation des murs et objets de la carte 1
	CarteInitZ1();

	//Lancement de la musique
	SD_Init();
	Charge_Wave(0);
	vTaskResume(Play_wavHandle);

	//lancement de l'introduction
	TextIntro();

	//affichage du fond
	ClearEcran();
	BSP_LCD_SelectLayer(0);
	BSP_LCD_DrawBitmap(0,0,(uint8_t*)choix_bmp);

	AfficheRect();

	vTaskDelay(1000);

	while(1)
	{

		ValJoystick();

		//deplacement du curseur
		if(joystick_v<1500 && joystick_h<3500 && joystick_h>1500 && c.y <= 13){
			c.y += 1; //bas

		}
		else if(joystick_v>3500 && joystick_h<3500 && joystick_h>1500 && c.y >= 13){
			c.y -= 1; //haut
		}

		AfficheRect();
		//choix pokemon
		sprintf(text1, "Bulbizarre le pokemon de type plante?");
		BSP_LCD_DisplayStringAt(2*tailleTuile,12*tailleTuile, (uint8_t*) text1, LEFT_MODE);
		sprintf(text1, "Salameche le pokemon de type feu?");
		BSP_LCD_DisplayStringAt(2*tailleTuile,13*tailleTuile, (uint8_t*) text1, LEFT_MODE);
		sprintf(text1, "Carapuce le pokemon de type eau?");
		BSP_LCD_DisplayStringAt(2*tailleTuile,14*tailleTuile, (uint8_t*) text1, LEFT_MODE);
		BSP_LCD_FillCircle(c.x*tailleTuile-8,c.y*tailleTuile+8, c.rad);


		if(HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin) == 0){

			switch(c.y){
			case 12:
				p = pokeInit(2,5);
				break;
			case 13:
				p = pokeInit(0,5);
				break;
			case 14:
				p = pokeInit(1,5);
			}
			joueur.pokemon1 = p;

			//Soin de l'equipe en cas de game over
			if(strcmp(joueur.pokemon2.nom,"\0") != 0){
				joueur.pokemon2.PV = joueur.pokemon2.PVMAX;
			}
			if(strcmp(joueur.pokemon3.nom,"\0") != 0){
				joueur.pokemon3.PV = joueur.pokemon3.PVMAX;
			}
			if(strcmp(joueur.pokemon4.nom,"\0") != 0){
				joueur.pokemon4.PV = joueur.pokemon4.PVMAX;
			}

			vTaskDelay(1000);

			vTaskSuspend(Play_wavHandle);
			Charge_Wave(1);
			vTaskResume(Play_wavHandle);

			vTaskResume(task_deplac_pokHandle);
			vTaskResume(task_deplacementHandle);
			vTaskResume(task_chgZoneHandle);
			vTaskResume(task_Affich_PicHandle);

			vTaskDelete(task_DemarrageHandle);
		}

	osDelay(100);}
  /* USER CODE END Demarrage */
}

/* USER CODE BEGIN Header_chgZone */
/**
 * @brief Function implementing the task_chgZone thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_chgZone */
void chgZone(void const * argument)
{
  /* USER CODE BEGIN chgZone */


	while(1)
	{
		if(joueur.zonePred == 1 && joueur.zone == 2){
			joueur.zonePred = joueur.zone;
			joueur.x = 28;
			joueur.y = 11;
			CarteInitZ2();
			initZ2 = 0;
			if(objetTrouves[1]==1){
				carte[3][5] = 1;
			}
			if(objetTrouves[2]==1){
				carte[11][13] = 1;
			}

			vTaskSuspend(Play_wavHandle);
			BSP_AUDIO_OUT_Pause();
			chgEcran(6);
			f_open(&SDFile, musique[1], FA_READ);
			f_lseek(&SDFile, 44+Bloc_Cursor*512);
			BSP_AUDIO_OUT_Resume();
			vTaskResume(Play_wavHandle);

			vTaskDelay(250);
			vTaskResume(task_deplac_pokHandle);
			vTaskResume(task_deplacementHandle);
			vTaskResume(task_Affich_PicHandle);
			vTaskSuspend(task_chgZoneHandle);
		}
		if(joueur.zonePred == 2 && joueur.zone == 1){
			joueur.zonePred = joueur.zone;
			joueur.x = 1;
			joueur.y = 11;
			CarteInitZ1();
			initZ1 = 0;
			if(objetTrouves[0]==1){
				carte[25][2] = 1;
			}

			vTaskSuspend(Play_wavHandle);
			BSP_AUDIO_OUT_Pause();
			chgEcran(0);
			f_open(&SDFile, musique[1], FA_READ);
			f_lseek(&SDFile, 44+Bloc_Cursor*512);
			BSP_AUDIO_OUT_Resume();
			vTaskResume(Play_wavHandle);

			vTaskDelay(250);
			vTaskResume(task_deplac_pokHandle);
			vTaskResume(task_deplacementHandle);
			vTaskResume(task_Affich_PicHandle);
			vTaskSuspend(task_chgZoneHandle);
		}

		if(joueur.zonePred == 2 && joueur.zone == 3){
			joueur.zonePred = joueur.zone;
			joueur.x = 26;
			joueur.y = 14;
			CarteInitZ3();
			initZ3 = 0;
			if(objetTrouves[3]==1){
				carte[8][7] = 1;
			}
			if(objetTrouves[4]==1){
				carte[17][10] = 1;
			}
			vTaskSuspend(Play_wavHandle);
			BSP_AUDIO_OUT_Pause();
			chgEcran(7);
			f_open(&SDFile, musique[1], FA_READ);
			f_lseek(&SDFile, 44+Bloc_Cursor*512);
			BSP_AUDIO_OUT_Resume();
			vTaskResume(Play_wavHandle);

			f_close(&F1);
			vTaskDelay(250);
			vTaskResume(task_deplac_pokHandle);
			vTaskResume(task_deplacementHandle);
			vTaskResume(task_Affich_PicHandle);
			vTaskSuspend(task_chgZoneHandle);
		}
		if(joueur.zonePred == 3 && joueur.zone == 2){
			joueur.zonePred = joueur.zone;
			joueur.x = 26;
			joueur.y = 1;
			CarteInitZ2();
			initZ2 = 0;
			if(objetTrouves[1]==1){
				carte[3][5] = 1;
			}
			if(objetTrouves[2]==1){
				carte[11][13] = 1;
			}
			vTaskSuspend(Play_wavHandle);
			BSP_AUDIO_OUT_Pause();
			chgEcran(6);
			f_open(&SDFile, musique[1], FA_READ);
			f_lseek(&SDFile, 44+Bloc_Cursor*512);
			BSP_AUDIO_OUT_Resume();
			vTaskResume(Play_wavHandle);

			f_close(&F1);
			vTaskDelay(250);
			vTaskResume(task_deplac_pokHandle);
			vTaskResume(task_deplacementHandle);
			vTaskResume(task_Affich_PicHandle);
			vTaskSuspend(task_chgZoneHandle);
		}
		if(joueur.zonePred == 3 && joueur.zone == 4){
			joueur.zonePred = joueur.zone;
			joueur.x = 6;
			joueur.y = 14;
			CarteInitZ4();
			initZ4 = 0;
			if(objetTrouves[5]==1){
				carte[26][12] = 1;
			}
			if(objetTrouves[6]==1){
				carte[13][2] = 1;
			}
			vTaskSuspend(Play_wavHandle);
			BSP_AUDIO_OUT_Pause();
			chgEcran(8);
			f_open(&SDFile, musique[1], FA_READ);
			f_lseek(&SDFile, 44+Bloc_Cursor*512);
			BSP_AUDIO_OUT_Resume();
			vTaskResume(Play_wavHandle);

			f_close(&F1);
			vTaskDelay(250);
			vTaskResume(task_deplac_pokHandle);
			vTaskResume(task_deplacementHandle);
			vTaskResume(task_Affich_PicHandle);
			vTaskSuspend(task_chgZoneHandle);
		}
		if(joueur.zonePred == 4 && joueur.zone == 3){
			joueur.zonePred = joueur.zone;
			joueur.x = 6;
			joueur.y = 1;
			CarteInitZ3();
			initZ3 = 0;
			if(objetTrouves[3]==1){
				carte[8][7] = 1;
			}
			if(objetTrouves[4]==1){
				carte[17][10] = 1;
			}
			vTaskSuspend(Play_wavHandle);
			BSP_AUDIO_OUT_Pause();
			chgEcran(7);
			f_open(&SDFile, musique[1], FA_READ);
			f_lseek(&SDFile, 44+Bloc_Cursor*512);
			BSP_AUDIO_OUT_Resume();
			vTaskResume(Play_wavHandle);

			f_close(&F1);
			vTaskDelay(250);
			vTaskResume(task_deplac_pokHandle);
			vTaskResume(task_deplacementHandle);
			vTaskResume(task_Affich_PicHandle);
			vTaskSuspend(task_chgZoneHandle);
		}
		if(joueur.zonePred == 5 && joueur.zone == 4){
			joueur.zonePred = joueur.zone;
			joueur.x = 28;
			joueur.y = 2;
			CarteInitZ4();
			initZ4 = 0;
			if(objetTrouves[5]==1){
				carte[26][12] = 1;
			}
			if(objetTrouves[6]==1){
				carte[13][2] = 1;
			}
			vTaskSuspend(Play_wavHandle);
			BSP_AUDIO_OUT_Pause();
			chgEcran(8);
			f_open(&SDFile, musique[1], FA_READ);
			f_lseek(&SDFile, 44+Bloc_Cursor*512);
			BSP_AUDIO_OUT_Resume();
			vTaskResume(Play_wavHandle);

			f_close(&F1);
			vTaskDelay(250);
			vTaskResume(task_deplac_pokHandle);
			vTaskResume(task_deplacementHandle);
			vTaskResume(task_Affich_PicHandle);
			vTaskSuspend(task_chgZoneHandle);
		}
		if(joueur.zonePred == 4 && joueur.zone == 5){
			joueur.zonePred = joueur.zone;
			joueur.x = 1;
			joueur.y = 2;
			CarteInitZ5();
			initZ5 = 0;
			if(objetTrouves[7]==1){
				carte[6][13] = 1;
			}
			if(objetTrouves[8]==1){
				carte[26][13] = 1;
			}
			vTaskSuspend(Play_wavHandle);
			BSP_AUDIO_OUT_Pause();
			chgEcran(9);
			f_open(&SDFile, musique[1], FA_READ);
			f_lseek(&SDFile, 44+Bloc_Cursor*512);
			BSP_AUDIO_OUT_Resume();
			vTaskResume(Play_wavHandle);

			f_close(&F1);
			vTaskDelay(250);
			vTaskResume(task_deplac_pokHandle);
			vTaskResume(task_deplacementHandle);
			vTaskResume(task_Affich_PicHandle);
			vTaskSuspend(task_chgZoneHandle);
		}

	osDelay(50);}
  /* USER CODE END chgZone */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
