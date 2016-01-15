#include <iostream>
#include <random>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
using namespace std;

//Costanti di sistema
#define X_MAX 80
#define Y_MAX 23
#define ROOMS 8
#define ROOM_SIZE 7
#define MAX_ENEMIES 50
#define HP_MAX 100
#define STARTING_ATK 5
#define STARTING_DEF 5
#define MAX_POTIONS_PER_FLOOR 5
#define ENEMY_HP 10

//Costanti globali per il rendering della mappa
unsigned char WALL = 0xB2;
const unsigned char EMPTY = ' ';
const unsigned char PLAYER = 0x02;
const unsigned char ENEMY = 'X';
const unsigned char EXIT = '>';
const unsigned char DOUBLELINE = 0xCD;
const unsigned char ITEM_SMALL_POTION = 'p';
const unsigned char ITEM_MEDIUM_POTION = 'n';
const unsigned char ITEM_BIG_POTION = 'm';

//Prototipiamo tutte le classi
class Entity;
class Player;
class Enemy;

//Variabili globali
//Lista di tutti i nemici nel livello
Enemy* list[MAX_ENEMIES];
//Numero del piano raggiunto dal giocatore
int depth = 1;
//Mappa del gioco
unsigned char map[X_MAX][Y_MAX];

//Prototipiamo tutte le funzioni, per semplificarci un po' la vita
void draw();
void inventory();
void init();
void room(int start_x, int start_y, int end_x, int end_y);
void corridor(int start_x, int start_y, int end_x, int end_y, bool verticale);
void generate();
void tick();
void attack(int x, int y);
Enemy* find(int x, int y);

//Classe entità generica, sia nemico sia giocatore
class Entity
{
    public:
        bool alive = true;
        int x;
        int y;
        int hp = HP_MAX;
        int hp_max = HP_MAX;
        int move();
        //Cura di x l'entità
        void heal(int x)
        {
            if(hp + x > hp_max)
            {
                hp = hp_max;
            }
            else
            {
                hp += x;
            }
        }
        //Danneggia di x l'entità
        void damage(int x)
        {
            if(hp - x <= 0)
            {
                kill();
            }
            else
            {
                hp -= x;
            }
        }
        //Uccide ed elimina l'entità
        void kill()
        {
            hp = 0;
            alive = false;
            map[x][y] = EMPTY;
        }
        //Visualizza la vita
        int gethp()
        {
            return hp;
        }
};

//Classe del giocatore
class Player : public Entity
{
    private:
        int base_atk = STARTING_ATK; //Attacco di base
        int base_def = STARTING_DEF; //Difesa di base
        int equipment_atk = 0; //Attacco ottenuto dall'equipaggiamento
        int equipment_def = 0; //Difesa ottenuta dall'equipaggiamento
    public:
        int atk()
        {
            int result = equipment_atk + base_atk;
            return result;
        }
        int def()
        {
            int result = equipment_def + base_def;
            return result;
        }
        int pozioni_vita_piccole = 3;
        int pozioni_vita_medie = 2;
        int pozioni_vita_grandi = 1;
        int move()
        {
            if(alive)
            {
                bool waiting = true;
                unsigned char* starting = &map[x][y]; //Casella attuale
                unsigned char* target; //Bersaglio del movimento
                //Rileva i tasti freccia
                while(waiting)
                {
                    unsigned char input = getch();
                    short int dir = 0; //Direzione del movimento, per modificare x e y.
                    if(input == 224)
                    {
                        switch(getch())
                        {
                            case 72: //Freccia su, 1
                                target = &map[x][y-1];
                                dir = 1;
                                break;
                            case 80: //Freccia giù, 2
                                target = &map[x][y+1];
                                dir = 2;
                                break;
                            case 75: //Freccia sinistra, 3
                                target = &map[x-1][y];
                                dir = 3;
                                break;
                            case 77: //Freccia destra, 4
                                target = &map[x+1][y];
                                dir = 4;
                                break;
                            default: //Pag su, pag giù, skippa
                                target = &map[x][y]; //Un po' hackerino, ma...
                                break;
                            //Aggiungere gestione del caso che non sia una delle quattro frecce
                        }
                        //Muoviti e agisci!
                        if(*target == EMPTY)
                        {
                            *starting = EMPTY;
                            *target = PLAYER;
                            waiting = false;
                        }
                        else if(*target == ITEM_SMALL_POTION)
                        {
                            *starting = EMPTY;
                            *target = PLAYER;
                            pozioni_vita_piccole++;
                            waiting = false;
                        }
                        else if(*target == ITEM_MEDIUM_POTION)
                        {
                            *starting = EMPTY;
                            *target = PLAYER;
                            pozioni_vita_medie++;
                            waiting = false;
                        }
                        else if(*target == ITEM_BIG_POTION)
                        {
                            *starting = EMPTY;
                            *target = PLAYER;
                            pozioni_vita_grandi++;
                            waiting = false;
                        }
                        else if(*target == EXIT)
                        {
                            return 1;
                        }
                    }
                    else if(input == 's') //S
                    {
                        //Salta un turno
                        waiting = false;
                    }
                    else if(input == 'i') //I
                    {
                        //Apri l'inventario
                        inventory();
                        //Torna alla mappa
                        draw();
                    }
                    else if(input == 'a') //A
                    {
                        printf("ATTACCO selezionato");
                        if(getch() == 224)
                        {
                            char atk=getch();
                            switch(atk)
                            {
                                case 72: //ATK SU
                                    if(map[x][y-1]==ENEMY)
                                    {
                                        attack(x, y-1);
                                    }
                                    break;
                                case 80: //ATK GIU
                                    if(map[x][y+1]==ENEMY)
                                    {
                                        attack(x, y+1);
                                    }
                                    break;
                                case 75: //ATK SX
                                    if(map[x-1][y]==ENEMY)
                                    {
                                        attack(x-1, y);
                                    }
                                    break;
                                case 77: //ATK DX
                                    if(map[x+1][y]==ENEMY)
                                    {
                                        attack(x+1, y);
                                    }
                                    break;
                            }
                            waiting = false;
                        }
                    }
                    //Se ti sei mosso, controlla in che direzione e aggiorna correttamente la x e la y.
                    //Non mi veniva in mente un modo per farlo meglio.
                    if(!waiting)
                    {
                        if(dir == 1)
                        {
                            y--;
                        }
                        else if(dir == 2)
                        {
                            y++;
                        }
                        else if(dir == 3)
                        {
                            x--;
                        }
                        else if(dir == 4)
                        {
                            x++;
                        }
                    }
                }
            }
            return 0;
        }
} player;

//Classe dei nemici
class Enemy : public Entity
{
    public:
        int move()
        {
            if(alive)
            {
                //Se intorno c'è il giocatore
                if(map[x-1][y] == PLAYER || map[x+1][y] == PLAYER || map[x][y-1] == PLAYER || map[x][y+1] == PLAYER)
                {
                    //Forse sarebbe meglio fare una funzione per togliere vita che controlla anche se va a 0...
                    player.damage(rand() % 5 + 1);
                }
                else
                {
                    //Se il giocatore è vicino, muoviti verso di lui
                    if(map[x-2][y] == PLAYER && map[x-1][y] == EMPTY) //Due a sinistra
                    {
                        map[x][y] = EMPTY;
                        map[x-1][y] = ENEMY;
                        x--;
                    }
                    else if(map[x+2][y] == PLAYER && map[x+1][y] == EMPTY) //Due a destra
                    {
                        map[x][y] = EMPTY;
                        map[x+1][y] = ENEMY;
                        x++;
                    }
                    else if(map[x][y-2] == PLAYER && map[x][y-1] == EMPTY) //Due in su
                    {
                        map[x][y] = EMPTY;
                        map[x][y-1] = ENEMY;
                        y--;
                    }
                    else if(map[x][y+2] == PLAYER && map[x][y+1] == EMPTY) //Due in giù
                    {
                        map[x][y] = EMPTY;
                        map[x][y+1] = ENEMY;
                        y++;
                    }
                    else if(map[x-1][y-1] == PLAYER) //In alto a sinistra
                    {
                        if(map[x][y-1] == EMPTY) //Vai in alto
                        {
                            map[x][y] = EMPTY;
                            map[x][y-1] = ENEMY;
                            y--;
                        }
                        else if(map[x-1][y] == EMPTY) //Vai a sinistra
                        {
                            map[x][y] = EMPTY;
                            map[x-1][y] = ENEMY;
                            x--;
                        }
                    }
                    else if(map[x-1][y+1] == PLAYER) //In basso a sinistra
                    {
                        if(map[x][y+1] == EMPTY) //Vai in basso
                        {
                            map[x][y] = EMPTY;
                            map[x][y+1] = ENEMY;
                            y++;
                        }
                        else if(map[x-1][y] == EMPTY) //Vai a sinistra
                        {
                            map[x][y] = EMPTY;
                            map[x-1][y] = ENEMY;
                            x--;
                        }
                    }
                    else if(map[x+1][y-1] == PLAYER) //In alto a destra
                    {
                        if(map[x][y-1] == EMPTY) //Vai in alto
                        {
                            map[x][y] = EMPTY;
                            map[x][y-1] = ENEMY;
                            y--;
                        }
                        else if(map[x+1][y] == EMPTY) //Vai a destra
                        {
                            map[x][y] = EMPTY;
                            map[x+1][y] = ENEMY;
                            x++;
                        }
                    }
                    else if(map[x+1][y+1] == PLAYER) //In basso a destra
                    {
                        if(map[x][y+1] == EMPTY) //Vai in basso
                        {
                            map[x][y] = EMPTY;
                            map[x][y+1] = ENEMY;
                            y++;
                        }
                        else if(map[x+1][y] == EMPTY) //Vai a destra
                        {
                            map[x][y] = EMPTY;
                            map[x+1][y] = ENEMY;
                            x++;
                        }
                    }
                    //Il giocatore non è vicino
                    else
                    {
                        if(map[x-1][y] == EMPTY || map[x+1][y] == EMPTY || map[x][y-1] == EMPTY || map[x][y+1] == EMPTY)
                        {
                            //Muoviti in una direzione casuale
                            bool moving = true;
                            while(moving)
                            {
                                int direction = rand() % 4;
                                switch(direction)
                                {
                                    case 0: //Sinistra
                                        if(map[x-1][y] == EMPTY)
                                        {
                                            map[x][y] = EMPTY;
                                            map[x-1][y] = ENEMY;
                                            x--;
                                            moving = false;
                                        }
                                        break;
                                    case 1: //Destra
                                        if(map[x+1][y] == EMPTY)
                                        {
                                            map[x][y] = EMPTY;
                                            map[x+1][y] = ENEMY;
                                            x++;
                                            moving = false;
                                        }
                                        break;
                                    case 2: //Su
                                        if(map[x][y-1] == EMPTY)
                                        {
                                            map[x][y] = EMPTY;
                                            map[x][y-1] = ENEMY;
                                            y--;
                                            moving = false;
                                        }
                                        break;
                                    case 3: //Giù
                                        if(map[x][y+1] == EMPTY)
                                        {
                                            map[x][y] = EMPTY;
                                            map[x][y+1] = ENEMY;
                                            y++;
                                            moving = false;
                                        }
                                        break;
                                }
                            }
                        }
                    }
                }
            }
        }
};

//Aggiorna la console con la situazione corrente del gioco.
void draw()
{
    //Svuota lo schermo della console. Sono sicuro che ci sia un modo molto migliore per farlo, ma non mi viene in mente...
    system("cls");
    for(int y=0; y<Y_MAX; y++)
    {
        for(int x=0; x<X_MAX; x++)
        {
            printf("%c", map[x][y]);
        }
    }
    printf("Piano: %d | Vita: %d/%d | x:%d y:%d\n", depth, player.gethp(), HP_MAX, player.x, player.y);
}

//Visualizza l'inventario
void inventory()
{
    system("cls");
    printf("Piano: %d | Vita: %d/%d | x:%d y:%d\n", depth, player.gethp(), HP_MAX, player.x, player.y);
    for(int i = 0; i < X_MAX; i++)
    {
        //TODO: Cambiare qui. Rallenta.
        printf("%c", (char) DOUBLELINE);
    }
    //Anche qui, credo si possa migliorare qualcosa...
    if(player.pozioni_vita_piccole > 0)
    {
        printf("%dx Pozione di Vita (p)iccola\tRipristina 10 Vita\n", player.pozioni_vita_piccole);
    }
    else
    {
        printf("\n");
    }
    if(player.pozioni_vita_medie > 0)
    {
        printf("%dx Pozione di Vita (n)ormale\tRipristina 20 Vita\n", player.pozioni_vita_medie);
    }
    else
    {
        printf("\n");
    }
    if(player.pozioni_vita_grandi > 0)
    {
        printf("%dx Pozione di Vita (m)aggiore\tRipristina 50 Vita\n", player.pozioni_vita_grandi);
    }
    else
    {
        printf("\n");
    }
    for(int i = 0; i < X_MAX; i++)
    {
        printf("%c", (char) DOUBLELINE);
    }
    //Selezione dell'oggetto da usare.
    printf("Scrivi la lettera corrispondente all'oggetto che vuoi usare.\nEsci con Esc.\n");
    while(true)
    {
        //Effetto degli oggetti
        unsigned char selezione = getch();
        if(selezione == 112) //p
        {
            if(player.pozioni_vita_piccole > 0)
            {
                player.pozioni_vita_piccole--;
                player.heal(10);
                break;
            }
        }
        else if(selezione == 110) //n
        {
            if(player.pozioni_vita_medie > 0)
            {
                player.pozioni_vita_medie--;
                player.heal(20);
                break;
            }
        }
        else if(selezione == 109) //m
        {
            if(player.pozioni_vita_grandi > 0)
            {
                player.pozioni_vita_grandi--;
                player.heal(50);
                break;
            }
        }
        else if(selezione == 27) //esc
        {
            break;
        }
    }
}

//Funzioni per la generazione della mappa
//Inizializza la mappa con spazi vuoti
void init()
{
    for(int y=0; y<Y_MAX; y++)
    {
        for(int x=0; x<X_MAX; x++)
        {
            map[x][y] = WALL;
        }
    }
}

//Crea una stanza quadrata
void room(int start_x, int start_y, int end_x, int end_y)
{
    for(int y=start_y; y<=end_y; y++)
    {
        for(int x=start_x; x<=end_x; x++)
        {
            map[x][y] = EMPTY;
        }
    }
}

//Crea un corridoio che connetta due punti
void corridor(int start_x, int start_y, int end_x, int end_y, bool verticale)
{
    if(verticale)
    {
        if(start_y > end_y)
        {
            for(int y=end_y; y<=start_y; y++)
            {
                map[start_x][y] = EMPTY;
            }
        }
        else
        {
            for(int y=start_y; y<=end_y; y++)
            {
                map[start_x][y] = EMPTY;
            }
        }
        if(start_x > end_x)
        {
            for(int x=end_x; x<=start_x; x++)
            {
                map[x][end_y] = EMPTY;
            }
        }
        else
        {
            for(int x=start_x; x<=end_x; x++)
            {
                map[x][end_y] = EMPTY;
            }
        }
    }
    else
    {
        if(start_x > end_x)
        {
            for(int x=end_x; x<=start_x; x++)
            {
                map[x][start_y] = EMPTY;
            }
        }
        else
        {
            for(int x=start_x; x<=end_x; x++)
            {
                map[x][start_y] = EMPTY;
            }
        }
        if(start_y > end_y)
        {
            for(int y=end_y; y<=start_y; y++)
            {
                map[end_x][y] = EMPTY;
            }
        }
        else
        {
            for(int y=start_y; y<=end_y; y++)
            {
                map[end_x][y] = EMPTY;
            }
        }
    }
}

//Genera il livello
void generate(int enemies_to_place)
{
    int corridor_x;
    int corridor_y;
    //Creazione delle stanze
    for(int r=0; r<ROOMS; r++)
    {
        int size_x = rand() % ROOM_SIZE + 1;
        int size_y = rand() % ROOM_SIZE + 1;
        int start_x = rand() % (X_MAX - size_x - 2) + 1;
        int start_y = rand() % (Y_MAX - size_y - 2) + 1;
        room(start_x, start_y, start_x + size_x, start_y + size_y);
        //Se non è la prima stanza, crea un corridoio che connetta quella appena generata con quella precedente
        if(r > 0)
        {
            int link_x = rand() % size_x + 1 + start_x;
            int link_y = rand() % size_y + 1 + start_y;
            corridor(link_x, link_y, corridor_x, corridor_y, rand() % 2);
        }
        corridor_x = rand() % size_x + start_x;
        corridor_y = rand() % size_y + start_y;
        //Posiziona il giocatore se è l'ultima stanza
        if(r == ROOMS - 1)
        {
            map[corridor_x][corridor_y] = PLAYER;
            player.x = corridor_x;
            player.y = corridor_y;
        }
    }
    //Posizionamento nemici
    for(int e=0; e<enemies_to_place; e++)
    {
        while(true)
        {
            int x = rand() % (X_MAX - 1) + 1;
            int y = rand() % (Y_MAX - 1) + 1;
            if(map[x][y] == EMPTY)
            {
                map[x][y] = ENEMY;
                Enemy* created = new Enemy();
                created->x = x;
                created->y = y;
                created->hp = ENEMY_HP;
                created->hp_max = ENEMY_HP;
                list[e] = created;
                break;
            }
        }
    }
    //Posizionamento uscita
    while(true)
    {
        int x = rand() % (X_MAX - 1) + 1;
        int y = rand() % (Y_MAX - 1) + 1;
        if(map[x][y] == EMPTY)
        {
            map[x][y] = EXIT;
            break;
        }
    }
    //Posizionamento pozioni di vita
    int placed_potions = 0;
    int potions_in_floor = rand() % MAX_POTIONS_PER_FLOOR;
    while(placed_potions < potions_in_floor)
    {
        int x = rand() % (X_MAX - 1) + 1;
        int y = rand() % (Y_MAX - 1) + 1;
        int size = rand() % 100;
        if(map[x][y] == EMPTY)
        {
            if(size < 60)
            {
                map[x][y] = ITEM_SMALL_POTION;
            }
            else if(size > 90)
            {
                map[x][y] = ITEM_BIG_POTION;
            }
            else
            {
                map[x][y] = ITEM_MEDIUM_POTION;
            }
            placed_potions++;
        }
    }
}

//Processa il resto di un turno, dopo il movimento del giocatore.
void tick(int enemies_to_tick)
{
    for(int e=0; e<enemies_to_tick; e++)
    {
        list[e]->move();
    }
}

//Trova il puntatore al nemico in una certa posizione
Enemy* find(int x, int y)
{
    for(int e=0; e<MAX_ENEMIES; e++)
    {
        //Se c'è un nemico in quella posizione ED E' VIVO
        if(list[e]->x == x && list[e]->y == y && list[e]->alive)
        {
            return list[e];
        }
    }
    return NULL;
}

void attack(int x, int y)
{
    find(x,y)->damage(player.atk());
}

int main()
{
    int seed; //Seed casuale per generare il livello
    srand (time(NULL));
    int titolo=0, cursorpos=1;
    string splash[10]{"Ora con ben 2 colori!", "E' come skyrim, ma con una bella grafica!", "Quasi senza bug", "Potrebbe contenere canditi", "Ora in grado di generare fame", "I colori hanno ben 1 sfumatura", "I SEE YOU", "WOLOLO", "AUTOMAAAAAH", "Stai davvero giocando a sta roba? WOW!"};
    cout << " "<<endl;
    cout << "  000000" << endl;
    cout << "  000000 0000 0000 0000 0000 0000 000         000000 0000 000000 000000 000000"<< endl;
    cout << "    00                            000         00          000000 000000 000000"<< endl;
    cout << "    00   0000 0000 0000 0000 0000 000         00  00 0000 00  00 000    00  00"<< endl;
    cout << "    00   0000 0000 0000 0000 0000 000         00  00 0000 00  00 000    00  00"<< endl;
    cout << "  000000 0000 0000 0000 0000 0000 000000      000000 0000 000000 000000 000000"<< endl;
    cout << "  000000 0000 0000 0000 0000 0000 000000      000000 0000 000000 000000 000000"<< endl;
    titolo= rand()%10;
    cout << "\n\n  Citazione all'avvio: "<< splash[titolo] <<endl;
    cout << "\n\n\n\n                        Premi INVIO per entrare nel menu!"<<endl;
    char a=getchar();
    while(1)
    {
    system("cls");
    if(cursorpos==1)
    {
    cout << " "<<endl;
    cout << "  000000" << endl;
    cout << "  000000 0000 0000 0000 0000 0000 000         000000 0000 000000 000000 000000"<< endl;
    cout << "    00                            000         00          000000 000000 000000"<< endl;
    cout << "    00   0000 0000 0000 0000 0000 000         00  00 0000 00  00 000    00  00"<< endl;
    cout << "    00   0000 0000 0000 0000 0000 000         00  00 0000 00  00 000    00  00"<< endl;
    cout << "  000000 0000 0000 0000 0000 0000 000000      000000 0000 000000 000000 000000"<< endl;
    cout << "  000000 0000 0000 0000 0000 0000 000000      000000 0000 000000 000000 000000"<< endl;
    cout << "\n\n                               >  NUOVA PARTITA  <"<<endl;
    cout << "\n                                     OPZIONI"<<endl;
    }
    if(cursorpos==2)
    {
    cout << " "<<endl;
    cout << "  000000" << endl;
    cout << "  000000 0000 0000 0000 0000 0000 000         000000 0000 000000 000000 000000"<< endl;
    cout << "    00                            000         00          000000 000000 000000"<< endl;
    cout << "    00   0000 0000 0000 0000 0000 000         00  00 0000 00  00 000    00  00"<< endl;
    cout << "    00   0000 0000 0000 0000 0000 000         00  00 0000 00  00 000    00  00"<< endl;
    cout << "  000000 0000 0000 0000 0000 0000 000000      000000 0000 000000 000000 000000"<< endl;
    cout << "  000000 0000 0000 0000 0000 0000 000000      000000 0000 000000 000000 000000"<< endl;
    cout << "\n\n                                  NUOVA PARTITA   "<<endl;
    cout << "\n                                  >  OPZIONI  <"<<endl;
    }
    a=getch();
    if(a==72)
    {
        cursorpos=cursorpos-1;
    }
    if(a==80)
    {
        cursorpos=cursorpos+1;
    }
    if (a==13)//PRESSIONE ENTER SU ELEMENTI MENU
        {
            if (cursorpos==1)//AVVIO GIOCO
            {
                break;
            }
            if (cursorpos==2)
            {
                system("cls");
                cout << "Modificare tipo blocco? (S/N)"<<endl;
                char scelta;
                cin >> scelta;
                if(scelta == 'S' || scelta== 's')
                {
                   WALL= 0xDB;
                }
            }
        }
    }
    printf("\n\nSeleziona un seed per la partita: ");
    cin >> seed;
    srand(seed);
    //Ciclo del gioco
    while(true)
    {
        int enemies_in_level;
        if(depth < MAX_ENEMIES)
        {
            enemies_in_level = depth;
        }
        else
        {
            enemies_in_level = MAX_ENEMIES;
        }
        init();
        generate(enemies_in_level);
        draw();
        //Ciclo di un livello
        while(true)
        {
            int trigger = player.move();
            if(trigger == 1) //Uscita toccata
            {
                break;
            }
            tick(enemies_in_level);
            draw();
        }
        depth++;
    }
    return 0;
}
